#!/usr/bin/env python3
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear
"""Generate an interactive HTML coverage report from cov.rpt + cov.txt.

"""

import argparse
import html
import json
import re
import sys

_FUNC_RE    = re.compile(r'^([0-9a-f]+)\s+<([A-Za-z0-9_.]+)>:')
_RPT_RE     = re.compile(r'^\s*(\d+)%:\s+(\S+)')
_CALLTGT_RE = re.compile(r'<([A-Za-z0-9_]+)>')

_CSS = """
* { box-sizing: border-box; margin: 0; padding: 0; }
body { font-family: sans-serif; display: flex; height: 100vh; overflow: hidden; }

#sidebar {
  width: 320px; min-width: 220px; max-width: 480px;
  border-right: 2px solid #555;
  display: flex; flex-direction: column;
  background: #1e1e1e; color: #ddd;
  resize: horizontal; overflow: auto;
}
#sidebar-header {
  padding: 8px 12px; background: #252526;
  border-bottom: 1px solid #444; font-size: 0.85em; color: #aaa;
  flex-shrink: 0;
}
#search {
  width: 100%; padding: 6px 8px; background: #3c3c3c; color: #ddd;
  border: none; border-bottom: 1px solid #555; font-size: 0.85em;
  outline: none;
}
#fn-list { overflow-y: auto; flex: 1; }
.fn-entry {
  display: flex; align-items: center; gap: 6px;
  padding: 3px 10px; cursor: pointer; font-size: 0.82em;
  font-family: monospace; white-space: nowrap;
  border-bottom: 1px solid #2a2a2a;
}
.fn-entry:hover { background: #2a2d2e; }
.fn-entry.active { background: #094771; color: #fff; }

.pct {
  display: inline-block; width: 36px; text-align: right;
  font-weight: bold; padding: 1px 4px; border-radius: 3px; font-size: 0.9em;
}
.pct-red    { background: #6e1a1a; color: #f99; }
.pct-yellow { background: #5a4a00; color: #fd7; }
.pct-green  { background: #1a4a1a; color: #8f8; }

/* Coverage threshold slider */
#threshold-bar {
  display: flex; flex-wrap: wrap; align-items: center; gap: 6px 8px;
  padding: 7px 10px; background: #252526; border-bottom: 1px solid #444;
  font-size: 0.8em; color: #aaa; flex-shrink: 0;
}
#threshold-bar label { white-space: nowrap; }
#threshold { flex: 1 1 100%; order: 3; accent-color: #0e7; cursor: pointer; }
#threshold-val { min-width: 46px; text-align: right; color: #ddd; font-family: monospace; }
#threshold-count {
  margin-left: auto; padding: 1px 7px; border-radius: 8px;
  background: #0d3a5c; color: #9cdcfe; font-family: monospace; font-size: 0.95em;
}

/* Functions / Files mode toggle */
#mode-toggle { display: flex; border-bottom: 1px solid #444; flex-shrink: 0; }
.mode-btn {
  flex: 1; padding: 6px 8px; background: #252526; color: #aaa; border: none;
  cursor: pointer; font-size: 0.8em; border-bottom: 2px solid transparent;
}
.mode-btn:hover { color: #ddd; }
.mode-btn.active { color: #fff; border-bottom-color: #0e7; background: #2a2d2e; }

/* Omit button (detail pane) + omit bar (sidebar) */
.omit-btn { margin-left: auto; color: #f99; }
.omit-btn:hover:not(:disabled) { color: #fbb; border-bottom-color: #6e1a1a; }
.omit-btn:disabled { color: #555; cursor: default; }
.omit-btn.is-omitted { color: #d18616; }
.omit-btn.is-omitted:hover:not(:disabled) { color: #f0a93a; border-bottom-color: #d18616; }
#omit-bar {
  display: flex; flex-wrap: wrap; align-items: center; gap: 6px;
  padding: 6px 10px; background: #2a1a1a; border-bottom: 1px solid #5a2a2a;
  font-size: 0.78em; color: #f99; flex-shrink: 0;
}
#omit-count { margin-right: auto; font-family: monospace; }
.omit-action {
  background: #3c2222; color: #fbb; border: 1px solid #6e1a1a;
  border-radius: 3px; padding: 2px 8px; font-size: 0.95em; cursor: pointer;
}
.omit-action:hover { background: #5a2a2a; color: #fff; }
/* View/restore panel listing every session-omitted function */
#omit-panel {
  flex: 1 1 100%; order: 9; max-height: 220px; overflow-y: auto;
  margin-top: 4px; border-top: 1px solid #5a2a2a;
}
.omit-panel-head {
  display: flex; align-items: center; justify-content: space-between;
  padding: 4px 2px; color: #c99; font-family: sans-serif; font-size: 0.95em;
}
.omit-restore-all {
  background: #3c2222; color: #fbb; border: 1px solid #6e1a1a;
  border-radius: 3px; padding: 1px 6px; font-size: 0.95em; cursor: pointer;
}
.omit-restore-all:hover { background: #5a2a2a; color: #fff; }
.omit-item {
  display: flex; align-items: center; gap: 6px;
  padding: 2px 2px; font-family: monospace; white-space: nowrap;
}
.omit-restore {
  background: #243a24; color: #8f8; border: 1px solid #1a4a1a;
  border-radius: 3px; padding: 0 5px; font-size: 1em; cursor: pointer; line-height: 1.4;
}
.omit-restore:hover { background: #1a4a1a; color: #fff; }
.omit-item-name { overflow: hidden; text-overflow: ellipsis; color: #ddd; }

/* File tree (Files mode) */
#file-tree { overflow-y: auto; flex: 1; font-family: monospace; font-size: 0.82em; }
.tree-row {
  display: flex; align-items: center; gap: 6px;
  padding: 2px 10px; white-space: nowrap; cursor: pointer;
  border-bottom: 1px solid #2a2a2a;
}
.tree-row:hover { background: #2a2d2e; }
.tree-arrow { display: inline-block; width: 12px; color: #888; font-size: 0.9em; }
.tree-label { overflow: hidden; text-overflow: ellipsis; }
.tree-dir  .tree-label { color: #9cdcfe; }
.tree-file .tree-label { color: #ddd; }
.tree-fn   { cursor: pointer; }
.tree-fn   .tree-label { color: #bbb; padding-left: 12px; }
.tree-fn.active { background: #094771; }
.tree-fn.active .tree-label { color: #fff; }
.tree-root { background: #252526; font-weight: bold; border-bottom: 1px solid #444; }
.tree-root .tree-label { color: #ccc; }

#detail { flex: 1; overflow: hidden; display: flex; flex-direction: column; padding: 16px 20px; background: #1e1e1e; color: #ccc; }
#detail h2 { font-size: 1em; margin-bottom: 10px; color: #9cdcfe; border-bottom: 1px solid #333; padding-bottom: 6px; flex-shrink: 0; }

#detail-tabs { display: flex; gap: 8px; margin-bottom: 12px; border-bottom: 1px solid #444; flex-shrink: 0; }
#detail-content { flex: 1; overflow-y: auto; min-height: 0; display: flex; flex-direction: column; }
#disasm-tab, #graph-tab { flex: 1; min-height: 0; }
.tab-btn {
  padding: 6px 12px; background: #2a2a2a; color: #aaa; border: none;
  cursor: pointer; font-size: 0.85em; border-bottom: 2px solid transparent;
}
.tab-btn.active { color: #fff; border-bottom-color: #0e7; }
.tab-btn:hover { color: #ddd; }

pre.disasm { font-family: 'Cascadia Code', 'Consolas', monospace; font-size: 0.82em; tab-size: 4; white-space: pre; }
.exec, .noexec, .cont-exec, .cont-noexec, .nodata { display: block; line-height: 1.3; }
.exec         { background: #1a3a1a; color: #b5e8b5; margin-top: 10px; }
.noexec       { background: #3a1a1a; color: #f88; font-weight: bold; margin-top: 10px; }
.cont-exec    { background: #1a3a1a; color: #b5e8b5; }
.cont-noexec  { background: #3a1a1a; color: #f88; }
.exec:first-child, .noexec:first-child { margin-top: 0; }
.nodata       { color: #777; font-style: italic; }
.exec .fn-link, .cont-exec .fn-link { color: #7df; text-decoration: underline; cursor: pointer; }
.exec .fn-link:hover, .cont-exec .fn-link:hover { color: #fff; }
.noexec .fn-link, .cont-noexec .fn-link { color: #f88; text-decoration: underline; cursor: pointer; }
.noexec .fn-link:hover, .cont-noexec .fn-link:hover { color: #ffb; }

#placeholder { color: #555; margin-top: 40px; font-size: 0.95em; }

#test-selector {
  padding: 6px 10px; border-bottom: 1px solid #333; background: #252526;
  display: flex; align-items: center; gap: 10px; font-size: 0.82em; flex-shrink: 0;
}
#test-selector label { color: #aaa; }
#test-selector select {
  background: #3c3c3c; color: #ddd; border: 1px solid #555;
  border-radius: 3px; padding: 3px 8px; font-size: 0.9em; flex: 1;
}
"""

