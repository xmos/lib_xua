#! /bin/bash
################################################################################

function usage {
    echo "USAGE: testdfu.sh device_pid device_string upgrade1 upgrade2"
    echo "  For example:"
    echo "    testdfu.sh XMOS_VF_SPK_BASE \"XMOS VocalFusion\" upgrade1.bin upgrade2.bin"
    echo "    testdfu.sh 0x0008 \"xCORE\" upgrade1.bin upgrade2.bin"
    exit 1
}

#find out were we are running from so we only exec this programs
PROGDIR=`dirname $0`

if echo $OSTYPE | grep -q darwin ; then
  platform=OSX64
elif echo $OSTYPE | grep -q abihf ; then
  platform=Rasp
elif arch | grep -q x86_64 ; then
  platform=Linux64
elif echo $OSTYPE | grep -q linux ; then
  platform=Linux32
else
  echo "Unknown OS $OSTYPE"
  exit 1
fi

#setup environment
export DYLD_LIBRARY_PATH=$PROGDIR/libusb/$platform:$DYLD_LIBRARY_PATH

if [ "$1" != "" ]; then
  device_pid=$1
else
  usage
fi

if [ "$2" != "" ]; then
  device_grep_string=$2
else
  usage
fi

if [ "$3" != "" ]; then
  update1=$3
else
  usage
fi

if [ "$4" != "" ]; then
  update2=$4
else
  usage
fi

#basic check for binary
if [ ! -f $update1 ]; then
  echo "FATAL: can't find update binary named $update1"
  exit 1
fi

if [ ! -f $update2 ]; then
  echo "FATAL: can't find update binary named $update2"
  exit 1
fi

#-------------------------------------------------------------------------------
echo ""
echo DFU test
echo --------
sleep 5
echo "Version Read:"
system_profiler SPUSBDataType|grep -A10 "$device_grep_string" |grep Version

echo "" 
echo "*** DFU download new firmware 1 ***"
$PROGDIR/bin/xmosdfu $device_pid --download $update1
sleep 2
echo "Version Read:"
system_profiler SPUSBDataType|grep -A10 "$device_grep_string" |grep Version

echo "" 
echo "*** DFU download new firmware 2 ***"
$PROGDIR/bin/xmosdfu $device_pid --download $update2
sleep 2
echo "Version Read:"
system_profiler SPUSBDataType|grep -A10 "$device_grep_string" |grep Version

echo "" 
echo "*** DFU upload existing firmware ***"
$PROGDIR/bin/xmosdfu $device_pid --upload upload.bin
sleep 2
echo "Version Read:"
system_profiler SPUSBDataType|grep -A10 "$device_grep_string" |grep Version

echo "" 
echo "*** DFU revert to factory ***"
$PROGDIR/bin/xmosdfu $device_pid --revertfactory 
sleep 2
echo "Version Read:"
system_profiler SPUSBDataType|grep -A10 "$device_grep_string" |grep Version

echo "" 
echo "*** DFU download uploaded firmware ***"
$PROGDIR/bin/xmosdfu $device_pid --download upload.bin
sleep 2
echo "Version Read:"
system_profiler SPUSBDataType|grep -A10 "$device_grep_string" |grep Version

echo "" 
echo "*** DFU revert to factory ***"
$PROGDIR/bin/xmosdfu $device_pid --revertfactory 
sleep 2
echo "Version Read:"
system_profiler SPUSBDataType|grep -A10 "$device_grep_string" |grep Version
echo "" 
echo DFU Test Complete!

