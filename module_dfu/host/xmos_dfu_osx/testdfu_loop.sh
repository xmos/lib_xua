#! /bin/bash
################################################################################

#find out were we are running from so we only exec this programs
PROGDIR=`dirname $0`

#setup environment
export DYLD_LIBRARY_PATH=$PWD:$DYLD_LIBRARY_PATH

#check if we were given a binary name, otherwise use the default one
updatebin=testbin
if [ "$1" != "" ]; then
  updatebin=$1
fi

#basic check for binary
if [ ! -f $updatebin ]; then
  echo "FATAL: can't find update binary named $updatebin"
  exit 1
else
    using $updatebin
fi

#-------------------------------------------------------------------------------
for i in 1 2 3 4 5 6 7 8 9 10; do
  echo ""
  echo DFU test iteration $i
  echo ---------------------
  echo ""
  echo "*** DFU revert to factory ***"
  $PROGDIR/xmosdfu --revertfactory 
  sleep 2
  echo "" 
  echo "*** DFU download new firmware ***"
  $PROGDIR/xmosdfu --download $updatebin
  sleep 2
  echo "" 
  echo "*** DFU upload existing firmware ***"
  $PROGDIR/xmosdfu --upload $updatebin
  sleep 2
  echo "" 
done

echo DFU Test Complete!

