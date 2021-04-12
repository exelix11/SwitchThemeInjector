#include "Platform.hpp"
#include <iostream>
#include "../UI/UIManagement.hpp"
#include "../UI/UI.hpp"
#include <cstring>

#ifdef __SWITCH__
#include <switch.h>
#include <unistd.h>
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

	hidInitializeTouchScreen();

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
	ImGuiIO& io = ImGui::GetIO();
	HidTouchScreenState state = { 0 };
	if (hidGetTouchScreenStates(&state, 1) && state.count)
	{
		auto x = state.touches[0].x / GFX::WRatio;
		auto y = state.touches[0].y / GFX::HRatio;

		io.MousePos = ImVec2(x, y);
		io.MouseDown[0] = true;
	}
	else io.MouseDown[0] = false;
}

void PlatformSleep(float time)
{
	usleep(time * 1000);
}

char InputBuffer[32];
const char* PlatformTextInput(const char* current)
{
	std::memset(InputBuffer, 0, sizeof(InputBuffer));

	SwkbdConfig kbd;

	Result rc = swkbdCreate(&kbd, 0);
	if (R_FAILED(rc))
		throw std::runtime_error("swkbdCreate failed : " + std::to_string(rc));

	swkbdConfigMakePresetDefault(&kbd);
	swkbdConfigSetTextDrawType(&kbd, SwkbdTextDrawType_Line);
	
	if (current)
		swkbdConfigSetInitialText(&kbd, current);

	swkbdConfigSetStringLenMax(&kbd, sizeof(InputBuffer) - 1);
	if (R_FAILED(swkbdShow(&kbd, InputBuffer, sizeof(InputBuffer))))
	{
		std::strncpy(InputBuffer, current, sizeof(InputBuffer));
		InputBuffer[sizeof(InputBuffer) - 1] = 0;
	}

	swkbdClose(&kbd);
	return InputBuffer;
}

void PlatformReboot() 
{
	bpcInitialize();
	bpcRebootSystem();
}
#endif