@echo off
ping 1.1.1.1 -n 1 -w 3000 > null
%1 info
echo *** DFU download new firmware 1 ***
%1 download %2 
ping 1.1.1.1 -n 1 -w 3000 > null
%1 info
echo *** DFU download new firmware 2 ***
%1 download %3
ping 1.1.1.1 -n 1 -w 5000 > null
%1 info

echo *** DFU upload firmware ***
%1 upload upload.bin
ping 1.1.1.1 -n 1 -w 3000 > null

echo *** DFU revert to factory ***
%1 revertfactory
ping 1.1.1.1 -n 1 -w 3000 > null
%1 info

echo *** DFU download uploaded firmware ***
%1 download upload.bin
ping 1.1.1.1 -n 1 -w 3000 > null
%1 info
echo *** DFU revert to factory ***
%1 revertfactory
ping 1.1.1.1 -n 1 -w 3000 > null
%1 info
echo DFU Test Complete!
