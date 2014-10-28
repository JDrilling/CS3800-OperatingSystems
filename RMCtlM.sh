#!/bin/bash
# input argument: a file name 
# removes Cntl-M characters which are hidden in the file 
## tr is a translate utility.
## \r is == cntl-M
cat $1 | tr "\r" " " > $1.tmp
if ( -e  $1.tmp ) then
     mv $1.tmp $1
fi

