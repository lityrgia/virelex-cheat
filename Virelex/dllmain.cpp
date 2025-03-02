#include <Windows.h>
#include <iostream>
#include "options.h"
#include "MinHook.h"

#include "GL/glew.h"

#if defined _M_X64
#pragma comment(lib, "libMinHook.x64.lib")
#elif defined _M_IX86
#pragma comment(lib, "libMinHook.x86.lib")
#endif

#include "hooks.h"
#include <GL/gl.h>
#pragma comment(lib, "OpenGL32.lib")

//Pop it, pop it, right up out the blue type shit
//Put the Double O in a mood
//Serotonin, that's a cool type mix
//Oxycontin, mix it with balloons(Alright)
//East Atlanta feelin' like a zoo type mix (Open up)
//Shawty, shawty strippin', takin' shrooms
//Eatin' all the Percs, on some food type shit (Open up)
//Shawty make it work, 9 - to - 5 type shit(Open up)

void cheatInitialize() {
    if (MH_Initialize() != MH_OK) {
        std::cout << "Failed to initialize MinHook.\n";
        return;
    }

     options::monitorX = (float)GetSystemMetrics(SM_CXSCREEN);
     options::monitorY = (float)GetSystemMetrics(SM_CYSCREEN);

    // ImGuiHook::Load(renderGui);

    if (MH_CreateHook(&glDrawElements, Hooks::get_glDrawElements(), reinterpret_cast<void**>(&fn_glDrawElements)) != MH_OK) std::cout << "[-] Failed to create hook for glDrawElements.\n";
    if (MH_CreateHook(&glReadPixels, Hooks::get_glReadPixels(), reinterpret_cast<void**>(&fn_glReadPixels)) != MH_OK) std::cout << "[-] Failed to create hook for glReadPixels.\n";
    if (MH_CreateHook(&glFlush, Hooks::get_glFlush(), reinterpret_cast<void**>(&fn_glFlush)) != MH_OK) std::cout << "[-] Failed to create hook for glFlush.\n";
    if (MH_CreateHook(&BitBlt, Hooks::get_BitBlt(), reinterpret_cast<void**>(&fn_BitBlt)) != MH_OK) std::cout << "[-] Failed to create hook for BitBlt.\n";

    if (MH_EnableHook(&glDrawElements) != MH_OK) std::cout << "[-] Failed to enable hook glDrawElements\n";
    if (MH_EnableHook(&glReadPixels) != MH_OK) std::cout << "[-] Failed to enable hook glReadPixels\n";
    if (MH_EnableHook(&glFlush) != MH_OK) std::cout << "[-] Failed to enable hook glFlush\n";
    if (MH_EnableHook(&BitBlt) != MH_OK) std::cout << "[-] Failed to enable hook BitBlt\n";
}

void createConsole() {
    AllocConsole();

    FILE* stream;
    freopen_s(&stream, "CONOUT$", "w", stdout);
    freopen_s(&stream, "CONOUT$", "w", stderr);
}

bool __stdcall DllMain(HMODULE hModule, DWORD ulReason, LPVOID lpReserved) {
    if (ulReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);

        //createConsole(); 

        cheatInitialize();
    }
    else if (ulReason == DLL_PROCESS_DETACH) {
        MH_Uninitialize();
    }

    return true;
}