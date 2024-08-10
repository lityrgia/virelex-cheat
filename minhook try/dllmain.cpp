#include <Windows.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include <string>
#include "options.h"
#include "MinHook.h"
#include "imguiHook.h"
#include "gui.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <fstream>

#if defined _M_X64
#pragma comment(lib, "libMinHook.x64.lib")
#elif defined _M_IX86
#pragma comment(lib, "libMinHook.x86.lib")
#endif

#include "hooks.h"

#include <GL/gl.h>
#pragma comment(lib, "OpenGL32.lib")

void cheatInitialize() {

      ImGuiHook::Load(renderGui);

      MH_CreateHook(&SwapBuffers, Hooks::getSwapBuffers(), reinterpret_cast<void**>(&fn_SwapBuffers));
      MH_CreateHook(&glDrawElements, Hooks::get_glDrawElements(), reinterpret_cast<void**>(&fn_glDrawElements));
      MH_CreateHook(&glDrawArrays, Hooks::get_glDrawArrays(), reinterpret_cast<void**>(&fn_glDrawArrays));

      MH_EnableHook(&SwapBuffers) == MH_OK;
      MH_EnableHook(&glDrawElements) == MH_OK;
      MH_EnableHook(&glDrawArrays) == MH_OK;

}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        cheatInitialize();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        ImGuiHook::Unload();
        break;
    }
    return TRUE;
}