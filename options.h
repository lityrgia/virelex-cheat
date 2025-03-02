#pragma once
#include "imgui/imgui.h"

namespace options {
	inline float monitorX;
	inline float monitorY;
	inline int screenShaderProgram = -1;
	inline int bypassShaderProgram = -1;
	inline ImVec4 chamsColor{ 0.0f, 1.0f, 0.0f, 1.0f };
	inline bool debugEn = false;
	inline bool bChamsEnabled{ false };
	inline unsigned short bToggleChams{ VK_F3 };
}