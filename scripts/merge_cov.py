#!/usr/bin/env python
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear


"""
Usage:

Insert informative documentation  here.

"""
import getopt
import sys
import re

# group(1) is PC, group(2) is name
function_patt = re.compile("(\w+)\s+\<(\w+)\>:")
covdata_patt = re.compile("\**\s+(\w+\s+cycles)*\s+(\w+):\s+(.+)")

class function_data(object):

   def __init__(self,name):
      self.name = name
      self.label = None
      self.text = dict()  #  disassembly lines, not including the function ... label
      self.ccount = dict() 
      self.offset = None
      self.firstfile = None

   def sprint(self,indent=""):
      output = ""
      output += self.name + "\n"
      pclist = self.text.keys()
      pclist.sort()
      if len(self.text) == 0:
         output += "No data!\n"
      for pc in pclist:
         ccount = ""
         zero = ""
         if self.ccount.has_key(pc):
            if self.ccount[pc] == 0:
               zero = "**"
            ccount = "%d cycles" % (self.ccount[pc])
         output += "%2s%20s %06x: %s\n" % (zero,ccount, pc,self.text[pc])  
      return output

   def set_offset(self,name,offset):
      if self.name != name:
         print "mismatching function name"
      self.offset = offset

   #  probably should check the CSV for function lengths to make sure they match.
   #  even better, at least store all the parse bits and make sure THOSE match.

   def check_sanity(self):
      pass

   def add_data(self,fh,fn=""):
      mode = "compare"
      if len(self.text) == 0:
         mode = "new"
         self.firstfile = fn
      for line in fh:
         line = line.splitlines()[0] 
         if not line: 
            return
         m = covdata_patt.match(line)
         if not m:
            print "covdata_patt match failed"
            sys.exit(1)
         ccount = None
         if m.group(1):
            ccount = long(m.group(1).replace(" cycles",""),10)
         pc = long(m.group(2),16) - self.offset
         text = m.group(3)

         if text.find("nop") != -1:  #  silently dump all nop lines
            continue

         #print "cycle count = %s, pc = 0x%08x, text = %s" %(ccount, pc, text)
         if ccount != None:
            if not self.ccount.has_key(pc):
               self.ccount[pc] = ccount
            else:
               self.ccount[pc] += ccount
         if mode == "new":
            self.text[pc] = text
         else:
            if not self.text.has_key(pc):
               print "%s:  additional pc (0x%08x) for %s doesn't match first (%s)" % (fn,pc,self.name,self.firstfile)
               return
            #  extended check -- check that the parse bits at least match.
            original_pp = (long(self.text[pc].split()[4],16) >> 14) & 3
            new_pp = (long(text.split()[4],16) >> 14) & 3
            if original_pp != new_pp:
               print "%s:  parse bit mismatch pc (0x%08x) for %s doesn't match first (%s)" % (fn,pc,self.name,self.firstfile)
               return

fdata = dict()  #  dict of function_data keyed by function name

#  Read in desired function list
def read_functions(file):
   for line in file:
      line = line.splitlines()[0]
      if line:
         if not line in fdata.keys():
            fdata[line] = function_data(line)

   #print "target functions " + str(fdata.keys())

def read_covfile(fn):
      fn = fn.splitlines()[0]
      if not fn:
         return

      #  first, find the test.cov_fns in the same 
      #  directory and pull in the functions we should be reading.
      local_fn_list = []
      function_file = fn[:fn.rfind("/")]+"/test.cov_fns"
      function_file = open(function_file,"r")
      for line in function_file:
         line = line.splitlines()[0]
         if line:
            local_fn_list.append(line)
            if not line in fdata.keys():
               fdata[line] = function_data(line)
      function_file.close()

      fh = open(fn,"r")
      if not fh:
         return
      #print "Reading " + fn
      counter = 1
      for line in fh:
         counter += 1
         line = line.splitlines()[0]
         match = function_patt.match(line)
         if match and match.group(2) in local_fn_list:  #  change this to the local function list
            pc = long(match.group(1),16)
            #print "%8d Function found:  %s" % (counter,match.group(2))
            #  add label
            #  start parsing until empty line
            fdata[match.group(2)].set_offset(match.group(2),pc)
            fdata[match.group(2)].add_data(fh,fn)

if __name__ == "__main__":

   try:
      opts, args = getopt.getopt(sys.argv[1:], "", ["help"])

   except getopt.GetoptError:
      # print help information and exit:
      print __doc__
      sys.exit(2)

   for o, a in opts:
      if o in ("-h", "--help"):
         print __doc__
         sys.exit(0)

   #  test.cov_fns is going to be implicit
   #fh = open("test.cov_fns","r")
   #read_functions(fh)

   for fn in sys.stdin:
      read_covfile(fn)

   for func in fdata.keys():
      print fdata[func].sprint()

