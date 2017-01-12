#include <imgui.h>

extern "C" void igText(const char* fmt);
extern "C" void igRender();
extern "C" bool igSliderFloat(const char* label, float* v, float v_min, float v_max, const char* display_format, float power);
extern "C" bool mouseEvent(int x, int y, bool gdown, bool gleft, bool gright);

void igText(const char* fmt)
{
    ImGui::Text(fmt);
}

void igRender()
{
    ImGui::Render();
}

bool igSliderFloat(const char* label, float* v, float v_min, float v_max, const char* display_format, float power)
{
    return ImGui::SliderFloat(label, v, v_min, v_max, display_format, power);
}

bool mouseEvent(int x, int y, bool gdown, bool gleft, bool gright)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2((float)x, (float)y);

    if (gdown && gleft)
        io.MouseDown[0] = true;
    else
        io.MouseDown[0] = false;

    if (gdown && gright)
        io.MouseDown[1] = true;
    else
        io.MouseDown[1] = false;

    return true;
}
