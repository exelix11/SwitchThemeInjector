# 11.0 Fixes.

This firmware added the NS online button that shifts the position of applet icons. The position is shifted with an animation file called RdtBase_SystemAppletPos. To make old layouts compatible we simply remove that animation.

The RdtBase_SystemAppletPos changes the position and size of the applet buttons root element, to make layout developers' lives easier this is automatically patched away.

It's possible to override this by including that file in the layout json.

## NoOnlineButton

Also for themes that want to emulate pre-11 layouts we have a patch to remove the new icon