#ifdef __SWITCH__

#include "Platform.hpp"
#include <iostream>
#include <switch.h>
#include <unistd.h>
#include "../UI/UIManagement.hpp"
#include "../UI/UI.hpp"

//#define __NXLINK_ENABLE__

#ifdef __NXLINK_ENABLE__
static int nxlink_sock = -1;
#endif

GLFWgamepadstate gamepad;
GLFWgamepadstate OldGamepad;

bool NAV_UP;
bool NAV_DOWN;
bool NAV_LEFT;
bool NAV_RIGHT;

bool UseLowMemory = false;

void PlatformInit() 
{
	AppletType at = appletGetAppletType();
	if (at != AppletType_Application && at != AppletType_SystemApplication)
		UseLowMemory = true;

	romfsInit();
	socketInitializeDefault();
#ifdef __NXLINK_ENABLE__
	nxlink_sock = nxlinkStdio();
#endif
}

void PlatformExit() 
{
#ifdef __NXLINK_ENABLE__
	if (nxlink_sock != -1)
		close(nxlink_sock);
#endif
	socketExit();
	romfsExit();
}

void PlatformAfterInit() 
{

}

void PlatformGetInputs()
{
	memcpy(&OldGamepad, &gamepad, sizeof(gamepad));
	if (!glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad))
		std::cout << "Error reading from gamepad";
}

void PlatformImguiBinds() 
{
	ImGuiIO &io = ImGui::GetIO();
	u32 touch_count = hidTouchCount();
	if (touch_count == 1)
	{
		touchPosition touch;
		hidTouchRead(&touch, 0);
		io.MousePos = ImVec2(touch.px / UIMNG::WRatio, touch.py / UIMNG::HRatio);
		io.MouseDown[0] = true;
	}
	else io.MouseDown[0] = false;
}

void PlatformSleep(float time)
{
	usleep(time * 1000);
}
#endif