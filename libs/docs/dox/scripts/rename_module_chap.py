# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

#
# New File LBoyters - 01/07/2011 Revision A Version 1 for Doxygen 1.7.0
#

import re
import sys
import os
import glob

# This script replaces the "Module Documentation" chapter title to "Interfaces" and replaces
# the "Modules" unnumbered heading to "Interfaces".

outfList        = []
usage           = "\n\n     Usage:\n\n          python rename_module_chap.py "
module_chap     = "\chapter{Module Documentation}" #Module Documentation chapter title search
interface_chap  = "\chapter{Interfaces}" #Replaces "Module Documentation" chapter title to "Interfaces" 
subsec_modules  = "{Modules}" #Replaces subsection "Modules" to "Interfaces"
count           = 0

dirList=glob.glob("*.tex")

if len(dirList) < 1:
    print "\n     There is no tex file in this directory with this usage." + usage
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
          # search each line for the "Module Documentation" chapter title search string and replace with
          # "Interfaces" chapter title
          lineList[i]=re.sub(module_chap,"chapter{Interfaces}",lineList[i])
          # search each line for "Modules" unnumbered heading and replace with "Interfaces" unnumbered heading
          lineList[i]=re.sub(subsec_modules,"{Interfaces}",lineList[i])
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

