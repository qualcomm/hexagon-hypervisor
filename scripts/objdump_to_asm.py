#!/usr/bin/env python3
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

"""Extract a Hexagon objdump function into standalone assembly skeleton.

The default conversion preserves objdump packet boundaries exactly.  The optional
--schedule-report adds a conservative dependency/resource analysis on stderr; it
never rewrites stdout or claims that a proposed packet is assembler-valid.

The static instruction tables below are curated from intrinsics.txt.  The script
deliberately does not load that text file at runtime: it is a human-readable ISA
reference, not a complete machine-readable scheduling specification.

"""

import argparse
from collections import Counter
import contextlib
import copy
from dataclasses import dataclass, field
import io
from pathlib import Path
import subprocess
import tempfile
import unittest
from typing import FrozenSet, Optional
import re
import shutil
import sys

FUNC_HEADER_RE = re.compile(r'^[0-9a-f]+ <([^>]+)>:\s*$')
INSN_LINE_RE = re.compile(r'^\s*([0-9a-f]+):\t(?:[0-9a-f]{2} ){3}[0-9a-f]{2}\t([0-9a-f]{8})(.*)$')
TARGET_RE = re.compile(r'\b(0x[0-9a-f]+)\s*<([^>]+)>')
ENDLOOP_RE = re.compile(r':endloop[01]\b')
REG_TOKEN_RE = re.compile(r'\br(\d+)(?::(\d+))?(\.new)?\b')
PRED_TOKEN_RE = re.compile(r'\bp([0-3])(\.new)?\b')
CALL_RE = re.compile(r'^\s*(?:if\s*\(.*\)\s*)?(call|callr)\b')

# Register policy for code this tool is allowed to describe as ordinary program
# allocation. r20-r27 are deliberately fixed by current kernel/test builds:
# they are context-switch scratch only after their thread values are saved.
CALLER_SAVED_SCRATCH = frozenset(range(0, 16))
USABLE_CALLEE_SAVED = frozenset(range(16, 20))
RESERVED_PROGRAM_REGS = frozenset(range(20, 28))
FIXED_REGS = {28: 'H2K_GP', 29: 'SP', 30: 'FP', 31: 'LR'}
ARCH_PATH_RE = re.compile(r'(?:^|[/\\\\])v(68|73|81)(?:[/\\\\]|$)')


def derive_architecture(offsets_path=None, objdump_path=None):
    """Derive v68/v73/v81 from the build path, preferring asm_offsets.h."""
    for path in (offsets_path, objdump_path):
        if path:
            match = ARCH_PATH_RE.search(str(path))
            if match:
                return 'v' + match.group(1)
    return 'v81'


# Curated from intrinsics.txt. These tables encode only the scalar forms this
# report understands; omitted forms are explicitly UNKNOWN/opaque rather than
# guessed. The class string is static provenance for the corresponding source
# material in intrinsics.txt.
ALU_OPS = frozenset({
    'add', 'addasl', 'addasr', 'addasl', 'and', 'asr', 'asl', 'aslh', 'asrh',
    'bitsclr', 'cl0', 'clrbit', 'combine', 'extract', 'extractu', 'insert',
    'lsl', 'lsr', 'max', 'maxu', 'min', 'minu', 'mpyi', 'neg', 'not', 'or',
    'setbit', 'sub', 'sxtb', 'sxth', 'vaddh', 'vavgh', 'vnavgh', 'vsubh',
    'vsubuh', 'xor', 'zxtb', 'zxth',
})
PREDICATE_OP_PREFIXES = ('cmp.', 'tstbit', 'bitsclr', 'all8', 'any8')
MEMORY_OP_RE = re.compile(r'\bmem(?:[bhwd]|u[bhw])[a-z]*\s*\(')
MEMORY_LHS_RE = re.compile(r'^\s*mem(?:[bhwd]|u[bhw])[a-z]*\s*\(')
CONTROL_OP_RE = re.compile(r'\b(?:jump|jumpr|call|callr|dealloc_return|loop[01]|endloop[01])\b')
VECTOR_MARKERS = ('v', 'q', 'hvx', 'vmem', 'vgather', 'vscatter')

# Exact solo list supplied for this task. allocframe/loop setup are deliberately
# excluded: real compiler output in the supplied objdump packs them with ALU ops.
SOLO_MNEMONICS = frozenset({
    'barrier', 'brkpt', 'icinva', 'isync', 'pause', 'syncht',
    'trap0', 'trap1', 'trap2', 'unpause',
    'dckill', 'ictagr', 'icdatar', 'icinvidx', 'ictagw', 'icdataw', 'ickill',
    'tlblock', 'k0lock', 'tlbunlock', 'k0unlock',
    'l2kill', 'l2gunlock', 'l2gclean', 'l2gcleaninv',
    'nmi', 'resume', 'rte', 'start', 'stop', 'wait',
    'tlbw', 'ctlbw', 'tlbr', 'tlbp', 'tlbpp', 'tlbinvasid', 'tlboc',
})

CATEGORY_ALU = 'ALU'
CATEGORY_PREDICATE = 'PREDICATE'
CATEGORY_LOAD = 'LOAD'
CATEGORY_STORE = 'STORE'
CATEGORY_MEM_RMW = 'MEM_RMW'
CATEGORY_CONTROL = 'CONTROL'
CATEGORY_SYSTEM = 'SYSTEM'
CATEGORY_VECTOR = 'VECTOR'
CATEGORY_UNKNOWN = 'UNKNOWN'


@dataclass
class Insn:
    addr: int
    text: str
    line_group: int
    sub_index: int
    packet_no: int = -1


@dataclass(frozen=True)
class InsnFacts:
    insn: Insn
    category: str
    provenance: str
    reads_old: FrozenSet[str]
    reads_new: FrozenSet[str]
    writes: FrozenSet[str]
    predicated: bool
    opaque_reason: Optional[str]

    @property
    def node(self):
        return 'P%03d.I%d' % (self.insn.packet_no, self.insn.sub_index)

    @property
    def scheduling_eligible(self):
        return self.category in (CATEGORY_ALU, CATEGORY_PREDICATE) and not self.predicated and not self.reads_new and self.opaque_reason is None


@dataclass(frozen=True)
class DependencyEdge:
    producer: str
    consumer: str
    resource: str
    kind: str


@dataclass
class PacketFacts:
    packet_no: int
    endloop: Optional[str]
    facts: list
    capacity: dict
    opaque_reasons: list = field(default_factory=list)

    @property
    def scheduling_eligible(self):
        groups = [fact.insn.line_group for fact in self.facts]
        return (not self.endloop and len(groups) == len(set(groups)) and
                all(fact.scheduling_eligible for fact in self.facts))


@dataclass(frozen=True)
class Proposal:
    rule: str
    status: str
    packet_span: tuple
    message: str
    before: tuple = ()
    after: tuple = ()
    blockers: tuple = ()
    metadata: tuple = ()


@dataclass
class BasicBlock:
    index: int
    packets: list
    successors: set = field(default_factory=set)
    predecessors: set = field(default_factory=set)
    live_in: set = field(default_factory=set)
    live_out: set = field(default_factory=set)


# ---------------------------------------------------------------------------
# Input parsing: objdump symbols, packets, labels, and register provenance.
# ---------------------------------------------------------------------------

def extract_function(dump_lines, func_name):
    """Return the symbol address and raw objdump lines for one exact function."""
    start = None
    function_address = None
    for i, line in enumerate(dump_lines):
        match = FUNC_HEADER_RE.match(line.rstrip('\n'))
        if match and match.group(1) == func_name:
            start = i + 1
            function_address = int(line.split()[0], 16)
            break
    if start is None:
        sys.exit('error: function %r not found in objdump' % func_name)
    for end in range(start, len(dump_lines)):
        if FUNC_HEADER_RE.match(dump_lines[end].rstrip('\n')):
            return function_address, dump_lines[start:end]
    return function_address, dump_lines[start:]


def extract_function_lines(dump_lines, func_name):
    """Compatibility wrapper for callers that need only disassembly lines."""
    _address, lines = extract_function(dump_lines, func_name)
    return lines


def parse_packets(func_lines):
    """Keep objdump packets and the original line group of duplex instructions."""
    packets = []
    current = None
    current_endloop = None
    line_group = 0

    for raw in func_lines:
        line = raw.rstrip('\n')
        match = INSN_LINE_RE.match(line)
        if not match:
            continue
        addr, _word, rest = match.groups()
        addr = int(addr, 16)
        opens = '{' in rest
        endloop_match = ENDLOOP_RE.search(rest)
        text = rest.split('{', 1)[1] if opens else rest
        closes = '}' in text
        if closes:
            text = text.rsplit('}', 1)[0]
        if endloop_match:
            text = text[:endloop_match.start()]

        if opens or current is None:
            current = []
            current_endloop = None
            packets.append((None, current))

        sub_index = 0
        for sub in text.replace('\t', ' ').split(';'):
            sub = sub.strip()
            if not sub or sub.startswith('immext('):
                continue
            current.append(Insn(addr, sub, line_group, sub_index))
            sub_index += 1
        line_group += 1

        if endloop_match:
            current_endloop = endloop_match.group(0)
        if closes:
            packets[-1] = (current_endloop, current)

    for packet_no, (_endloop, insns) in enumerate(packets):
        for instruction_index, insn in enumerate(insns):
            insn.packet_no = packet_no
            # sub_index is the packet-unique node identity used by the
            # dependency report. line_group still preserves duplex origin.
            insn.sub_index = instruction_index
    return packets


# ---------------------------------------------------------------------------
# Instruction facts: classify forms and extract old/new reads and writes.
# ---------------------------------------------------------------------------

def strip_predicate_guard(text):
    """Return (guard, body), retaining nested compare parentheses in guard."""
    text = text.strip()
    if not text.startswith('if'):
        return None, text
    match = re.match(r'if\s*\(', text)
    if not match:
        return None, text
    depth = 0
    for i in range(match.end() - 1, len(text)):
        if text[i] == '(':
            depth += 1
        elif text[i] == ')':
            depth -= 1
            if depth == 0:
                return text[match.end():i], text[i + 1:].strip()
    return None, text


def split_lhs_rhs(text):
    """Split an assignment outside parentheses; return lhs, rhs, compound."""
    depth = 0
    for i, char in enumerate(text):
        if char == '(':
            depth += 1
        elif char == ')':
            depth -= 1
        elif char == '=' and depth == 0:
            previous = text[i - 1] if i else ''
            following = text[i + 1] if i + 1 < len(text) else ''
            if previous in '!<>=' or following == '=':
                continue
            lhs = text[:i].rstrip()
            compound = bool(lhs) and lhs[-1] in '+-|&^'
            return (lhs[:-1].rstrip() if compound else lhs), text[i + 1:], compound
    return None, text, False


def register_refs(text):
    old, new = set(), set()
    for match in REG_TOKEN_RE.finditer(text):
        target = new if match.group(3) else old
        target.add('r' + match.group(1))
        if match.group(2) is not None:
            target.add('r' + match.group(2))
    for match in PRED_TOKEN_RE.finditer(text):
        (new if match.group(2) else old).add('p' + match.group(1))
    return old, new


def simple_destination(lhs):
    """Return normalized GPR/predicate write set or None for non-register LHS."""
    lhs = lhs.strip()
    if re.fullmatch(r'r\d+(?::\d+)?', lhs):
        old, _new = register_refs(lhs)
        return old
    if re.fullmatch(r'p[0-3]', lhs):
        return {lhs}
    return None


def opcode_of(body):
    """Extract the opcode from an assignment RHS or a direct instruction."""
    _lhs, rhs, _compound = split_lhs_rhs(body)
    probe = rhs.strip() if _lhs is not None else body.strip()
    match = re.match(r'\+?([A-Za-z][A-Za-z0-9_.]*)', probe)
    return match.group(1).lstrip('+') if match else ''


def classify_instruction(text, predicated):
    """Classify a scalar objdump syntax form using the static curated tables."""
    _guard, body = strip_predicate_guard(text)
    opcode = opcode_of(body)
    lower = body.lower()
    lhs, _rhs, _compound = split_lhs_rhs(body)

    if opcode in SOLO_MNEMONICS:
        return CATEGORY_SYSTEM, 'SYSTEM/solo', 'solo'
    if opcode in ('allocframe', 'deallocframe') or CONTROL_OP_RE.search(body):
        return CATEGORY_CONTROL, 'J/CFI/CR', 'control-flow'
    if MEMORY_OP_RE.search(body):
        if lhs and MEMORY_LHS_RE.match(lhs):
            if re.search(r'(\|=|&=|\+=|-=)', body):
                return CATEGORY_MEM_RMW, 'MEMOP', 'memory-rmw'
            return CATEGORY_STORE, 'ST/NV/ST', 'memory-store'
        return CATEGORY_LOAD, 'LD', 'memory-load'
    if opcode.startswith('v') or any(marker in lower for marker in ('hvx', 'vgather', 'vscatter', 'vmem')):
        return CATEGORY_VECTOR, 'HVX/*', 'vector'
    if opcode.startswith(PREDICATE_OP_PREFIXES) or opcode in ('all8', 'any8'):
        return CATEGORY_PREDICATE, 'ALU32/PRED', None
    if (opcode in ALU_OPS or
            (lhs and re.fullmatch(r'r\d+(?::\d+)?', lhs.strip()) and
             (re.fullmatch(r'r\d+(?::\d+)?', _rhs.strip()) or
              re.fullmatch(r'#[-+]?(?:0x)?[0-9a-f]+', _rhs.strip())))):
        return CATEGORY_ALU, 'ALU32/ALU', None
    if opcode == 'nop':
        return CATEGORY_ALU, 'ALU32/ALU', None
    return CATEGORY_UNKNOWN, 'unmapped', 'unknown-vocabulary'


def parse_instruction(insn):
    guard, body = strip_predicate_guard(insn.text)
    lhs, rhs, compound = split_lhs_rhs(body)
    category, provenance, opaque_reason = classify_instruction(insn.text, guard is not None)
    reads_old, reads_new = register_refs((guard or '') + ' ' + rhs)
    writes = simple_destination(lhs) if lhs is not None else None
    if writes is None:
        writes = set()
    # Store addresses live on the assignment's left side. Capture them even
    # though all memory instructions remain fixed for scheduling today; this
    # makes lifetime diagnostics and future address rules honest.
    if lhs and MEMORY_LHS_RE.match(lhs):
        address_reads, address_new = register_refs(lhs)
        reads_old |= address_reads
        reads_new |= address_new
        post_update = re.search(r'\b(r\d+)\s*(?:\+\+|--)', lhs)
        if post_update:
            writes.add(post_update.group(1))
    if compound:
        reads_old |= writes
    if guard is not None and opaque_reason is None:
        opaque_reason = 'predicated-write'
    gpr_reads = {int(resource[1:]) for resource in reads_old | reads_new if resource.startswith('r')}
    gpr_writes = {int(resource[1:]) for resource in writes if resource.startswith('r')}
    # Kernel code legitimately reads fixed bases such as r28 (H2K_GP) and r29
    # (SP) for addressing. Only writes to r28-r31 are forbidden here; r20-r27
    # remain unavailable for ordinary program allocation even as read operands.
    if gpr_writes & (RESERVED_PROGRAM_REGS | set(FIXED_REGS)):
        if opaque_reason is None:
            opaque_reason = 'reserved-register-write'
    elif gpr_reads & RESERVED_PROGRAM_REGS:
        if opaque_reason is None:
            opaque_reason = 'reserved-register-read'
    return InsnFacts(insn=insn, category=category, provenance=provenance,
                     reads_old=frozenset(reads_old), reads_new=frozenset(reads_new),
                     writes=frozenset(writes), predicated=guard is not None,
                     opaque_reason=opaque_reason)


# ---------------------------------------------------------------------------
# Analysis: packet resources, old/new dependencies, CFG, and liveness.
# ---------------------------------------------------------------------------

def resource_counts(facts):
    categories = [fact.category for fact in facts]
    return {
        'logical': len(facts),
        'mem': sum(category in (CATEGORY_LOAD, CATEGORY_STORE, CATEGORY_MEM_RMW) for category in categories),
        'control': categories.count(CATEGORY_CONTROL),
        'alu_results': sum(bool(fact.writes) and fact.category in (CATEGORY_ALU, CATEGORY_PREDICATE) for fact in facts),
        'solo': sum(fact.opaque_reason == 'solo' for fact in facts),
    }


