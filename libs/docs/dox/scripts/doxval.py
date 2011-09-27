# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

#
# New File 09/22/2010 Revision C Version 1 - Same as master file
from __future__ import division  # necessary to make integer division work
import re                        # regular expressions
import sys
import os
import glob
import datetime

# 
# This app validates all .h header files by checking for various syntax errors in Doxygen commands.
# 
# It operates over all .h files in the current directory.  There are no flags.  Usage:
# 
#      python doxval.py
# 
# A file doxlog.txt is produced one up from the cwd, containg all warnings and errors snd their 
# line numbers 
# 

warning      = "WARNING: "
add          = ""
error        = "ERROR:   "
usage        = "\n\n     Usage:\n\n          python doxval.py "
spacer       = "     "
change       = "\n>" + spacer + "change: "
to           = ">" + spacer + "    to: "
logFile      = []
warnum       = 0
errnum       = 0
hasfile      = {} # a dictionary
incomment    = 0
okay         = 0
openbracket  = 0 
ignorestars  = 0 
closebracket = 0 
vicount      = 0
vifixed      = 0
rule1        = 0
rule2        = 0
rule3        = 0
opencond     = 0
closecond    = 0
openlatex    = 0
closelatex   = 0
openhtml     = 0
closehtml    = 0
openverbatim = 0
closeverbatim= 0
delineator   = "--------------------------------------"
tagerror     = ">\n> Found /*!.  This should be /**, for consistency."
trueerror    = ">\n> Found lowercase true.  TRUE should always be uppercase."
falseerror   = ">\n> Found lowercase false.  FALSE should always be uppercase."
returnerror  = ">\n> Found @retval <argument>.  This should be replaced by @return."
linefixed    = "\nLine fixed.\n"
lineunchanged= "\nLine unchanged.\n"
unused       = "\n\n     @class\n     @const\n     @define\n     @enum\n     @include\n" + \
               "     @mainpage   info goes in overview\n     @namespace\n     @retval     use return instead\n     @struct\n     @typedef\n     @union\n"
hasstars     = 0
hasaddtogroup= 0

now          = datetime.datetime.now()
mytime       = "DATE: " + str(now.month) + "/" + str(now.day) + "/" + str(now.year) + " " + str(now.hour) + ":" + str(now.minute) + ":" + str(now.second)
custerror    = "No rule violations found."
outfile      = "../batch.log"
outmsg       = "\nRule violations have been found.  See batch.log."
errmsg       = "\nNo logfile written. " + custerror
dirList      = glob.glob("*.h")

logFile.insert(0,"\n-----------------------------PER LINE WARNINGS----------------------------------\n\n")


def validateme (msgtype, errormsg, lineum, filename, codeline):

          if re.search("warn", msgtype, re.IGNORECASE): 
               global warnum
               warnum += 1
          if re.search("erro", msgtype, re.IGNORECASE): 
               global errnum
               errnum += 1

          logFile.append(filename + ": line " + str(lineum + 1) + "\n")
          logFile.append(spacer + msgtype + errormsg + "\n")
          logFile.append(spacer + codeline + "\n")

def promptuser(message):
     global user
     user = ""
     while not re.match("^\s*y\s*$|^\s*n\s*$|^\s*yes\s*$|^\s*no\s*$|^\s*q\s*$|^\s*quit\s*$|^\s*exit\s*$",user,re.IGNORECASE):
          user=raw_input(spacer + message + "\n>\n> Approve? (y/n): ")

def givecontext(lineList,i,myfile):
     print "(" + myfile + ")" + "\n" + delineator 
     print "..."
     for n in range(-5,5):
         if n==0:
            add=">>> "
         else:
            add=""
         if not ( i+n<0 ) and not ( (i+n)>=len(lineList) ):
              print add + lineList[i+n].rstrip() 
     print "..."
     print delineator

def fixit(wrong,right,fixedline):
     fixedline=re.sub(wrong,right,fixedline)
     return fixedline

if len(dirList) < 1:
     print "\n     There are no header files in this directory." + usage
     sys.exit()