_JS = r"""
var active       = null;
var currentFunc  = null;
var currentDepth = 3;
var currentTest  = '__global__';  // '__global__' or a test name key
var sidebarMode  = 'functions';   // 'functions' or 'files'
var treeCollapsed = {};           // { dirPath: true } — collapsed tree nodes
var maxPct       = 100;           // threshold: only show functions with pct <= maxPct

// ── graph layout constants ──────────────────────────────────────────────
var NW = 200, NH = 30, HGAP = 60, VGAP = 8, ROW = NH + VGAP;

// ── utilities ──────────────────────────────────────────────────────────
function escSvg(s) {
  return String(s).replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');
}
function pctColor(p) {
  if (p === undefined) return '#999';
  return p < 50 ? '#f99' : p < 80 ? '#fd7' : '#8f8';
}
function pctClass(p) {
  return p < 50 ? 'pct-red' : p < 80 ? 'pct-yellow' : 'pct-green';
}

// ── tree building ──────────────────────────────────────────────────────
function buildTree(rootName, graph, maxDepth) {
  var total = [0];
  function dfs(name, depth, path) {
    if (total[0]++ > 400)
      return {name:'…(limit)', depth:depth, children:[], rows:1, isCycle:false, row:0};
    var isCycle = path.indexOf(name) >= 0;
    var children = [];
    if (!isCycle && depth < maxDepth) {
      path.push(name);
      var targets = graph[name] || [];
      for (var i = 0; i < targets.length; i++) {
        // in per-test mode, skip nodes not exercised by the current test
        if (currentTest !== '__global__') {
          var td = testCovData[currentTest];
          if (!td || !td[targets[i]]) continue;
        }
        children.push(dfs(targets[i], depth + 1, path));
      }
      path.pop();
    }
    var rows = children.length
      ? children.reduce(function(s,c){ return s + c.rows; }, 0) : 1;
    return {name:name, depth:depth, isCycle:isCycle, children:children, rows:rows, row:0};
  }
  return dfs(rootName, 0, []);
}

function assignRows(node, start) {
  if (!node.children.length) { node.row = start; return start + 1; }
  var r = start;
  for (var i = 0; i < node.children.length; i++) r = assignRows(node.children[i], r);
  node.row = (node.children[0].row + node.children[node.children.length-1].row) / 2;
  return r;
}

function maxDepthOf(node) {
  if (!node.children.length) return node.depth;
  var m = node.depth;
  for (var i = 0; i < node.children.length; i++) m = Math.max(m, maxDepthOf(node.children[i]));
  return m;
}

// ── SVG rendering ──────────────────────────────────────────────────────
function svgRootNode(x, y, name, pct) {
  var label  = name.length > 26 ? name.slice(0,23)+'…' : name;
  var hit    = nodeHitByTest(name);
  var pcol   = hit ? pctColor(pct) : '#555';
  var pctStr = pct !== undefined ? pct+'%' : '?';
  var safe   = name.replace(/'/g, "\\'");
  var omitted = isOmitted(name);
  var stroke  = omitted ? '#d18616' : '#9cdcfe';
  var dash    = omitted ? ' stroke-dasharray="5,3"' : '';
  var flag    = omitted ? '<title>omitted</title>' : '';
  return '<g transform="translate(' + x + ',' + y + ')" onclick="show(\'' + safe + '\')" style="cursor:pointer">'
    + flag
    + '<rect width="' + NW + '" height="' + NH + '" rx="4" fill="#0d3a5c" stroke="' + stroke + '" stroke-width="2"' + dash + '/>'
    + '<text x="8" y="20" font-family="monospace" font-size="11" fill="#9cdcfe">' + escSvg(label) + (omitted ? ' ⊘' : '') + '</text>'
    + '<text x="' + (NW-6) + '" y="20" text-anchor="end" font-family="monospace" font-size="11" fill="' + pcol + '">' + pctStr + '</text>'
    + '</g>';
}

// isLeft=true → callers side (grows left); isLeft=false → callees side (grows right)
// parentConnX: the x-coordinate where the parent connects (parent's right edge for callees,
//              parent's left edge for callers)
// parentMidY:  the y midpoint of the parent node
function renderSubtree(node, isLeft, rowOff, originX, originY,
                        parentConnX, parentMidY, edges, nodes) {
  var x = isLeft
    ? originX - node.depth * (NW + HGAP)
    : originX + node.depth * (NW + HGAP);
  var y    = Math.round((node.row + rowOff) * ROW) + originY;
  var midY = y + NH / 2;

  // bezier edge — always drawn left→right
  var ex1 = isLeft ? (x + NW)      : parentConnX;
  var ey1 = isLeft ? midY          : parentMidY;
  var ex2 = isLeft ? parentConnX   : x;
  var ey2 = isLeft ? parentMidY    : midY;
  var cpx = (ex2 - ex1) * 0.5;
  edges.push('<path d="M' + ex1 + ',' + ey1
    + ' C' + (ex1+cpx) + ',' + ey1
    + ' '  + (ex2-cpx) + ',' + ey2
    + ' '  + ex2 + ',' + ey2
    + '" fill="none" stroke="#4a4a4a" stroke-width="1.5" marker-end="url(#arr)"/>');
  // node box
  var pct     = fnPcts[node.name];
  var pcol    = pctColor(pct);
  var pctStr  = pct !== undefined ? pct+'%' : '?';
  var label   = node.name.length > 26 ? node.name.slice(0,23)+'…' : node.name;
  var omitted = isOmitted(node.name);
  var fill    = node.isCycle ? '#1e1e2e' : '#252526';
  var stroke  = omitted ? '#d18616' : node.isCycle ? '#4a4a9a' : '#444';
  var tfill   = node.isCycle ? '#7070c0' : '#ccc';
  var dash    = omitted ? ' stroke-dasharray="5,3"' : '';
  var safe    = node.name.replace(/'/g, "\\'");
  var click   = node.isCycle ? '' : ' onclick="show(\'' + safe + '\')" style="cursor:pointer"';
  nodes.push('<g transform="translate(' + x + ',' + y + ')"' + click + '>'
    + (omitted ? '<title>omitted</title>' : '')
    + '<rect width="' + NW + '" height="' + NH + '" rx="4" fill="' + fill + '" stroke="' + stroke + '"' + dash + '/>'
    + '<text x="8" y="20" font-family="monospace" font-size="11" fill="' + tfill + '">'
      + escSvg(label) + (node.isCycle ? ' ↩' : '') + (omitted ? ' ⊘' : '') + '</text>'
    + '<text x="' + (NW-6) + '" y="20" text-anchor="end" font-family="monospace" font-size="11" fill="' + pcol + '">' + pctStr + '</text>'
    + '</g>');

  // recurse — pass this node's outward edge as the connection point for children
  var childConn = isLeft ? x : x + NW;
  for (var i = 0; i < node.children.length; i++)
    renderSubtree(node.children[i], isLeft, rowOff, originX, originY, childConn, midY, edges, nodes);
}

// ── main SVG builder ───────────────────────────────────────────────────
function renderBiGraph(rootName) {
  var graphs   = getCallGraphs(rootName);
  var calleeT  = buildTree(rootName, graphs.cg, currentDepth);
  var callerT  = buildTree(rootName, graphs.rg, currentDepth);
  assignRows(calleeT, 0);
  assignRows(callerT, 0);

  var totalRows = Math.max(calleeT.rows, callerT.rows, 1);
  var calleeOff = (totalRows - calleeT.rows) / 2;
  var callerOff = (totalRows - callerT.rows) / 2;

  // synchronise root row between both sides
  var rootRow      = calleeT.row + calleeOff;
  var callerRootRow = callerT.row + callerOff;
  callerOff += (rootRow - callerRootRow);

  var maxCD = maxDepthOf(calleeT);
  var maxRD = maxDepthOf(callerT);
  var PAD   = 30;
  var svgW  = (maxCD + maxRD + 1) * (NW + HGAP) + PAD * 2;
  var svgH  = Math.max(totalRows * ROW + PAD * 2, NH + PAD * 2);

  var originX  = maxRD * (NW + HGAP) + PAD;
  var originY  = PAD;
  var rootY    = Math.round(rootRow * ROW) + originY;
  var rootMidY = rootY + NH / 2;

  var edges = [], nodes = [];
  nodes.push(svgRootNode(originX, rootY, rootName, fnPcts[rootName]));

  for (var i = 0; i < calleeT.children.length; i++)
    renderSubtree(calleeT.children[i], false, calleeOff, originX, originY, originX + NW, rootMidY, edges, nodes);
  for (var i = 0; i < callerT.children.length; i++)
    renderSubtree(callerT.children[i], true,  callerOff, originX, originY, originX,      rootMidY, edges, nodes);

  return {
    svgHtml: '<svg id="cg-svg" width="' + svgW + '" height="' + svgH
      + '" style="display:block;position:absolute;left:0;top:0;">'
      + '<defs>'
      + '<marker id="arr" markerWidth="8" markerHeight="8" refX="8" refY="3" orient="auto">'
      + '<path d="M0,0 L0,6 L8,3 z" fill="#4a4a4a"/>'
      + '</marker>'
      + '</defs>'
      + edges.join('') + nodes.join('') + '</svg>',
    svgW: svgW, svgH: svgH, originX: originX, rootY: rootY
  };
}

// ── drag / pan ─────────────────────────────────────────────────────────
var _dragging = false, _lastX = 0, _lastY = 0, _panX = 0, _panY = 0;

document.addEventListener('mousemove', function(e) {
  if (!_dragging) return;
  _panX += e.clientX - _lastX;
  _panY += e.clientY - _lastY;
  _lastX = e.clientX; _lastY = e.clientY;
  var svg = document.getElementById('cg-svg');
  if (svg) svg.style.transform = 'translate(' + _panX + 'px,' + _panY + 'px)';
});
document.addEventListener('mouseup', function(e) { if (e.button === 2) _dragging = false; });

function setupDrag(initPanX, initPanY) {
  _panX = initPanX; _panY = initPanY;
  var svg = document.getElementById('cg-svg');
  if (svg) svg.style.transform = 'translate(' + _panX + 'px,' + _panY + 'px)';
  var wrap = document.getElementById('cg-wrap');
  if (!wrap) return;
  wrap.addEventListener('mousedown', function(e) {
    if (e.button === 2) { _dragging = true; _lastX = e.clientX; _lastY = e.clientY; e.preventDefault(); }
  });
  wrap.addEventListener('contextmenu', function(e) { e.preventDefault(); });
}

// ── updateCallGraph ────────────────────────────────────────────────────
function updateCallGraph() {
  var depthEl = document.getElementById('depth-input');
  if (depthEl) currentDepth = parseInt(depthEl.value) || currentDepth;
  var pct = fnPcts[currentFunc] !== undefined ? fnPcts[currentFunc] : '?';
  var r   = renderBiGraph(currentFunc);
  var graphTab = document.getElementById('graph-tab');
  graphTab.innerHTML =
    '<div style="display:flex;align-items:center;gap:20px;margin-bottom:10px;flex-shrink:0;">'
    + '<h2 style="margin:0;">' + currentFunc + '  —  ' + pct + '%</h2>'
    + '<span style="font-size:0.85em;color:#aaa;">Depth:&nbsp;'
    + '<input id="depth-input" type="number" min="1" value="' + currentDepth + '"'
    + ' style="width:46px;padding:4px;background:#3c3c3c;color:#ddd;border:1px solid #555;border-radius:3px;"'
    + ' onchange="updateCallGraph()"></span>'
    + '<span style="font-size:0.75em;color:#555;">right-click drag to pan</span>'
    + '</div>'
    + '<div id="cg-wrap" style="position:relative;overflow:hidden;width:100%;flex:1;min-height:0;background:#141414;border-radius:6px;">'
    + r.svgHtml
    + '</div>';
  // centre the root node in the visible area
  var wrap = document.getElementById('cg-wrap');
  var wrapW = wrap ? (wrap.offsetWidth || 700) : 700;
  var wrapH = wrap ? (wrap.offsetHeight || 560) : 560;
  setupDrag(
    Math.round(wrapW / 2  - r.originX - NW / 2),
    Math.round(wrapH / 2  - r.rootY   - NH / 2)
  );
}

// ── tabs ───────────────────────────────────────────────────────────────
function showTab(tab) {
  var dTab = document.getElementById('disasm-tab');
  var gTab = document.getElementById('graph-tab');
  var dBtn = document.querySelector('[data-tab="disasm"]');
  var gBtn = document.querySelector('[data-tab="graph"]');
  if (tab === 'disasm') {
    dTab.style.display = 'block'; gTab.style.display = 'none';
    dBtn.classList.add('active'); gBtn.classList.remove('active');
  } else {
    dTab.style.display = 'none';  gTab.style.display = 'flex';
    dBtn.classList.remove('active'); gBtn.classList.add('active');
    updateCallGraph();
  }
}

// ── per-test disasm renderer ───────────────────────────────────────────
function getDisasmHtml(name) {
  if (currentTest === '__global__') {
    return fnData[name] || '<p style="color:#777;font-style:italic;">No data.</p>';
  }
  var td = testCovData[currentTest];
  if (!td) return '<p style="color:#777;font-style:italic;">No data for this test.</p>';
  var lines = td[name];
  if (!lines) return '<p style="color:#777;font-style:italic;">Function not exercised by this test.</p>';
  return lines;
}

function getCallGraphs(name) {
  // Always use global call graph for structure — per-test cov.txt only contains
  // executed functions so per-test graphs are missing most edges.
  return {cg: calleeGraph, rg: callerGraph};
}

function getNodePct(name) {
  if (currentTest === '__global__') return fnPcts[name];
  var tp = testPcts[currentTest];
  return tp ? tp[name] : undefined;
}

function nodeHitByTest(name) {
  if (currentTest === '__global__') return true;
  var td = testCovData[currentTest];
  return !!(td && td[name]);
}

function onTestChange() {
  var sel = document.getElementById('test-select');
  if (sel) currentTest = sel.value;

  var pcts = currentTest === '__global__' ? fnPcts : (testPcts[currentTest] || {});
  var q    = (document.getElementById('search').value || '').toLowerCase();

  updateThresholdLabel();   // count is View-dependent, refresh it here too

  // update sidebar: filter visibility (test membership + threshold + search) and pct badges
  var entries = document.querySelectorAll('.fn-entry');
  entries.forEach(function(el) {
    var fname = el.dataset.fn;
    var p = currentTest === '__global__' ? fnPcts[fname] : pcts[fname];
    // hidden if: omitted this session, not exercised by this test, above
    // threshold, or filtered by search
    var hide = isOmitted(fname) || (p === undefined) || (p > maxPct)
             || (q && fname.toLowerCase().indexOf(q) < 0);
    el.style.display = hide ? 'none' : '';
    if (p !== undefined) {
      var cls = p < 50 ? 'pct-red' : p < 80 ? 'pct-yellow' : 'pct-green';
      var badge = el.querySelector('.pct');
      if (badge) { badge.textContent = p + '%'; badge.className = 'pct ' + cls; }
    }
  });

  // re-sort visible entries by pct ascending, then name — a total order, so it
  // matches the initial server-side sort and is independent of prior DOM order
  // (otherwise omit/restore would leave entries stranded out of place).
  var sidebar = document.getElementById('fn-list');
  if (sidebar) {
    var visibleEntries = Array.from(entries).filter(function(el) {
      return el.style.display !== 'none';
    });
    visibleEntries.sort(function(a, b) {
      var pa = pcts[a.dataset.fn];
      var pb = pcts[b.dataset.fn];
      if (pa === undefined) pa = currentTest === '__global__' ? fnPcts[a.dataset.fn] : 0;
      if (pb === undefined) pb = currentTest === '__global__' ? fnPcts[b.dataset.fn] : 0;
      if (pa !== pb) return pa - pb;
      return a.dataset.fn < b.dataset.fn ? -1 : a.dataset.fn > b.dataset.fn ? 1 : 0;
    });
    visibleEntries.forEach(function(el) { sidebar.appendChild(el); });
  }

  // if Files mode is active, rebuild the tree for the new View
  if (sidebarMode === 'files') renderFileTree();

  // if current function is not visible in this test, jump to first visible one
  if (currentFunc) {
    var funcVisible = currentTest === '__global__' || (pcts[currentFunc] !== undefined);
    if (!funcVisible) {
      var first = document.querySelector('.fn-entry:not([style*="display: none"]):not([style*="display:none"])');
      if (first) { show(first.dataset.fn); return; }
    }
    refreshDetail();
  }
}

// coverage threshold slider: show only functions with pct <= maxPct
function onThresholdChange(v) {
  maxPct = parseInt(v, 10);
  updateThresholdLabel();
  onTestChange();
}

// count functions at or below the threshold in the current View, and show it
function updateThresholdLabel() {
  var pcts  = currentTest === '__global__' ? fnPcts : (testPcts[currentTest] || {});
  var count = 0;
  for (var fn in pcts) if (pcts[fn] <= maxPct) count++;
  var lbl = document.getElementById('threshold-val');
  if (lbl) lbl.textContent = '≤ ' + maxPct + '%';
  var cnt = document.getElementById('threshold-count');
  if (cnt) cnt.textContent = count + ' fn' + (count === 1 ? '' : 's');
}

function refreshDetail() {
  var disasmHtml = getDisasmHtml(currentFunc);
  var disasmTab  = document.getElementById('disasm-tab');
  if (disasmTab) disasmTab.innerHTML = disasmHtml;
  var graphTab = document.getElementById('graph-tab');
  if (graphTab && graphTab.style.display !== 'none') updateCallGraph();
}

// ── show (sidebar + SVG node clicks) ──────────────────────────────────
function show(name) {
  currentFunc = name;
  var detail     = document.getElementById('detail');
  var disasmHtml = getDisasmHtml(name);
  detail.innerHTML =
    '<div id="detail-tabs">'
    + '<button class="tab-btn active" data-tab="disasm" onclick="showTab(\'disasm\')">Disasm</button>'
    + '<button class="tab-btn" data-tab="graph" onclick="showTab(\'graph\')">Call Graph</button>'
    + '<button id="omit-btn" class="tab-btn omit-btn" onclick="omitCurrent()" '
    + 'title="Hide this function and add it to the omit list">⊘ Omit</button>'
    + '</div>'
    + '<div id="detail-content">'
    + '<div id="disasm-tab">' + disasmHtml + '</div>'
    + '<div id="graph-tab" style="display:none;flex-direction:column;"></div>'
    + '</div>';
  if (active) active.classList.remove('active');
  var entries = document.querySelectorAll('.fn-entry');
  for (var i = 0; i < entries.length; i++) {
    if (entries[i].dataset.fn === name) {
      entries[i].classList.add('active');
      entries[i].scrollIntoView({block:'nearest'});
      active = entries[i]; break;
    }
  }
  // highlight in the file tree too (Files mode)
  document.querySelectorAll('#file-tree .tree-fn').forEach(function(el) {
    el.classList.toggle('active', el.dataset.fn === name);
  });
  refreshOmitUi();
}

// ── file tree (Files mode) ─────────────────────────────────────────────
// Build a nested tree from fnSrc paths, accumulating [exec,total] packet
// counts up every ancestor.  pkts is the packet source for the current View
// (fnPackets globally, or testPackets[test] per-test); visible limits which
// functions are included (per-test: only exercised functions).
function buildFileTree(pkts, visible) {
  var root = { name: '', path: '', dirs: {}, files: {}, exec: 0, total: 0 };
  Object.keys(fnSrc).forEach(function(fn) {
    if (isOmitted(fn)) return;             // omitted this session
    if (visible && !visible[fn]) return;
    if (curPct(fn) > maxPct) return;       // above coverage threshold
    var pc = pkts[fn];
    if (!pc) return;                       // no packet data in this View
    var src   = fnSrc[fn] || '(unknown)';
    var parts = src.split('/');
    var fname = parts.pop();               // file basename
    var node  = root;
    node.exec += pc[0]; node.total += pc[1];
    for (var i = 0; i < parts.length; i++) {
      var seg = parts[i];
      if (!node.dirs[seg])
        node.dirs[seg] = { name: seg, path: (node.path ? node.path+'/' : '')+seg,
                           dirs: {}, files: {}, exec: 0, total: 0 };
      node = node.dirs[seg];
      node.exec += pc[0]; node.total += pc[1];
    }
    if (!node.files[fname])
      node.files[fname] = { name: fname, path: (node.path ? node.path+'/' : '')+fname,
                            fns: [], exec: 0, total: 0 };
    var f = node.files[fname];
    f.fns.push(fn); f.exec += pc[0]; f.total += pc[1];
  });
  return root;
}

function nodePct(n) {
  return n.total ? Math.round(100 * n.exec / n.total) : 0;
}

// Collapse single-child directory chains (a/b/c → a/b/c) for compactness.
function treeRowHtml(label, pct, depth, kind, key, isOpen, dataAttr) {
  var pad   = 10 + depth * 14;
  var arrow = kind === 'leaf' ? '' :
    '<span class="tree-arrow">' + (isOpen ? '▾' : '▸') + '</span>';
  var badge = '<span class="pct ' + pctClass(pct) + '">' + pct + '%</span>';
  return '<div class="tree-row tree-' + kind + '" style="padding-left:' + pad + 'px;" '
    + dataAttr + '>' + arrow + badge
    + '<span class="tree-label">' + escSvg(label) + '</span></div>';
}

function renderTreeNode(node, depth, out) {
  // directories (sorted by pct asc, then name)
  var dirKeys = Object.keys(node.dirs).sort(function(a, b) {
    var pa = nodePct(node.dirs[a]), pb = nodePct(node.dirs[b]);
    return pa !== pb ? pa - pb : a.localeCompare(b);
  });
  dirKeys.forEach(function(k) {
    var d = node.dirs[k];
    var isOpen = !treeCollapsed[d.path];
    out.push(treeRowHtml(d.name + '/', nodePct(d), depth, 'dir', d.path, isOpen,
                         'onclick="toggleTreeNode(\'' + d.path.replace(/'/g, "\\'") + '\')"'));
    if (isOpen) renderTreeNode(d, depth + 1, out);
  });
  // files (sorted by pct asc, then name)
  var fileKeys = Object.keys(node.files).sort(function(a, b) {
    var pa = nodePct(node.files[a]), pb = nodePct(node.files[b]);
    return pa !== pb ? pa - pb : a.localeCompare(b);
  });
  fileKeys.forEach(function(k) {
    var f = node.files[k];
    var isOpen = !treeCollapsed[f.path];
    out.push(treeRowHtml(f.name, nodePct(f), depth, 'file', f.path, isOpen,
                         'onclick="toggleTreeNode(\'' + f.path.replace(/'/g, "\\'") + '\')"'));
    if (isOpen) {
      // functions in this file, sorted by pct asc
      f.fns.slice().sort(function(a, b) {
        var pa = curPct(a), pb = curPct(b);
        return pa !== pb ? pa - pb : a.localeCompare(b);
      }).forEach(function(fn) {
        var p = curPct(fn);
        var safe = fn.replace(/'/g, "\\'");
        out.push('<div class="tree-row tree-fn" style="padding-left:' + (10 + (depth+1)*14) + 'px;" '
          + 'onclick="show(\'' + safe + '\')" data-fn="' + escSvg(fn) + '">'
          + '<span class="pct ' + pctClass(p) + '">' + p + '%</span>'
          + '<span class="tree-label">' + escSvg(fn) + '</span></div>');
      });
    }
  });
}

// current-View pct for a function (global or per-test)
function curPct(fn) {
  if (currentTest === '__global__') return fnPcts[fn] !== undefined ? fnPcts[fn] : 0;
  var tp = testPcts[currentTest];
  return tp && tp[fn] !== undefined ? tp[fn] : 0;
}

function renderFileTree() {
  var pkts    = currentTest === '__global__' ? fnPackets : (testPackets[currentTest] || {});
  var visible = currentTest === '__global__' ? null : (testPcts[currentTest] || {});
  var root    = buildFileTree(pkts, visible);
  var out     = [];
  out.push('<div class="tree-row tree-root">'
    + '<span class="pct ' + pctClass(nodePct(root)) + '">' + nodePct(root) + '%</span>'
    + '<span class="tree-label">' + (root.total ? 'all (' + root.exec + '/' + root.total + ' packets)'
                                                : 'no coverage') + '</span></div>');
  renderTreeNode(root, 0, out);
  document.getElementById('file-tree').innerHTML = out.join('');
}

function toggleTreeNode(path) {
  treeCollapsed[path] = !treeCollapsed[path];
  renderFileTree();
}

function setSidebarMode(mode) {
  sidebarMode = mode;
  document.getElementById('mode-functions').classList.toggle('active', mode === 'functions');
  document.getElementById('mode-files').classList.toggle('active', mode === 'files');
  document.getElementById('fn-list').style.display   = mode === 'functions' ? '' : 'none';
  document.getElementById('file-tree').style.display = mode === 'files' ? '' : 'none';
  document.getElementById('search').style.display    = mode === 'functions' ? '' : 'none';
  if (mode === 'files') renderFileTree();
}

// ── omit list (Omit button) ─────────────────────────────────────────────
// committedOmit / omitHeader come from scripts/cov_omit_functions (embedded at
// build time).  localOmit holds picks made in this browser via the Omit button,
// persisted in localStorage so they survive reloads of this report.  A static
// file:// page can't write back to scripts/, so the workflow is: omit here to
// preview, then "Download omit list" and commit the file.
var OMIT_KEY = 'h2cov_omit';
var localOmit = {};
try { localOmit = JSON.parse(localStorage.getItem(OMIT_KEY) || '{}') || {}; } catch (e) { localOmit = {}; }

function isOmitted(fn) { return !!localOmit[fn]; }

function saveLocalOmit() {
  try { localStorage.setItem(OMIT_KEY, JSON.stringify(localOmit)); } catch (e) {}
}

function localOmitNames() { return Object.keys(localOmit).filter(function(k){ return localOmit[k]; }); }

// Full omit set for download = committed ∪ local, sorted & de-duped.
function fullOmitList() {
  var s = {};
  committedOmit.forEach(function(n){ s[n] = 1; });
  localOmitNames().forEach(function(n){ s[n] = 1; });
  return Object.keys(s).sort();
}

function omitCurrent() {
  if (!currentFunc) return;
  if (isOmitted(currentFunc)) return;
  if (!confirm('Omit "' + currentFunc + '" from coverage tracking?\n\n'
             + 'It will be hidden from the function list now (still shown, flagged, '
             + 'in the call graph). To make it permanent, use "Download omit list" '
             + 'and commit scripts/cov_omit_functions.'))
    return;
  // find the next visible sibling so we can move there after hiding this one
  var entry = document.querySelector('.fn-entry[data-fn="' + cssEsc(currentFunc) + '"]');
  var nextFn = null;
  if (entry) {
    var sib = entry.nextElementSibling;
    while (sib && sib.style.display === 'none') sib = sib.nextElementSibling;
    if (sib) nextFn = sib.dataset.fn;
  }
  localOmit[currentFunc] = 1;
  saveLocalOmit();
  refreshOmitUi();
  onTestChange();        // re-filters: the entry is now hidden (not destroyed)
  refreshDetail();       // re-render graph so the node picks up its omitted flag
  if (nextFn) show(nextFn);
}

// Restore a single session-omitted function (live — no reload needed).
function restoreOmit(fn) {
  delete localOmit[fn];
  saveLocalOmit();
  refreshOmitUi();
  renderOmitPanel();
  onTestChange();
  refreshDetail();
}

function cssEsc(s) { return String(s).replace(/(["\\])/g, '\\$1'); }

// Update the small omit indicator (count + view/download/clear controls).
function refreshOmitUi() {
  var n   = localOmitNames().length;
  var bar = document.getElementById('omit-bar');
  if (!bar) return;
  if (n === 0) {
    bar.style.display = 'none';
    var panel = document.getElementById('omit-panel');
    if (panel) panel.style.display = 'none';
    refreshOmitBtn();
    return;
  }
  bar.style.display = '';
  var cnt = document.getElementById('omit-count');
  if (cnt) cnt.textContent = '• ' + n + ' omitted this session';
  if (document.getElementById('omit-panel') &&
      document.getElementById('omit-panel').style.display !== 'none')
    renderOmitPanel();
  refreshOmitBtn();
}

// Reflect the current function's omit state on the detail-pane Omit button.
function refreshOmitBtn() {
  var btn = document.getElementById('omit-btn');
  if (!btn) return;
  var omitted = currentFunc ? isOmitted(currentFunc) : false;
  btn.disabled = !currentFunc;
  btn.classList.toggle('is-omitted', omitted);
  if (omitted) {
    btn.textContent = '↺ Restore';
    btn.onclick = function() { restoreOmit(currentFunc); };
  } else {
    btn.textContent = '⊘ Omit';
    btn.onclick = omitCurrent;
  }
}

// Toggle the "view / restore" panel listing every session-omitted function.
function toggleOmitPanel() {
  var panel = document.getElementById('omit-panel');
  if (!panel) return;
  if (panel.style.display === 'none' || !panel.style.display) {
    renderOmitPanel();
    panel.style.display = '';
  } else {
    panel.style.display = 'none';
  }
}

// Render the omitted-function list: each row has a ↺ restore button so the user
// can selectively keep some omitted and bring others back.
function renderOmitPanel() {
  var panel = document.getElementById('omit-panel');
  if (!panel) return;
  var names = localOmitNames().sort();
  if (!names.length) { panel.innerHTML = ''; panel.style.display = 'none'; return; }
  var rows = names.map(function(fn) {
    var p    = curPct(fn);
    var pcls = (p !== undefined && p !== null) ? pctClass(p) : 'pct-red';
    var pstr = (p !== undefined && p !== null) ? p + '%' : '–';
    var safe = cssEsc(fn);
    return '<div class="omit-item">'
      + '<button class="omit-restore" title="Restore this function" '
      + 'onclick="restoreOmit(\'' + safe + '\')">↺</button>'
      + '<span class="pct ' + pcls + '">' + pstr + '</span>'
      + '<span class="omit-item-name" title="' + escSvg(fn) + '">' + escSvg(fn) + '</span>'
      + '</div>';
  }).join('');
  panel.innerHTML =
    '<div class="omit-panel-head">Omitted this session — ↺ to restore'
    + '<button class="omit-restore-all" onclick="restoreAllOmit()">Restore all</button></div>'
    + rows;
}

function restoreAllOmit() {
  if (!localOmitNames().length) return;
  if (!confirm('Restore all session-omitted functions?')) return;
  localOmit = {};
  saveLocalOmit();
  refreshOmitUi();
  renderOmitPanel();
  onTestChange();
  refreshDetail();
}

function downloadOmitList() {
  var body = omitHeader + fullOmitList().join('\n') + '\n';
  var blob = new Blob([body], {type: 'text/plain'});
  var url  = URL.createObjectURL(blob);
  var a    = document.createElement('a');
  a.href = url; a.download = 'cov_omit_functions';
  document.body.appendChild(a); a.click(); a.remove();
  URL.revokeObjectURL(url);
}

function clearLocalOmit() {
  if (!localOmitNames().length) return;
  if (!confirm('Clear all session omits?')) return;
  localOmit = {};
  saveLocalOmit();
  refreshOmitUi();
  renderOmitPanel();
  onTestChange();
  refreshDetail();
}

// ── search + init ──────────────────────────────────────────────────────
// search delegates to onTestChange, which applies search + threshold + test together
document.getElementById('search').addEventListener('input', onTestChange);
window.addEventListener('load', function() {
  // apply any session omits saved from a previous visit
  if (localOmitNames().length) onTestChange();
  refreshOmitUi();
  var first = document.querySelector('.fn-entry:not([style*="display: none"]):not([style*="display:none"])');
  if (first) show(first.dataset.fn);
});
"""


