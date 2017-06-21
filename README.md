# fl2000
Fresco Logic FL2000 Linux/Android Kernel driver

1. What it is?
    This is a official driver release from Fresco Logic, in an attempt to help
    open-source community adopting the developement and use of the FL2000DX device.
    The driver contains only the USB part of the display logic. It does not support
    Linux desktop logic (eg. extended desktop vs mirrored desktop).
    
2. What is the Linux kerenl version that this release works on?
    This driver is tested on Ubuntu 14 LTS, as well as some other Android platform
    where the kernel version is 3.10.x.  This driver source might not compile on
    newer kernel (eg. 4.0 or above) because of fast-moving API changes in the 
    mainstream kernel. You need to adapt it for your own use.
    
3. Target audiance
    This release is targeted to open-source developer, as opposed to end user. 
    
4. How do I enable extended desktop/mirrored desktop on my X Window?
    Currently Fresco Logic does not provide desktop related manipulation. 
    Fresco Logic hopes the community would contribute to this area such that end
    user could easily adopt this solution.
    
5. FL2000DX limitation.
    FL2000DX chip is cheap by design where it does not have frame buffer of its
    own. It relies heavily on USB 3.0 transfer speed to accommodate contiguous 
    USB flow. The larger the image is, the heavier it depends on USB bandwidth.
    A typical 1920x1080@60 Hz requires 1920x 1080x 24bpp x 60 = 373,248,000 bytes/sec
    of traffic over the USB bus.
    
    More than 1 FL2000DX devices connected to the bus is depricated.
    
    As such, USB2.0 speed is not supported.
    
6.  How do I compile & test the kernel driver?
6.1 Find your kernel source tree, and edit src/Makefile. Locate the following lines:

KERNEL_PATH = /usr/src/linux-headers-4.4.0-72-generic

    Modify this line so that it points to correct source tree.
    Then use "make" to create "fl2000.ko". Use "insmod fl2000.ko" to load the driver.
    
6.2 In the "sample" folder, use "make" to create "fltest". If you you are using 
    cross compiler to build binary on specific platform, you need to specify specific
    compiler in the src/Makefile. 
    
    Then use "sudo ./fltest 0" to invoke the test. The driver provides several
    user mode buffer access method (e.g  copy to kernel internal buffer, or
    directly locking down user buffer, ...etc). Look at fl2000_ioctl.h for detailed
    information.
    
7.  How do I file bug to Fresco Logic developer?
    Goto https://github.com/fresco-fl2000/fl2000/issues , and file one.
    
    

    