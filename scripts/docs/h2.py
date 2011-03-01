# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

import docutils
import re
import string

from docutils import nodes

from sphinx.environment import NoUri
from sphinx.util.compat import Directive, make_admonition

class h2_function_info:
   def __init__(self):
      self.functions = dict()
      pass

   def set_fdata(self,fdata):
      if fdata[0] in self.functions:
         return
      self.functions[fdata[0]] = dict()
      self.functions[fdata[0]]['fdata'] = fdata

   def set_function_property(self,fdata,property,data):
      self.functions[fdata[0]][property] = data

   def dump_debug_functions(self,indent=""):
      output = ""
      for fname in self.functions.keys():
         returnstr = ""
         fdata = self.functions[fname]['fdata']
         output += indent + "static %s %s_debug(%s) {" % (fdata[1],fdata[0],",".join([item[1] + " " + item[0] for item in fdata[2]])) + "\n"
         indent += "\t"
         count = 0;
         for arg in fdata[2]: #  create the argument copies
            output += indent + "%s arg%d = %s;\n" % (arg[1],count,arg[0])
            count += 1
         if fdata[1] != "void":
            output += indent + "%s retval;\n" % (fdata[1])
            returnstr = "retval = "
         if 'iassert' in self.functions[fname]:
           for line in self.functions[fname]['iassert']:
            output += indent + line + "\n"
         output += indent + returnstr + "%s(%s);\n" % (fdata[0],",".join([item[0] for item in fdata[2]]))

         if 'oassert' in self.functions[fname]:
           for line in self.functions[fname]['oassert']:
            output += indent + line + "\n"
         if fdata[1] != "void": 
            output += indent + "return(retval);\n"
         indent = indent[:-1]
         output += indent + "}\n\n"
      return output

def grab_text(node):
   output = ""
   for child in node.children:
      if child.__class__ == docutils.nodes.Text:
         output += child
      else:
         output += grab_text(child)
   return output

def get_arglist(cfunction,declaration):
    arg_pattern = re.compile(r'%s\((.*)\)' % cfunction)
    var_pattern = re.compile(r'(\w+)\Z')

    args=arg_pattern.search(declaration).groups()[0].split(",")
    retval = []
    if len(args) == 1 and args[0].strip() == "void":
       return retval

    for arg in args:
       arg = arg.strip()
       argname = var_pattern.search(arg).groups()[0]
       argtype = var_pattern.sub("",arg).strip()
       retval.append((argname,argtype))
    return retval

def get_function_data(state_machine):
   retval = []
   for sibling in state_machine.node.parent.children:  #  grandiose way of saying "find siblings"
      if sibling.hasattr("desctype") and sibling['desctype'] == "cfunction":
         params=sibling.children[0]
         retval.append(grab_text(params[1]).strip())  #  function name
         #  Todo:  prune "static inline" out of the return type...
         ret_type = grab_text(params[0]).strip()
         if ret_type[:7] == "static ":
            ret_type = ret_type[7:]
         if ret_type[:7] == "inline ":
            ret_type = ret_type[7:]

         retval.append(ret_type)  #  return type
         retval.append(get_arglist(retval[0],sibling.children[0].rawsource))
   return retval

def create_debug_file(docname,doc_data):
   #print "Creating " + docname + "_debug.h"
   f = file(docname+"_debug.h","w")
   f.write("/*  This file is generated from the documentation!  Do not hand edit!  */\n")
   f.write("#include <assert.h>\n");
   f.write(doc_data.dump_debug_functions())
   f.close()

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

	#  Third input is the admonition title string.

        ad = make_admonition(inputassert_node, self.name, [_('Input Assertions')], self.options,
                             self.content, self.lineno, self.content_offset,
                             self.block_text, self.state, self.state_machine)

        fdata = get_function_data(self.state_machine)
        if not hasattr(env, 'h2_functions'):
           env.h2_functions = dict()
        if not env.docname in env.h2_functions.keys():
           env.h2_functions[env.docname] = h2_function_info()

        env.h2_functions[env.docname].set_fdata(fdata)
        env.h2_functions[env.docname].set_function_property(fdata,'iassert',self.content)
           
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

	#  Third input is the admonition title string.
        ad = make_admonition(inputassert_node, self.name, [_('Output Assertions')], self.options,
                             self.content, self.lineno, self.content_offset,
                             self.block_text, self.state, self.state_machine)

        fdata = get_function_data(self.state_machine)
        if not hasattr(env, 'h2_functions'):
           env.h2_functions = dict()
        if not env.docname in env.h2_functions:
           env.h2_functions[env.docname] = h2_function_info()

        env.h2_functions[env.docname].set_fdata(fdata)
        env.h2_functions[env.docname].set_function_property(fdata,'oassert',self.content)

        return [targetnode] + ad

def process_h2_nodes(app, doctree, fromdocname):
   env=app.builder.env

   for docname in env.h2_functions.keys():
      create_debug_file(docname,env.h2_functions[docname])
      del env.h2_functions[docname]

def purge_h2_nodes(app, env, docname):
   if not hasattr(env,'h2_functions'):
      return

def visit_inputassert_node(self, node):
    self.visit_admonition(node)

def depart_inputassert_node(self, node):
    self.depart_admonition(node)

def setup(app):
    app.add_config_value('use_h2_ext', True, False)

    app.add_node(inputassert_node,
                 html=(visit_inputassert_node, depart_inputassert_node),
                 latex=(visit_inputassert_node, depart_inputassert_node),
                 text=(visit_inputassert_node, depart_inputassert_node))

    app.add_directive('inputassert', InputAssertDirective)
    app.add_directive('outputassert', OutputAssertDirective)
    app.connect('doctree-resolved', process_h2_nodes)
    app.connect('env-purge-doc', purge_h2_nodes)

