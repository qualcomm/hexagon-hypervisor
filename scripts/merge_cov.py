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
nopcfunc_patt = re.compile("(\w+):")
function_patt = re.compile("(\w+)\s+(\w+):")
covdata_patt = re.compile("\**\s+(\w+\s+(cycles|0))*\s+(\w+):\s+(.+)")
covdata_patt2 = re.compile("\**\s+(\w+)\s+(1.0|0.0)*\s+(\w+):\s+(.+)")
skip_patt = re.compile('No data!|--\sOut\sof\srange\s--.*')
nop_patt = re.compile('.*\{\s+(\w+)')
packet_start_patt = re.compile('.*\{')
packet_end_patt = re.compile('.*\}')

do_offsets = 0

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
      if self.offset:
         output += "%08x <" %(self.offset)
      else:
         output += "0x00000000 <"
      output += self.name + ">:\n"
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
         match = nop_patt.match(self.text[pc])
         if match.group(1) == "nop" and zero == "**":
            output += "\n"
         else:
            output += "%2s%20s %08x: %s\n" % (zero,ccount, pc,self.text[pc])  
      return output

   def set_offset(self,name,offset):
      # print "Set offset %s %08x" % (name, offset)
      if self.name != name:
         print "mismatching function name"
      self.offset = offset

   #  probably should check the CSV for function lengths to make sure they match.
   #  even better, at least store all the parse bits and make sure THOSE match.

   def check_sanity(self):
      pass

   def add_data(self,fh,fn=""):
      mode = "compare"
      skip_packet = 0;

      if len(self.text) == 0:
         mode = "new"
         self.firstfile = fn
      for line in fh:
         line = line.splitlines()[0] 
         # print "LINE: %s" % (line)
         if not line: 
            return
         if skip_packet:
            if packet_end_patt.match(line):
               skip_packet = 0
            continue
         m = skip_patt.match(line)
         if m:
            if packet_start_patt.match(line):  # found '{'
               if not packet_end_patt.match(line): # packet ends on subsequent line
                  skip_packet = 1;
            continue
         m = covdata_patt.match(line)
         if not m:
            m = covdata_patt2.match(line)
            #print "covdata_patt match failed"
            #sys.exit(1)
         ccount = None
         if m and m.group(1):
            countstr = m.group(1).replace(" cycles","")
            countstr = countstr.replace(" 0", "")
            ccount = long(countstr, 10)
         # FIXME: hexagon-cov sometimes fails to mark instructions as -- Out of
         # range -- when they in fact are.  If there is no annotation, assume
         # for now that it's out of range.
         else:
            continue
         # print "XXX %s %08x" %(m.group(3), self.offset)
         pc = long(m.group(3),16)
         if (do_offsets):
            pc -= self.offset
         text = m.group(4)

         #if text.find("nop") != -1:  #  silently dump all nop lines
            #continue

         if text.find("unknown") != -1:  #  silently dump all 0-content lines
            continue

         # print "cycle count = %s, pc = 0x%08x, text = %s" %(ccount, pc, text)
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
            #  Todo:  make sure cycles are only reported for instructions at start of packet, or single instruction packets.
            original_pp = (long(self.text[pc].split()[4],16) >> 14) & 3
            new_pp = (long(text.split()[4],16) >> 14) & 3
            if original_pp != new_pp:
               print "%s:  parse bit mismatch pc (0x%08x) for %s doesn't match first (%s)" % (fn,pc,self.name,self.firstfile)
               return

fn_list = []
fdata = dict()  #  dict of function_data keyed by function name

#  Read in desired function list
def read_functions(file):
   for line in file:
      line = line.splitlines()[0]
      if line:
         if not line in fdata.keys():
            fn_list.append(line)
            fdata[line] = function_data(line)
   file.close()

def get_veropt(file):
   ver_patt = re.compile("(v\d+)\s(ref|opt)")
   for line in file:
      match = ver_patt.match(line)
      if match:
         return (match.group(1),match.group(2))
      else:
         sys.exit("No version information found for compiled h2")

def read_covfile(fn):
      fn = fn.splitlines()[0]
      if not fn:
         return

      fh = open(fn,"r")
      if not fh:
         return
      #print "Reading " + fn

      #  first, find the test.cov_fns in the same 
      #  directory and pull in the functions we should be reading.
      ignore_fn_list = []
      ignore_file = fn[:fn.rfind("/")]+"/ignore_fns"
      ignore_file = open(ignore_file,"r")
      for line in ignore_file:
         line = line.splitlines()[0]
         if line:
            ignore_fn_list.append(line)
            # if not line in fdata.keys():
            #    fdata[line] = function_data(line)
      ignore_file.close()

      ftemp = fn+"_temp"  # copy the test file to temporary file
      fhtmp = open(ftemp,"w+")
      if not fhtmp:
         return
      for line in fh:
         currline = line.splitlines()[0]
         match = nopcfunc_patt.match(currline)
         if match and match.group(1) in fn_list and match.group(1) not in ignore_fn_list:
            nextline = next(fh).rstrip("\n")
            m = covdata_patt.match(nextline)
            if not m:
              m = covdata_patt2.match(nextline)
              if m:
                pc = str(hex(long(m.group(3),16))).lstrip("0x").rstrip("L")
                currline = pc + " " + currline + "\n" + nextline
         currline = currline + "\n"
         fhtmp.write(currline)
      fhtmp.close()
      fh.close()
      fh = open(ftemp,"r") # then operate on temporary file
      if not fh:
         return
      counter = 1
      for line in fh:
         counter += 1
         line = line.splitlines()[0]
         match = function_patt.match(line)
         if match and match.group(2) in fn_list and match.group(2) not in ignore_fn_list:
            pc = long(match.group(1),16)
            # print "%8d Function found:  %s" % (counter,match.group(2))
            #  add label
            #  start parsing until empty line
            fdata[match.group(2)].set_offset(match.group(2),pc)
            fdata[match.group(2)].add_data(fh,fn)
      fh.close()

if __name__ == "__main__":

   try:
      opts, args = getopt.getopt(sys.argv[1:], "", ["help", "dir=", "offset_pc"])

   except getopt.GetoptError:
      # print help information and exit:
      print __doc__
      sys.exit(2)

   h2dir = "."

   for o, a in opts:
      if o in ("-h", "--help"):
         print __doc__
         sys.exit(0)

      if o in ("-d", "--dir"):
         h2dir = a

      if o in ("-o", "--offset_pc"):
         do_offsets = 1

   # Which style/version are we working on
   fh = open(h2dir+"/install/ver","r")
   ver,opt = get_veropt(fh)

   fname = h2dir+"/scripts/"+ver+opt+"_cov_fns"

   #  cov_fns is going to be explicit
   fh = open(fname,"r")
   read_functions(fh)

   for fn in sys.stdin:
      read_covfile(fn)

   print ver+" "+opt
   for func in fdata.keys():
      print fdata[func].sprint()