def pct_class(pct):
    if pct < 50:  return 'pct-red'
    if pct < 80:  return 'pct-yellow'
    return 'pct-green'


def parse_rpt(path):
    funcs = []
    with open(path) as f:
        for line in f:
            m = _RPT_RE.match(line)
            if m:
                funcs.append((int(m.group(1)), m.group(2).strip()))
    return funcs


def parse_cov(path):
    result = {}
    cur_name = None
    cur_lines = []
    last_kind = 'exec'

    def flush():
        if cur_name is not None:
            result[cur_name] = list(cur_lines)

    with open(path, errors='replace') as f:
        for raw in f:
            line = raw.rstrip('\n')
            if not line.strip():
                flush(); cur_name = None; cur_lines = []; continue
            m = _FUNC_RE.match(line)
            if m:
                flush(); cur_name = m.group(2); cur_lines = []; last_kind = 'exec'; continue
            if cur_name is None:
                continue
            if line.strip() == 'No data!':
                cur_lines.append(('nodata', line)); continue
            if line.lstrip().startswith('**'):
                last_kind = 'noexec'; cur_lines.append(('noexec', line)); continue
            if re.match(r'^\s+\d', line) and ('cycles' in line or '1.0' in line):
                # normalise "  4604 1.0   addr: ..." → "  4604 cycles  addr: ..."
                line = re.sub(r'^(\s*\d+)\s+1\.0\s+', lambda m: m.group(1) + ' cycles  ', line)
                last_kind = 'exec'; cur_lines.append(('exec', line)); continue
            cont_kind = 'cont-exec' if last_kind == 'exec' else 'cont-noexec'
            cur_lines.append((cont_kind, line))

    flush()
    return result


