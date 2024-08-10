#include "include/imgui/imgui.h"
#include "options.h"
#include <string>
#include <Windows.h>

void setImguiStyle() {
    auto& colors = ImGui::GetStyle().Colors;

    colors[ImGuiCol_ResizeGrip] = ImColor(0, 0, 0, 0);
    colors[ImGuiCol_ResizeGripActive] = ImColor(0, 0, 0, 0);
    colors[ImGuiCol_ResizeGripHovered] = ImColor(0, 0, 0, 0);

    colors[ImGuiCol_Button] = ImColor(0, 0, 0, 0);
    colors[ImGuiCol_ButtonActive] = ImColor(255, 122, 0, 255);
    colors[ImGuiCol_ButtonHovered] = ImColor(255, 140, 0, 255);

    colors[ImGuiCol_SliderGrab] = ImColor(16, 24, 32, 255);
    colors[ImGuiCol_SliderGrabActive] = ImColor(16, 24, 32, 255);

    colors[ImGuiCol_CheckMark] = ImColor(0, 0, 0, 255);

    colors[ImGuiCol_FrameBg] = ImColor(255, 255, 255);
    colors[ImGuiCol_FrameBgActive] = ImColor(255, 255, 255);
    colors[ImGuiCol_FrameBgHovered] = ImColor(255, 255, 255);

    colors[ImGuiCol_PopupBg] = ImColor(255, 255, 255, 255);

    colors[ImGuiCol_WindowBg] = ImColor(16, 24, 32, 255);

    colors[ImGuiCol_TitleBg] = ImColor(16, 24, 32, 255);
    colors[ImGuiCol_TitleBgActive] = ImColor(16, 24, 32, 255);
    colors[ImGuiCol_TitleBgCollapsed] = ImColor(16, 24, 32, 255);

    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 0.0f;
    style.FramePadding = ImVec2(0, 0);
    style.WindowPadding = ImVec2(0, 0);
    style.ItemSpacing = ImVec2(0, 0);
    style.WindowRounding = 5.f;
    style.FrameRounding = 4.0f;
    style.GrabMinSize = 9.f;
    style.GrabRounding = 100.f;
}

bool ToggleButton(const char* str_id, bool* v, float button_width = 0.0f, float button_height = 0.0f) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // ¬ычисление размеров по умолчанию, если не указаны
    if (button_width == 0.0f) {
        button_width = ImGui::GetFrameHeight() * 1.55f;
    }
    if (button_height == 0.0f) {
        button_height = ImGui::GetFrameHeight();
    }

    float radius = button_height * 0.50f;

    ImGui::InvisibleButton(str_id, ImVec2(button_width, button_height));
    if (ImGui::IsItemClicked())
        *v = !*v;

    ImU32 col_bg = *v ? IM_COL32(255, 140, 0, 255) : IM_COL32(255, 255, 255, 255);
    ImU32 circle_bg = *v ? IM_COL32(255, 255, 255, 255) : IM_COL32(16, 24, 32, 255);

    draw_list->AddRectFilled(p, ImVec2(p.x + button_width, p.y + button_height), col_bg, button_height * 0.5f);
    draw_list->AddCircleFilled(ImVec2(*v ? (p.x + button_width - radius) : (p.x + radius), p.y + radius), radius - 1.5f, circle_bg);

    return *v;
}

void ShowColorPickerButton(ImVec4* color, int id) {
    ImGui::PushID(id);

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 1));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 1));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 1));

    if (ImGui::ColorButton("ColorButton", *color)) {
        ImGui::OpenPopup("ColorPickerPopup");
    }

    if (ImGui::BeginPopup("ColorPickerPopup")) {
        ImGui::ColorPicker4("##picker", (float*)color, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
        ImGui::EndPopup();
    }

    ImGui::PopStyleColor(3);
    ImGui::PopID();
}

ImVec2 menuSize(200, 365);
ImVec2 paddings(0.f, 15.f);
float leftMargin = 15.f;

static std::string keyName = "Press a key";
bool waitingForInsightOneKey = false;
bool waitingForInsightTwoKey = false;
bool waitingForChamsKey = false;

bool keyPressed = false;

void renderGui()
{
    ImGui::Begin("Virelex");
    ImGui::SetWindowSize(menuSize, ImGuiCond_Always);

    ImGui::Indent(leftMargin);
    ImGui::Dummy(paddings);

    ToggleButton("#chamsid", &menu::bChamsEnabled, 30, 20);
    ImGui::SameLine();
    ImGui::Text(" Chams ");
    ImGui::SameLine();
    ShowColorPickerButton(&options::chamsColor, 1);

    ImGui::Dummy(paddings);

    ToggleButton("#insight1", &menu::bInsightOne, 30, 20);
    ImGui::SameLine();
    ImGui::Text(" Insight 1 ");

    ImGui::Dummy(paddings);

    ToggleButton("#insight2", &menu::bInsightTwo, 30, 20);
    ImGui::SameLine();
    ImGui::Text(" Insight 2 ");

    ImGui::Dummy(paddings);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.f, 140.f / 255, 0, 1.f));
    if (ImGui::Button("Chams key", ImVec2(130, 25))) {
        waitingForChamsKey = true;
    }

    ImGui::Dummy(paddings);

    if (waitingForChamsKey) {
        ImGui::Text("Press key...");
        ImGui::SetWindowSize({menuSize.x, menuSize.y + 10}, ImGuiCond_Always);

        for (int key = 0; key < 256; ++key) {
            if (GetAsyncKeyState(key) & 0x8000) {
                menu::bToggleChams = key;
                waitingForChamsKey = false;
                break;
            }
        }
    }
    else {
        ImGui::Text("Key: % s", ImGui::GetKeyName(static_cast<ImGuiKey>(menu::bToggleChams)));
    }

    ImGui::Dummy(paddings);

    if (ImGui::Button(" Insight 1 key", ImVec2(130, 25))) {
        waitingForInsightOneKey = true;
    }

    ImGui::Dummy(paddings);

    if (waitingForInsightOneKey) {
        ImGui::Text("Press key...");

        for (int key = 0; key < 256; ++key) {
            if (GetAsyncKeyState(key) & 0x8000) {
                menu::bToggleInsightOne = key;
                waitingForInsightOneKey = false;
                break;
            }
        }
    }
    else {
        ImGui::Text("Key: % s", ImGui::GetKeyName(static_cast<ImGuiKey>(menu::bToggleInsightOne)));
    }

    ImGui::Dummy(paddings);

    if (ImGui::Button(" Insight 2 key", ImVec2(130, 25))) {
        waitingForInsightTwoKey = true;
    }

    ImGui::Dummy(paddings);

    if (waitingForInsightTwoKey) {
        ImGui::Text("Press key...");

        for (int key = 0; key < 256; ++key) {
            if (GetAsyncKeyState(key) & 0x8000) {
                menu::bToggleInsightTwo = key;
                waitingForInsightTwoKey = false;
                break;
            }
        }
    }
    else {
        ImGui::Text("Key: % s", ImGui::GetKeyName(static_cast<ImGuiKey>(menu::bToggleInsightTwo)));
    }
    
    ImGui::PopStyleColor();

    ImGui::End();
}