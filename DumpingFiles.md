# Dumping Files
This guide explains how to obtain the files needed to make custom themes from your own switch.\
The automated way using the theme injector is windows only, mac and linux users scroll to the end of the page.

## Tools you'll need
- [Latest version of switch theme injector and NXThemes Installer](https://github.com/exelix11/SwitchThemeInjector/releases/latest)
- [Lockpick by shchmue](https://github.com/shchmue/Lockpick/releases)
- [hekate - CTCaer mod](https://github.com/CTCaer/hekate/releases)

**Payloads have full control on your console so aways download them from the authors' official link**

## Guide
### Setup
*Make sure to use the latest version of the tools, when a new switch system update drops new keys may be added so some of the tools have to be updated as well* \
Copy the NXThemes installer and Lockpick NROs in the `switch` directory of your sd card.
### Getting the switch keys
**Warning for SX emunand users :** Some users reported that lockpick can't properly dump the keys when using emunand, if you have this issue, read `Sx emunand users` on the bottom of the page first.

1) Boot your switch in RCM mode and launch the hekate payload (**Even if it's not your cfw, we are just using it to get some keys, you DON'T have to use atmosphere**), if you have autoboot enabled press VOL- during boot to enter the menu.
2) Using the VOL buttons to navigate and power to confirm, open `Console info...` then `Print fuse info` and press Power to save fuse info to SD card. 
3) Go back and select `Print TSEC keys` then press Power to save TSEC keys to SD card. 
4) Now boot your favorite CFW and open the homebrew launcher and launch Lockpick, in a few seconds it should dump all the keys we need, press + to exit. (it may ask you to dump title keys as well but we don't need those so just exit). 
5) Finally launch NXThemes installer, go to the `Extract home menu` tab, select the extract home menu button and press + to confirm. Once the process is over you will be able to install nxtheme files.

### SX emunand users
The only solution for you is to search the switch keys on google, they are like *really* easy to find, if you do search on google make sure you have `master_key_06`, that means that the keys are up to date, download them and copy them in the root of your sd card in a file called `prod.keys`. Then skip to step 5 of the guide.
