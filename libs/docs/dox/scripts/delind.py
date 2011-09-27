# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

import re       # regular expressions
import os

# 
# This script comments out indexes in the file refman.tex in the cwd.  Usage: 
# 
#      python delind.py
# 
# A log file is written (sc_refmanlog.txt) one up from the cwd. 
# 
# New File 09/22/2010 Revision C Version 1 - Same as master file

myfile  = "refman.tex"
outFile = []
logFile = []
comcount= 0

custerror    = "No indexes found."
outfile      = "../batch.log"
outmsg       = "Log file " + outfile + " written to directory above working directory."
errmsg       = "No logfile written. " + custerror

# See if you can modify the file
if not os.access(myfile, os.W_OK): 
     print "\n     " + myfile + " is not accessible.  You do not have permission to modify this file."

# FILE IN Open and store file in an array (list)
fin = open(myfile, "r")
lineList = fin.readlines()
fin.close()

# If you see the indicated search patterns, comment out the lines:
for i in range(0,len(lineList)): 
     if re.search("^[^%]\\chapter\{.*?Index\}", lineList[i], re.IGNORECASE):
          outFile.append("%" + lineList[i])
          logFile.append("Commented out " + lineList[i])
          comcount += 1
     
     elif re.search("^[^%]\\input\{.*?\}", lineList[i], re.IGNORECASE) and (re.search("\\chapter\{.*?Index\}", lineList[i-1], re.IGNORECASE)):
          outFile.append("%" + lineList[i])
          logFile.append("Commented out " + lineList[i])
          comcount += 1

     else:
        outFile.append(lineList[i])

# Write the outfile
fou = open(myfile, "w")
for i in range(len(outFile)):
     fou.write(outFile[i])
fou.close

# Clear out outFile
outFile = []

# Put a total in the logfile
logFile.insert(0,"\nCommented out " + str(comcount) + " lines.\n\n")

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