def check_packet_legality(func_name, packet_no, insns):
    """Compatibility warning pass; input packets are the objdump ground truth."""
    facts = [parse_instruction(insn) for insn in insns]
    counts = resource_counts(facts)
    problems = []
    if counts['solo'] and len(insns) > 1:
        problems.append('solo instruction is not alone')
    if counts['mem'] > 2:
        problems.append('%d memory ops in one packet (max 2)' % counts['mem'])
    if counts['control'] > 1:
        problems.append('%d control-flow ops in one packet (max 1)' % counts['control'])
    if len(insns) > 4:
        problems.append('%d logical instructions in one packet (max 4 non-duplex slots)' % len(insns))
    for problem in problems:
        print('warning: %s packet #%d looks illegal (%s) -- likely a parser/table gap: %r'
              % (func_name, packet_no, problem, [insn.text for insn in insns]), file=sys.stderr)


def check_call_clobber_liveness(func_name, packets):
    """Warn only when r0-r15 are read before a post-call reload."""
    clobbered = set()
    for packet_no, (_endloop, insns) in enumerate(packets):
        packet_facts = [parse_instruction(insn) for insn in insns]
        reads = set().union(*(fact.reads_old | fact.reads_new for fact in packet_facts)) if packet_facts else set()
        writes = set().union(*(fact.writes for fact in packet_facts)) if packet_facts else set()
        for resource in sorted((reads - writes) & {'r%d' % reg for reg in clobbered}):
            print('warning: %s packet #%d reads %s before reloading it after an earlier call '
                  '(%s is caller-saved scratch, r0-r15)'
                  % (func_name, packet_no, resource, resource), file=sys.stderr)
        clobbered -= {int(resource[1:]) for resource in writes if resource.startswith('r')}
        if any(CALL_RE.match(insn.text) for insn in insns):
            clobbered |= CALLER_SAVED_SCRATCH


def resolve_old_read_update_edges(packet_facts):
    """Describe ordinary same-packet old-read/write pipeline relations.

    This is intentionally not a data-forwarding dependency: the reader sees
    the packet-entry value while the writer prepares the next value for a
    later packet or iteration. It must never justify address-folding the reader
    as though it consumed the writer's result.
    """
    edges = []
    for writer in packet_facts.facts:
        for reader in packet_facts.facts:
            if writer.node == reader.node:
                continue
            for resource in sorted(writer.writes & reader.reads_old):
                edges.append(DependencyEdge(writer.node, reader.node, resource,
                                            'OLD_READ_UPDATE'))
    return edges


def resolve_new_edges(packet_facts, diagnostics):
    """Resolve .new only to a distinct instruction in this packet.

    A read of rN.new/pN.new is a same-packet bypass. It cannot refer to the
    instruction that consumes it, nor can it refer to a definition in an older
    packet. Ambiguity remains diagnostic rather than becoming a guessed edge.
    """
    edges = []
    producers = {}
    for fact in packet_facts.facts:
        for resource in fact.writes:
            producers.setdefault(resource, []).append(fact)
    for fact in packet_facts.facts:
        for resource in sorted(fact.reads_new):
            candidates = [producer for producer in producers.get(resource, [])
                          if producer.node != fact.node]
            if len(candidates) == 1:
                kind = 'NEW_PRED' if resource.startswith('p') else 'NEW_GPR'
                edges.append(DependencyEdge(candidates[0].node, fact.node, resource, kind))
            elif not candidates:
                diagnostics.append('%s: %s.new has no distinct same-packet producer' %
                                   (fact.node, resource))
            else:
                diagnostics.append('%s: %s.new has %d distinct same-packet producers' %
                                   (fact.node, resource, len(candidates)))
    return edges


def build_packet_facts(packets):
    result = []
    for packet_no, (endloop, insns) in enumerate(packets):
        facts = [parse_instruction(insn) for insn in insns]
        packet = PacketFacts(packet_no, endloop, facts, resource_counts(facts))
        packet.opaque_reasons = sorted({fact.opaque_reason for fact in facts if fact.opaque_reason})
        if endloop:
            packet.opaque_reasons.append('endloop')
        if len({fact.insn.line_group for fact in facts}) != len(facts):
            packet.opaque_reasons.append('duplex-or-compound')
        result.append(packet)
    return result


def build_dependencies(packet_facts):
    """Build edges only through contiguous movable scalar packet regions.

    Normal reads in a packet observe the old register state. Consequently, we
    snapshot reaching definitions/readers at packet entry and apply all writes
    only after processing that whole packet. The sole legal intra-packet data
    edge this report represents is an explicit .new edge.
    """
    edges, diagnostics = [], []
    definitions, readers = {}, {}
    for packet in packet_facts:
        edges.extend(resolve_old_read_update_edges(packet))
        edges.extend(resolve_new_edges(packet, diagnostics))
        if not packet.scheduling_eligible:
            definitions.clear()
            readers.clear()
            continue

        prior_definitions = dict(definitions)
        prior_readers = {resource: set(nodes) for resource, nodes in readers.items()}
        packet_reads = {}
        packet_writes = {}

        for fact in packet.facts:
            for resource in sorted(fact.reads_old):
                if resource in prior_definitions:
                    edges.append(DependencyEdge(prior_definitions[resource], fact.node, resource, 'RAW'))
                packet_reads.setdefault(resource, set()).add(fact.node)
            for resource in sorted(fact.writes):
                packet_writes.setdefault(resource, []).append(fact.node)
                if resource in prior_definitions:
                    edges.append(DependencyEdge(prior_definitions[resource], fact.node, resource, 'WAW'))
                for reader in sorted(prior_readers.get(resource, ())):
                    edges.append(DependencyEdge(reader, fact.node, resource, 'WAR'))

        # The report refuses to model multiple ordinary writers in one packet
        # as a scheduling opportunity. Preserve it as a diagnostic instead.
        for resource, writers in sorted(packet_writes.items()):
            if len(writers) > 1:
                diagnostics.append('P%03d: %s has %d same-packet writers' %
                                   (packet.packet_no, resource, len(writers)))
            definitions[resource] = writers[-1]
            readers[resource] = set()
        for resource, nodes in packet_reads.items():
            if resource not in packet_writes:
                readers.setdefault(resource, set()).update(nodes)
    return edges, diagnostics


def packet_internal_targets(packet, func_name, function_start, address_to_packet):
    """Return resolved internal direct-control targets for a packet.

    An unresolved target is intentionally omitted. CFG users treat it as an
    opaque exit rather than assuming it is an ordinary fallthrough edge.
    """
    targets = set()
    for fact in packet.facts:
        for match in TARGET_RE.finditer(fact.insn.text):
            symbol = match.group(2)
            name, separator, offset = symbol.rpartition('+')
            if not separator:
                name, offset = symbol, '0x0'
            if name == func_name:
                address = function_start + int(offset, 16)
                if address in address_to_packet:
                    targets.add(address_to_packet[address])
        for match in re.finditer(r'loop[01]\((0x[0-9a-f]+)', fact.insn.text):
            address = int(match.group(1), 16)
            if address in address_to_packet:
                targets.add(address_to_packet[address])
    return targets


def packet_has_conditional_control(packet):
    return any(strip_predicate_guard(fact.insn.text)[0] is not None and
               fact.category == CATEGORY_CONTROL for fact in packet.facts)


def packet_is_return_or_indirect_exit(packet):
    return any(re.search(r'\b(jumpr|dealloc_return|rte)\b', fact.insn.text)
               for fact in packet.facts)


def build_cfg(packet_facts, func_name, function_start=None):
    """Build a conservative packet-basic-block CFG within one objdump function.

    Calls remain fallthrough barriers; direct external targets and unresolved
    control forms terminate a block rather than creating speculative edges.
    """
    if not packet_facts:
        return []
    address_to_packet = {packet.facts[0].insn.addr: packet.packet_no
                         for packet in packet_facts}
    if function_start is None:
        function_start = packet_facts[0].facts[0].insn.addr
    leaders = {0}
    targets_by_packet = {}
    loop_targets = []

    for packet in packet_facts:
        targets = packet_internal_targets(packet, func_name, function_start,
                                          address_to_packet)
        targets_by_packet[packet.packet_no] = targets
        leaders.update(targets)
        if targets and any(re.search(r'\bloop[01]\(', fact.insn.text)
                           for fact in packet.facts):
            loop_targets.append(next(iter(targets)))
        if (packet.capacity['control'] or packet.endloop) and packet.packet_no + 1 < len(packet_facts):
            leaders.add(packet.packet_no + 1)

    starts = sorted(leaders)
    blocks = []
    packet_to_block = {}
    for index, start in enumerate(starts):
        stop = starts[index + 1] if index + 1 < len(starts) else len(packet_facts)
        packets = list(range(start, stop))
        block = BasicBlock(index=index, packets=packets)
        blocks.append(block)
        for packet_no in packets:
            packet_to_block[packet_no] = index

    for block in blocks:
        last_no = block.packets[-1]
        packet = packet_facts[last_no]
        direct_targets = targets_by_packet[last_no]
        has_call = any(CALL_RE.match(fact.insn.text) for fact in packet.facts)
        has_direct_jump = any(re.search(r'\bjump\b', fact.insn.text) for fact in packet.facts)

        for target in direct_targets:
            block.successors.add(packet_to_block[target])
        if packet.endloop and loop_targets:
            block.successors.add(packet_to_block[loop_targets[-1]])
        if not packet_is_return_or_indirect_exit(packet):
            fallthrough = last_no + 1
            if fallthrough < len(packet_facts) and (has_call or packet_has_conditional_control(packet) or not has_direct_jump):
                block.successors.add(packet_to_block[fallthrough])
        for successor in block.successors:
            blocks[successor].predecessors.add(block.index)
    return blocks


def packet_uses_defs(packet):
    """Return packet-entry uses and packet-exit definitions conservatively."""
    uses, defs = set(), set()
    for fact in packet.facts:
        uses.update(fact.reads_old)
        uses.update(fact.reads_new)
        if fact.predicated:
            # A predicated write can leave its old value intact.
            uses.update(fact.writes)
        else:
            defs.update(fact.writes)
        if CALL_RE.match(fact.insn.text):
            defs.update('r%d' % reg for reg in CALLER_SAVED_SCRATCH)
            defs.update('p%d' % pred for pred in range(4))
    return uses, defs


def compute_liveness(packet_facts, blocks):
    """Compute block and packet liveness to a fixed point over the CFG."""
    block_use, block_def = {}, {}
    for block in blocks:
        uses, defs = set(), set()
        for packet_no in block.packets:
            packet_use, packet_def = packet_uses_defs(packet_facts[packet_no])
            uses.update(packet_use - defs)
            defs.update(packet_def)
        block_use[block.index], block_def[block.index] = uses, defs

    changed = True
    while changed:
        changed = False
        for block in reversed(blocks):
            live_out = set().union(*(blocks[successor].live_in for successor in block.successors)) if block.successors else set()
            live_in = block_use[block.index] | (live_out - block_def[block.index])
            if live_in != block.live_in or live_out != block.live_out:
                block.live_in, block.live_out = live_in, live_out
                changed = True

    packet_live_in, packet_live_out = {}, {}
    for block in blocks:
        live = set(block.live_out)
        for packet_no in reversed(block.packets):
            packet_live_out[packet_no] = set(live)
            packet_use, packet_def = packet_uses_defs(packet_facts[packet_no])
            live = packet_use | (live - packet_def)
            packet_live_in[packet_no] = set(live)
    return packet_live_in, packet_live_out


def canonical_pure_expression(fact):
    """Fingerprint only a strict pure scalar ALU subset for local CSE.

    Memory, predicates, pairs, compound updates, .new, and special forms are
    excluded. Same-packet reads observe old state, so a producer is useful only
    from a preceding packet in the same straight-line block.
    """
    if (fact.category != CATEGORY_ALU or fact.predicated or fact.reads_new or
            fact.opaque_reason or len(fact.writes) != 1 or
            any(':' in match.group(0) for match in REG_TOKEN_RE.finditer(fact.insn.text))):
        return None
    guard, body = strip_predicate_guard(fact.insn.text)
    lhs, rhs, compound = split_lhs_rhs(body)
    if guard or compound or not lhs or not re.fullmatch(r'r\d+', lhs.strip()):
        return None
    opcode = opcode_of(body)
    if opcode not in ALU_OPS:
        return None
    # Keep the operands textual but normalize whitespace. This avoids treating
    # different immediate syntaxes as equivalent without an ISA numeric model.
    return opcode, re.sub(r'\s+', '', rhs)


def analyze_register_value_reuse(packet_facts, blocks, packet_live_in, packet_live_out):
    """Return physical-register windows and available-pure-value proposals."""
    windows, values = [], []
    allocatable = {'r%d' % reg for reg in range(20)}

    for packet in packet_facts:
        dead_after = sorted(allocatable - packet_live_out.get(packet.packet_no, set()))
        if dead_after:
            windows.append(Proposal(
                rule='register-reuse-window', status='ADVISORY',
                packet_span=(packet.packet_no, packet.packet_no),
                message='dead after P%03d: %s' % (packet.packet_no, ','.join(dead_after)),
                blockers=('physical role reuse still needs packet/register-allocation review',)))

    for block in blocks:
        available = {}
        for packet_no in block.packets:
            packet = packet_facts[packet_no]
            # Any opaque packet is a value-availability barrier. It may change
            # memory or hidden state even when it has no explicit GPR write.
            if not packet.scheduling_eligible:
                available.clear()
                continue
            for fact in packet.facts:
                expression = canonical_pure_expression(fact)
                if expression is not None:
                    producer = available.get(expression)
                    destination = next(iter(fact.writes))
                    if producer and producer[0] != destination:
                        values.append(Proposal(
                            rule='available-computed-value', status='ADVISORY',
                            packet_span=(producer[1], packet_no),
                            message='%s already holds %s at %s' %
                            (producer[0], expression[0], fact.node),
                            before=(fact.insn.text,),
                            after=('%s = %s' % (destination, producer[0]),),
                            blockers=('rewrite only if downstream uses can be renamed or duplicate result is dead',)))
                    available[expression] = (destination, packet_no, fact.node)
                # A write invalidates expressions whose destination or operand
                # register is overwritten. Conservatively discard all matches
                # mentioning the written resource in their source spelling.
                for written in fact.writes:
                    for expression in list(available):
                        if written == available[expression][0] or written in expression[1]:
                            del available[expression]
    return windows, values


def candidate_packets(packet_facts, edges):
    """Abstract grouping only: no output rewrite, no slot-placement claim."""
    predecessors = {}
    for edge in edges:
        predecessors.setdefault(edge.consumer, set()).add(edge.producer)
    candidates = []
    run = []

    def flush_run():
        if not run:
            return
        emitted = set()
        current, footprint, alu_results = [], set(), 0
        for fact in run:
            resources = set(fact.reads_old | fact.writes)
            produces_alu = bool(fact.writes) and fact.category in (CATEGORY_ALU, CATEGORY_PREDICATE)
            deps = predecessors.get(fact.node, set())
            if (current and (len(current) == 4 or alu_results + produces_alu > 3 or
                             resources & footprint or
                             any(dep in {node.node for node in current} for dep in deps))):
                candidates.append(list(current))
                emitted.update(node.node for node in current)
                current, footprint, alu_results = [], set(), 0
            current.append(fact)
            footprint |= resources
            alu_results += produces_alu
        if current:
            candidates.append(list(current))
        run.clear()

    for packet in packet_facts:
        if packet.scheduling_eligible:
            run.extend(packet.facts)
        else:
            flush_run()
    flush_run()
    return [candidate for candidate in candidates if len(candidate) > 1]


def format_resources(resources):
    return '{%s}' % ','.join(sorted(resources)) if resources else '{}'


def renumber_packets(packets):
    """Restore packet/node provenance after a mechanical regrouping."""
    for packet_no, (_endloop, insns) in enumerate(packets):
        for instruction_no, insn in enumerate(insns):
            insn.packet_no = packet_no
            insn.sub_index = instruction_no
    return packets


def squeeze_packet_accepts(target, source, moving):
    """Phase-one resource filter before optional assembler-oracle validation.

    A simple scalar may be extracted from a normal multi-instruction source
    packet only after `squeeze_move_kind` proves it is independent of the
    instructions left behind. Duplex groups remain indivisible.
    """
    if target.endloop or source.endloop:
        return False, 'loop/endloop packet'
    if len({fact.insn.line_group for fact in target.facts}) != len(target.facts):
        return False, 'duplex target packet'
    if (moving.category != CATEGORY_ALU or moving.predicated or moving.reads_new or
            moving.opaque_reason or len(moving.writes) > 1):
        return False, 'source is not a simple scalar ALU'
    allowed = {CATEGORY_ALU, CATEGORY_LOAD, CATEGORY_STORE}
    def target_fact_allowed(fact):
        if fact.category not in allowed or fact.predicated or fact.reads_new:
            return False
        if fact.category == CATEGORY_ALU:
            return fact.opaque_reason is None
        # Loads/stores are fixed as *moving* instructions, but a scalar ALU
        # may safely fill their packet after dependency proof. Their ordinary
        # memory opaque reason must therefore not reject the target outright.
        return fact.opaque_reason in ('memory-load', 'memory-store')
    if any(not target_fact_allowed(fact) for fact in target.facts):
        return False, 'target contains non-phase-one form'
    candidate = target.facts + [moving]
    counts = resource_counts(candidate)
    if counts['logical'] > 4 or counts['mem'] > 2 or counts['control'] or counts['alu_results'] > 3:
        return False, 'packet resource limit'
    return True, 'accepted'


