# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

#
# New File LBoytes - 01/21/2011 Revision A Version 1 for Doxygen 1.7.0
#

import re
import sys
import os
import glob

# This script replaces the "Module Index" chapter title to "Interface Index", renames
# the "Modules" heading to "Interfaces", renames the "Modules" subsection heading to
# "Interfaces (if one exists); and renames "Here is a list of all modules:" to
# "Here is a list of all interfaces:"

outfList        = []
usage           = "\n\n     Usage:\n\n          python rename_module_index.py "
module_ind      = "\chapter{Module Index}" #Interfaces chapter title search"
modules         = "section{Modules}" #Renames "Modules" heading to "Interfaces" heading "
hereis         = "Here is a list of all modules:" #Change to "Here is a list of all interfaces:"
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
          # search each line for the "Module Documentation" search string and replace with "Interfaces Index"
          lineList[i]=re.sub(module_ind,"chapter{Interfaces Index}",lineList[i])
          # search each ine for the "Modules" heading search string and replace with "Interfaces"
          lineList[i]=re.sub(modules,"section{Interfaces}",lineList[i])
          # search each line for the string "Here is a list of all modules:" and replace with "Here is a list of all interfaces:"
          lineList[i]=re.sub(hereis,"Here is a list of all interfaces:",lineList[i])    
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