def packet_counts(lines):
    """Return (exec_packets, total_packets) for a function's parsed lines."""
    total = sum(1 for k, _ in lines if k in ('exec', 'noexec'))
    execd = sum(1 for k, _ in lines if k == 'exec')
    return execd, total


def calc_pct(lines):
    """Return integer % of exec packets out of total (exec+noexec) packets."""
    execd, total = packet_counts(lines)
    if total == 0:
        return 0
    return round(100 * execd / total)


def build_call_graphs(cov_data, tracked_fns):
    """Return (callees, callers) dicts: { funcname: sorted list }."""
    callees = {name: set() for name in tracked_fns}
    callers = {name: set() for name in tracked_fns}
    for funcname, lines in cov_data.items():
        if funcname not in tracked_fns:
            continue
        for _kind, text in lines:
            for m in _CALLTGT_RE.finditer(text):
                callee = m.group(1)
                if callee in tracked_fns and callee != funcname:
                    callees[funcname].add(callee)
                    callers[callee].add(funcname)
    return (
        {k: sorted(v) for k, v in callees.items()},
        {k: sorted(v) for k, v in callers.items()},
    )


def render_line(kind, text, tracked_fns):
    h = html.escape
    parts_out = []
    last = 0
    for m in _CALLTGT_RE.finditer(text):
        fn = m.group(1)
        parts_out.append(h(text[last:m.start()]))
        if fn in tracked_fns:
            parts_out.append(
                f'&lt;<a class="fn-link" href="#" '
                f'onclick="show(\'{h(fn)}\');return false;">{h(fn)}</a>&gt;')
        else:
            parts_out.append(h(m.group(0)))
        last = m.end()
    parts_out.append(h(text[last:]))
    return f'<span class="{kind}">{"".join(parts_out)}</span>'


