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

"""

000017f0 <H2K_check_sanity_unlock>:
         2 cycles           17f0:       02 c0 9d a0     a09dc002     allocframe (#16)
         2 cycles           17f4:       1a 40 01 f5     f501401a     { r27:26 = combine (r1, r0)
                            17f8:       ff fa de a7     a7defaff       memd (r30 + #-8) = r27:26 }
         2 cycles           17fc:       52 59 00 5a     5a005952     { call 4aa0 <checker_kernel_locked>
                            1800:       fe f8 de a7     a7def8fe       memd (r30 + #-16) = r25:24 }
         2 cycles           1804:       30 40 80 61     61804030     { if (r0 == #0) jump:nt 1864 <H2K_check_
                            1808:       01 c0 00 78     7800c001       r1 = #0 }
         2 cycles           180c:       00 5a 1b f5     f51b5a00     { r1:0 = combine (r27, r26)
                            1810:       92 ff ff 5b     5bffff92       call 1730 <H2K_check_sanity> }
         2 cycles           1814:       18 40 01 f5     f5014018     { r25:24 = combine (r1, r0)
                            1818:       46 d9 00 5a     5a00d946       call 4aa0 <checker_kernel_locked> }
         2 cycles           181c:       1c 40 80 61     6180401c     { if (r0 == #0) jump:nt 1854 <H2K_check_
                            1820:       01 c0 00 78     7800c001       r1 = #0 }
         2 cycles           1824:       00 40 20 72     72204000     { r0.h = #0
                            1828:       00 5a 98 d2     d2985a00       p0 = cmp.eq (r25:24, r27:26)
                            182c:       01 c0 00 78     7800c001       r1 = #0 }
         2 cycles           1830:       68 63 e0 71     71e06368     { r0.l = #58216
                            1834:       06 c0 00 5c     5c00c006       if (p0) jump 183c <H2K_check_sanity_un
**       0 cycles           1838:       bc e5 00 5a     5a00e5bc     call 63b0 <_Assert>
         2 cycles           183c:       80 c0 20 6c     6c20c080     k0unlock
         2 cycles           1840:       00 5a 1b f5     f51b5a00     { r1:0 = combine (r27, r26)
                            1844:       d8 7f de 97     97de7fd8       r25:24 = memd (r30 + #-16)
                            1848:       fa ff de 97     97defffa       r27:26 = memd (r30 + #-8) }
         2 cycles           184c:       1e c0 1e 90     901ec01e     deallocframe
         2 cycles           1850:       00 c0 9f 52     529fc000     jumpr r31
**       0 cycles           1854:       00 c0 20 72     7220c000     r0.h = #0
**       0 cycles           1858:       98 63 e0 71     71e06398     { r0.l = #58264
                            185c:       ac e5 00 5a     5a00e5ac       call 63b0 <_Assert> }
**       0 cycles           1860:       e2 ff ff 59     59ffffe2     jump 1824 <H2K_check_sanity_unlock+0x34>
**       0 cycles           1864:       00 c0 20 72     7220c000     r0.h = #0
**       0 cycles           1868:       d0 63 e0 71     71e063d0     { r0.l = #58320
                            186c:       a4 e5 00 5a     5a00e5a4       call 63b0 <_Assert> }
**       0 cycles           1870:       ce ff ff 59     59ffffce     jump 180c <H2K_check_sanity_unlock+0x1c>
**       0 cycles           1874:       00 40 00 7f     7f004000     { nop
                            1878:       00 40 00 7f     7f004000       nop
                            187c:       00 c0 00 7f     7f00c000       nop }

"""

class function_data(object):

   def __init__(self,name):
      self.name = name
      self.label = None
      self.text = dict()  #  disassembly lines, not including the function ... label
      self.ccount = dict()  #
      self.offset = None

   def sprint(self,indent=""):
      pclist = self.text.keys()
      pclist.sort()
      for pc in pclist:
         print "%s" % (self.text[pc])  

   def set_offset(self,name,offset):
      if self.name != name:
         print "error"
      self.offset = offset

   def check_sanity(self):
      pass

   def add_data(self,fh):
      mode = "compare"
      if len(self.text) == 0:
         mode = "new"
      for line in fh:
         #print line
         line = line.splitlines()[0] 
         if not line: 
            return
         m = covdata_patt.match(line)
         if not m:
            print "What the dilly, yo?!"
            sys.exit(1)
         ccount = m.group(1)
         pc = long(m.group(2),16) - self.offset
         text = m.group(3)

         print "cycle count = %s, pc = 0x%08x, text = %s" %(ccount, pc, text)
         if ccount:
            if not self.ccount.has_key(pc):
               self.ccount[pc] = ccount
            else:
               self.ccount[pc] += ccount
         if mode == "new":
            self.text[pc] = text
#         else:
            #if self.text[pc] != text:
#               print "What the dilly, yo?!!?!?!"

fdata = dict()  #  dict of function_data keyed by function name

#  Read in desired function list
def read_functions(file):
   for line in file:
      line = line.splitlines()[0]
      if line:
         if not line in fdata.keys():
            fdata[line] = function_data(line)

   print "target functions " + str(fdata.keys())

def read_covfile(fn):
      fn = fn.splitlines()[0]
      if not fn:
         return
      fh = open(fn,"r")
      if not fh:
         return
      print "Reading " + fn
      counter = 1
      for line in fh:
         counter += 1
         line = line.splitlines()[0]
         match = function_patt.match(line)
         if match and match.group(2) in fdata.keys():
            pc = long(match.group(1),16)
            print "%8d Function found:  %s" % (counter,match.group(2))
            #  add label
            #  start parsing until empty line
            fdata[match.group(2)].set_offset(match.group(2),pc)
            fdata[match.group(2)].add_data(fh)

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
   fh = open("test.cov_fns","r")
   read_functions(fh)

   for fn in sys.stdin:
      read_covfile(fn)

   for func in fdata.keys():
      print fdata[func].sprint()

