# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

#
# New File 09/21/2010 Revision C Version 1 - Same as master file
import re
import sys
import os
import glob

usage = "\n\n     Usage:\n\n          python delpack.py [-b]   " + \
          "\n                                                  " + \
          "\n          This script must be run with *tex files " + \
          "\n          in the cwd!                             " + \
          "\n                                                  " + \
          "\n          -b causes the script to run over all    " + \
          "\n          files without prompting the user.       " 

#VARIABLES
prompt = 0

# if args contain -b, turn off the promt, else turn it on
for arg in sys.argv: 
    if re.match("-b", arg):
       prompt = 0
    else:
       prompt = 1

insidef = 0
insidit = 0
enditem = 0
ispack  = 0
beginit = 0
outlist = []
logFile = []
per     = ""
hadpack = 0
exclude = 0

custerror    = "No pack defines found."
outfile      = "../batch.log"
outmsg       = "Log file " + outfile + " written to directory above working directory."
errmsg       = "No logfile written. " + custerror

dirList=glob.glob("*.tex")

if len(dirList) < 1:
    print "\n     There are no *tex files in this directory." + usage
    sys.exit()

print ""

for myfile in dirList:

     #FILE IN
     # Open and store file in an array (list)
     fin = open(myfile, "r")
     lineList = fin.readlines()
     fin.close()
     
     for i in range(0,len(lineList)): 
     
     #TESTS:
      
          # definelist:
     
          if                         re.search("subsection..Defines.", lineList[i], re.IGNORECASE):
                insidef = 1 # We're inside a define list
     
          if insidef and             re.search(".end.CompactItemize.", lineList[i], re.IGNORECASE):
                insidef = 0 # We're not inside a define list anymore
                insidit = 0 # Nor can we be inside an item
     
          # item: 
     
          if insidef and             re.search("^.item",               lineList[i]               ):
                insidit = 1 # We're inside an item
                beginit = i # The item began on this line
     
          # has pack:
                
          if insidef and insidit and re.search("pack",                 lineList[i], re.IGNORECASE):
                ispack  = 1 # The item contains pack info
     
     
          if insidef and insidit and re.search("^$",                   lineList[i], re.IGNORECASE):
                insidit = 0
                enditem = i # The item ends on this line
                if ispack:
                     hadpack = 1
                     if prompt:
                          for k in range(beginit,enditem): 
                               print "> " + lineList[k].rstrip('\n') # If prompt is turned on,
                          per=raw_input('\nDo you wish to delete this item? (y/n):') # prompt the usr to delete the item
                     else:
                          per = "y"                                  # else act as if the user approves all
                     if re.search("n", per):
                          for k in range(beginit,enditem):           # If user chooses n then inclue it
                               outlist.append(lineList[k])
                     else:
                          exclude += 1
                else:
                     for k in range(beginit,enditem): 
                          outlist.append(lineList[k])
                ispack  = 0
     
          if not insidit:
                outlist.append(lineList[i])
     
     #FILE OUT
     # Write the file

     fou = open(myfile, "w")
     for i in range(len(outlist)):
        fou.write(outlist[i])
     fou.close

     outlist = []
     lineList = []

     if hadpack:
          logFile.append(str(exclude) + " pack defines removed in " + myfile + ".\n")
     else:
          logFile.append(str(exclude) + " pack defines removed in " + myfile + ".\n")
     hadpack = 0
     exclude = 0

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


