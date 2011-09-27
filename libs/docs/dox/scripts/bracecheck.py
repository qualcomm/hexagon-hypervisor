# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

#
# New File 10/27/2010 Revision C Version 2 for Doxygen 1.7.0 - Changed 
# "..\batch.log" to "../batch.log"

from __future__ import division  # necessary to make integer division work
import re                        # regular expressions
import sys
import os
import glob

# 
# This app prints out all lines in a header file that are not contained within 
# section braces (@{ ... @}).
# 
# It operates over all .h files in the current directory.  There are no flags.  
# Usage:
# 
#      python bracecheck.py
# 
# The output of this script is stored in a file called bracecheck.txt
# 
# Note that this operates on files even if braces are mismatched, but it warns 
# the user in the logfile that there are mismatches
# 
# In cases where the braces are mismatched, the script prints any lines that 
# appear after a section close with a matching open brace before.
# 

usage        = "\n\n     Usage:\n\n          python bracecheck.py "
logFile      = []
dirList      = []
openbrace    = 0 
closebrace   = 0 
opencount    = 0
closecount   = 0
titleborder  = "-------------------------------------------------------------------------\n"

custerror    = "No marker mismatches."
outfile      = "../batch.log"
outmsg       = "Log file " + outfile + " written to directory above working directory."
errmsg       = "No logfile written. " + custerror

dirList=glob.glob("*.h")

if len(dirList) < 1:
     print "\n     There are no header files in this directory." + usage
     sys.exit()

for myfile in dirList:

     # See if you can modify the file
     if not os.access(myfile, os.R_OK): 
          print "\n     " + myfile + " is not accessible.  You do not have " + \
                "permission to modify this file."
          continue
     # FILE IN Open and store file in an array (list)
     fin = open(myfile, "r")
     lineList = fin.readlines()
     fin.close()

     logFile.append(titleborder)
     logFile.append("TESTING " + myfile + " for non-section lines.\n")
     logFile.append(titleborder)

     for i in range(0,len(lineList)): 

               if re.search("@{", lineList[i], re.IGNORECASE):
                    openbrace   += 1
                    opencount   +=1

               # each open brace zeroes out the closebrace var, so we start fresh
               if re.search("@{", lineList[i], re.IGNORECASE) and closebrace > openbrace:
                    closebrace = 0
                    openbrace  = 1

               if re.search("@}", lineList[i], re.IGNORECASE):
                    closebrace += 1
                    closecount  +=1
          
               if openbrace - closebrace <= 0 and not re.search("@{|@}", lineList[i], re.IGNORECASE):
                    lineList[i] = lineList[i].rstrip("\n")
                    lineList[i] = lineList[i].rstrip("\r")
                    logFile.append(lineList[i] + "\n")

     # Clear out the linelist before proceeding to the next file
     lineList = []

     # Check for mismatches between open and close braces, warn user if modulo is TRUE
     if (opencount + 1) % (closecount + 1):
          logFile.insert(0,"ERROR!!:\n" + myfile + " contains unmatched markers (@{ ... @}): " + \
          str(opencount) + " open markers and " + str(closecount) + " closed markers.\n")
     opencount  = 0
     closecount = 0

# If batch.log doesn't exist, add it
if not os.path.exists(outfile):
    open(outfile, "a")

# Then, if the logFile has something in it, write it
if len(logFile) > 0:
     fou = open(outfile, "a")
     for i in range(len(logFile)):
          fou.write(logFile[i])
     fou.close
     print outmsg
# Else, tell the user there was nothing
else:
     print errmsg

# Clear out logFile
logFile = []