def squeeze_move_kind(target, source, moving, crossed_packets, packet_live_out):
    """Prove a backward scalar move preserves packet-entry old-read semantics.

    `crossed_packets` lie strictly between target and source. Moving a write
    across them is valid only when none reads or writes that value, and moving
    a read across them is valid only when none produces that read's value.
    """
    source_siblings = [fact for fact in source.facts if fact.node != moving.node]
    for sibling in source_siblings:
        # Normal same-packet reads see packet-entry state, but relocating one
        # writer backward could make the sibling observe a different state in a
        # later packet. Do not split any such relation in phase one.
        if (moving.reads_old & sibling.writes or moving.writes & sibling.reads_old or
                moving.writes & sibling.writes or moving.reads_new or sibling.reads_new or
                moving.predicated or sibling.predicated):
            return None, 'source packet sibling dependency'
    target_reads = set().union(*(fact.reads_old for fact in target.facts)) if target.facts else set()
    target_writes = set().union(*(fact.writes for fact in target.facts)) if target.facts else set()
    if moving.reads_old & target_writes:
        return None, 'source needs a value newly written by target'
    if moving.writes & target_writes:
        return None, 'same-packet WAW conflict'

    for crossed in crossed_packets:
        if crossed.endloop or crossed.capacity['control']:
            return None, 'crosses control or loop packet'
        crossed_reads = set().union(*(fact.reads_old for fact in crossed.facts)) if crossed.facts else set()
        crossed_writes = set().union(*(fact.writes for fact in crossed.facts)) if crossed.facts else set()
        if moving.reads_old & crossed_writes:
            return None, 'source needs a value written by crossed packet'
        if moving.writes & crossed_writes:
            return None, 'crossed-packet WAW conflict'
        if moving.writes & crossed_reads:
            return None, 'crossed packet must retain old written value'

    old_read_updates = moving.writes & target_reads
    if old_read_updates:
        # The target intentionally consumes the old value while the moved ALU
        # produces the next value for later packets. Never treat this as .new.
        if not old_read_updates <= packet_live_out.get(source.packet_no, set()):
            return None, 'old-read update is not live after source packet'
        return 'OLD_READ_UPDATE', 'pipeline old-read/update: %s' % ','.join(sorted(old_read_updates))
    return 'INDEPENDENT_SCALAR_FILL', 'independent scalar fill'


def squeeze_block(packets, packet_facts, block, packet_live_out):
    """Find the earliest legal backward scalar fill in one CFG block."""
    for source_no in block.packets[1:]:
        source = packet_facts[source_no]
        # Preserve source packet order as a deterministic tie-breaker.
        for moving in source.facts:
            # Prefer the earliest legal target, not merely the adjacent one.
            # This creates room for dependent work on the next fixed-point
            # iteration.
            for target_no in reversed(block.packets[:block.packets.index(source_no)]):
                target = packet_facts[target_no]
                accepted, _reason = squeeze_packet_accepts(target, source, moving)
                if not accepted:
                    continue
                crossed = [packet_facts[packet_no]
                           for packet_no in range(target_no + 1, source_no)]
                kind, reason = squeeze_move_kind(target, source, moving, crossed,
                                                  packet_live_out)
                if kind is not None:
                    return source_no, target_no, moving.insn.sub_index, kind, reason
    return None


CONST_ASSIGN_RE = re.compile(
    r'^\s*(?P<register>r(?:[0-9]|1[0-9]))\s*=\s*#(?P<value>-?(?:0x[0-9a-f]+|\d+))\s*$')
CONST_COMPARE_RE = re.compile(
    r'^\s*(?P<predicate>p[0-3])\s*=\s*cmp\.eq\((?P<register>r(?:[0-9]|1[0-9])),#(?P<value>-?(?:0x[0-9a-f]+|\d+))\)\s*$')
PREDICATE_BRANCH_RE = re.compile(
    r'^\s*if\s*\((?P<negated>!)?(?P<predicate>p[0-3])\.new\)\s*jump(?::[a-z]+)?\s+(?P<target>.+?)\s*$')
FUSED_CONST_BRANCH_RE = re.compile(
    r'^\s*if\s*\(\s*!(?P<compare>cmp\.eq)\((?P<register>r(?:[0-9]|1[0-9]))\.new,#(?P<value>-?(?:0x[0-9a-f]+|\d+))\)\s*\)\s*jump(?::[a-z]+)?\s+(?P<target>.+?)\s*$')


RETURN_LOAD_RE = re.compile(
    r'^\s*(?P<tmp>r(?:[1-9]|1[0-9]))\s*=\s*memw\(r29\+#(?P<offset>-?(?:0x[0-9a-f]+|\d+))\)\s*$')
RETURN_COPY_RE = re.compile(r'^\s*r0\s*=\s*(?P<tmp>r(?:[1-9]|1[0-9]))\s*$')


def internal_target_packet_numbers(packets, func_name, function_start):
    """Return packets named by a direct internal branch/loop target."""
    if function_start is None:
        function_start = packets[0][1][0].addr
    addresses = {insns[0].addr: packet_no for packet_no, (_endloop, insns) in enumerate(packets)}
    targets = set()
    for _endloop, insns in packets:
        for insn in insns:
            for match in TARGET_RE.finditer(insn.text):
                symbol = match.group(2)
                name, separator, offset = symbol.rpartition('+')
                if not separator:
                    name, offset = symbol, '0x0'
                if name == func_name:
                    address = function_start + int(offset, 16)
                    if address in addresses:
                        targets.add(addresses[address])
    return targets


def value_overwritten_before_fallthrough_read(packets, start_packet, register):
    """Prove one fallthrough value is dead without reasoning about the target.

    Starting after a branch proven never-taken, accept only when the register is
    overwritten before any ordinary/.new read on the continued path. A call or
    another control transfer before that proof is a conservative blocker.
    """
    for packet_no in range(start_packet, len(packets)):
        _endloop, insns = packets[packet_no]
        facts = [parse_instruction(insn) for insn in insns]
        if any(register in (fact.reads_old | fact.reads_new) for fact in facts):
            return False
        # Packet writes commit together. A loop setup or other control in this
        # packet does not prevent rN being killed when it neither reads rN nor
        # transfers control away before the next packet can see the new rN.
        if any(register in fact.writes for fact in facts):
            return True
        if any(fact.category == CATEGORY_CONTROL for fact in facts):
            return all(re.search(r'\b(dealloc_return|jumpr|rte)\b', insn.text)
                       for insn in insns if parse_instruction(insn).category == CATEGORY_CONTROL)
    return True


def find_constant_fallthrough_proposals(packets, func_name, function_start):
    """Eliminate a strict locally constant, never-taken cmp.eq branch.

    This pass does not delete the branch target or any path. It only removes
    `rN=#imm; pN=cmp.eq(rN,#imm); if (!pN.new) jump target`, whose branch is
    known false and whose normal fallthrough overwrites rN before it can read
    the constant value.
    """
    proposals = []
    facts = build_packet_facts(packets)
    blocks = build_cfg(facts, func_name, function_start)
    _live_in, live_out = compute_liveness(facts, blocks)
    targets = internal_target_packet_numbers(packets, func_name, function_start)
    packet_to_block = {packet_no: block for block in blocks for packet_no in block.packets}

    # Fused form: { rN=#imm; if (!cmp.eq(rN.new,#imm2)) jump target }.
    # The .new explicitly consumes the assignment in this packet, so equal
    # constants make the negated branch never taken.
    for packet_no, packet in enumerate(facts):
        if packet_no in targets or packet.endloop or len(packet.facts) != 2:
            continue
        assignment = CONST_ASSIGN_RE.fullmatch(packet.facts[0].insn.text)
        branch = FUSED_CONST_BRANCH_RE.fullmatch(packet.facts[1].insn.text)
        if not assignment or not branch:
            continue
        if (assignment.group('register') != branch.group('register') or
                parse_immediate(assignment.group('value')) != parse_immediate(branch.group('value'))):
            continue
        register = assignment.group('register')
        if not value_overwritten_before_fallthrough_read(packets, packet_no + 1,
                                                         register):
            continue
        proposals.append(Proposal(
            rule='constant-fallthrough', status='EMITTABLE',
            packet_span=(packet_no, packet_no),
            message='%s.new=#%s makes fused !cmp.eq branch never take' %
                    (register, assignment.group('value')),
            before=(packet.facts[0].insn.text, packet.facts[1].insn.text),
            after=(), blockers=()))

    for assignment_no in range(len(packets) - 1):
        compare_no = assignment_no + 1
        assignment_packet = facts[assignment_no]
        compare_packet = facts[compare_no]
        if (assignment_no in targets or compare_no in targets or
                assignment_packet.endloop or compare_packet.endloop or
                len(assignment_packet.facts) != 1 or len(compare_packet.facts) != 2):
            continue
        if packet_to_block.get(assignment_no) is not packet_to_block.get(compare_no):
            continue
        assignment = CONST_ASSIGN_RE.fullmatch(assignment_packet.facts[0].insn.text)
        compare = CONST_COMPARE_RE.fullmatch(compare_packet.facts[0].insn.text)
        branch = PREDICATE_BRANCH_RE.fullmatch(compare_packet.facts[1].insn.text)
        if not assignment or not compare or not branch:
            continue
        if (assignment.group('register') != compare.group('register') or
                compare.group('predicate') != branch.group('predicate') or
                not branch.group('negated')):
            continue
        if parse_immediate(assignment.group('value')) != parse_immediate(compare.group('value')):
            continue
        register = assignment.group('register')
        predicate = compare.group('predicate')
        # Full CFG liveness includes the branch target, where rN may be used,
        # but that edge is provably unreachable. On the sole continued path,
        # rN must be overwritten before any read. pN is packet-local `.new`
        # state and has no later consumer because the compare packet is removed.
        if not value_overwritten_before_fallthrough_read(packets, compare_no + 1,
                                                         register):
            continue
        proposals.append(Proposal(
            rule='constant-fallthrough', status='EMITTABLE',
            packet_span=(assignment_no, compare_no),
            message='%s=#%s makes cmp.eq true, so !%s.new branch never takes' %
                    (register, assignment.group('value'), predicate),
            before=(assignment_packet.facts[0].insn.text,
                    compare_packet.facts[0].insn.text,
                    compare_packet.facts[1].insn.text),
            after=(), blockers=()))
    return proposals


def apply_constant_fallthrough_proposals(packets, proposals):
    rewritten = copy.deepcopy(packets)
    for proposal in sorted(proposals, key=lambda proposal: proposal.packet_span[0], reverse=True):
        assignment_no, compare_no = proposal.packet_span
        if assignment_no == compare_no:
            del rewritten[assignment_no]
        else:
            del rewritten[compare_no]
            del rewritten[assignment_no]
    return renumber_packets(rewritten)


def find_return_load_proposals(packets, func_name, function_start):
    """Find a strict load→copy→dealloc_return epilogue candidate.

    This is deliberately oracle-gated: the direct r0 word-load plus
    dealloc_return packet has no matching corpus precedent even though pair
    loads with dealloc_return and r0 loads with jumpr are both observed.
    """
    proposals = []
    targets = internal_target_packet_numbers(packets, func_name, function_start)
    for source_no in range(len(packets) - 1):
        source_endloop, source_insns = packets[source_no]
        return_endloop, return_insns = packets[source_no + 1]
        if source_no in targets or source_endloop or return_endloop or len(source_insns) != 1:
            continue
        load = RETURN_LOAD_RE.fullmatch(source_insns[0].text)
        if not load:
            continue
        if len(return_insns) != 2 or return_insns[1].text.strip() != 'dealloc_return':
            continue
        copy = RETURN_COPY_RE.fullmatch(return_insns[0].text)
        if not copy or copy.group('tmp') != load.group('tmp'):
            continue
        temporary = load.group('tmp')
        # This load overwrites any earlier physical rN value. Its reaching
        # definition is consumed immediately by `r0 = tmp`, then the function
        # returns, so unrelated prior uses of the same physical register do not
        # disqualify the local propagation.
        direct_load = 'r0 = memw(r29+#%s)' % load.group('offset')
        proposals.append(Proposal(
            rule='return-load-propagation', status='ORACLE_REQUIRED',
            packet_span=(source_no, source_no + 1),
            message='%s only forwards a frame load into r0 before dealloc_return' % temporary,
            before=(source_insns[0].text, return_insns[0].text, return_insns[1].text),
            after=(direct_load, 'dealloc_return'),
            blockers=('requires --assembler-oracle for direct r0 word-load + dealloc_return',)))
    return proposals


def apply_return_load_proposals(packets, proposals):
    rewritten = copy.deepcopy(packets)
    for proposal in sorted(proposals, key=lambda proposal: proposal.packet_span[0], reverse=True):
        source_no, return_no = proposal.packet_span
        _source_endloop, _source_insns = rewritten[source_no]
        return_endloop, return_insns = rewritten[return_no]
        return_insns[0].text = proposal.after[0]
        rewritten[return_no] = (return_endloop, return_insns)
        del rewritten[source_no]
    return renumber_packets(rewritten)


GENERIC_RETURN_COPY_RE = re.compile(r'^\s*r0\s*=\s*(r(?:[1-9]|1[0-9]))\s*$')
GENERIC_DEST_RE = re.compile(r'^(\s*)r(?:[1-9]|1[0-9])(\s*=.*)$')


def find_generic_return_coalescing_proposals(packets, func_name, function_start):
    """Find straight-line pure-ALU result -> r0 return-copy coalescing.

    The destination rewrite is safe only when one pure scalar definition reaches
    the final `r0 = tmp; dealloc_return` packet, no intervening instruction
    reads/writes tmp, and no call/control can invalidate the old r0 or tmp.
    """
    facts = build_packet_facts(packets)
    blocks = build_cfg(facts, func_name, function_start)
    targets = internal_target_packet_numbers(packets, func_name, function_start)
    proposals = []
    for return_no, (return_endloop, return_insns) in enumerate(packets):
        if (return_endloop or return_no in targets or len(return_insns) != 2 or
                return_insns[1].text.strip() != 'dealloc_return'):
            continue
        copy = GENERIC_RETURN_COPY_RE.fullmatch(return_insns[0].text)
        if not copy:
            continue
        temporary = copy.group(1)
        block = next((block for block in blocks if return_no in block.packets), None)
        if block is None:
            continue
        block_position = block.packets.index(return_no)
        for definition_no in reversed(block.packets[:block_position]):
            definition_packet = facts[definition_no]
            if (definition_no in targets or definition_packet.endloop or
                    definition_packet.capacity['control']):
                break
            candidates = [fact for fact in definition_packet.facts
                          if fact.writes == frozenset((temporary,))]
            if len(candidates) != 1:
                continue
            definition = candidates[0]
            if (canonical_pure_expression(definition) is None or
                    'r0' in definition.reads_new or definition.predicated):
                continue
            blocked = False
            definition_position = block.packets.index(definition_no)
            if len(definition_packet.facts) != 1:
                continue
            for packet_no in block.packets[definition_position + 1:block_position]:
                packet = facts[packet_no]
                if any(temporary in (fact.reads_old | fact.reads_new | fact.writes) or
                       'r0' in (fact.reads_old | fact.reads_new | fact.writes)
                       for fact in packet.facts):
                    blocked = True
                    break
                if any(CALL_RE.match(fact.insn.text) or fact.category == CATEGORY_CONTROL
                       for fact in packet.facts):
                    blocked = True
                    break
            if blocked:
                continue
            replacement = GENERIC_DEST_RE.match(definition.insn.text)
            if replacement is None:
                continue
            rewritten_definition = replacement.group(1) + 'r0' + replacement.group(2)
            proposals.append(Proposal(
                rule='return-value-coalescing', status='ORACLE_REQUIRED',
                packet_span=(definition_no, return_no),
                message='compute pure result directly in r0 and remove r0=%s return copy' % temporary,
                before=(definition.insn.text, return_insns[0].text, return_insns[1].text),
                after=(rewritten_definition, 'dealloc_return'),
                blockers=('requires --assembler-oracle; straight-line pure-ALU result only',)))
            break
    return proposals


