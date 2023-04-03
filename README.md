# Fresco Logic FL2000 Linux/Android kernel driver

### 1. What is this?

This is an official driver release from Fresco Logic in an attempt to help the open-source community adopting the development and use of the FL2000DX device.
This driver only covers the USB part of the display logic. It does not support the Linux desktop logic (eg. extended desktop vs mirrored desktop).

### 2. On which kernel versions does this driver work?

This driver is tested on Ubuntu 14 LTS as well as some Android platforms with kernel version 3.10.x.
This driver source might not compile on newer kernels (eg. 4.0 or above) because of the fast-moving API changes in the
mainstream kernel. You might need to adapt it for your own use.

### 3. Target audience

This release is targeted to open-source developers, as opposed to end-users.

### 4. How do I enable extended desktop/mirrored desktop on my X Window?

Currently Fresco Logic does not provide desktop related manipulation.
Fresco Logic hopes the community will contribute to this area so that end-users can easily adopt this solution.

### 5. FL2000DX limitation.

The FL2000DX chip is cheap by design where it doesn't have a frame buffer on its own.
It relies heavily on USB 3.0 transfer speed to accommodate continuous USB flow.
The larger the image is, the heavier it depends on USB bandwidth.
A typical 1920x1080@60 Hz requires 1920 * 1080 * 24bpp * 60 = 373,248,000 bytes/sec of traffic over the USB bus.
As such, USB2.0 speed is not supported.

Connecting more than one FL2000DX device to the same bus is deprecated.

### 6. How do I compile & test the kernel driver?
#### 6a. Compile the driver

Find your kernel source tree, and edit `src/Makefile`. Locate the following line:
    
    KERNEL_PATH = /usr/src/linux-headers-4.4.0-72-generic`
    
Modify this line so that it points to the correct source tree.
After that, run `make` to create `fl2000.ko` and run `insmod fl2000.ko` to load the driver.


**Note:**

- If you miss some program (like 'flex'), just do 

  ```
  $ sudo apt-get install _program_name_
  ```  
  
  to get it.

- If you get some errors in `insmod` command like:

  - **"Key was rejected by service":** You are working in Secure Boot, so you have to sign the `fl2000.ko` module. 
  
    1. Create `openssl.conf` file with this text inside (under `req_distinguished_name` you can edit the fields with custom preference, but it's optional):
  
    ```
    # This definition stops the following lines choking if HOME isn't
    # defined.
    HOME                    = .
    RANDFILE                = $ENV::HOME/.rnd 
    [ req ]
    distinguished_name      = req_distinguished_name
    x509_extensions         = v3
    string_mask             = utf8only
    prompt                  = no

    [ req_distinguished_name ]
    countryName             = CA
    stateOrProvinceName     = Quebec
    localityName            = Montreal
    0.organizationName      = cyphermox
    commonName              = Secure Boot Signing
    emailAddress            = example@example.com

    [ v3 ]
    subjectKeyIdentifier    = hash
    authorityKeyIdentifier  = keyid:always,issuer
    basicConstraints        = critical,CA:FALSE
    extendedKeyUsage        = codeSigning,1.3.6.1.4.1.311.10.3.6,1.3.6.1.4.1.2312.16.1.2
    nsComment               = "OpenSSL Generated Certificate"
    ```
  
    2. Now, create public and private keys using openssl command as:
  
    ```
    $ openssl req -config openssl.conf -new -x509 -newkey rsa:2048 -nodes -days 36500 -outform DER -keyout "MOK.priv" -out "MOK.der"
    ```
  
    This command, will create the “MOK.priv” and “MOK.der” files in present working directory (PWD) by accepting openssl.conf as input file.
  
    3. Now, the next step is to enroll this keys with the SHIM UEFI Bootloader ( SHIM is a simple software package that is designed to work as a first-  
       stage bootloader on UEFI systems ) . This enrolling can be done as:
  
    ```
    $ sudo mokutil --import MOK.der
    ```

    Then you should input password and confirm it. Remember the password you entered here, as we would need to add this when we reboot the 
    machine.
  
    4. Now reboot and at start press "Enroll MOK", then "Enroll the keys", now press "Yes", "Continue", "Yes" again, enter the password 
       inserted in the step before and then reboot again.
     
    5. Now, after your Linux machine is rebooted completely, lets go back to signing the kernel module and try to insert it:
  
    ```
    $ kmodsign sha512 MOK.priv MOK.der fl2000.ko
    ```
  
    6. Now retry with the insmod command and if you have another error message, go to its description.
  
  - **"Invalid module format":** You have compiled the fl2000.ko module with the wrong linux-headers version or you have not installed it well.

    1. Check that the actual version in use by the sistem is the same you have written in the KERNEL_PATH variable:
  
    ```
    $ uname -r
    ```
  
    2. If it's not the same, then change it, do `$ make clean`, then `$ make`, resign with `kmodsign` (if needed) and redo the `insmod` command.
  
    3. If it is the same, you need to uninstall and reinstall it, so you have to follow these steps:
  
    ```
    $ sudo apt update && sudo apt upgrade
    $ sudo apt remove --purge linux-headers-*
    $ sudo apt autoremove && sudo apt autoclean
    $ sudo apt install linux-headers-generic
    ```
  
    4. If the last "generic" version hasn't been installed, try to install it manually by:
  
    ```
    $ sudo apt-get install linux-headers- 
    ```
  
    press `TAB`, look to the greater "-generic" version, install it and `reboot`.
  
    5. Now write in the ./FL2000/src/Makefile into the `KERNEL_PATH` variable the right linux-headers generic version and redo the steps to insmod.
  
  - **"Bad message":** You have generated the `fl2000.ko` module, in a wrong way. Try to check every steps and commands.


#### 6b. Test the driver

In the `sample` folder, run `make` to create `fltest`. If you you are using a
cross compiler to build the binary for specific platforms, you need to specify that specific
compiler in `src/Makefile`.
    
Run `./fltest 0` as superuser to run the test. The driver provides several
user mode buffer access methods (e.g  copy to kernel internal buffer, or
directly locking down user buffer). Look at `fl2000_ioctl.h` for detailed
information.


**Note:**

If something goes wrong (like it cannot find the device) check the right name of the device in `/dev/` directory (for example could be "fl2000-2") and edit it in `./FL2000/sample/main.c` file in the line:

 ```
 #define	FL2K_NAME	"/dev/fl2000-2"
 ```

Then delete `fltest` file if it is present and do:

 ```
 $ sudo make fltest
 ```
 
Retry to run `./fltest 0`

### 7. How do I file a bug to the Fresco Logic developers?

You can file bugs to [Github Issues](https://github.com/fresco-fl2000/fl2000/issues)


