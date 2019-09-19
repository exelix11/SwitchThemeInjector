#ifndef __SWITCH__

#include "Platform.hpp"
#include <iostream>
#include "../UI/UIManagement.hpp"
#include "../UI/UI.hpp"

using namespace UIMNG;

GLFWgamepadstate gamepad;
GLFWgamepadstate OldGamepad;

bool NAV_UP;
bool NAV_DOWN;
bool NAV_LEFT;
bool NAV_RIGHT;

bool UseLowMemory = false;

static void windowKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Check for toggle-fullscreen combo
	if (key == GLFW_KEY_ENTER && mods == GLFW_MOD_ALT && action == GLFW_PRESS)
	{
		static int saved_x, saved_y, saved_width, saved_height;

		if (!glfwGetWindowMonitor(window))
		{
			// Back up window position/size
			glfwGetWindowPos(window, &saved_x, &saved_y);
			glfwGetWindowSize(window, &saved_width, &saved_height);

			// Switch to fullscreen mode
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		}
		else
		{
			// Switch back to windowed mode
			glfwSetWindowMonitor(window, nullptr, saved_x, saved_y, saved_width, saved_height, GLFW_DONT_CARE);
		}
	}
}

void PlatformAfterInit()
{
	glfwSetKeyCallback(mainWindow, windowKeyCallback);
}

void PlatformGetInputs()
{
	memcpy(&OldGamepad,&gamepad, sizeof(gamepad));

	if (glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad)) return;
	
	gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] = glfwGetKey(mainWindow, GLFW_KEY_LEFT);
	gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] = glfwGetKey(mainWindow, GLFW_KEY_RIGHT);
	gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] = glfwGetKey(mainWindow, GLFW_KEY_UP);
	gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] = glfwGetKey(mainWindow, GLFW_KEY_DOWN);
	gamepad.buttons[GLFW_GAMEPAD_BUTTON_START] = glfwGetKey(mainWindow, GLFW_KEY_ESCAPE);

	gamepad.buttons[GLFW_GAMEPAD_BUTTON_A] = glfwGetKey(mainWindow, GLFW_KEY_A) || glfwGetKey(mainWindow, GLFW_KEY_ENTER);
	gamepad.buttons[GLFW_GAMEPAD_BUTTON_B] = glfwGetKey(mainWindow, GLFW_KEY_S);
	gamepad.buttons[GLFW_GAMEPAD_BUTTON_X] = glfwGetKey(mainWindow, GLFW_KEY_X);
	gamepad.buttons[GLFW_GAMEPAD_BUTTON_Y] = glfwGetKey(mainWindow, GLFW_KEY_Z);
	gamepad.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] = glfwGetKey(mainWindow, GLFW_KEY_Q);
	gamepad.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] = glfwGetKey(mainWindow, GLFW_KEY_W);
}

void PlatformImguiBinds() 
{
	double mouseX, mouseY;
	glfwGetCursorPos(UIMNG::mainWindow, &mouseX, &mouseY);
	ImGuiIO &io = ImGui::GetIO();
	io.MousePos = ImVec2((float)mouseX / WRatio, (float)mouseY / HRatio);
	io.MouseDown[0] = glfwGetMouseButton(mainWindow, 0) == GLFW_PRESS;
}

void PlatformInit() {}
void PlatformExit() {}

void PlatformSleep(float time)
{
	_sleep((unsigned long)time);
}
#endif