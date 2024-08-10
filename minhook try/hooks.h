#pragma once
#include <Windows.h>
#include <functional>
#include <GL/gl.h>

inline decltype(&SwapBuffers) fn_SwapBuffers = &SwapBuffers;
inline decltype(&glDrawElements) fn_glDrawElements = &glDrawElements;
inline decltype(&glDrawArrays) fn_glDrawArrays = &glDrawArrays;

class Hooks {
public:
	static void WINAPI hk_glDrawArrays(GLenum mode, GLint first, GLsizei count);
    static void WINAPI hk_glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices);
    static BOOL WINAPI hk_SwapBuffers(HDC hDC);

    static void* getSwapBuffers() {
        return reinterpret_cast<void*>(&hk_SwapBuffers);
    }

    static void* get_glDrawArrays() {
        return reinterpret_cast<void*>(&hk_glDrawArrays);
    }

    static void* get_glDrawElements() {
        return reinterpret_cast<void*>(&hk_glDrawElements);
    }
};