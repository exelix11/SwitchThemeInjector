# Switch theme injector

![ThemeScreenshot](ThemeScreenshot.jpg)

The Switch theme injector project is split into three parts:
- Switch theme injector (Windows app): An app to create and edit custom themes
- NXThemes installer: An homebrew app that runs on the switch itself and can be used to install and manage themes.
- [Switch theme injector online](http://exelix11.github.io/SwitchThemeInjector/v2) (also called WebInjector): A port of the windows injector as a web app, it lacks some features like custom applet icons and common.szs layouts support.

The main objective is to develop a complete toolset to create and install custom themes on the switch. As the console os doesn't implement custom themes natively most of this is done by patching system SZS files to get the desidered aspect.\
\
Unfortunately SZS files from the switch os contain copyrighted data so to make theme sharing legal the **nxtheme format** has been developed, it's 100% legal and works on every firmware >= 5.0, unless you're dealing with making your own patches and custom layouts you should only use nxtheme files.

# Usage
Open the injector and go to the NXThemes builder tab, open any 720p image (1280x720 pixels) and fill the form, then click on build nxtheme.\
To install nxthemes files on your console download the NxThemes installer homebrew from the releases then just select any nxtheme file from your sd.\
This app works on windows, you can use the CLI through mono on linux using the package `mono-complete`. \
Note that on linux you must use DDS files as the built-in image converter works only on windows.

## Command line args
You can automate theme creation using command line args.
### Building nxthemes
The syntax is:
```
SwitchThemes.exe buildNX home "<your image.png/jpg/dds>" "<json layout file, optional>" "name=<theme name>" "author=<author name>" "out=<OutputPath>.nxtheme"
```
this will build a theme for the home menu, instead of `home` you can use: `lock` for lockscreen, `apps` for the all apps screen, `set` for the settings applet, `user` for the user page applet and `news` for the news applet. Only the image and out file args are needed. \
Other options specific to the theme target such as applet icons are availbale as well, run `SwitchThemes.exe help` for more info
### Patching SZS files
```
SwitchThemes.exe szs "<input file>" "<your image.png/jpg/dds>" "<json layout file, optional>" "album=<custom album icon.png/dds>" "out=<OutputPath>.szs"
```
`album` will only be used if patching a residentMenu szs.
### Remote install
Using the remote install feature in the NXThemesInstaller homebrew
```
SwitchThemes.exe install 192.168.X.Y "<nxtheme/szs file>"
```

# Useful resources
Check out how to make your own layouts and animations [here](https://github.com/exelix11/SwitchThemeInjector/blob/master/CustomLayouts.md) \
You can also find more info about the szs patching process [here](https://github.com/exelix11/SwitchThemeInjector/blob/master/SzsPatching.md)
