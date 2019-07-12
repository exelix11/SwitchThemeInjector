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
#define _sleep(x) usleep(x * 1000)
#endif

#define LOGf(...) printf(__VA_ARGS__)

extern GLFWgamepadstate gamepad;
extern GLFWgamepadstate OldGamepad;

static inline bool KeyPressed(int glfwKey)
{
	return gamepad.buttons[glfwKey] && !OldGamepad.buttons[glfwKey];
}

static inline float StickAsButton(int index)
{
	return (gamepad.axes[index] != 0 && OldGamepad.axes[index] == 0) ? gamepad.axes[index] : 0;
}

static inline bool AnyNavButtonPressed()
{
	return	gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] || gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] || 
			gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] || gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT];
}

void PlatformInit();
void PlatformExit();
void PlatformAfterInit();
void PlatformGetInputs();
void PlatformImguiBinds();