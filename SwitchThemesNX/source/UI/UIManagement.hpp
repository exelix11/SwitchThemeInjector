#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "glad.h"

#include "imgui/imgui.h"

namespace UIMNG {

	extern float WRatio, HRatio;

	extern GLFWwindow* mainWindow;

	bool InitUI();
	void ExitUI();
}

bool AppMainLoop();
void SetAppShouldClose();

void UiStartFrame();
void UiEndFrame();