for myfile in dirList:

     # See if you can modify the file
     if not os.access(myfile, os.W_OK): 
          print "\n     " + myfile + " is not accessible.  You do not have permission to modify this file."
          continue
     # FILE IN Open and store file in an array (list)
     fin = open(myfile, "r")
     lineList = fin.readlines()
     fin.close()

     # Interactive mode 
     if len(sys.argv) > 1 and (sys.argv[1] == "-i" or sys.argv[1] == "-user_input"):
 
          # Rule violation gauntlet, through which each line must pass
          i = 0
          while i < len(lineList): 
             if re.search("\/\*\*|\/\/\/|\/\*\!|\/\*\<|\/\*\>", lineList[i], re.IGNORECASE):
                  incomment = 1
     
             if incomment:
                  # Enforce /**, not /*!
                  if re.search("\/\*\!",lineList[i]):
                     vicount+=1
                     givecontext(lineList,i,myfile)
                     fixedline=fixit("\/\*\!","/**",lineList[i])
                     promptuser(change + lineList[i] + to + fixedline + tagerror)
          
                     if re.match("^\s*y\s*$|^\s*yes\s*$", user,re.IGNORECASE):
                          lineList[i]=fixedline
                          print linefixed
                          vifixed+=1
                     if re.match("^\s*n\s*$|^\s*no\s*$", user,re.IGNORECASE):
                          print lineunchanged
                     user=""
          
                  # Enforce uppercase TRUE
                  if re.search("true",lineList[i]):
                     vicount+=1
                     givecontext(lineList,i,myfile)
                     fixedline=fixit("true","TRUE",lineList[i])
                     promptuser(change + lineList[i] + to + fixedline + trueerror)
          
                     if re.match("^\s*y\s*$|^\s*yes\s*$", user,re.IGNORECASE):
                          lineList[i]=fixedline
                          print linefixed
                          vifixed+=1
                     if re.match("^\s*n\s*$|^\s*no\s*$", user,re.IGNORECASE):
                          print lineunchanged
                     user=""
          
                  # Enforce uppercase FALSE
                  if re.search("false",lineList[i]):
                     vicount+=1
                     givecontext(lineList,i,myfile)
                     fixedline=fixit("false","FALSE",lineList[i])
                     promptuser(change + lineList[i] + to + fixedline + falseerror)
          
                     if re.match("^\s*y\s*$|^\s*yes\s*$", user,re.IGNORECASE):
                          lineList[i]=fixedline
                          print linefixed
                          vifixed+=1
                     if re.match("^\s*n\s*$|^\s*no\s*$", user,re.IGNORECASE):
                          print lineunchanged
                     user=""
          
                  # Enforce @return instead of @retval <argument>
                  if re.search("@retval\s+\w*?\s+",lineList[i]):
                     vicount+=1
                     givecontext(lineList,i,myfile)
                     fixedline=fixit("@retval\s+\w*?\s+","@return ",lineList[i])
                     promptuser(change + lineList[i] + to + fixedline + returnerror)
          
                     if re.match("^\s*y\s*$|^\s*yes\s*$", user,re.IGNORECASE):
                          lineList[i]=fixedline
                          print linefixed
                          vifixed+=1
                     if re.match("^\s*n\s*$|^\s*no\s*$", user,re.IGNORECASE):
                          print lineunchanged
                     user=""
          
                  # Look for lines beginning with " * "
                  if re.search("^\s*\*\s+",lineList[i]) and not ignorestars:
                     vicount+=1
                     givecontext(lineList,i,myfile)
                   # fixedline=fixit("@retval\s+\w*?\s+","@return ",lineList[i])
                     promptuser("\n> Comment lines should not begin with a * -- Change all?")
          
                     if re.match("^\s*y\s*$|^\s*yes\s*$", user,re.IGNORECASE):
                          for x in range(len(lineList)):
                               if re.search("\/\*\*|\/\/\/|\/\*\!|\/\*\<|\/\*\>", lineList[x], re.IGNORECASE): okay = 1
                               if okay: lineList[x]=re.sub("^\s*\*\s+","",lineList[x])
                               if re.search("\*\/", lineList[x], re.IGNORECASE): okay = 0
                          print linefixed
                          vifixed+=1
                     if re.match("^\s*n\s*$|^\s*no\s*$", user,re.IGNORECASE):
                          ignorestars=1
                          print lineunchanged
                     user=""
     
             if re.search("\*\/", lineList[i], re.IGNORECASE):
                  incomment = 0
     
             i += 1 

          print myfile + ": " + str(vicount) + " violations found in interactive mode. " + str(vifixed) + " violations fixed."
          vicount = 0
          vifixed = 0

          fou = open(myfile, "w")
          for i in range(len(lineList)):
              fou.write(lineList[i])
          fou.close

     ignorestars=0
     lineList=[]
     incomment=0

