# This is inspirated by @NSG650/NoMoreBugCheck

# NoMoreBugCheck
Prevent Windows from BSODing no matter what happens!
Windows can give you a bluescreen when the call comes from within the kernel where the function isn't hooked.
You might get in trouble with Patch Guards too because they will be overwritten.

# Warning
Using this is like turning off the circuit breaker in your home. Windows triggers a BSOD (Blue Screen of Death) during critical issues to prevent memory and data corruption. While this won't physically harm your computer, it can lead to data or memory corruption. This was created purely for fun and is not intended for any serious use. I am not responsible for any data loss or system damage that may occur.

# Quick guide how to install and use this driver
1. You have to disable secure boot in your BIOS/UEFI settings (You can't do it from Windows).
2. Then you have to enable test signing. For that open up CMD with administrator priviliges and type in the following command:
```
bcdedit /set testsigning on
```
3. Create the service using SC. You can use the same command line as before and enter the following command:
```
sc create NoMoreBugCheck binPath=C:\where\ever\the\driver\is\NoMoreBugCheck.sys type=kernel start=manual
```
4. Run the service you just created! Now enter this command:
```
sc start NoMoreBugCheck
```
5. If you want to revert the changes just unload the driver by running
```
sc stop NoMoreBugCheck
```