def apply_generic_return_coalescing_proposals(packets, proposals):
    rewritten = copy.deepcopy(packets)
    for proposal in sorted(proposals, key=lambda proposal: proposal.packet_span[0], reverse=True):
        definition_no, return_no = proposal.packet_span
        definition = proposal.before[0]
        for insn in rewritten[definition_no][1]:
            if insn.text == definition:
                insn.text = proposal.after[0]
                break
        return_endloop, return_insns = rewritten[return_no]
        return_insns = [insn for insn in return_insns if insn.text.strip() != proposal.before[1].strip()]
        rewritten[return_no] = (return_endloop, return_insns)
    return renumber_packets(rewritten)


GUARDED_ADDRESS_RE = re.compile(
    r'^\s*(?P<address>r\d+)\s*=\s*addasl\((?P<base>r\d+),(?P<index>r\d+),#(?:\d+|0x[0-9a-f]+)\)\s*$')
GUARDED_NULL_COMPARE_RE = re.compile(
    r'^\s*(?P<predicate>p[0-3])\s*=\s*cmp\.eq\((?P<address>r\d+),#(?:0|0x0)\)\s*$')
GUARDED_BRANCH_RE = re.compile(
    r'^\s*if\s*\((?P<predicate>p[0-3])\.new\)\s*jump(?::[a-z]+)?\s+(?P<target>.+?)\s*$')
GUARDED_FALLTHROUGH_SUB_RE = re.compile(
    r'^\s*(?P<result>r\d+)\s*=\s*sub\((?P<address>r\d+),(?P<base>r\d+)\)\s*$')
GUARDED_SECOND_COMPARE_RE = re.compile(
    r'^\s*(?P<predicate>p[0-3])\s*=\s*cmp\.eq\((?P<register>r\d+),#(?:0|0x0)\)\s*$')
GUARDED_MEMORY_LOAD_RE = re.compile(
    r'^\s*(?P<result>r\d+)\s*=\s*memw\((?P<address>r\d+)\+#(?:\d+|0x[0-9a-f]+)\)\s*$')
GUARDED_PREDICATE_CONSUMER_RE = re.compile(
    r'^\s*if\s*\(!(?P<predicate>p[0-3])\)\s*jump(?::[a-z]+)?\s+(?P<target>.+?)\s*$')


def predicate_used_elsewhere(packets, predicate, excluded):
    for packet_no, (_endloop, insns) in enumerate(packets):
        for instruction_no, insn in enumerate(insns):
            if (packet_no, instruction_no) in excluded:
                continue
            old, new = register_refs(insn.text)
            if predicate in old or predicate in new:
                return True
    return False


def predicate_resources_in_range(packets, start, end):
    used = set()
    for _packet_no, (_endloop, insns) in enumerate(packets[start:end], start):
        for insn in insns:
            old, new = register_refs(insn.text)
            used.update(resource for resource in old | new if resource.startswith('p'))
    return used


def predicate_used_outside_range(packets, predicate, start, end):
    for packet_no, (_endloop, insns) in enumerate(packets):
        if start <= packet_no < end:
            continue
        for insn in insns:
            old, new = register_refs(insn.text)
            if predicate in old or predicate in new:
                return True
    return False


def find_guarded_fallthrough_predication_proposals(packets, func_name, function_start):
    """Find a validated branch/fallthrough packet-predication pattern.

    Register and predicate names are captured from the input. The packet shape
    is intentionally strict until more validated descriptors are added.
    """
    proposals = []
    targets = internal_target_packet_numbers(packets, func_name, function_start)
    for start in range(len(packets) - 4):
        packets_slice = packets[start:start + 5]
        if (any(endloop for endloop, _insns in packets_slice) or
                any(packet_no in targets for packet_no in (start + 1, start + 2, start + 4))):
            continue
        address_packet, branch_packet, fallthrough_packet, _next_packet, consumer_packet = packets_slice
        if (len(address_packet[1]) != 1 or len(branch_packet[1]) != 2 or
                len(fallthrough_packet[1]) != 3 or len(consumer_packet[1]) != 3):
            continue

        address_match = GUARDED_ADDRESS_RE.fullmatch(address_packet[1][0].text)
        null_match = GUARDED_NULL_COMPARE_RE.fullmatch(branch_packet[1][0].text)
        branch_match = GUARDED_BRANCH_RE.fullmatch(branch_packet[1][1].text)
        sub_match = GUARDED_FALLTHROUGH_SUB_RE.fullmatch(fallthrough_packet[1][0].text)
        second_match = GUARDED_SECOND_COMPARE_RE.fullmatch(fallthrough_packet[1][1].text)
        load_match = GUARDED_MEMORY_LOAD_RE.fullmatch(fallthrough_packet[1][2].text)
        if not all((address_match, null_match, branch_match, sub_match,
                    second_match, load_match)):
            continue

        branch_pred = null_match.group('predicate')
        second_pred = second_match.group('predicate')
        if (branch_match.group('predicate') != branch_pred or
                null_match.group('address') != address_match.group('address') or
                sub_match.group('address') != address_match.group('address') or
                sub_match.group('base') != address_match.group('base') or
                load_match.group('address') != address_match.group('address')):
            continue

        consumer_index = None
        consumer_target = None
        for instruction_no, insn in enumerate(consumer_packet[1]):
            match = GUARDED_PREDICATE_CONSUMER_RE.fullmatch(insn.text)
            if match and match.group('predicate') == second_pred:
                consumer_index, consumer_target = instruction_no, match.group('target')
                break
        if consumer_index is None:
            continue

        # Keep the later predicate when it is not the branch predicate and is
        # unused outside this five-packet range. Otherwise select a free p0-p3.
        used = predicate_resources_in_range(packets, start, start + 5)
        replacement_pred = second_pred
        if (replacement_pred == branch_pred or
                predicate_used_outside_range(packets, replacement_pred, start, start + 5)):
            candidates = [predicate for predicate in ('p0', 'p1', 'p2', 'p3')
                          if predicate != branch_pred and
                          predicate not in used and
                          not predicate_used_outside_range(packets, predicate, start, start + 5)]
            if not candidates:
                continue
            replacement_pred = candidates[0]

        proposals.append(Proposal(
            rule='guarded-fallthrough-predication', status='ORACLE_REQUIRED',
            packet_span=(start, start + 4),
            message='predicate fallthrough work on !%s.new; preserve %s as %s' %
                    (branch_pred, second_pred, replacement_pred),
            before=tuple(insn.text for _endloop, insns in packets_slice for insn in insns),
            after=(),
            blockers=('requires --assembler-oracle; exact validated packet descriptor',),
            metadata=(('branch_pred', branch_pred), ('second_pred', second_pred),
                      ('replacement_pred', replacement_pred), ('consumer_index', str(consumer_index)),
                      ('compare', second_match.group(0)), ('sub', sub_match.group(0)),
                      ('load', load_match.group(0)))))
    return proposals


def apply_guarded_fallthrough_predication_proposals(packets, proposals):
    rewritten = copy.deepcopy(packets)
    for proposal in sorted(proposals, key=lambda proposal: proposal.packet_span[0], reverse=True):
        metadata = dict(proposal.metadata)
        start, _end = proposal.packet_span
        address_endloop, address_insns = rewritten[start]
        branch_endloop, branch_insns = rewritten[start + 1]
        _fallthrough_endloop, _fallthrough_insns = rewritten[start + 2]
        _next_endloop, _next_insns = rewritten[start + 3]
        _consumer_endloop, consumer_insns = rewritten[start + 4]
        branch_pred = metadata['branch_pred']
        second_pred = metadata['second_pred']
        replacement_pred = metadata['replacement_pred']
        compare_rhs = metadata['compare'].split('=', 1)[1].strip()
        address_insns.append(Insn(address_insns[0].addr,
                                  '%s = %s' % (replacement_pred, compare_rhs),
                                  address_insns[0].line_group + 1000000, len(address_insns)))
        guarded_sub = 'if (!%s.new) %s' % (branch_pred, metadata['sub'])
        guarded_load = 'if (!%s.new) %s' % (branch_pred, metadata['load'])
        branch_insns.extend([
            Insn(branch_insns[0].addr, guarded_sub,
                 branch_insns[0].line_group + 1000001, len(branch_insns)),
            Insn(branch_insns[0].addr, guarded_load,
                 branch_insns[0].line_group + 1000002, len(branch_insns) + 1),
        ])
        consumer_index = int(metadata['consumer_index'])
        consumer_insns[consumer_index].text = consumer_insns[consumer_index].text.replace(
            '!%s' % second_pred, '!%s' % replacement_pred, 1)
        rewritten[start] = (address_endloop, address_insns)
        rewritten[start + 1] = (branch_endloop, branch_insns)
        del rewritten[start + 2]
    return renumber_packets(rewritten)


GUARDED_RESULT_SHIFT_RE = re.compile(
    r'^\s*(?P<result>r\d+)\s*=\s*asr\((?P<source>r\d+),#(?:3|0x3)\)\s*$')
GUARDED_INCREMENT_RE = re.compile(
    r'^\s*(?P<result>r\d+)\s*=\s*add\((?P<source>r\d+),#(?:1|0x1)\)\s*$')
GUARDED_RETURN_COPY_RE = re.compile(r'^\s*r0\s*=\s*(?P<source>r\d+)\s*$')
GUARDED_PREDICATED_OP_RE = re.compile(
    r'^\s*if\s*\(!(?P<predicate>p[0-3])\.new\)\s+(?P<body>.+?)\s*$')


def find_guarded_return_value_coalescing_proposals(packets, func_name, function_start):
    """Find a guarded pure-result chain ending in an r0 return copy.

    The branch predicate, result registers, and later predicate are captured
    from the input; no fixed physical register number is required.
    """
    proposals = []
    targets = internal_target_packet_numbers(packets, func_name, function_start)
    for start in range(len(packets) - 4):
        p0, p1, p2, p3, p4 = packets[start:start + 5]
        if any(_endloop for _endloop, _insns in (p0, p1, p2, p3, p4)):
            continue
        if any(packet_no in targets for packet_no in (start + 1, start + 2, start + 3, start + 4)):
            continue
        if (len(p0[1]) != 2 or len(p1[1]) != 4 or len(p2[1]) != 2 or
                len(p3[1]) != 3 or len(p4[1]) != 2):
            continue
        address_match = GUARDED_ADDRESS_RE.fullmatch(p0[1][0].text)
        branch_compare = GUARDED_NULL_COMPARE_RE.fullmatch(p1[1][0].text)
        branch = GUARDED_BRANCH_RE.fullmatch(p1[1][1].text)
        sub = GUARDED_PREDICATED_OP_RE.fullmatch(p1[1][2].text)
        load = GUARDED_PREDICATED_OP_RE.fullmatch(p1[1][3].text)
        result_shift = GUARDED_RESULT_SHIFT_RE.fullmatch(p2[1][0].text)
        increment = GUARDED_INCREMENT_RE.fullmatch(p2[1][1].text)
        return_copy = GUARDED_RETURN_COPY_RE.fullmatch(p4[1][0].text)
        if not all((address_match, branch_compare, branch, sub, load,
                    result_shift, increment, return_copy)):
            continue
        branch_pred = branch_compare.group('predicate')
        sub_pred = sub.group('predicate')
        load_pred = load.group('predicate')
        if (branch.group('predicate') != branch_pred or sub_pred != branch_pred or
                load_pred != branch_pred or result_shift.group('source') != result_shift.group('result') or
                increment.group('source') != load.group('body').split('=', 1)[0].strip() or
                return_copy.group('source') != result_shift.group('result') or
                p4[1][1].text.strip() != 'dealloc_return'):
            continue
        second_pred = None
        for insn in p3[1]:
            match = GUARDED_PREDICATE_CONSUMER_RE.fullmatch(insn.text)
            if match:
                second_pred = match.group('predicate')
                break
        if second_pred is None:
            continue
        result = result_shift.group('result')
        proposals.append(Proposal(
            rule='guarded-return-value-coalescing', status='ORACLE_REQUIRED',
            packet_span=(start, start + 4),
            message='coalesce guarded %s result directly into r0 and remove return copy' % result,
            before=tuple(insn.text for _endloop, insns in (p0, p1, p2, p3, p4) for insn in insns),
            after=(),
            blockers=('requires --assembler-oracle; preserves r0 on the branch path',),
            metadata=(('branch_pred', branch_pred), ('result', result),
                      ('consumer_pred', second_pred), ('sub_body', sub.group('body')),)))
    return proposals


def apply_guarded_return_value_coalescing_proposals(packets, proposals):
    rewritten = copy.deepcopy(packets)
    for proposal in sorted(proposals, key=lambda proposal: proposal.packet_span[0], reverse=True):
        metadata = dict(proposal.metadata)
        start, _end = proposal.packet_span
        p1_endloop, p1_insns = rewritten[start + 1]
        p2_endloop, p2_insns = rewritten[start + 2]
        p4_endloop, p4_insns = rewritten[start + 4]
        _sub_lhs, sub_rhs, _compound = split_lhs_rhs(metadata['sub_body'])
        p1_insns[2].text = 'if (!%s.new) r0 = %s' % (metadata['branch_pred'], sub_rhs.strip())
        p2_insns[0].text = 'r0 = asr(r0,#0x3)'
        for insn in rewritten[start + 3][1]:
            if insn.text.strip().startswith('if (!%s)' % metadata['consumer_pred']):
                break
        rewritten[start + 1] = (p1_endloop, p1_insns)
        rewritten[start + 2] = (p2_endloop, p2_insns)
        del p4_insns[0]
        rewritten[start + 4] = (p4_endloop, p4_insns)
    return renumber_packets(rewritten)


def is_nop(insn):
    return insn.text.strip() == 'nop'


def remove_redundant_nops(packets):
    """Remove packet-filler nops, preserving standalone and hardware-loop tails.

    A nop sharing a normal packet with real instructions has no architectural
    result and only consumes a slot. An `:endloop` nop may be required loop-tail
    padding, while a standalone nop can be a branch target; keep both until an
    architecture-specific assembler/CFG proof says otherwise.
    """
    rewritten = copy.deepcopy(packets)
    removed = []
    for packet_no, (endloop, insns) in enumerate(rewritten):
        if endloop or len(insns) <= 1:
            continue
        kept = [insn for insn in insns if not is_nop(insn)]
        if len(kept) != len(insns):
            removed.extend(insn.text for insn in insns if is_nop(insn))
            rewritten[packet_no] = (endloop, kept)
    return rewritten, removed


def squeeze_packets(packets, func_name, function_start=None):
    """Mechanically squeeze phase-one scalar instructions within CFG blocks.

    It never moves memory/control instructions. Scalar motion is adjacent and
    block-local, so memory ordering, calls, labels, and branch structure stay
    intact. Existing `.new` groups are excluded by the acceptance filter.
    """
    before_calls = call_fingerprint(packets)
    before_memory = memory_fingerprint(packets)
    before_non_nops = Counter(insn.text for _endloop, insns in packets
                              for insn in insns if not is_nop(insn))
    rewritten, removed_nops = remove_redundant_nops(packets)
    constant_proposals = find_constant_fallthrough_proposals(
        rewritten, func_name, function_start)
    removed_constants = Counter(
        instruction for proposal in constant_proposals for instruction in proposal.before)
    if constant_proposals:
        rewritten = apply_constant_fallthrough_proposals(rewritten, constant_proposals)
    renumber_packets(rewritten)
    moves = []
    for proposal in constant_proposals:
        moves.append((-1, -1, 'CONSTANT_FALLTHROUGH', proposal.message))
    # Rebuild the analysis after every accepted move. This is essential for
    # cascades: after packing a table-base update with an old-base load, its
    # dependent scaled-address operation may become movable into the newly
    # opened packet on the next pass.
    for _iteration in range(len(rewritten)):
        facts = build_packet_facts(rewritten)
        blocks = build_cfg(facts, func_name, function_start)
        _live_in, live_out = compute_liveness(facts, blocks)
        next_move = None
        for block in blocks:
            candidate = squeeze_block(rewritten, facts, block, live_out)
            if candidate is not None:
                next_move = candidate
                break
        if next_move is None:
            break
        source_no, target_no, moving_index, kind, reason = next_move
        source_endloop, source_insns = rewritten[source_no]
        _target_endloop, target_insns = rewritten[target_no]
        target_insns.append(source_insns.pop(moving_index))
        if not source_insns:
            del rewritten[source_no]
        else:
            rewritten[source_no] = (source_endloop, source_insns)
        renumber_packets(rewritten)
        moves.append((source_no, target_no, kind, reason))
    else:
        raise RuntimeError('squeeze did not converge')
    if call_fingerprint(rewritten) != before_calls:
        raise RuntimeError('squeeze changed call fingerprint')
    if memory_fingerprint(rewritten) != before_memory:
        raise RuntimeError('squeeze changed memory operation order/count')
    expected_non_nops = before_non_nops - removed_constants
    if Counter(insn.text for _endloop, insns in rewritten for insn in insns
               if not is_nop(insn)) != expected_non_nops:
        raise RuntimeError('squeeze changed non-nop instruction multiset')
    if removed_nops:
        moves.insert(0, (-1, -1, 'REMOVE_NOP', '%d non-loop packet filler nop(s)' % len(removed_nops)))
    return rewritten, moves


