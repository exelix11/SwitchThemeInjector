# Dumping Files
This guide explains how to obtain the files needed to make custom themes from your own switch.
**This guide is outdated, you should use the [new automated one](https://github.com/exelix11/SwitchThemeInjector/blob/master/DumpingFiles.md)**

## Tools you'll need
- [Latest version of switch theme injector](https://github.com/exelix11/SwitchThemeInjector/releases/latest)
- [BisKeyDump](https://files.sshnuke.net/biskeydumpv6.zip)
- [HacDiskMount](https://files.sshnuke.net/HacDiskMount1055.zip)
- [HacTool](https://github.com/SciresM/hactool/releases/latest)
- A way to read the console's nand: a usb-c cable OR a full nand backup (RawNand.bin)

**Payloads have full control on your console so aways download them from the authors' official link**

## Guide
### Setup
Go to the switch theme injector directory and create a new folder called `hactool`, inside of it copy `hactool.exe` and its dlls, **make sure you're using latest version**.\
Get all the switch keys, either dump them from your console or search on google (coff coff pastebin) and put them in a file called `keys.dat` in the `hactool` folder. **Make sure you have up to date keys check that you have `master_key` 0 to 5** \
Now open switch theme injector, if you did everything correctly you should see the "Extract NCA" tab. Close it for now, you'll need it later.
### Getting your nand keys
Boot your switch in RCM mode and launch the BisKeyDump payload, it will show several keys and a qr code.\
We need the two `BIS Key 2` keys one called `tweak` and the other `crypt`, copy them to your pc and double check. Alternatively you can scan the QR which is a text file with all the keys but not all phones scan it correctly.
### Mounting the nand
There two ways to do this : From a nand backup or directly through usb. The usb way is faster but since **we are going to directly mount the switch nand make sure to strictly follow this guide and don't modify any file or you might brick**. Using a nand backup is safer because you can just delete it if you mess something up.
Follow ONLY ONE of these options :
#### From USB
Fully read this part before attempting it, also make sure to have a nand backup. \
Download the memloader payload by rajkosto and unzip it. Inside the zip there should be a folder called `samples`, copy its contents to the root of the sd card.\
Run the `memloader.bin` payload, with the volume buttons choose `ums_emmc.ini` and press the power button to confirm, windows should detect a new device **if it asks to format it CLICK NO or else you'll brick**.\
Now open HacDiskMount, click on File and Open physical drive, select your switch from the list (should be called `Linux UMS drive`, the size should be around 29GB). \
If you did everything correctly now you should see a list of all the partitions on your switch nand, skip to the "Dumping the home menu" section
#### From Nand backup
**The files we're dumping are firmware-dependent if your backup is on a different version than your switch themes won't work** (but will if you restore your backup)
Open HacDiskMount click on File and Open file and open your nand backup, now you should see a list of  all the partitions on your switch nand.
### Dumping the home menu
Double click on the SYSTEM partition, a window should appear, write the two `BIS Key 2` you got earlier and press on Test, if it doesn't say OK then you got the wrong keys.\
*If you don't have it already click on Install to install the virtual device driver, HacDiskMount must be running as administrator.*\
Select an unused drive letter, **check the read-only box** and click on mount.\
Now open Switch theme injector and go to the Extract NCA tab. \
In the Switch mount path box write the root of the drive you mounted the console to. Example: `D:\` (**: and \\ matter !**) \
For output path select an empty folder on your pc in which the home menu will be extracted.
(you can click on ... to browse) \
Click on RUN \
The process shouldn't take more than 5 minutes (it usually takes much less), at the end if everything went fine it should have extracted the home menu and user settings applet romfs to the folder you selected earlier.\
The files you're looking for are in the `lyt` folder.

Before closing hactool remember to click on unmount.

#### Troubleshooting
If the process failed check you did all the steps correctly, especially the keys. \
To know if you have the correct keys check the LOG, it should look like this:
```
Checking file ........
Nca: .....
Magic: ..........
[many lines with other info about the file]
Done!

Checking file ........
```
If your log is missing the file info or contains errors about decryption you're missing some keys. \
If the log looks ok manually check if hactool works: open the command prompt in the hactool folder and type `hactool -k keys.dat *your switch nand*:/contents/registered/*any folder*/00` doesn't matter which folder you pick, this is just to check if hactool works properly. Check if the output contains any errors.


If you still have issues try the [old guide](https://github.com/exelix11/SwitchThemeInjector/blob/master/DumpingFilesOld.md) or save the log and contact me.
