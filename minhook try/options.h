#pragma once
#include "include/imgui/imgui.h"

namespace menu {
	inline bool isMenuVisible{ true };
	inline bool bChamsEnabled{ false };
	inline bool bInsightOne{ false };
	inline bool bInsightTwo{ false };

	inline unsigned short bToggleInsightOne{ 0x0 };
	inline unsigned short bToggleInsightTwo{ 0x0 };
	inline unsigned short bToggleChams{ 0x0 };

	inline unsigned short bToggleButton{ 0x2d };
}

namespace options {
	inline ImVec4 chamsColor{0.0f, 1.0f, 0.0f, 1.0f};
}