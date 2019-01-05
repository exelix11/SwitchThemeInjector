# Switch theme injector
The Switch theme injector project is composed of three parts:
- Switch theme injector (Windows app): An app to create and edit custom themes
- NXThemes installer: An homebrew app that runs on the switch itself and can be used to install and manage themes.
- [Switch theme injector online](http://exelix11.github.io/SwitchThemeInjector) (also called WebInjector): A port of the windows injector as a web app, it lacks some features like image to DDS conversion.

The main objective is to develop a complete toolset to create and install custom themes on the switch. As the console os doesn't implement custom themes natively most of this is done by patching system SZS files to get the desidered aspect.\
\
Unfortunately SZS files from the switch os contain copyrighted data so to make theme sharing legal the **nxtheme format** has been developed, it's 100% legal and works on every firmware, unless you're dealing with making your own patches and custom layouts you should only use nxtheme files.

# Useful resources
To dump common.szs and the other files you need to make themes read [DumpingFiles.md](https://github.com/exelix11/SwitchThemeInjector/blob/master/DumpingFiles.md). \
To learn how to make your own custom layouts and patch templates read [Templates.md](https://github.com/exelix11/SwitchThemeInjector/blob/master/templates.md)
