
 This is the start of what I hope will be an easy way to build a replacement
 firmware for the gp2x. The idea I have is the following:

  1) $(OPEN2X)/firmware will be where the root filesystem is built.

  2) The kernel will be build and installed into that directory.

  3) Programs here will be built and installed into that directory.

  4) Shared libraries from $(OPEN2X)/lib will be copied to that directory.

  5) The root filesystem will be generated with a mkfs.* of some sort.

  6) Ready to flash upgrade and enjoy!

 That's the rough idea. No doubt it will change as time goes on.