# ---------------------------------------------------------------------------
# Optimizer: packet squeezing, nop removal, and control-flow simplification.
# ---------------------------------------------------------------------------
# Static architecture profiles are deliberately small. They encode only the
# scalar forms promoted from intrinsics.txt into concrete local rewrites.
# Unknown forms are report-only; this is not a general packet legality model.
ARCH_RULES = {
    'v68': frozenset({('memb', 0), ('memh', 0), ('memw', 0), ('memd', 0),
                      ('memb', 1), ('memh', 1), ('memw', 1), ('memd', 1),
                      ('memb', 2), ('memh', 2), ('memw', 2), ('memd', 2),
                      ('memb', 3), ('memh', 3), ('memw', 3), ('memd', 3)}),
    'v73': frozenset({('memb', 0), ('memh', 0), ('memw', 0), ('memd', 0),
                      ('memb', 1), ('memh', 1), ('memw', 1), ('memd', 1),
                      ('memb', 2), ('memh', 2), ('memw', 2), ('memd', 2),
                      ('memb', 3), ('memh', 3), ('memw', 3), ('memd', 3)}),
    'v81': frozenset({('memb', 0), ('memh', 0), ('memw', 0), ('memd', 0),
                      ('memb', 1), ('memh', 1), ('memw', 1), ('memd', 1),
                      ('memb', 2), ('memh', 2), ('memw', 2), ('memd', 2),
                      ('memb', 3), ('memh', 3), ('memw', 3), ('memd', 3)}),
}

# The first concrete rules deliberately use only encodings whose scalar base +
# signed displacement form is listed in intrinsics.txt for LD/ST on v68-v81.
# Exact slot placement remains untouched: a rewrite only removes an instruction
# or changes one operand within its existing packet.
MEM_BASE_DISP_RE = re.compile(
    r'(?P<prefix>mem(?:[bhwd]|u[bhw])[a-z]*\()'
    r'(?P<base>r\d+)\+#(?P<disp>-?(?:0x[0-9a-f]+|\d+))'
    r'(?P<suffix>\))')
MEM_INDEXED_ZERO_RE = re.compile(
    r'(?P<prefix>mem(?P<opcode>[bhwd])\()'
    r'(?P<base>r\d+)\+#(?:0|0x0)'
    r'(?P<suffix>\))')
ADDASL_RE = re.compile(
    r'^\s*(?P<dst>r\d+)\s*=\s*addasl\((?P<base>r\d+),(?P<index>r\d+),#(?P<shift>[0-3])\)\s*$')
ADD_IMMEDIATE_RE = re.compile(
    r'^\s*(?P<dst>r\d+)\s*=\s*add\((?P<base>r\d+),#(?P<imm>-?(?:0x[0-9a-f]+|\d+))\)\s*$')
GP_COPY_RE = re.compile(r'^\s*(?P<dst>r(?:[0-9]|1[0-9]))\s*=\s*r28\s*$')


def parse_immediate(text):
    return int(text, 16) if text.lower().startswith('-0x') is False and text.lower().startswith('0x') else int(text, 0)


def format_immediate(value):
    return '#%d' % value if value < 0 else '#0x%x' % value


def replace_memory_base_displacement(text, old_base, new_base, addend):
    """Fold `old_base + addend` into one exact scalar memory address.

    Returns None when the instruction contains no uniquely-supported reference
    to old_base, more than one reference, or an out-of-range signed s11
    displacement. The s11 range is the scalar LD/ST base+offset form recorded
    in intrinsics.txt; width alignment is intentionally left to the assembler.
    """
    matches = list(MEM_BASE_DISP_RE.finditer(text))
    matched = [match for match in matches if match.group('base') == old_base]
    if len(matched) != 1 or len(matches) != 1:
        return None
    match = matched[0]
    displacement = parse_immediate(match.group('disp')) + addend
    if not -(1 << 10) <= displacement < (1 << 10):
        return None
    return text[:match.start('base')] + new_base + '+' + format_immediate(displacement) + text[match.end('disp'):]


def packet_is_removable(packet_no, packet_facts, blocks):
    """Only remove a one-instruction packet that is internal to a block.

    No target/leader/endloop/control packet is removed. The block-size check
    means this first emitter never changes a branch, loop, or packet boundary
    visible to another CFG predecessor.
    """
    for block in blocks:
        if packet_no in block.packets:
            return (len(block.packets) > 1 and packet_no != block.packets[0] and
                    not packet_facts[packet_no].endloop and
                    packet_facts[packet_no].capacity['control'] == 0)
    return False


def replace_indexed_memory_base(text, old_base, new_base, index, shift, arch):
    """Replace the validated `memX(old_base+#0)` scalar form with indexed form."""
    match = MEM_INDEXED_ZERO_RE.search(text)
    if not match or match.group('base') != old_base:
        return None
    opcode = 'mem' + match.group('opcode')
    if (opcode, shift) not in ARCH_RULES[arch]:
        return None
    return (text[:match.start('base')] + new_base + '+' + index + '<<#' + str(shift) +
            text[match.start('suffix'):])


def local_address_use_is_rewritable(packet_no, temporary, packet_facts, blocks,
                                     packet_live_out):
    """Prove the two-packet address island does not leave `temporary` live."""
    consumer_no = packet_no + 1
    if consumer_no >= len(packet_facts):
        return False
    producer, consumer = packet_facts[packet_no], packet_facts[consumer_no]
    if (len(producer.facts) != 1 or len(consumer.facts) != 1 or
            not packet_is_removable(packet_no, packet_facts, blocks) or
            producer.capacity['control'] or consumer.capacity['control'] or
            temporary in packet_live_out.get(consumer_no, set())):
        return False
    return any(consumer_no in block.packets and packet_no in block.packets
               for block in blocks)


def find_gp_copy_proposals(packets, packet_facts, blocks, packet_live_out):
    proposals = []
    for packet_no, packet in enumerate(packet_facts[:-1]):
        copy_match = GP_COPY_RE.fullmatch(packet.facts[0].insn.text) if len(packet.facts) == 1 else None
        if not copy_match:
            continue
        destination = copy_match.group('dst')
        if not local_address_use_is_rewritable(packet_no, destination, packet_facts,
                                               blocks, packet_live_out):
            continue
        following = packet_facts[packet_no + 1]
        rewritten = replace_memory_base_displacement(following.facts[0].insn.text,
                                                      destination, 'r28', 0)
        if rewritten is None:
            continue
        proposals.append(Proposal(
            rule='gp-copy', status='EMITTABLE',
            packet_span=(packet_no, packet_no + 1),
            message='%s is a single-use H2K_GP address-base copy' % destination,
            before=(packet.facts[0].insn.text, following.facts[0].insn.text),
            after=(rewritten,), blockers=()))
    return proposals


def find_address_fold_proposals(packet_facts, blocks, packet_live_out):
    proposals = []
    for packet_no, packet in enumerate(packet_facts[:-1]):
        definition = ADD_IMMEDIATE_RE.fullmatch(packet.facts[0].insn.text) if len(packet.facts) == 1 else None
        if not definition:
            continue
        destination, base = definition.group('dst'), definition.group('base')
        if int(destination[1:]) >= 20 or destination == base:
            continue
        if not local_address_use_is_rewritable(packet_no, destination, packet_facts,
                                               blocks, packet_live_out):
            continue
        following = packet_facts[packet_no + 1]
        rewritten = replace_memory_base_displacement(
            following.facts[0].insn.text, destination, base,
            parse_immediate(definition.group('imm')))
        if rewritten is None:
            continue
        proposals.append(Proposal(
            rule='address-fold', status='EMITTABLE',
            packet_span=(packet_no, packet_no + 1),
            message='%s has one immediate scalar memory-address use' % destination,
            before=(packet.facts[0].insn.text, following.facts[0].insn.text),
            after=(rewritten,), blockers=()))
    return proposals


def find_addasl_indexed_memory_proposals(packet_facts, blocks, packet_live_out, arch):
    proposals = []
    for packet_no, packet in enumerate(packet_facts[:-1]):
        definition = ADDASL_RE.fullmatch(packet.facts[0].insn.text) if len(packet.facts) == 1 else None
        if not definition:
            continue
        temporary = definition.group('dst')
        base, index, shift = definition.group('base'), definition.group('index'), int(definition.group('shift'))
        if (int(temporary[1:]) >= 20 or temporary in (base, index) or
                not local_address_use_is_rewritable(packet_no, temporary, packet_facts,
                                                    blocks, packet_live_out)):
            continue
        following = packet_facts[packet_no + 1]
        consumer = following.facts[0]
        if (consumer.predicated or consumer.reads_new or
                len({consumer.insn.line_group}) != 1):
            continue
        rewritten = replace_indexed_memory_base(consumer.insn.text, temporary,
                                                base, index, shift, arch)
        if rewritten is None:
            continue
        proposals.append(Proposal(
            rule='addasl-indexed-memory', status='EMITTABLE',
            packet_span=(packet_no, packet_no + 1),
            message='%s is a dead single-use scaled address on %s' % (temporary, arch),
            before=(packet.facts[0].insn.text, consumer.insn.text),
            after=(rewritten,),
            blockers=()))
    return proposals


def find_local_pure_cse_proposals(packet_facts, blocks, packet_live_out):
    proposals = []
    for block in blocks:
        available = {}
        for packet_no in block.packets:
            packet = packet_facts[packet_no]
            if not packet.scheduling_eligible or len(packet.facts) != 1:
                available.clear()
                continue
            fact = packet.facts[0]
            expression = canonical_pure_expression(fact)
            if expression is None:
                available.clear()
                continue
            destination = next(iter(fact.writes))
            producer = available.get(expression)
            if (producer and producer[0] != destination and
                    destination not in packet_live_out.get(packet_no, set()) and
                    packet_is_removable(packet_no, packet_facts, blocks)):
                proposals.append(Proposal(
                    rule='local-pure-cse', status='EMITTABLE',
                    packet_span=(packet_no, packet_no),
                    message='%s recomputes a pure expression already in %s and is dead after this packet' %
                            (destination, producer[0]),
                    before=(fact.insn.text,), after=(), blockers=()))
            for written in fact.writes:
                for key in list(available):
                    if written == available[key][0] or written in key[1]:
                        del available[key]
            available[expression] = (destination, packet_no, fact.node)
    return proposals


def review_only_proposals(packet_facts, blocks):
    """Recognize hand-written corpus idioms without claiming a rewrite proof.

    Every finding below is deliberately non-emitting: its optimized counterpart
    changes packet design, predicate timing, memory fault/atomicity behavior, or
    needs cross-block value lineage that the local emitter does not yet prove.
    """
    proposals = []
    flattened = [(packet.packet_no, fact) for packet in packet_facts for fact in packet.facts]
    for packet in packet_facts:
        text = ' ; '.join(fact.insn.text for fact in packet.facts)
        if re.search(r'\br\d+\s*=\s*cmp\.', text) and re.search(r'\bcmp\.eq\(r\d+,#0\)', text):
            proposals.append(Proposal('direct-compare-branch', 'ADVISORY',
                                      (packet.packet_no, packet.packet_no),
                                      'boolean materialization/test may fuse into direct compare-and-branch',
                                      blockers=('exact v68-v81 .new compare/branch packet form requires assembler validation',)))
        if re.search(r'\br\d+\s*=\s*sub\(r\d+,r\d+\)', text):
            proposals.append(Proposal('retain-scaled-index', 'ADVISORY',
                                      (packet.packet_no, packet.packet_no),
                                      'pointer-difference recovery may be avoidable if its original index remains live',
                                      blockers=('requires CFG-wide index→scaled-pointer→recovery lineage proof',)))
        if any(fact.category == CATEGORY_LOAD and 'memd(' in fact.insn.text for fact in packet.facts):
            proposals.append(Proposal('key-first-narrow-probe', 'CONTRACT_REQUIRED',
                                      (packet.packet_no, packet.packet_no),
                                      'wide entry load may permit key-first narrow probe',
                                      blockers=('requires stable nonvolatile memory, reload/fault equivalence, and concurrency proof',)))
        if packet.capacity['mem'] == 1 and any('++#' in fact.insn.text for fact in packet.facts):
            proposals.append(Proposal('auto-update-induction', 'CONTRACT_REQUIRED',
                                      (packet.packet_no, packet.packet_no),
                                      'post-update memory induction form is present',
                                      blockers=('requires exact architecture form, loop trip/path proof, pointer liveness, and exception-semantics proof',)))
        if any('memd(' in fact.insn.text or len(fact.writes) == 2 for fact in packet.facts):
            proposals.append(Proposal('pair-memory-op', 'ADVISORY',
                                      (packet.packet_no, packet.packet_no),
                                      'pair/doubleword operation may be a hand-allocation opportunity',
                                      blockers=('pair alignment, ABI lifetime, alignment, and packet-resource proof required',)))

    for index in range(len(flattened) - 2):
        first_packet, first = flattened[index]
        second_packet, second = flattened[index + 1]
        third_packet, third = flattened[index + 2]
        # STLB-style predicate -> scalar boolean -> zero-test branch.
        if (first.writes and any(resource.startswith('p') for resource in first.writes) and
                len(second.writes) == 1 and any(resource.startswith('r') for resource in second.writes) and
                any(resource.startswith('p') for resource in second.reads_old) and
                third.category == CATEGORY_CONTROL and second.writes <= third.reads_new):
            proposals.append(Proposal('predicate-materialization-branch', 'ADVISORY',
                                      (first_packet, third_packet),
                                      'predicate is materialized in a GPR then immediately re-tested for a branch',
                                      blockers=('preserve polarity/target/fallthrough and validate exact predicate-.new branch packet form',)))
        # TLB-fill-style mux-to-overwrite candidate.
        if ('mux(' in second.insn.text and second.writes and
                third.category == CATEGORY_STORE and second.writes & (third.reads_old | third.reads_new)):
            proposals.append(Proposal('select-to-predicated-overwrite', 'ADVISORY',
                                      (second_packet, third_packet),
                                      'select result feeds a store and may permit predicated overwrite of its nonconstant input',
                                      blockers=('requires old-value liveness, predicate lifetime, memory/fault ordering, and packet proof',)))
        # Generic load/bit-update/store is deliberately annotation-only.
        if (first.category == CATEGORY_LOAD and second.category == CATEGORY_ALU and
                third.category == CATEGORY_STORE and first.writes & second.reads_old and
                second.writes & (third.reads_old | third.reads_new)):
            proposals.append(Proposal('load-bitop-store', 'CONTRACT_REQUIRED',
                                      (first_packet, third_packet),
                                      'load/ALU/store window resembles direct memory RMW',
                                      blockers=('no generic RMW rewrite: requires ISA ordering, atomicity, MMIO/volatile, alias, and concurrency proof',)))
    return proposals


def collect_handopt_proposals(packets, packet_facts, func_name, arch):
    blocks = build_cfg(packet_facts, func_name)
    packet_live_in, packet_live_out = compute_liveness(packet_facts, blocks)
    reuse_windows, available_values = analyze_register_value_reuse(
        packet_facts, blocks, packet_live_in, packet_live_out)
    concrete = (find_gp_copy_proposals(packets, packet_facts, blocks, packet_live_out) +
                find_address_fold_proposals(packet_facts, blocks, packet_live_out) +
                find_addasl_indexed_memory_proposals(packet_facts, blocks, packet_live_out, arch) +
                find_local_pure_cse_proposals(packet_facts, blocks, packet_live_out))
    review = review_only_proposals(packet_facts, blocks)
    return blocks, packet_live_in, packet_live_out, reuse_windows, available_values, concrete, review


def call_fingerprint(packets):
    """Ordered direct-call sequence: fixed decomposition must preserve it."""
    calls = []
    for _endloop, insns in packets:
        for insn in insns:
            match = re.search(r'\bcall\s+(?:0x[0-9a-f]+\s+<)?([A-Za-z_][A-Za-z0-9_]*)', insn.text)
            if match:
                calls.append(match.group(1))
    return tuple(calls)


def memory_fingerprint(packets):
    """Order and spelling of memory instructions, except rewritten addresses."""
    return tuple(fact.category for packet in build_packet_facts(packets)
                 for fact in packet.facts if fact.category in
                 (CATEGORY_LOAD, CATEGORY_STORE, CATEGORY_MEM_RMW))


