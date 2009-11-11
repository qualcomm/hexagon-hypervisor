# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

from docutils import nodes

from sphinx.environment import NoUri
from sphinx.util.compat import Directive, make_admonition

class inputassert_node(nodes.Admonition, nodes.Element): pass
class outputassert_node(nodes.Admonition, nodes.Element): pass

class InputAssertDirective(Directive):
    has_content = True
    required_arguments = 0
    optional_arguments = 0
    final_argument_whitespace = False
    option_spec = {}

    def run(self):
        env = self.state.document.settings.env

        targetid = "inputassert-%s" % env.index_num
        env.index_num += 1
        targetnode = nodes.target('', '', ids=[targetid])

	#  Great docs.  Third input is the admonition title string.
        ad = make_admonition(inputassert_node, self.name, [_('Input Assertions')], self.options,
                             self.content, self.lineno, self.content_offset,
                             self.block_text, self.state, self.state_machine)

        if not hasattr(env, 'blast_nodes'):
            env.blast_nodes = []
        env.blast_nodes.append({
            'docname': env.docname,
            'lineno': self.lineno,
            'inputassert': ad[0].deepcopy(),
            'target': targetnode,
        })

        return [targetnode] + ad

class OutputAssertDirective(Directive):
    has_content = True
    required_arguments = 0
    optional_arguments = 0
    final_argument_whitespace = False
    option_spec = {}

    def run(self):
        env = self.state.document.settings.env

        targetid = "outputassert-%s" % env.index_num
        env.index_num += 1
        targetnode = nodes.target('', '', ids=[targetid])

	#  Great docs.  Third input is the admonition title string.
        ad = make_admonition(inputassert_node, self.name, [_('Output Assertions')], self.options,
                             self.content, self.lineno, self.content_offset,
                             self.block_text, self.state, self.state_machine)

        if not hasattr(env, 'blast_nodes'):
            env.blast_nodes = []
        env.blast_nodes.append({
            'docname': env.docname,
            'lineno': self.lineno,
            'outputassert': ad[0].deepcopy(),
            'target': targetnode,
        })

        return [targetnode] + ad

def process_blast_nodes(app, doctree, fromdocname):
    if not app.config['use_blast_ext']:
        for node in doctree.traverse(inputassert_node):
            node.parent.remove(node)

def purge_blast_nodes(app, env, docname):
    if not hasattr(env, 'blast_nodes'):
        return
    env.blast_nodes = [item for item in env.blast_nodes
                          if item['docname'] != docname]

def visit_inputassert_node(self, node):
    self.visit_admonition(node)

def depart_inputassert_node(self, node):
    self.depart_admonition(node)

def setup(app):
    app.add_config_value('use_blast_ext', True, False)

    app.add_node(inputassert_node,
                 html=(visit_inputassert_node, depart_inputassert_node),
                 latex=(visit_inputassert_node, depart_inputassert_node),
                 text=(visit_inputassert_node, depart_inputassert_node))

    app.add_directive('inputassert', InputAssertDirective)
    app.add_directive('outputassert', OutputAssertDirective)
    app.connect('doctree-resolved', process_blast_nodes)
    app.connect('env-purge-doc', purge_blast_nodes)

