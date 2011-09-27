# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

import re
import sys
import os
import glob

# This script removes hyperlinks from items of the form:
#
#     file \hyperlink{_sample_header_file1_8h}{SampleHeaderFile1.h}
#
# New File 09/21/2010 Revision C Version 1 - Same as master file

outfList     = []
usage        = "\n\n     Usage:\n\n          python delpack.py "
count        = 0
previousitem = 0
logFile      = []

custerror    = "No item hyperlinks found."
outfile      = "../batch.log"
outmsg       = "Log file " + outfile + " written to directory above working directory."
errmsg       = "No logfile written. " + custerror

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
          
          # if it's an item
          if re.search("\\item", lineList[i], re.IGNORECASE):
               previousitem = 1
          # and it matches the search string, then go in and delete the hyperlink
          if re.search("file .hyperlink\{.*\}(\{.*\})", lineList[i], re.IGNORECASE) and previousitem:
               logFile.append("deleted hyperlink on line " + str(i+1) + ": " + lineList[i])
               outfList.append(re.sub(".hyperlink\{.*?\}","",lineList[i]))
               previousitem = 0
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
