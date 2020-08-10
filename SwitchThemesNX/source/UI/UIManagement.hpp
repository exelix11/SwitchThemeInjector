#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "glad.h"

#include "imgui/imgui.h"

namespace GFX {

	extern float WRatio, HRatio;

	extern GLFWwindow* mainWindow;

	bool Init();
	void Exit();

	void StartFrame();
	void EndFrame();
}

namespace App {
	bool MainLoop();
	void Quit();
}