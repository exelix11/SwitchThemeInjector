# Dumping Files
This guide explains how to obtain the files needed to make custom themes from your own switch.\
The automated way using the theme injector is windows only, mac and linux users scroll to the end of the page.

## Tools you'll need
- [Latest version of switch theme injector and NXThemes Installer](https://github.com/exelix11/SwitchThemeInjector/releases/latest)
- [Lockpick by shchmue](https://github.com/shchmue/Lockpick/releases)
- [hekate - CTCaer mod](https://github.com/CTCaer/hekate/releases)
- [HacTool](https://github.com/SciresM/hactool/releases/latest)

**Payloads have full control on your console so aways download them from the authors' official link**

## Guide
### Setup
*Make sure to use the latest version of the tools, when a new switch system update drops new keys may be added so some of the tools have to be updated as well* \
Copy the NXThemes installer and Lockpick NROs in the `switch` directory of your sd card.
### Getting the switch keys
1) Boot your switch in RCM mode and launch the hekate payload (even if it's not your cfw, we just need to get some keys), if you have autoboot enabled press VOL- during boot to enter the menu.
2) Using the VOL buttons to navigate and power to confirm, open `Console info...` then `Print fuse info` and press Power to save fuse info to SD card. 
3) Go back and select `Print TSEC keys` then press Power to save TSEC keys to SD card. 
4) Now boot your favorite CFW and open the homebrew launcher and launch Lockpick, in a few seconds it should dump all the keys we need, press + to exit. (it may ask you to dump title keys as well but we don't need those so just exit). 
5) Finally launch NXThemes installer, go to the `Dump NCA` tab, select the dump button and press + to confirm. When the process finishes you can shut down your switch.
### Dumping the home menu
1) Put your sd card in your pc and run switch theme injector, go to the `Dump NCA` tab.
2) Press on the first `...` button to select the key file, it's called `prod.keys` and it's in the `switch` folder in your sd card. 
2) Press on the second `...` button and select the `systemData` folder in the `themes` folder on your sd card
3) Click on `RUN`, in a few seconds the needed files will be directly extracted to your sd card.\
Done, you're ready to go install and make some themes !
#### If you can't plug your sd card in your pc 
With an ftp client copy the themes and switch folders from the sd card to your pc and follow the guide, when you finish just copy the files that have been extracted in themes/systemData on your pc to your sd.
#### If you get `lyt dir not found` error
Make sure to be using the latest version of hactool, if you do make sure to have all the keys for your firmware (aka that lockpick didn't fail). If you still face issues check the hactool.log file that has been generated and ask for help on the qcean discord.

## Dumping the home menu on mac and linux
Get a hactool build for your os and follow `Getting the switch keys`, then copy these three files from your sd card to your pc:

- `/themes/systemData/home.nca`
- `/themes/systemData/user.nca`
- `/switch/prod.keys`

then run: \
`hactool -k prod.keys --romfsdir=homeromfs home.nca` and \
`hactool -k prod.keys --romfsdir=userromfs home.nca` \
Now you should have two folders called homeromfs and userromfs, in both of them there should be a folder called lyt, copy the content of the one in homeromfs and just the mypage.szs file from userromfs to the systemData folder on your sd. \
Done, you're ready to go install and make some themes !
