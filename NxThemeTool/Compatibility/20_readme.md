# 20.0 Fixes

This firmware changes the position of multiple items and most importantly removes the SystemAppletPos animation.
To handle the major breakage we introduce the LayoutTargetVersion in the layout json. This helps us detect pre-20 layouts and fix them with this patch.

## DowngradeTo19
This patch "downgrades" the positions of the various root elements and removes the new applet buttons (except for the NS online button) in the hope that breakage will be minor for most layouts.

## LegacyAppletButtons
Also, to match the behavior of NoOnlineButton11 we extend it to also remove all the new applet buttons.
Note that this is additive to the DowngradeTo19 patch and must be added on top of it.

## FlowLayoutFix
20.0 changes the size of applet icons. The flow layout takes over the notification applet icon background to make its top bar. I tried adding a global fix but the button sizes are set by an animation, adding that to the fix patch would quickly grow too big to maintain and port to future qlaunch versions if needed.
Instead, we just change the scale of the button in the flow layout to work with the new size. 

## CarefulLayout
Same issue with careful layout, but also bump the scale of the album icon