def apply_handopt_proposals(packets, proposals):
    """Apply only EMITTABLE local rules, keeping every remaining packet intact."""
    before_calls = call_fingerprint(packets)
    before_memory = memory_fingerprint(packets)
    rewritten = copy.deepcopy(packets)
    remove_packets, replacements = set(), {}
    for proposal in proposals:
        if proposal.status != 'EMITTABLE':
            continue
        start, end = proposal.packet_span
        if proposal.rule in ('gp-copy', 'address-fold', 'addasl-indexed-memory'):
            remove_packets.add(start)
            replacements[(end, 0)] = proposal.after[0]
        elif proposal.rule == 'local-pure-cse':
            remove_packets.add(start)
    output = []
    for packet_no, packet in enumerate(rewritten):
        if packet_no in remove_packets:
            continue
        endloop, insns = packet
        for instruction_no, insn in enumerate(insns):
            replacement = replacements.get((packet_no, instruction_no))
            if replacement is not None:
                insn.text = replacement
        output.append((endloop, insns))
    if call_fingerprint(output) != before_calls:
        raise RuntimeError('local handopt changed call fingerprint')
    if memory_fingerprint(output) != before_memory:
        raise RuntimeError('local handopt changed memory operation order/count')
    return output


def render_handopt_report(func_name, blocks, packet_live_in, packet_live_out,
                           reuse_windows, available_values, concrete, review):
    lines = [
        'handopt-report: %s' % func_name,
        'scope: fixed objdump function boundaries; calls/inlining/helper expansion excluded',
        'arch: v68-v81 scalar local rules only',
        'register-policy: allocatable=r0-r19 reserved=r20-r27 fixed=r28-r31',
        'cfg-blocks:',
    ]
    for block in blocks:
        lines.append('  B%d packets=%s predecessors=%s successors=%s live-in=%s live-out=%s' %
                     (block.index, ','.join('P%03d' % packet for packet in block.packets),
                      ','.join('B%d' % predecessor for predecessor in sorted(block.predecessors)) or '-',
                      ','.join('B%d' % successor for successor in sorted(block.successors)) or '-',
                      format_resources(block.live_in), format_resources(block.live_out)))
    lines.append('register-reuse-windows:')
    if reuse_windows:
        for proposal in reuse_windows:
            lines.append('  P%03d %s [%s]' % (proposal.packet_span[0], proposal.message, proposal.status))
    else:
        lines.append('  none')
    lines.append('available-computed-values:')
    if available_values:
        for proposal in available_values:
            lines.append('  P%03d..P%03d %s [%s]' %
                         (proposal.packet_span[0], proposal.packet_span[1], proposal.message, proposal.status))
    else:
        lines.append('  none')
    lines.append('concrete-local-rewrites:')
    if concrete:
        for proposal in concrete:
            lines.append('  %s P%03d..P%03d [%s]: %s' %
                         (proposal.rule, proposal.packet_span[0], proposal.packet_span[1],
                          proposal.status, proposal.message))
            lines.extend('    before: ' + text for text in proposal.before)
            lines.extend('    after:  ' + text for text in proposal.after)
    else:
        lines.append('  none')
    lines.append('review-only-patterns:')
    if review:
        for proposal in review:
            lines.append('  %s P%03d [%s]: %s' %
                         (proposal.rule, proposal.packet_span[0], proposal.status, proposal.message))
            lines.extend('    requires: ' + blocker for blocker in proposal.blockers)
    else:
        lines.append('  none')
    return '\n'.join(lines)


def render_schedule_report(func_name, packet_facts, edges, diagnostics):
    lines = [
        'schedule-report: %s' % func_name,
        'register-policy: caller-saved=r0-r15 usable-callee-saved=r16-r19',
        'register-policy: reserved=r20-r27 fixed=r28(H2K_GP),r29(SP),r30(FP),r31(LR)',
        'resource-model: abstract only; slots<=4 mem<=2 control<=1 alu-results<=3; assembler-validation-required',
        '',
    ]
    for packet in packet_facts:
        counts = packet.capacity
        status = 'eligible' if packet.scheduling_eligible else 'fixed'
        if packet.opaque_reasons:
            status += ' (%s)' % ','.join(packet.opaque_reasons)
        capacity = 'abstract' if packet.scheduling_eligible else 'unverified'
        lines.append('packet P%03d @0x%x status=%s capacity=%s' %
                     (packet.packet_no, packet.facts[0].insn.addr, status, capacity))
        lines.append('  resources: logical=%d slots<=4 mem=%d/2 control=%d/1 alu-results=%d/3 solo=%d' %
                     (counts['logical'], counts['mem'], counts['control'], counts['alu_results'], counts['solo']))
        for fact in packet.facts:
            detail = '%s %s [%s] old=%s new=%s writes=%s' % (
                fact.node, fact.category, fact.provenance,
                format_resources(fact.reads_old), format_resources(fact.reads_new),
                format_resources(fact.writes))
            if fact.opaque_reason:
                detail += ' opaque=%s' % fact.opaque_reason
            lines.append('  %s: %s' % (detail, fact.insn.text))
    lines.append('')
    lines.append('dependencies:')
    if edges:
        for edge in sorted(edges, key=lambda edge: (edge.consumer, edge.producer, edge.resource, edge.kind)):
            lines.append('  %s -> %s resource=%s kind=%s' %
                         (edge.producer, edge.consumer, edge.resource, edge.kind))
    else:
        lines.append('  none')
    candidates = candidate_packets(packet_facts, edges)
    lines.append('candidate-packets:')
    if candidates:
        for index, candidate in enumerate(candidates):
            lines.append('  C%d: %s [abstract; assembler-validation-required]' %
                         (index, ', '.join(fact.node for fact in candidate)))
    else:
        lines.append('  none')
    lines.append('diagnostics:')
    if diagnostics:
        lines.extend('  ' + diagnostic for diagnostic in diagnostics)
    else:
        lines.append('  none')
    return '\n'.join(lines)


# ---------------------------------------------------------------------------
# Emission: rewrite branch labels, substitute offsets, and write standalone asm.
# ---------------------------------------------------------------------------

def rewrite_targets(text, func_name, func_start_addr, target_label):
    def replace(match):
        _addr_hex, symbol = match.groups()
        name, _, offset = symbol.rpartition('+')
        if not name:
            name, offset = symbol, '0x0'
        if name == func_name:
            return target_label(func_start_addr + int(offset, 16))
        return name
    return TARGET_RE.sub(replace, text)


def resolve_bare_loop_target(text, addresses, label_of):
    def replace(match):
        address = int(match.group(2), 16)
        return match.group(1) + label_of(address) if address in addresses else match.group(0)
    return re.sub(r'(loop[01]\()(0x[0-9a-f]+)', replace, text)


def load_offsets(path):
    by_value, ambiguous = {}, set()
    define_re = re.compile(r'^#define\s+(\w+)\s+(-?\d+)\s*$')
    with open(path) as source:
        for line in source:
            match = define_re.match(line.strip())
            if not match:
                continue
            name, value = match.group(1), int(match.group(2))
            if value in by_value and by_value[value] != name:
                ambiguous.add(value)
            by_value[value] = name
    for value in ambiguous:
        del by_value[value]
    return by_value


GP_OFFSET_RE = re.compile(r'\br28\+#(0x[0-9a-f]+|\d+)\b|\badd\(r28,#(0x[0-9a-f]+|\d+)\)')


def substitute_gp_offsets(text, offsets_by_value):
    if not offsets_by_value:
        return text

    def replace(match):
        raw = match.group(1) or match.group(2)
        value = int(raw, 16) if raw.startswith('0x') else int(raw)
        name = offsets_by_value.get(value)
        if name is None:
            return match.group(0) + ' /* offset? */'
        return match.group(0).replace('#' + raw, '#' + name)
    return GP_OFFSET_RE.sub(replace, text)


def assemble_candidate(assembly, arch, repo_root, offsets_path=None):
    """Ask the existing project compiler whether a packet candidate assembles.

    This validates packet syntax/resource legality only. The Python scheduler is
    still responsible for dependency and memory/control preservation. No tools
    are installed or configured here; unavailable toolchains produce a clear
    non-success result for the caller to report.
    """
    compiler = shutil.which('hexagon-clang')
    if compiler is None:
        return False, 'hexagon-clang not found on PATH'
    include_dir = (Path(offsets_path).parent if offsets_path else
                   repo_root / 'artifacts' / arch / 'ref' / 'build' / 'kernel' / 'include')
    if not include_dir.is_dir():
        return False, 'kernel include directory not found: %s' % include_dir
    arch_number = arch.removeprefix('v')
    preamble = '#include <asm_std.h>\n#include <asm_offsets.h>\n#include <max.h>\n'
    with tempfile.TemporaryDirectory(prefix='objdump-to-asm-') as directory:
        source = Path(directory) / 'candidate.S'
        output = Path(directory) / 'candidate.o'
        source.write_text(preamble + assembly + '\n')
        command = [compiler, '-mv' + arch_number, '-DARCHV=' + arch_number,
                   '-I' + str(include_dir),
                   '-I' + str(repo_root / 'libs' / 'h2' / 'common'),
                   '-c', str(source), '-o', str(output)]
        result = subprocess.run(command, text=True, capture_output=True)
    if result.returncode == 0:
        return True, 'assembled with %s' % compiler
    diagnostic = (result.stderr or result.stdout).strip()
    return False, diagnostic or 'hexagon-clang failed without diagnostic'


def emit_assembly(func_name, packets, section, align, offsets_by_value,
                  function_start=None):
    if function_start is None:
        function_start = packets[0][1][0].addr
    func_start_addr = function_start
    instruction_addresses = sorted({insn.addr for _endloop, insns in packets for insn in insns})
    addresses = set(instruction_addresses)

    def label_of(address):
        return '.L%x' % (address - func_start_addr)

    # Objdump prints an immext as its own word, but this extractor deliberately
    # emits only the consuming instruction. A branch may target that extension
    # word, so bind its original .L offset as an alias on the next emitted
    # packet. Never redirect a target backward or invent a new instruction.
    def emitted_address_for(address):
        for instruction_address in instruction_addresses:
            if instruction_address >= address:
                return instruction_address
        return None

    label_aliases = {}
    referenced = set()
    for _endloop, insns in packets:
        for insn in insns:
            for match in TARGET_RE.finditer(insn.text):
                symbol = match.group(2)
                name, separator, offset = symbol.rpartition('+')
                if not separator:
                    name, offset = symbol, '0x0'
                if name == func_name:
                    target = func_start_addr + int(offset, 16)
                    emitted = emitted_address_for(target)
                    if emitted is not None:
                        referenced.add(emitted)
                        label_aliases.setdefault(emitted, []).append(label_of(target))
            for match in re.finditer(r'loop[01]\((0x[0-9a-f]+)', insn.text):
                address = int(match.group(1), 16)
                if address in addresses:
                    referenced.add(address)
                    label_aliases.setdefault(address, []).append(label_of(address))

    def target_label(address):
        return label_of(address)

    output = ['FUNC_START %s %s .p2align %s' % (func_name, section, align)]
    for endloop, insns in packets:
        packet_address = insns[0].addr
        if packet_address in referenced:
            for label in dict.fromkeys(label_aliases.get(packet_address, [label_of(packet_address)])):
                output.append('%s:' % label)
        output.append('\t{')
        for insn in insns:
            text = rewrite_targets(insn.text, func_name, func_start_addr, target_label)
            text = resolve_bare_loop_target(text, addresses, target_label)
            text = substitute_gp_offsets(text, offsets_by_value)
            text = re.sub(r'\br28\b', 'H2K_GP', text)
            output.append('\t\t%s' % text)
        closer = '\t}'
        if endloop:
            closer += ' :%s' % endloop.lstrip(':')
        output.append(closer)
    output.append('FUNC_END %s' % func_name)
    return '\n'.join(output)



# ---------------------------------------------------------------------------
# Pipeline orchestration: run analysis, rewrites, oracle checks, and emission.
# ---------------------------------------------------------------------------

def process_function(objdump_lines, function_name, args, offsets, arch):
    function_start, function_lines = extract_function(objdump_lines, function_name)
    packets = parse_packets(function_lines)
    if not packets or not packets[0][1]:
        sys.exit('error: no instructions parsed for %r -- check the function name' % function_name)

    for packet_no, (_endloop, insns) in enumerate(packets):
        check_packet_legality(function_name, packet_no, insns)
    check_call_clobber_liveness(function_name, packets)

    packet_facts = None
    if args.schedule_report or args.handopt_report or args.handopt:
        packet_facts = build_packet_facts(packets)

    if args.schedule_report:
        edges, diagnostics = build_dependencies(packet_facts)
        print(render_schedule_report(function_name, packet_facts, edges, diagnostics), file=sys.stderr)

    if args.handopt_report or args.handopt:
        (blocks, packet_live_in, packet_live_out, reuse_windows, available_values,
         concrete, review) = collect_handopt_proposals(packets, packet_facts, function_name, args.arch)
        print(render_handopt_report(function_name, blocks, packet_live_in,
                                    packet_live_out, reuse_windows, available_values,
                                    concrete, review), file=sys.stderr)
        if args.handopt:
            packets = apply_handopt_proposals(packets, concrete)
            # Rules only remove complete local packets or alter a supported
            # scalar memory operand. Rebuild and validate before emission.
            packet_facts = build_packet_facts(packets)
            for packet_no, (_endloop, insns) in enumerate(packets):
                check_packet_legality(function_name, packet_no, insns)

    offsets = load_offsets(args.offsets) if args.offsets else {}
    if args.squeeze:
        original_packets = copy.deepcopy(packets)
        squeezed_packets, moves = squeeze_packets(packets, function_name, function_start)
        if args.squeeze_report:
            print('squeeze-report: %s' % function_name, file=sys.stderr)
            if moves:
                for source, target, kind, reason in moves:
                    print('  P%03d -> P%03d %s: %s' %
                          (source, target, kind, reason), file=sys.stderr)
            else:
                print('  no phase-one moves accepted', file=sys.stderr)
        if args.assembler_oracle:
            repo_root = Path(__file__).resolve().parent.parent
            candidate = emit_assembly(function_name, squeezed_packets, args.section,
                                      args.align, offsets, function_start)
            accepted, diagnostic = assemble_candidate(candidate, args.arch,
                                                       repo_root, args.offsets)
            print('assembler-oracle: %s' % diagnostic, file=sys.stderr)
            if not accepted:
                if args.squeeze_report:
                    print('  rollback: assembler rejected squeezed candidate', file=sys.stderr)
                squeezed_packets = original_packets
        packets = squeezed_packets

    if args.return_coalesce_opt:
        proposals = find_generic_return_coalescing_proposals(
            packets, function_name, function_start)
        print('return-coalesce-report: %s' % function_name, file=sys.stderr)
        if not proposals:
            print('  no straight-line pure result-copy candidates', file=sys.stderr)
        for proposal in proposals:
            print('  P%03d..P%03d ORACLE_REQUIRED: %s' %
                  (proposal.packet_span[0], proposal.packet_span[1], proposal.message),
                  file=sys.stderr)
        if proposals and not args.assembler_oracle:
            print('  not applied: requires --assembler-oracle', file=sys.stderr)
        elif proposals:
            original_packets = copy.deepcopy(packets)
            candidate_packets = apply_generic_return_coalescing_proposals(
                packets, proposals)
            candidate = emit_assembly(function_name, candidate_packets, args.section,
                                      args.align, offsets, function_start)
            accepted, diagnostic = assemble_candidate(candidate, args.arch,
                                                       Path(__file__).resolve().parent.parent,
                                                       args.offsets)
            print('return-coalesce-oracle: %s' % diagnostic, file=sys.stderr)
            if accepted:
                packets = candidate_packets
            else:
                print('  rollback: assembler rejected generic return coalescing candidate',
                      file=sys.stderr)
                packets = original_packets

    if args.return_load_opt:
        return_loads = find_return_load_proposals(packets, function_name, function_start)
        print('return-load-report: %s' % function_name, file=sys.stderr)
        if not return_loads:
            print('  no strict tmp-load → r0 → dealloc_return candidates', file=sys.stderr)
        for proposal in return_loads:
            print('  P%03d..P%03d ORACLE_REQUIRED: %s' %
                  (proposal.packet_span[0], proposal.packet_span[1], proposal.message),
                  file=sys.stderr)
        if return_loads and not args.assembler_oracle:
            print('  not applied: requires --assembler-oracle', file=sys.stderr)
        elif return_loads:
            original_packets = copy.deepcopy(packets)
            candidate_packets = apply_return_load_proposals(packets, return_loads)
            candidate = emit_assembly(function_name, candidate_packets, args.section,
                                      args.align, offsets, function_start)
            accepted, diagnostic = assemble_candidate(candidate, args.arch,
                                                       Path(__file__).resolve().parent.parent,
                                                       args.offsets)
            print('return-load-oracle: %s' % diagnostic, file=sys.stderr)
            if accepted:
                packets = candidate_packets
            else:
                print('  rollback: assembler rejected return-load candidate', file=sys.stderr)
                packets = original_packets

    if args.guarded_fallthrough_opt:
        proposals = find_guarded_fallthrough_predication_proposals(
            packets, function_name, function_start)
        print('guarded-fallthrough-report: %s' % function_name, file=sys.stderr)
        if not proposals:
            print('  no exact p1 fallthrough-predication candidates', file=sys.stderr)
        for proposal in proposals:
            print('  P%03d..P%03d ORACLE_REQUIRED: %s' %
                  (proposal.packet_span[0], proposal.packet_span[1], proposal.message),
                  file=sys.stderr)
        if proposals and not args.assembler_oracle:
            print('  not applied: requires --assembler-oracle', file=sys.stderr)
        elif proposals:
            original_packets = copy.deepcopy(packets)
            candidate_packets = apply_guarded_fallthrough_predication_proposals(
                packets, proposals)
            candidate = emit_assembly(function_name, candidate_packets, args.section,
                                      args.align, offsets, function_start)
            accepted, diagnostic = assemble_candidate(candidate, args.arch,
                                                       Path(__file__).resolve().parent.parent,
                                                       args.offsets)
            print('guarded-fallthrough-oracle: %s' % diagnostic, file=sys.stderr)
            if accepted:
                packets = candidate_packets
            else:
                print('  rollback: assembler rejected guarded fallthrough candidate',
                      file=sys.stderr)
                packets = original_packets

    if args.guarded_return_coalesce_opt:
        proposals = find_guarded_return_value_coalescing_proposals(
            packets, function_name, function_start)
        print('guarded-return-coalesce-report: %s' % function_name, file=sys.stderr)
        if not proposals:
            print('  no exact predicated r4 → r0 coalescing candidates', file=sys.stderr)
        for proposal in proposals:
            print('  P%03d..P%03d ORACLE_REQUIRED: %s' %
                  (proposal.packet_span[0], proposal.packet_span[1], proposal.message),
                  file=sys.stderr)
        if proposals and not args.assembler_oracle:
            print('  not applied: requires --assembler-oracle', file=sys.stderr)
        elif proposals:
            original_packets = copy.deepcopy(packets)
            candidate_packets = apply_guarded_return_value_coalescing_proposals(
                packets, proposals)
            candidate = emit_assembly(function_name, candidate_packets, args.section,
                                      args.align, offsets, function_start)
            accepted, diagnostic = assemble_candidate(candidate, args.arch,
                                                       Path(__file__).resolve().parent.parent,
                                                       args.offsets)
            print('guarded-return-coalesce-oracle: %s' % diagnostic, file=sys.stderr)
            if accepted:
                packets = candidate_packets
            else:
                print('  rollback: assembler rejected guarded return coalescing candidate',
                      file=sys.stderr)
                packets = original_packets

    return emit_assembly(function_name, packets, args.section, args.align, offsets,
                         function_start)

