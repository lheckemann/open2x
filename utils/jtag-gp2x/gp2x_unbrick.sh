echo Attempting GP2X bootloader reflash...
jtag
halt
echo Disabling watchdog.
poke C0000A14 00000000
echo Disabling interrupts.
poke C0000808 ffffffff	
echo Uploading bootloader ELF image to GP2X...
load bootloader.elf 3D00000
pause 2
echo Attempting to run ELF image...
echo (Please check SERIAL output for more infomation)
run 3D00000
halt
run 3D00000
