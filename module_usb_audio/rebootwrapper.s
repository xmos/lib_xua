# Wrapper generated from this xc code
# Then modified to fix maxchanends to 0
# Then simplified
#void device_reboot_implementation(chanend spare);
#
#// This version just exists so generate an assembly wrapper function for me.
#void device_reboot(chanend spare) 
#{
#    device_reboot_implementation(spare);
#}

.extern device_reboot_implementation, "f{0}(chd)"
          .text
          .align    2
.cc_top device_reboot.function,device_reboot
          .align    4
.call device_reboot, device_reboot_implementation
.globl device_reboot, "f{0}(chd)"
.globl device_reboot.nstackwords
.globl device_reboot.maxthreads
.globl device_reboot.maxtimers
.globl device_reboot.maxchanends
.globl device_reboot.maxsync
.type  device_reboot, @function
.linkset device_reboot.locnoside, 1
.linkset device_reboot.locnochandec, 1
.linkset device_reboot.nstackwords, device_reboot_implementation.nstackwords + 1
device_reboot:
          entsp     0x1 
          bl        device_reboot_implementation 
          retsp     0x1 
.size device_reboot, .-device_reboot
.cc_bottom device_reboot.function
.linkset device_reboot.maxchanends, 0#device_reboot_implementation.maxchanends
.linkset device_reboot.maxtimers, device_reboot_implementation.maxtimers
.linkset device_reboot.maxthreads, device_reboot_implementation.maxthreads
