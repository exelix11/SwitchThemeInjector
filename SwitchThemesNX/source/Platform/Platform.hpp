#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifndef __SWITCH__
#define ASSET(_str) "./romfs/" _str
#define SD_PREFIX "F:"
#else
#include <switch.h>
#define ASSET(_str) "romfs:/" _str
#define SD_PREFIX "sdmc:"
#endif

#define LOGf(...) printf(__VA_ARGS__)

extern GLFWgamepadstate gamepad;
extern GLFWgamepadstate OldGamepad;

//imgui navbuttons are cleared after end of the frame
//these are needed for getting the pressed directions either from dpad or sticks in Update or out of the drawing loop 
extern bool NAV_UP;
extern bool NAV_DOWN;
extern bool NAV_LEFT;
extern bool NAV_RIGHT;

//Used when running in applet mode on switch
extern bool UseLowMemory;

static inline bool KeyPressed(int glfwKey)
{
	return gamepad.buttons[glfwKey] && !OldGamepad.buttons[glfwKey];
}

static inline float StickAsButton(int index)
{
#define ABS(x) (x < 0 ? -x : x)
#define STICK_BTN(x) (ABS(x) > .3f)
	if (STICK_BTN(gamepad.axes[index]) && !STICK_BTN(OldGamepad.axes[index]))
		return gamepad.axes[index];
	else return 0;
#undef STICK_BTN
#undef ABS
}

void PlatformInit();
void PlatformExit();
void PlatformAfterInit();
void PlatformGetInputs();
void PlatformImguiBinds();
void PlatformSleep(float time);