# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

#
# New File 09/21/2010 Revision C Version 1 - Same as master file
import re
import sys
import os
import glob

# This script removes different types of "mbox" tags in all tex files in the current directory.
# The types include [static], [virtual], [private], [inherited], [read], and [write] and are of the form:
#
#      {\tt \mbox {[}<bracket word here>\mbox {]}}
#

outfList     = []
usage        = "\n\n     Usage:\n\n          python delpack.py "
searchstring = "\{.tt.*\s*.mbox\{\[\}static.mbox\{\]\}\}|" + \
               "\{.tt.*\s*.mbox\{\[\}inherited.mbox\{\]\}\}|" + \
               "\{.tt.*\s*.mbox\{\[\}read.mbox\{\]\}\}|"+ \
               "\{.tt.*\s*.mbox\{\[\}write.mbox\{\]\}\}"
#              "\{.tt.*\s*.mbox\{\[\}virtual.mbox\{\]\}\}|" + \
#              "\{.tt.*\s*.mbox\{\[\}private.mbox\{\]\}\}|" + \
count        = 0
found        = 0

custerror    = "No item hyperlinks found."
outfile      = "../batch.log"
outmsg       = "Log file " + outfile + " written to directory above working directory."
errmsg       = "No logfile written. " + custerror
logFile      = []

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
          if re.search(searchstring, lineList[i], re.IGNORECASE):
                if not found: 
                     found = 1 # set found test to one...we found at least one
                     logFile.append("") # this just makes space for the stdout lines
                outfList.append(re.sub(searchstring,"",lineList[i]))
                count+=1
                logFile.append("Deleted mbox from file " + myfile + " (" + str(count) + ")") 
          else:
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

if found:
     logFile.append("\n" + str(count) + " mbox lines found.")
else: 
     logFile.append("\nNo mbox lines found.")


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
