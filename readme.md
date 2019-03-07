# Switch theme injector
The Switch theme injector project is composed of three parts:
- Switch theme injector (Windows app): An app to create and edit custom themes
- NXThemes installer: An homebrew app that runs on the switch itself and can be used to install and manage themes.
- [Switch theme injector online](http://exelix11.github.io/SwitchThemeInjector) (also called WebInjector): A port of the windows injector as a web app, it lacks some features like image to DDS conversion.

The main objective is to develop a complete toolset to create and install custom themes on the switch. As the console os doesn't implement custom themes natively most of this is done by patching system SZS files to get the desidered aspect.\
\
Unfortunately SZS files from the switch os contain copyrighted data so to make theme sharing legal the **nxtheme format** has been developed, it's 100% legal and works on every firmware, unless you're dealing with making your own patches and custom layouts you should only use nxtheme files.

# Usage
Open the injector and go to the NXThemes builder tab, open any 720p image (1280x720 pixels) and fill the form, then click on build nxtheme.\
To install nxthemes files on your console download the NxThemes installer homebrew from the releases then just select any nxtheme file from your sd.\
This app works on windows, you can use the CLI through mono on linux.

## Command line args.
You can automate theme creation using command line args.
  
Here's an example to build an nxtheme:\
`SwitchThemes.exe buildNX home "<your image.png/jpg/dds>" "<json layout file, optional>" "name=<theme name>" "author=<author name>" "commonlyt=<custom common.szs layout>" "album=<custom album icon.png/dds>" "out=<OutputPath>.nxtheme"`\
This will build a theme for the home menu, instead of `home` you can use: `lock` for lockscreen, `apps` for the all apps screen, `set` for the settings applet, `user` for the user page applet and `news` for the news applet. Only the image and out file args are needed.
  
It is also possible to patch SZS files directly with an image:  
`SwitchThemes.exe szs "<input szs file>" "<your image.png/jpg/dds>" "<json layout file, optional>" "album=<custom album icon.png/dds>" "out=<OutputPath>.szs"`

# Useful resources
You can make your own custom layouts using [Switch Layout Editor](https://github.com/FuryBaguette/SwitchLayoutEditor/) \
To learn how to convert your layouts to json or make your own patch templates read [Templates.md](https://github.com/exelix11/SwitchThemeInjector/blob/master/templates.md)