for myfile in dirList:

     # FILE IN Open and store file in an array (list)
     fin = open(myfile, "r")
     lineList = fin.readlines()
     fin.close()

     # Validations:
     for i in range(0,len(lineList)): 
     #    Test whether you're beginning a comment
          if re.search("\/\*\*|\/\/\/|\/\*\!|\/\*\<|\/\*\>", lineList[i], re.IGNORECASE):
              incomment = 1

          if incomment:
 # Val rule 01 /// is discouraged, so warn the user
               if re.search("\/\/\/", lineList[i], re.IGNORECASE):
                    validateme(warning, "Invalid Doxygen comment syntax \"///\".  Use /** ... */.", i, myfile, lineList[i])
     
 # Val rule 02 /*! is discouraged, so warn the user
               if re.search("\/\*\!", lineList[i], re.IGNORECASE):
                    validateme(warning, "Invalid Doxygen comment syntax \"/*!\".  Use /** ... */.", i, myfile, lineList[i])
     
 # Val rule 03 Direction for all params is required
               if re.search("@param\s+", lineList[i], re.IGNORECASE):
                    validateme(error, "Space after @param command.  All params must have direction and the direction " + \
                    "must\n" + spacer + "         immediately follow @param (e.g., @param[in]).", i, myfile, lineList[i])
 # Val rule 04 @file param must be present and @file argument must match actual filename
               if re.search("@file", lineList[i], re.IGNORECASE):
               # a  first check to see whether we've already seen an @file command
                    if myfile in hasfile:
                         validateme(error, "Duplicate @file in " + myfile, i, myfile, lineList[i])
                         continue
               # b  make an entry in the dictionary showing there is an @file command for this file
                    hasfile[myfile] = 1
               # c  snip out the filename
                    thefile = re.sub("@file","",lineList[i])
               # d  remove all whitespace
                    thefile = re.sub('\s', '', thefile)
               # e  test that it equals the actual filename
                    if thefile != myfile:
                         validateme(error, "The name after @file in " + myfile + " (" + thefile + ")" + " does not match the actual filename.", i, myfile, lineList[i])
 # Val rule 06 The words TRUE and FALSE must always be all-caps.  
               if re.search("true|false", lineList[i]):
                    validateme(warning, "TRUE and FALSE must be all-capitals.", i, myfile, lineList[i])

 # Val rule 07 Do not use these commands: @class|@enum|@typedef|@mainpage|@retval
               if re.search("@class|@enum|@typedef|@const|@define|@include|@namespace|@struct|@typedef|@union|@mainpage|@retval", lineList[i], re.IGNORECASE):
                    validateme(warning, "Contains a Doxygen command that should not be used." + unused, i, myfile, lineList[i])

 # Val rule 08 Count @{s and @}s
               if re.search("@{", lineList[i], re.IGNORECASE):
                    openbracket += 1
               if re.search("@}", lineList[i], re.IGNORECASE):
                    closebracket += 1

 # Val rule 09 Count conds and endconds
               if re.search("@cond", lineList[i], re.IGNORECASE):
                    opencond += 1
               if re.search("@endcond", lineList[i], re.IGNORECASE):
                    closecond += 1

 # Val rule 10 Count latexonlys and endlatexonlys
               if re.search("@latexonly", lineList[i], re.IGNORECASE):
                    openlatex += 1
               if re.search("@endlatexonly", lineList[i], re.IGNORECASE):
                    closelatex += 1

 # Val rule 11 Count htmlonlys and endhtmlonlys
               if re.search("@htmlonly", lineList[i], re.IGNORECASE):
                    openhtml += 1
               if re.search("@endhtmlonly", lineList[i], re.IGNORECASE):
                    closehtml += 1

 # Val rule 12 Count verbatims and endverbatims
               if re.search("@verbatim", lineList[i], re.IGNORECASE):
                    openverbatim += 1
               if re.search("@endverbatim", lineList[i], re.IGNORECASE):
                    closeverbatim += 1

 # Val rule 13 Direction for all params is required
               if re.search("@param\[.*?[A-Z].*?\]", lineList[i]):
                    validateme(error, "Capital letter(s) in @param command.  All params directions must be lower case not upper case. ", i, myfile, lineList[i])

 # Val rule 14 Checking @param[in,out] punctuation and formatting
               if re.search("@param\[in\s+,out\]|@param\[in,\s+out\]|@param\[in\s*\/\s*out\]|@param\[in\s*[^,]\s*out\]", lineList[i], re.IGNORECASE):
                    validateme(error, "@param command [in.out] direction is incorrect. Must be formatted as \"@param[in,out}\"", i, myfile, lineList[i])

 # Val rule 15 Discourage "^ * "
               if re.search("^ \* ", lineList[i], re.IGNORECASE):
                    hasstars = 1
 # Val rule 16 Check to ensure that @ingroup or @defgroup are not used in header files
               if not re.match("^mainpage.h$",myfile) and re.search("@ingroup|@defgroup", lineList[i], re.IGNORECASE):
                    validateme(error, "@ingroup or @defgroup commands used in header file. You may use these commands only in the mainpage file", i, myfile, lineList[i])

 # Val rule 17 Check to ensure that at least one @addtogroup is used in a header file.
               if re.search("@addtogroup", lineList[i], re.IGNORECASE):
                    hasaddtogroup = 1

 #        Test whether you're ending a comment
          if re.search("\*\/", lineList[i], re.IGNORECASE):
              incomment = 0

     # Clear arrays
     lineList = []


     # Before going to the next file, test for * borders
     if hasstars:
          logFile.insert(0,myfile + ":" + "\n" + spacer + warning + "Contains asterisk border(s) in comment(s) (^ * ).  Please remove these.\n\n")
          warnum += 1

     # Before going to the next file, test for addtogroup
     if not re.match("^mainpage.h$",myfile) and not hasaddtogroup:
          logFile.insert(0,myfile + ":" + "\n" + spacer + warning + "Does not contain any addtogroup commands.\n\n")
          errnum += 1

     # Before going to the next file, test for unmatched brackets (add one to avoid division by zero)
     if openbracket != closebracket:
          logFile.insert(0,myfile + ":" + "\n" + spacer + warning + "Contains unmatched markers (@{ ... @}): " + str(openbracket) + " open markers and " + str(closebracket) + " close markers.\n\n")
          warnum += 1

     # Before going to the next file, test for unmatched conds (add one to avoid division by zero)
     if opencond != closecond:
          logFile.insert(0,myfile + ":" + "\n" + spacer + error + "Contains unmatched conds (@cond ... @endcond): " + str(opencond) + " open conds and " + str(closecond) + " endconds.\n\n")
          errnum += 1

     # Before going to the next file, test for unmatched latexonlys (add one to avoid division by zero)
     if openlatex != closelatex:
          logFile.insert(0,myfile + ":" + "\n" + spacer + error + "Contains unmatched latexonlys (@latexonly ... @endlatexonly): " + str(openlatex) + " open latexonly and " + str(closelatex) + " endlatexonlys.\n\n")
          errnum += 1

     # Before going to the next file, test for unmatched htmlonlys (add one to avoid division by zero)
     if openhtml != closehtml:
          logFile.insert(0,myfile + ":" + "\n" + spacer + error + "Contains unmatched htmlonlys (@htmlonly ... @endhtmlonly): " + str(openhtml) + " open htmlonly and " + str(closehtml) + " endlatexonlys.\n\n")
          errnum += 1

     # Before going to the next file, test for unmatched verbatims (add one to avoid division by zero)
     if openverbatim != closeverbatim:
          logFile.insert(0,myfile + ":" + "\n" + spacer + error + "Contains unmatched verbatims (@verbatim ... @endverbatim): " + str(openverbatim) + " open verbatims and " + str(closeverbatim) + " endverbatims.\n\n")
          errnum += 1

     # Clear these vars before moving to the next file 
     openbracket  = 0
     closebracket = 0
     opencond     = 0
     closecond    = 0
     openlatex    = 0
     closelatex   = 0
     openhtml     = 0
     closehtml    = 0
     openverbatim = 0
     closeverbatim= 0
     hasstars     = 0
     hasaddtogroup= 0
     hasatbrief   = 0
     hasatfile    = 0

