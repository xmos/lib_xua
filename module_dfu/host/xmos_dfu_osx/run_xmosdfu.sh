#! /bin/bash

#find out were we are running from so we only exec this program
PROGDIR=`dirname $0`

#setup environment
export DYLD_LIBRARY_PATH=$PROGDIR:$DYLD_LIBRARY_PATH

$PROGDIR/xmosdfu $1 $2 $3
