# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

#
# New File - 09/21/2010 Revision C Version 1 for Doxygen 1.7.0 - Same as master file

import re
import sys
import os
import glob

# This script replaces escaped double or triple dashes with en- or em-dashes, respectively

outfList     = []
usage        = "\n\n     Usage:\n\n          python delpack.py "
endash       = " -\\\/-\\\/ "       #en dash search
emdash       = " -\\\/-\\\/-\\\/ "  #em dash search
count        = 0

dirList=glob.glob("*.tex")

if len(dirList) < 1:
    print "\n     There are no *tex files in this directory." + usage
    sys.exit()

for myfile in dirList:

     # First, see if you can modify the file
     if not os.access(myfile, os.W_OK): 
          print "\n     " + myfile + " is not accessible.  You do not have permission to modify this file."
          continue
     # FILE IN Open and store file in an array (list)
     fin = open(myfile, "r")
     lineList = fin.readlines()
     fin.close()
          
     for i in range(0,len(lineList)): 
          # search each line for the stat search string
          lineList[i]=re.sub(endash," -- ",lineList[i])
          lineList[i]=re.sub(emdash," --- ",lineList[i])
          outfList.append(lineList[i])
          
     # FILE OUT
     # Write the file
     fou = open(myfile, "w")
     for i in range(len(outfList)):
        fou.write(outfList[i])
     fou.close

     # Clear arrays
     lineList = []
     outfList  = []

# Clear out logFile
logFile = []