# ---------------------------------------------------------------------------
# Embedded regression scenarios.
# Add a rule test here: build a tiny `packets(...)` fixture, call the rule,
# assert the rewritten packets and its rejection cases, then run
# `--check-script-regression`.
# ---------------------------------------------------------------------------

def packets(*packet_texts):
    parsed = []
    address = 0x100
    for packet_no, texts in enumerate(packet_texts):
        insns = []
        for index, text in enumerate(texts):
            insns.append(Insn(address, text, packet_no * 16 + index, index, packet_no))
        parsed.append((None, insns))
        address += 4
    return parsed


class ObjdumpToAsmRegressionTests(unittest.TestCase):
    def test_register_policy_reserves_kernel_context_scratch(self):
        self.assertEqual(USABLE_CALLEE_SAVED, frozenset(range(16, 20)))
        self.assertEqual(RESERVED_PROGRAM_REGS, frozenset(range(20, 28)))
        self.assertEqual(FIXED_REGS, {28: 'H2K_GP', 29: 'SP', 30: 'FP', 31: 'LR'})

    def test_architecture_derives_from_offsets_or_objdump_path(self):
        self.assertEqual(derive_architecture('artifacts/v68/ref/include/asm_offsets.h'), 'v68')
        self.assertEqual(derive_architecture(None, 'artifacts/v73/ref/build/test.objdump'), 'v73')
        self.assertEqual(derive_architecture('/tmp/asm_offsets.h', '/tmp/test.objdump'), 'v81')

    def test_generic_return_value_coalescing_removes_dead_copy(self):
        parsed = packets(
            ('r5 = add(r3,#4)',),
            ('r0 = r5', 'dealloc_return'),
        )
        proposals = find_generic_return_coalescing_proposals(parsed, 'example', 0x100)
        self.assertEqual(len(proposals), 1)
        rewritten = apply_generic_return_coalescing_proposals(parsed, proposals)
        self.assertEqual([tuple(insn.text for insn in insns) for _endloop, insns in rewritten],
                         [('r0 = add(r3,#4)',), ('dealloc_return',)])

    def test_generic_return_value_coalescing_rejects_intervening_r0_use(self):
        parsed = packets(
            ('r5 = add(r3,#4)',),
            ('r6 = add(r0,#1)',),
            ('r0 = r5', 'dealloc_return'),
        )
        self.assertEqual(find_generic_return_coalescing_proposals(parsed, 'example', 0x100), [])

    def test_new_predicate_edge_is_same_packet_and_distinct(self):
        packet_facts = build_packet_facts(packets((
            'p0 = cmp.eq(r1,#0)',
            'if (p0.new) jump:t 0x108 <target>',
        )))
        edges, diagnostics = build_dependencies(packet_facts)
        self.assertEqual(diagnostics, [])
        self.assertEqual(
            edges,
            [DependencyEdge('P000.I0', 'P000.I1', 'p0', 'NEW_PRED')],
        )

    def test_new_register_store_edge_is_same_packet_and_distinct(self):
        packet_facts = build_packet_facts(packets((
            'r4 = add(r3,#4)',
            'memw(r2+#4) = r4.new',
        )))
        edges, diagnostics = build_dependencies(packet_facts)
        self.assertEqual(diagnostics, [])
        self.assertIn(DependencyEdge('P000.I0', 'P000.I1', 'r4', 'NEW_GPR'), edges)
        self.assertFalse(packet_facts[0].scheduling_eligible)

    def test_missing_new_producer_is_diagnostic_not_self_edge(self):
        packet_facts = build_packet_facts(packets((
            'if (p0.new) jump:t 0x108 <target>',
        )))
        edges, diagnostics = build_dependencies(packet_facts)
        self.assertEqual(edges, [])
        self.assertEqual(diagnostics, ['P000.I0: p0.new has no distinct same-packet producer'])

    def test_ordinary_reads_depend_on_prior_packet_not_same_packet_writer(self):
        packet_facts = build_packet_facts(packets(
            ('r4 = #1',),
            ('r5 = add(r4,#1)',),
        ))
        edges, diagnostics = build_dependencies(packet_facts)
        self.assertEqual(diagnostics, [])
        self.assertIn(DependencyEdge('P000.I0', 'P001.I0', 'r4', 'RAW'), edges)

    def test_same_packet_old_read_update_is_not_new_value_forwarding(self):
        facts = build_packet_facts(packets((
            'r5 = add(r4,#8)',
            'r2 = memw(r5+#4)',
        )))
        edges, diagnostics = build_dependencies(facts)
        self.assertEqual(diagnostics, [])
        self.assertIn(DependencyEdge('P000.I0', 'P000.I1', 'r5', 'OLD_READ_UPDATE'), edges)
        self.assertNotIn(DependencyEdge('P000.I0', 'P000.I1', 'r5', 'NEW_GPR'), edges)
        blocks = build_cfg(facts, 'example')
        _live_in, live_out = compute_liveness(facts, blocks)
        self.assertEqual(find_address_fold_proposals(facts, blocks, live_out), [])

    def test_memory_and_control_packets_are_classified_but_fixed(self):
        packet_facts = build_packet_facts(packets(
            ('r4 = memw(r3+#0)',),
            ('jump 0x108 <target>',),
            ('tlblock',),
        ))
        self.assertEqual(packet_facts[0].facts[0].category, CATEGORY_LOAD)
        self.assertEqual(packet_facts[1].facts[0].category, CATEGORY_CONTROL)
        self.assertEqual(packet_facts[2].facts[0].category, CATEGORY_SYSTEM)
        self.assertTrue(all(not packet.scheduling_eligible for packet in packet_facts))

    def test_squeeze_moves_independent_scalar_into_earlier_packet(self):
        parsed = packets(
            ('r1 = #1',),
            ('r2 = #2',),
            ('r3 = add(r1,r2)',),
        )
        squeezed, moves = squeeze_packets(parsed, 'example')
        self.assertEqual(len(moves), 1)
        self.assertEqual(moves[0][2], 'INDEPENDENT_SCALAR_FILL')
        self.assertEqual([tuple(insn.text for insn in insns) for _endloop, insns in squeezed],
                         [('r1 = #1', 'r2 = #2'), ('r3 = add(r1,r2)',)])

    def test_squeeze_old_base_memory_pipeline_preserves_old_address(self):
        parsed = packets(
            ('r2 = memw(r5+#4)',),
            ('r5 = add(r5,#8)',),
            ('r3 = add(r5,#1)',),
        )
        squeezed, moves = squeeze_packets(parsed, 'example')
        self.assertEqual(len(moves), 1)
        self.assertEqual(moves[0][2], 'OLD_READ_UPDATE')
        self.assertEqual([tuple(insn.text for insn in insns) for _endloop, insns in squeezed],
                         [('r2 = memw(r5+#4)', 'r5 = add(r5,#8)'), ('r3 = add(r5,#1)',)])
        self.assertIn('r2 = memw(r5+#4)', squeezed[0][1][0].text)

    def test_squeeze_iterates_to_guarded_pipeline_cascade(self):
        parsed = packets(
            ('r6 = extractu(r5,#7,#25)', 'r5 = memw(r4+#0)'),
            ('r8 = setbit(r6,#0)', 'r4 = add(r28,#0x1800)'),
            ('r7 = addasl(r4,r6,#3)',),
            ('r9 = memw(r7+#4)',),
        )
        squeezed, moves = squeeze_packets(parsed, 'example')
        self.assertEqual([move[2] for move in moves],
                         ['OLD_READ_UPDATE', 'INDEPENDENT_SCALAR_FILL'])
        self.assertEqual([tuple(insn.text for insn in insns) for _endloop, insns in squeezed], [
            ('r6 = extractu(r5,#7,#25)', 'r5 = memw(r4+#0)', 'r4 = add(r28,#0x1800)'),
            ('r8 = setbit(r6,#0)', 'r7 = addasl(r4,r6,#3)'),
            ('r9 = memw(r7+#4)',),
        ])

    def test_squeeze_refuses_scalar_that_needs_target_new_value(self):
        parsed = packets(
            ('r5 = add(r4,#8)',),
            ('r2 = memw(r5+#4)',),
        )
        squeezed, moves = squeeze_packets(parsed, 'example')
        self.assertEqual(moves, [])
        self.assertEqual([tuple(insn.text for insn in insns) for _endloop, insns in squeezed],
                         [('r5 = add(r4,#8)',), ('r2 = memw(r5+#4)',)])

    def test_squeeze_eliminates_constant_never_taken_fallthrough_branch(self):
        parsed = packets(
            ('r9 = #0',),
            ('p0 = cmp.eq(r9,#0)', 'if (!p0.new) jump:t 0x10c <example+0xc>'),
            ('r0 = #-1', 'dealloc_return'),
        )
        squeezed, moves = squeeze_packets(parsed, 'example', 0x100)
        self.assertIn('CONSTANT_FALLTHROUGH', [move[2] for move in moves])
        self.assertEqual([tuple(insn.text for insn in insns) for _endloop, insns in squeezed],
                         [('r0 = #-1', 'dealloc_return')])

    def test_squeeze_eliminates_fused_constant_never_taken_branch(self):
        parsed = packets(
            ('r2 = #0', 'if (!cmp.eq(r2.new,#0)) jump:t 0x108 <example+0x8>'),
            ('r0 = #-1', 'dealloc_return'),
        )
        squeezed, moves = squeeze_packets(parsed, 'example', 0x100)
        self.assertIn('CONSTANT_FALLTHROUGH', [move[2] for move in moves])
        self.assertEqual([tuple(insn.text for insn in insns) for _endloop, insns in squeezed],
                         [('r0 = #-1', 'dealloc_return')])

    def test_squeeze_keeps_nonconstant_or_taken_branch(self):
        mismatch = packets(
            ('r9 = #1',),
            ('p0 = cmp.eq(r9,#0)', 'if (!p0.new) jump:t 0x10c <example+0xc>'),
            ('r0 = #-1', 'dealloc_return'),
        )
        squeezed, moves = squeeze_packets(mismatch, 'example', 0x100)
        self.assertNotIn('CONSTANT_FALLTHROUGH', [move[2] for move in moves])
        self.assertEqual(len(squeezed), 3)

    def test_guarded_fallthrough_predication_exact_shape(self):
        parsed = packets(
            ('r9 = addasl(r4,r9,#3)',),
            ('p0 = cmp.eq(r9,#0)', 'if (p0.new) jump:nt 0x120 <example+0x20>'),
            ('r4 = sub(r9,r4)', 'p0 = cmp.eq(r2,#0)', 'r3 = memw(r9+#4)'),
            ('r4 = asr(r4,#3)', 'r2 = add(r3,#1)'),
            ('r3 = insert(r2,#12,#0)', 'if (!p0) jump:nt 0x140 <example+0x40>',
             'memw(r9+#4) = r3.new'),
        )
        proposals = find_guarded_fallthrough_predication_proposals(parsed, 'example', 0x100)
        self.assertEqual(len(proposals), 1)
        rewritten = apply_guarded_fallthrough_predication_proposals(parsed, proposals)
        self.assertEqual([tuple(insn.text for insn in insns) for _endloop, insns in rewritten], [
            ('r9 = addasl(r4,r9,#3)', 'p1 = cmp.eq(r2,#0)'),
            ('p0 = cmp.eq(r9,#0)', 'if (p0.new) jump:nt 0x120 <example+0x20>',
             'if (!p0.new) r4 = sub(r9,r4)', 'if (!p0.new) r3 = memw(r9+#4)'),
            ('r4 = asr(r4,#3)', 'r2 = add(r3,#1)'),
            ('r3 = insert(r2,#12,#0)', 'if (!p1) jump:nt 0x140 <example+0x40>',
             'memw(r9+#4) = r3.new'),
        ])

    def test_guarded_fallthrough_predication_allocates_free_predicate(self):
        parsed = packets(
            ('p1 = cmp.eq(r0,#0)',),
            ('r9 = addasl(r4,r9,#3)',),
            ('p0 = cmp.eq(r9,#0)', 'if (p0.new) jump:nt 0x124 <example+0x24>'),
            ('r4 = sub(r9,r4)', 'p0 = cmp.eq(r2,#0)', 'r3 = memw(r9+#4)'),
            ('r4 = asr(r4,#3)', 'r2 = add(r3,#1)'),
            ('r3 = insert(r2,#0xc,#0)', 'if (!p0) jump:nt 0x144 <example+0x44>',
             'memw(r9+#4) = r3.new'),
        )
        proposals = find_guarded_fallthrough_predication_proposals(parsed, 'example', 0x100)
        self.assertEqual(len(proposals), 1)
        self.assertIn(('replacement_pred', 'p2'), proposals[0].metadata)

    def test_guarded_return_value_coalescing_exact_shape(self):
        parsed = packets(
            ('r9 = addasl(r4,r9,#3)', 'p1 = cmp.eq(r2,#0x0)'),
            ('p0 = cmp.eq(r9,#0x0)', 'if (p0.new) jump:nt 0x120 <example+0x20>',
             'if (!p0.new) r4 = sub(r9,r4)', 'if (!p0.new) r3 = memw(r9+#0x4)'),
            ('r4 = asr(r4,#0x3)', 'r2 = add(r3,#0x1)'),
            ('r3 = insert(r2,#0xc,#0x0)', 'if (!p1) jump:nt 0x140 <example+0x40>',
             'memw(r9+#0x4) = r3.new'),
            ('r0 = r4', 'dealloc_return'),
        )
        proposals = find_guarded_return_value_coalescing_proposals(parsed, 'example', 0x100)
        self.assertEqual(len(proposals), 1)
        rewritten = apply_guarded_return_value_coalescing_proposals(parsed, proposals)
        self.assertEqual([tuple(insn.text for insn in insns) for _endloop, insns in rewritten], [
            ('r9 = addasl(r4,r9,#3)', 'p1 = cmp.eq(r2,#0x0)'),
            ('p0 = cmp.eq(r9,#0x0)', 'if (p0.new) jump:nt 0x120 <example+0x20>',
             'if (!p0.new) r0 = sub(r9,r4)', 'if (!p0.new) r3 = memw(r9+#0x4)'),
            ('r0 = asr(r0,#0x3)', 'r2 = add(r3,#0x1)'),
            ('r3 = insert(r2,#0xc,#0x0)', 'if (!p1) jump:nt 0x140 <example+0x40>',
             'memw(r9+#0x4) = r3.new'),
            ('dealloc_return',),
        ])

    def test_return_load_propagation_detects_strict_epilogue(self):
        parsed = packets(
            ('r4 = memw(r29+#4)',),
            ('r0 = r4', 'dealloc_return'),
        )
        proposals = find_return_load_proposals(parsed, 'example', 0x100)
        self.assertEqual(len(proposals), 1)
        self.assertEqual(proposals[0].after, ('r0 = memw(r29+#4)', 'dealloc_return'))
        rewritten = apply_return_load_proposals(parsed, proposals)
        self.assertEqual([tuple(insn.text for insn in insns) for _endloop, insns in rewritten],
                         [('r0 = memw(r29+#4)', 'dealloc_return')])

    def test_return_load_propagation_rejects_multiple_uses_and_nonreturn_copy(self):
        multiple_use = packets(
            ('r4 = memw(r29+#4)',),
            ('r3 = add(r4,#1)',),
            ('r0 = r4', 'dealloc_return'),
        )
        self.assertEqual(find_return_load_proposals(multiple_use, 'example', 0x100), [])

        nonreturn_copy = packets(
            ('r4 = memw(r29+#4)',),
            ('r1 = r4', 'dealloc_return'),
        )
        self.assertEqual(find_return_load_proposals(nonreturn_copy, 'example', 0x100), [])

    def test_squeeze_removes_only_non_loop_packet_filler_nops(self):
        parsed = packets(
            ('r1 = #1', 'nop'),
            ('jump 0x108 <example+0x8>',),
        )
        squeezed, moves = squeeze_packets(parsed, 'example')
        self.assertIn((-1, -1, 'REMOVE_NOP', '1 non-loop packet filler nop(s)'), moves)
        self.assertEqual(tuple(insn.text for insn in squeezed[0][1]), ('r1 = #1',))

        loop_tail = packets(('r1 = #1', 'nop'))
        loop_tail[0] = (':endloop0', loop_tail[0][1])
        squeezed, moves = squeeze_packets(loop_tail, 'example')
        self.assertEqual(tuple(insn.text for insn in squeezed[0][1]), ('r1 = #1', 'nop'))
        self.assertNotIn('REMOVE_NOP', [move[2] for move in moves])

    def test_squeeze_keeps_new_and_control_packets_fixed(self):
        new_packet = packets(
            ('r1 = #1',),
            ('r2 = add(r1,#1)', 'memw(r3+#0) = r2.new'),
        )
        squeezed, moves = squeeze_packets(new_packet, 'example')
        self.assertEqual(moves, [])
        self.assertEqual(len(squeezed), 2)

        control_packet = packets(
            ('r1 = #1',),
            ('jump 0x108 <example+0x8>',),
            ('r2 = #2',),
        )
        squeezed, moves = squeeze_packets(control_packet, 'example')
        self.assertEqual(moves, [])
        self.assertEqual(len(squeezed), 3)

    def test_squeeze_preserves_call_and_memory_fingerprints(self):
        parsed = packets(
            ('r2 = memw(r5+#4)',),
            ('r5 = add(r5,#8)',),
            ('call H2K_helper',),
            ('r3 = add(r5,#1)',),
        )
        before_calls = call_fingerprint(parsed)
        before_memory = memory_fingerprint(parsed)
        squeezed, _moves = squeeze_packets(parsed, 'example')
        self.assertEqual(call_fingerprint(squeezed), before_calls)
        self.assertEqual(memory_fingerprint(squeezed), before_memory)

    def test_independent_scalar_instructions_form_abstract_candidate(self):
        packet_facts = build_packet_facts(packets(
            ('r1 = #1',),
            ('r3 = #2',),
        ))
        edges, diagnostics = build_dependencies(packet_facts)
        candidates = candidate_packets(packet_facts, edges)
        self.assertEqual(diagnostics, [])
        self.assertEqual([[fact.node for fact in candidate] for candidate in candidates], [['P000.I0', 'P001.I0']])

    def test_memory_store_address_is_a_read_and_post_update_is_a_write(self):
        fact = parse_instruction(Insn(0x100, 'memw(r4++#4) = r2', 0x100, 0, 0))
        self.assertIn('r4', fact.reads_old)
        self.assertIn('r4', fact.writes)
        self.assertIn('r2', fact.reads_old)

    def test_cfg_liveness_handles_branch_join_without_textual_last_use(self):
        parsed = packets(
            ('r2 = #1',),
            ('if (p0) jump:t 0x10c <example+0xc>',),
            ('r3 = add(r2,#1)',),
            ('r4 = add(r2,#2)',),
        )
        facts = build_packet_facts(parsed)
        blocks = build_cfg(facts, 'example')
        live_in, live_out = compute_liveness(facts, blocks)
        self.assertIn('r2', live_out[0])
        self.assertIn('r2', live_in[3])

    def test_gp_copy_rewrite_is_local_and_does_not_cross_boundaries(self):
        parsed = packets(
            ('r1 = #0',),
            ('r5 = r28',),
            ('r2 = memw(r5+#4)',),
        )
        facts = build_packet_facts(parsed)
        blocks = build_cfg(facts, 'example')
        _live_in, live_out = compute_liveness(facts, blocks)
        proposals = find_gp_copy_proposals(parsed, facts, blocks, live_out)
        self.assertEqual(len(proposals), 1)
        self.assertEqual(proposals[0].after, ('r2 = memw(r28+#0x4)',))
        rewritten = apply_handopt_proposals(parsed, proposals)
        self.assertEqual([insn.text for _endloop, insns in rewritten for insn in insns],
                         ['r1 = #0', 'r2 = memw(r28+#0x4)'])

    def test_gp_copy_refuses_branch_target_or_non_address_use(self):
        parsed = packets(
            ('r5 = r28',),
            ('r2 = add(r5,#1)',),
        )
        facts = build_packet_facts(parsed)
        blocks = build_cfg(facts, 'example')
        _live_in, live_out = compute_liveness(facts, blocks)
        self.assertEqual(find_gp_copy_proposals(parsed, facts, blocks, live_out), [])

    def test_address_fold_rewrite_and_immediate_range_rejection(self):
        parsed = packets(
            ('r1 = #0',),
            ('r5 = add(r4,#8)',),
            ('r2 = memw(r5+#4)',),
        )
        facts = build_packet_facts(parsed)
        blocks = build_cfg(facts, 'example')
        _live_in, live_out = compute_liveness(facts, blocks)
        proposals = find_address_fold_proposals(facts, blocks, live_out)
        self.assertEqual(len(proposals), 1)
        self.assertEqual(proposals[0].after, ('r2 = memw(r4+#0xc)',))

        out_of_range = packets(
            ('r1 = #0',),
            ('r5 = add(r4,#1023)',),
            ('r2 = memw(r5+#4)',),
        )
        facts = build_packet_facts(out_of_range)
        blocks = build_cfg(facts, 'example')
        _live_in, live_out = compute_liveness(facts, blocks)
        self.assertEqual(find_address_fold_proposals(facts, blocks, live_out), [])

    def test_address_rewrites_refuse_temporary_live_after_consumer(self):
        address = packets(
            ('r1 = #0',),
            ('r5 = add(r4,#8)',),
            ('r2 = memw(r5+#4)',),
            ('r3 = add(r5,#1)',),
        )
        facts = build_packet_facts(address)
        blocks = build_cfg(facts, 'example')
        _live_in, live_out = compute_liveness(facts, blocks)
        self.assertEqual(find_address_fold_proposals(facts, blocks, live_out), [])

        gp = packets(
            ('r1 = #0',),
            ('r5 = r28',),
            ('r2 = memw(r5+#4)',),
            ('r3 = add(r5,#1)',),
        )
        facts = build_packet_facts(gp)
        blocks = build_cfg(facts, 'example')
        _live_in, live_out = compute_liveness(facts, blocks)
        self.assertEqual(find_gp_copy_proposals(gp, facts, blocks, live_out), [])

    def test_addasl_indexed_memory_rewrite_is_table_gated(self):
        parsed = packets(
            ('r1 = #0',),
            ('r5 = addasl(r4,r3,#2)',),
            ('r2 = memw(r5+#0)',),
        )
        facts = build_packet_facts(parsed)
        blocks = build_cfg(facts, 'example')
        _live_in, live_out = compute_liveness(facts, blocks)
        for arch in ('v68', 'v73', 'v81'):
            proposals = find_addasl_indexed_memory_proposals(facts, blocks, live_out, arch)
            self.assertEqual(len(proposals), 1)
            self.assertEqual(proposals[0].after, ('r2 = memw(r4+r3<<#2)',))
        rewritten = apply_handopt_proposals(
            parsed, find_addasl_indexed_memory_proposals(facts, blocks, live_out, 'v81'))
        self.assertEqual([insn.text for _endloop, insns in rewritten for insn in insns],
                         ['r1 = #0', 'r2 = memw(r4+r3<<#2)'])

    def test_addasl_indexed_memory_rejects_live_and_nonzero_offset_forms(self):
        live_temp = packets(
            ('r1 = #0',),
            ('r5 = addasl(r4,r3,#2)',),
            ('r2 = memw(r5+#0)',),
            ('r6 = add(r5,#1)',),
        )
        facts = build_packet_facts(live_temp)
        blocks = build_cfg(facts, 'example')
        _live_in, live_out = compute_liveness(facts, blocks)
        self.assertEqual(find_addasl_indexed_memory_proposals(facts, blocks, live_out, 'v81'), [])

        nonzero_offset = packets(
            ('r1 = #0',),
            ('r5 = addasl(r4,r3,#2)',),
            ('r2 = memw(r5+#4)',),
        )
        facts = build_packet_facts(nonzero_offset)
        blocks = build_cfg(facts, 'example')
        _live_in, live_out = compute_liveness(facts, blocks)
        self.assertEqual(find_addasl_indexed_memory_proposals(facts, blocks, live_out, 'v81'), [])

    def test_local_pure_cse_only_removes_dead_duplicate_in_straight_line_block(self):
        parsed = packets(
            ('r1 = #0',),
            ('r5 = add(r3,#4)',),
            ('r6 = add(r3,#4)',),
            ('r7 = #1',),
        )
        facts = build_packet_facts(parsed)
        blocks = build_cfg(facts, 'example')
        _in, live_out = compute_liveness(facts, blocks)
        proposals = find_local_pure_cse_proposals(facts, blocks, live_out)
        self.assertEqual(len(proposals), 1)
        self.assertEqual(proposals[0].rule, 'local-pure-cse')
        rewritten = apply_handopt_proposals(parsed, proposals)
        self.assertNotIn('r6 = add(r3,#4)', [insn.text for _endloop, insns in rewritten for insn in insns])

    def test_local_pure_cse_refuses_live_duplicate_and_reserved_registers(self):
        live_duplicate = packets(
            ('r1 = #0',),
            ('r5 = add(r3,#4)',),
            ('r6 = add(r3,#4)',),
            ('r7 = add(r6,#1)',),
        )
        facts = build_packet_facts(live_duplicate)
        blocks = build_cfg(facts, 'example')
        _in, live_out = compute_liveness(facts, blocks)
        self.assertEqual(find_local_pure_cse_proposals(facts, blocks, live_out), [])

        reserved = packets(
            ('r1 = #0',),
            ('r20 = add(r3,#4)',),
            ('r6 = add(r3,#4)',),
        )
        facts = build_packet_facts(reserved)
        blocks = build_cfg(facts, 'example')
        _in, live_out = compute_liveness(facts, blocks)
        self.assertEqual(find_local_pure_cse_proposals(facts, blocks, live_out), [])

    def test_candidate_packets_do_not_exceed_three_alu_results(self):
        facts = build_packet_facts(packets(
            ('r1 = #1',), ('r2 = #2',), ('r3 = #3',), ('r4 = #4',),
        ))
        edges, _diagnostics = build_dependencies(facts)
        candidates = candidate_packets(facts, edges)
        self.assertEqual([[fact.node for fact in candidate] for candidate in candidates],
                         [['P000.I0', 'P001.I0', 'P002.I0']])

    def test_report_mode_does_not_change_emitted_assembly(self):
        dump_lines = [
            '00000100 <example>:\n',
            '     100:\t01 40 00 78\t78004001 { \tr1 = #0x1 } \n',
            '     104:\t02 40 00 78\t78004002 { \tr2 = #0x2 } \n',
            '00000108 <next_function>:\n',
        ]
        parsed = parse_packets(extract_function_lines(dump_lines, 'example'))
        normal = emit_assembly('example', parsed, '.text', '2', {})
        packet_facts = build_packet_facts(parsed)
        edges, diagnostics = build_dependencies(packet_facts)
        report = render_schedule_report('example', packet_facts, edges, diagnostics)
        self.assertIn('schedule-report: example', report)
        self.assertEqual(normal, emit_assembly('example', parsed, '.text', '2', {}))