def render_detail(funcname, pct, lines, tracked_fns):
    h = html.escape
    parts = [f'<h2>{h(funcname)} &nbsp;—&nbsp; {pct}%</h2>', '<pre class="disasm">']
    for kind, text in lines:
        parts.append(render_line(kind, text, tracked_fns))
    parts.append('</pre>')
    pre_start = parts.index('<pre class="disasm">')
    return ('\n'.join(parts[:pre_start + 1])
            + ''.join(parts[pre_start + 1:-1])
            + parts[-1])


def build_html(funcs, cov_data, test_covs, fn_src, omit_names=None, omit_header=''):
    """test_covs: list of (test_name, cov_data_dict) — per-test datasets.
    fn_src: { funcname: "repo/relative/source.c" } for the file-tree view.
    omit_names: set of already-committed omitted function names.
    omit_header: leading comment block of cov_omit_functions, reused verbatim
    when the report's "Omit" button reproduces a downloadable copy of the file."""
    h = html.escape
    omit_names = omit_names or set()

    sidebar_items = []
    for pct, name in funcs:
        pc = pct_class(pct)
        sidebar_items.append(
            f'<div class="fn-entry" data-fn="{h(name)}" onclick="show(\'{h(name)}\')">'
            f'<span class="pct {pc}">{pct}%</span>{h(name)}</div>'
        )

    tracked_fns = {name for _, name in funcs}
    fn_pcts     = {name: pct for pct, name in funcs}

    fn_data_entries = []
    fn_packets      = {}   # { funcname: [exec, total] } for file-tree rollup
    for pct, name in funcs:
        lines = cov_data.get(name, [('nodata', 'No data!')])
        fn_data_entries.append(f'  {json.dumps(name)}: {json.dumps(render_detail(name, pct, lines, tracked_fns))}')
        execd, total = packet_counts(lines)
        fn_packets[name] = [execd, total]

    callees, callers = build_call_graphs(cov_data, tracked_fns)

    fn_data_js    = 'var fnData = {\n'      + ',\n'.join(fn_data_entries) + '\n};'
    fn_pcts_js    = 'var fnPcts = '         + json.dumps(fn_pcts)     + ';'
    fn_packets_js = 'var fnPackets = '      + json.dumps(fn_packets)  + ';'
    fn_src_js     = 'var fnSrc = '          + json.dumps(fn_src)      + ';'
    callee_js     = 'var calleeGraph = '    + json.dumps(callees)     + ';'
    caller_js     = 'var callerGraph = '    + json.dumps(callers)     + ';'

    # Omit list: the committed names plus the file header, so the "Omit" button
    # can persist new picks (localStorage) and reproduce a full file to commit.
    omit_js        = 'var committedOmit = ' + json.dumps(sorted(omit_names)) + ';'
    omit_header_js = 'var omitHeader = '    + json.dumps(omit_header)        + ';'

    # per-test disasm: { test_name: { funcname: rendered_html } }
    # only functions with >0 executed packets are included
    test_cov_data_js_parts = []
    test_callee_js_parts   = []
    test_caller_js_parts   = []
    test_pcts_js_parts     = []
    test_packets_js_parts  = []
    test_names = []
    for tname, tcov in test_covs:
        test_names.append(tname)
        entries = {}
        tpcts   = {}
        tpackets = {}
        for fname, lines in tcov.items():
            if fname not in tracked_fns:
                continue
            if any(k == 'exec' for k, _ in lines):
                p = calc_pct(lines)
                entries[fname] = render_detail(fname, p, lines, tracked_fns)
                tpcts[fname]   = p
                tpackets[fname] = list(packet_counts(lines))
        test_cov_data_js_parts.append(f'  {json.dumps(tname)}: {json.dumps(entries)}')
        test_pcts_js_parts.append(f'  {json.dumps(tname)}: {json.dumps(tpcts)}')
        test_packets_js_parts.append(f'  {json.dumps(tname)}: {json.dumps(tpackets)}')
        tc, tr = build_call_graphs(tcov, tracked_fns)
        test_callee_js_parts.append(f'  {json.dumps(tname)}: {json.dumps(tc)}')
        test_caller_js_parts.append(f'  {json.dumps(tname)}: {json.dumps(tr)}')

    test_cov_js     = 'var testCovData = {\n'      + ',\n'.join(test_cov_data_js_parts) + '\n};'
    test_pcts_js    = 'var testPcts = {\n'         + ',\n'.join(test_pcts_js_parts)     + '\n};'
    test_packets_js = 'var testPackets = {\n'      + ',\n'.join(test_packets_js_parts)  + '\n};'
    test_callee_js  = 'var testCalleeGraphs = {\n' + ',\n'.join(test_callee_js_parts)   + '\n};'
    test_caller_js  = 'var testCallerGraphs = {\n' + ',\n'.join(test_caller_js_parts)   + '\n};'

    total        = len(funcs)
    covered_100  = sum(1 for p, _ in funcs if p == 100)
    covered_any  = sum(1 for p, _ in funcs if p > 0)

    # test selector dropdown
    options = ['<option value="__global__">— Global (all tests merged) —</option>']
    for tname in sorted(test_names):
        options.append(f'<option value="{h(tname)}">{h(tname)}</option>')
    test_selector = (
        '<div id="test-selector">'
        '<label>View:</label>'
        f'<select id="test-select" onchange="onTestChange()">'
        + ''.join(options) +
        '</select>'
        '</div>'
    )

    # Coverage threshold slider — show only functions with pct <= maxPct
    threshold = (
        '<div id="threshold-bar">'
        '<label>Show coverage</label>'
        '<input id="threshold" type="range" min="0" max="100" value="100" '
        'oninput="onThresholdChange(this.value)">'
        '<span id="threshold-val">≤ 100%</span>'
        f'<span id="threshold-count">{total} fns</span>'
        '</div>'
    )

    # Functions / Files mode toggle
    mode_toggle = (
        '<div id="mode-toggle">'
        '<button id="mode-functions" class="mode-btn active" '
        'onclick="setSidebarMode(\'functions\')">Functions</button>'
        '<button id="mode-files" class="mode-btn" '
        'onclick="setSidebarMode(\'files\')">Files</button>'
        '</div>'
    )

    # Omit bar — appears once functions are omitted this session.  Lets the user
    # view/restore individual omits, download an updated cov_omit_functions to
    # commit, or clear all session omits.
    omit_bar = (
        '<div id="omit-bar" style="display:none;">'
        '<span id="omit-count">• 0 omitted this session</span>'
        '<button class="omit-action" onclick="toggleOmitPanel()" '
        'title="View and selectively restore omitted functions">View</button>'
        '<button class="omit-action" onclick="downloadOmitList()" '
        'title="Download an updated cov_omit_functions to commit">Download</button>'
        '<button class="omit-action" onclick="clearLocalOmit()" '
        'title="Restore every function omitted this session">Clear</button>'
        '<div id="omit-panel" style="display:none;"></div>'
        '</div>'
    )

    script = (f'<script>{fn_data_js}\n{fn_pcts_js}\n{fn_packets_js}\n{fn_src_js}\n'
              f'{omit_js}\n{omit_header_js}\n'
              f'{callee_js}\n{caller_js}\n'
              f'{test_cov_js}\n{test_pcts_js}\n{test_packets_js}\n'
              f'{test_callee_js}\n{test_caller_js}\n{_JS}</script>')

    return '\n'.join([
        '<!DOCTYPE html>',
        '<html lang="en">',
        '<head>',
        '<meta charset="utf-8">',
        '<meta name="viewport" content="width=device-width, initial-scale=1">',
        '<title>Hexagon Coverage Report</title>',
        f'<style>{_CSS}</style>',
        '</head>',
        '<body>',

        '<div id="sidebar">',
        '<div id="sidebar-header">',
        '  <strong>Coverage Report</strong><br>',
        f'  {total} functions &nbsp;·&nbsp; {covered_any} hit &nbsp;·&nbsp; {covered_100} at 100%',
        '</div>',
        test_selector,
        threshold,
        mode_toggle,
        omit_bar,
        '<input id="search" type="text" placeholder="Filter functions…">',
        '<div id="fn-list">',
        '\n'.join(sidebar_items),
        '</div>',
        '<div id="file-tree" style="display:none;"></div>',
        '</div>',

        '<div id="detail">',
        '<p id="placeholder">Select a function from the list.</p>',
        '</div>',

        script,
        '</body>',
        '</html>',
    ])


