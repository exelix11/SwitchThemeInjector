# Dumping Files
This guide explains how to obtain the files needed to make custom themes from your own switch.
**This guide is outdated, you should use the [new automated one](https://github.com/exelix11/SwitchThemeInjector/blob/master/DumpingFiles.md) **

## Tools you'll need
- BisKeyDump
- HacDiskMount
- Hactool
- A Full nand backup (RawNand.bin) OR a usb-c cable

**Payloads have full control on your console so aways download them from the authors' official link**

## Guide
### Getting your nand keys
Boot your switch in RCM mode and launch the BisKeyDump payload, it will show several keys and a qr code.\
We need the two "BIS Key 2" keys one called "tweak" and the other "crypt", copy them to your pc and double check. Alternatively you can scan the QR which is a text file with all the keys but not all phones scan it correctly.
### Mounting the nand
There two ways to do this : From a nand backup or directly through usb. The usb way is faster but since **we are going to directly mount the switch nand make sure to not modify any file or you might brick**. Using a nand backup is safer because you can just delete it if you mess something up.
Follow ONLY ONE of these options :
#### From USB
Fully read this part before attempting it, also make sure to have a nand backup. \
Download the memloader payload by rajkosto and run it, choose ums_emmci.ini, connect the switch to your pc, **windows will not recognize the filesystem and might ask to format CLICK NO or else you'll brick**.\
Now open HacDiskMount click on File and Open physical drive, select your switch from the list (should be called Linux UMS drive, the size should be ~29GB). \
If you did everything correctly now you should see a list of  all the partitions on your switch nand, skip to the "Dumping the home menu" section
#### From Nand backup
The files we're dumping are firmware-depended if your backup is on a different version than your switch themes won't work on your switch (but will if you restore your backup)
Open HacDiskMount click on File and Open file and open your nand backup, now you should see a list of  all the partitions on your switch nand.
### Dumping the home menu
Double click on the SYSTEM partition, a window should appear, write the two "BIS Key 2" you got earlier and press on Test, if it doesn't say OK then you got the wrong keys.\
*If you don't have it already click on Install to install the virtual device driver, HacDiskMount must be running as administrator.*\
Select an unused drive letter, check the read-only box and click on mount.
Now open the drive you just mounted and go in the "Contents/registered/" folder, you should see many folder with alphanumeric names, find the right one for your firmware :
Home menu :
- for 6.1.0 0cb8843f6a8f7cfb0609f303a4ca000e
- for 6.0.1 96860a2605d8890dee7a8d4154bbe483
- for 6.0.0 33dd03103f1ac6a50e13d27ce5e00b2f or 97c64495df444e948dcfaec986a5da02
- for 5.1.0 8684b0ddab1581d300a15ebc96c6bf2c
- for 5.0.1/2 a4d9e98bf95e906a8f7176a25cc57ca1
- for 5.0.0 7e31371be988f89efd80843417496947
- for 4.1.0 16314a48995e607d61d514edc3fc2336


You might also want to get the user settings applet : 
- for 6.0.1 2a59543bf2eced1ed352cacd2067ba48
- for 6.0.0 b359c72323454d948066f0ff0b46e762 or 2a59543bf2eced1ed352cacd2067ba48
- for 5.1.0 326e844de97bb17899afd0eacf474c0d
- for 5.0.2 c27a76572faa4263ae12258ba991f4e5
- for 5.0.1 0c927ffd11ae2a021e3e551fde524c8a
- for 5.0.0 2c3d42bd455e0bcdf8a23aa758e581bd
- for 4.1.0 a4d340a9b2ba9077a9495eadd9d87561

Once you find the right folder copy it to your pc, inside it you should find a file called "00", it's an nca, use hactool to extract it:
```
hactool 00 --romfsdir=romfs
```
Now you should have a folder called romfs with all the files you need to make custom themes.\
\
Before closing hactool remember to unmount.