# Prepend the logfile header with basic stats
logFile.insert(0,"\n\n")

# Check to see if @file is in each file by checking each filename against the dictionary compiled in step 4b
for myfile in dirList:
     if not re.match("^mainpage.h$",myfile) and myfile not in hasfile:
          logFile.insert(0,myfile + ":\n     " + error + "Has no @file command.\n")
          errnum += 1
# Check for @file in the mainpage file
     if re.match("^mainpage.h$",myfile) and myfile in hasfile:
          logFile.insert(0,myfile + ":\n     " + error + "Mainpage has an @file command.\n")
          errnum += 1

# Add basic stats:
logFile.insert(0,"-----------------------------FILE SCOPE WARNINGS--------------------------------\n\n")
logFile.insert(0,spacer + str(errnum) + " errors.\n\n")
logFile.insert(0,spacer + str(warnum) + " warnings.\n")
logFile.insert(0,"Doxygen validation completed with:\n")

# Last, put header:
logFile.insert(0,"(For files developed to be run with Doxygen 1.7.0+)\n\n")
logFile.insert(0,"Doxygen validation logfile\n==========================\n")

logFile.append("\nEND Doxygen validation logfile\n" + mytime + "\n")

# If batch.log doesn't exist, add it
if not os.path.exists(outfile):
    open(outfile, "a")

# Then, if the logFile has something in it, write it
if len(logFile) > 1 and (errnum or warnum):
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