def run_script_regression_tests():
    suite = unittest.defaultTestLoader.loadTestsFromTestCase(ObjdumpToAsmRegressionTests)
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    return result.wasSuccessful()


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--check-script-regression', action='store_true',
                        help='run the built-in script regression suite and exit')
    parser.add_argument('--objdump', required=False, metavar='PATH',
                        help='path to objdump text')
    parser.add_argument('--function-name', required=False, metavar='FUNCTION',
                        help='exact function name to extract from the objdump')
    parser.add_argument('--output', required=False, metavar='BASE',
                        help='output base; writes BASE.S and BASE.txt')
    parser.add_argument('--section', default='.text', help='FUNC_START section')
    parser.add_argument('--align', default='2', help='.p2align amount')
    parser.add_argument('--offsets', help='built asm_offsets.h for KG_* substitution')
    parser.add_argument('--handopt', action='store_true',
                        help='run the complete mechanical optimizer and emit a bare-register candidate')
    parser.add_argument('--arch', choices=('v68', 'v73', 'v81'), default=None,
                        help=argparse.SUPPRESS)
    parser.add_argument('--schedule-report', action='store_true', help=argparse.SUPPRESS)
    parser.add_argument('--handopt-report', action='store_true', help=argparse.SUPPRESS)
    parser.add_argument('--squeeze', action='store_true', help=argparse.SUPPRESS)
    parser.add_argument('--squeeze-report', action='store_true', help=argparse.SUPPRESS)
    parser.add_argument('--assembler-oracle', action='store_true', help=argparse.SUPPRESS)
    parser.add_argument('--return-load-opt', action='store_true', help=argparse.SUPPRESS)
    parser.add_argument('--return-coalesce-opt', action='store_true', help=argparse.SUPPRESS)
    parser.add_argument('--guarded-fallthrough-opt', action='store_true', help=argparse.SUPPRESS)
    parser.add_argument('--guarded-return-coalesce-opt', action='store_true', help=argparse.SUPPRESS)
    args = parser.parse_args()

    if args.check_script_regression:
        raise SystemExit(0 if run_script_regression_tests() else 1)
    if not args.objdump or not args.function_name or not args.output:
        parser.error('--objdump, --function-name, and --output are required')

    args.arch = args.arch or derive_architecture(args.offsets, args.objdump)
    if args.handopt:
        args.schedule_report = True
        args.handopt_report = True
        args.squeeze = True
        args.squeeze_report = True
        args.assembler_oracle = True
        args.return_load_opt = True
        args.return_coalesce_opt = True
        args.guarded_fallthrough_opt = True
        args.guarded_return_coalesce_opt = True

    with open(args.objdump) as source:
        objdump_lines = source.readlines()
    offsets = load_offsets(args.offsets) if args.offsets else {}
    diagnostics = io.StringIO()
    with contextlib.redirect_stderr(diagnostics):
        assembly = process_function(objdump_lines, args.function_name, args,
                                    offsets, args.arch)

    output_base = Path(args.output)
    if output_base.suffix in ('.S', '.s', '.txt'):
        output_base = output_base.with_suffix('')
    output_base.with_suffix('.S').write_text(assembly + '\n')
    output_base.with_suffix('.txt').write_text(diagnostics.getvalue())


if __name__ == '__main__':
    main()