def parse_fn_files(path):
    """Parse the function->source map ('funcname<TAB>path' per line)."""
    fn_src = {}
    with open(path, errors='replace') as f:
        for line in f:
            line = line.rstrip('\n')
            if not line or '\t' not in line:
                continue
            name, src = line.split('\t', 1)
            fn_src[name] = src
    return fn_src


def parse_omit(path):
    """Parse scripts/cov_omit_functions.

    Returns (names, header) where names is the set of omitted function names and
    header is the leading comment block (kept verbatim so the report's "Download
    omit list" button can reproduce a file in the same style).
    """
    names = set()
    header_lines = []
    in_header = True
    with open(path, errors='replace') as f:
        for raw in f:
            line = raw.rstrip('\n')
            stripped = line.split('#', 1)[0].strip()
            if in_header and (not line.strip() or line.lstrip().startswith('#')):
                header_lines.append(line)
                continue
            in_header = False
            if stripped:
                names.add(stripped)
    header = '\n'.join(header_lines).rstrip('\n')
    if header:
        header += '\n'
    return names, header


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--rpt',      required=True,  metavar='FILE')
    ap.add_argument('--cov',      required=True,  metavar='FILE')
    ap.add_argument('--output',   required=True,  metavar='FILE')
    ap.add_argument('--fn-files', metavar='FILE', dest='fn_files', default=None,
                    help='function->source map (from gen_cov_files.py) for the file-tree view.')
    ap.add_argument('--omit-file', metavar='FILE', dest='omit_file', default=None,
                    help='committed omit list (scripts/cov_omit_functions); shown in the '
                         'report and seeded into the "Omit" button download.')
    ap.add_argument('--test-cov', action='append', metavar='FILE', default=[],
                    dest='test_covs',
                    help='Per-test cov.txt path; may be repeated. Test name derived from path.')
    args = ap.parse_args()

    funcs    = parse_rpt(args.rpt)
    cov_data = parse_cov(args.cov)
    fn_src   = parse_fn_files(args.fn_files) if args.fn_files else {}
    omit_names, omit_header = parse_omit(args.omit_file) if args.omit_file else (set(), '')

    if not funcs:
        print('cov_html_report: no functions found in rpt file', file=sys.stderr)
        sys.exit(1)

    # Committed omits (scripts/cov_omit_functions) are "just not in the coverage":
    # drop them from funcs entirely so they never appear as a sidebar entry, a
    # call-graph node/edge, per-test data, or a restorable item.  gen_cov_fns.pl
    # already strips them upstream; doing it here too keeps the report self-
    # consistent if it is ever run against a cov.rpt that still lists them.  Only
    # session omits (the in-page "Omit" button) remain restorable.
    if omit_names:
        present = {n for _, n in funcs} & omit_names
        if present:
            funcs = [(p, n) for p, n in funcs if n not in omit_names]
            print(f'[cov_html_report] dropped {len(present)} committed-omit functions',
                  file=sys.stderr)

    func_minimal_packet_length = 1
    trivial = {name for _, name in funcs
               if packet_counts(cov_data.get(name, []))[1] <= func_minimal_packet_length}
    if trivial:
        funcs = [(p, n) for p, n in funcs if n not in trivial]
        print(f'[cov_html_report] filtered {len(trivial)} trivial (<=1 packet) functions',
              file=sys.stderr)

    # load per-test datasets — derive name from path, e.g.
    # .../build/tests/kernel/data/readylist/test/cov.txt → kernel/data/readylist/test
    test_covs = []
    for path in args.test_covs:
        norm = path.replace('\\', '/')
        # strip everything up to and including /tests/
        idx = norm.find('/tests/')
        tname = norm[idx + len('/tests/'):].removesuffix('/cov.txt') if idx >= 0 else path
        test_covs.append((tname, parse_cov(path)))

    with open(args.output, 'w') as f:
        f.write(build_html(funcs, cov_data, test_covs, fn_src, omit_names, omit_header))

    print(f'[cov_html_report] {len(funcs)} functions, {len(test_covs)} tests → {args.output}')


if __name__ == '__main__':
    main()
