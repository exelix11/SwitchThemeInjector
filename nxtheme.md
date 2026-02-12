# The nxtheme format

Back when themes were first introduced people would share the raw modified szs files from the home menu, this was a problem due to copyright issues but also because these files were version specific. Nxtheme is a custom format that can be used to freely share themes that mostly work across firmware versions.

## Nxtheme version 1

The first iteration of the nxtheme consists of a szs file with custom content, namely some json metadata, jpg images and json layouts. 

Nxtheme v1 files start with the magic bytes `Yaz0`.

## Nxtheme version 2

To improve on the limitations of nxtheme v1 we are working on a version 2 which is described in this file. This is currently work in progress and not not supported by all tools.

Nxtheme v2 files have the same extension as v1 and will transparently be handled both by the theme installer and themezer. The plan is to eventually migrate all themes to v2 and drop support for v1.

nxtheme version 2 is a zip file containing a `majifest.json` file and at least one theme part. Each theme part affects a specific part of the home menu.

Each part is stored in a folder named after its `nxpartname`. The folder must contain one or more of the following files:
- `layout.json` : the layout file for this part, this is the same as the layout files used in v1 themes.
- One of `bg.jpg` or `bg.dds` : the main wallpaper for this part, this is the same as the bg files used in v1 themes.
- A variable number of "extra" png or dds images, these are used for the icons and other UI elements of the theme, these are the same as the png files used in v1 themes.

The valid home menu parts are the same as v1 themes, except now they can all coexist in the same file while previously each had to be a standalone theme. The valid parts are:
- `home` : the main home menu.
- `lock` : the lock screen.
- `apps` : the "all games" menu.
- `set` : the settings applet.
- `news` : the news applet.
- `user` : the user profile menu.
- `psl` : the player select menu.

Additionally, v2 introduces `qlaunch_common`, this part targets the common.szs file present in qlaunch which as of v1 was special cased as part of `home` themes. `qlaunch_common` can only contain a layout file but no main or extra images.

The following parts support extra images:
- `home` supports the following
	- `album` : The album applet button.
	- `news` : The news applet button.
	- `shop` :  The shop applet button.
	- `controller` : The controller settings button.
	- `settings` : The system settings applet button.
	- `power` : The power button.
	- `nso` : The Nintendo Switch Online applet button.
	- `card` : The virtual game card applet button.
	- `share` : The game sharing applet button.
- `lock` supports the following
	- `lock` : The default home menu icon shown on the lock screen.

More folders and extra images may be added in the future.

An example of a valid nxtheme v2 file structure is the following:
```
mytheme.nxtheme
├── manifest.json
├── home
│   ├── layout.json
│   ├── bg.jpg
│   ├── album.png
│   ├── news.png
├── lock
│   ├── layout.json
├── set
│   ├── bg.dds
```