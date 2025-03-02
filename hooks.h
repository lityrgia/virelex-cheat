#pragma once
#include <Windows.h>
#include <functional>
#include <GL/gl.h>

inline decltype(&glDrawElements) fn_glDrawElements = &glDrawElements;
inline decltype(&glReadPixels) fn_glReadPixels = &glReadPixels;
inline decltype(&glFlush) fn_glFlush = &glFlush;
inline decltype(&BitBlt) fn_BitBlt = &BitBlt;

class Hooks {
public:
    static void WINAPI hk_glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices);
    static void WINAPI hk_glReadPixels(GLint x,GLint y,GLsizei width, GLsizei height, GLenum format,  GLenum type,void* data);
    static void WINAPI hk_glFlush();
    static void WINAPI hk_BitBlt(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, DWORD rop);

    static void* get_glDrawElements() {
        return reinterpret_cast<void*>(&hk_glDrawElements);
    }

    static void* get_glFlush() {
        return reinterpret_cast<void*>(&hk_glFlush);
    }

    static void* get_BitBlt() {
        return reinterpret_cast<void*>(&hk_BitBlt);
    }

    static void* get_glReadPixels() {
        return reinterpret_cast<void*>(&hk_glReadPixels);
    }
};