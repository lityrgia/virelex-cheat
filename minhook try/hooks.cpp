#include <Windows.h>
#include <string>
#include <iostream>
#include "include/GL/glew.h"
#include "options.h"
#pragma comment(lib, "glew32s.lib")
#include "hooks.h"
#include <GL/gl.h>
#pragma comment(lib, "OpenGL32.lib")

void WINAPI Hooks::hk_glDrawArrays(GLenum mode, GLint first, GLsizei count) {

    static bool toggleSee2 = false;
    static bool keyDown = false;

    GLenum currMode = mode;

    if (GetAsyncKeyState(menu::bToggleInsightOne) & 0x8000) {
        if (!keyDown) {
            toggleSee2 = !toggleSee2;
            keyDown = true;
        }
    }
    else {
        keyDown = false;
    }

    if (toggleSee2 && menu::bInsightOne) {
        mode = GL_POINTS;
    }

    fn_glDrawArrays(mode, first, count);

    if (toggleSee2 && menu::bInsightOne) {
        mode = currMode;
    }
}

unsigned char color[4] = { options::chamsColor.x, options::chamsColor.y, options::chamsColor.z, options::chamsColor.w};

void WINAPI Hooks::hk_glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {

    static bool glewInitialized = false;

    if (!glewInitialized) {
        if (glewInit() != GLEW_OK) {
            MessageBoxA(NULL, "Не удалось инициализировать GLEW", "Ошибка", MB_ICONERROR);
        }
        glewInitialized = true;
    }

    static bool toggleChams = false;
    static bool keyDown = false;

    if (GetAsyncKeyState(menu::bToggleChams) & 0x8000) {
        if (!keyDown) {
            toggleChams = !toggleChams;
            keyDown = true;
        }
    }
    else {
        keyDown = false;
    }

    GLint currentTexture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentTexture);

    bool isReady = count > 5000 && count < 300000 && mode == 4;
    bool canRet = false;

    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);

    GLfloat emissionColor[4];

    if (toggleChams && menu::bChamsEnabled) {
        if (isReady) {
            GLint location = glGetUniformLocation(currentProgram, "isAnimated");
            GLint isAnimated;

            if (location == -1) {
                goto THE_END;
            }
            else {
                glGetUniformiv(currentProgram, location, &isAnimated);
            }

            if (isAnimated != 1) goto THE_END;

            location = glGetUniformLocation(currentProgram, "emissionColor");

            if (location == -1) {
                goto THE_END;
            }
            else {
                glGetUniformfv(currentProgram, location, emissionColor);
                glUniform3f(location, options::chamsColor.x, options::chamsColor.y, options::chamsColor.z);
            }

            GLint numTextures;
            glGetProgramiv(currentProgram, GL_ACTIVE_UNIFORMS, &numTextures);

            canRet = true;

            static GLuint textureGreen = 0;

            if (textureGreen == 0) {
                glGenTextures(1, &textureGreen);
            }

            for (GLint i = 0; i < numTextures; ++i) {
                location = glGetUniformLocation(currentProgram, ("usedTextures[" + std::to_string(i) + "]").c_str());

                if (location != -1) {
                    GLint sampler;
                    glGetUniformiv(currentProgram, location, &sampler);

                    glActiveTexture(GL_TEXTURE0 + sampler);
                    glBindTexture(GL_TEXTURE_2D, textureGreen);

                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, color);

                    glDepthRange(0.0, 0.1);
                }
            }
        }
    }

THE_END:

    fn_glDrawElements(mode, count, type, indices);

    if (toggleChams && menu::bChamsEnabled) {
        if (isReady && canRet) {
            glDepthRange(0.0, 1.0);
            GLint location = glGetUniformLocation(currentProgram, "emissionColor");
            glUniform3f(location, emissionColor[0], emissionColor[1], emissionColor[2]);
            glBindTexture(GL_TEXTURE_2D, currentTexture);
        }
    }
}

BOOL WINAPI Hooks::hk_SwapBuffers(HDC hDC) {

    glPopAttrib();
    glDepthFunc(GL_LEQUAL);

    static bool toggleSee = false;
    static bool keyDown = false;

    if (GetAsyncKeyState(menu::bToggleInsightTwo) & 0x8000) {
        if (!keyDown) {
            toggleSee = !toggleSee;
            keyDown = true;
        }
    }
    else {
        keyDown = false;
    }

    if (toggleSee && menu::bInsightTwo) {
        glDepthFunc(GL_ALWAYS);
    }

    glPushAttrib(GL_DEPTH);
    return fn_SwapBuffers(hDC);
}
