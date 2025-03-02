#include <Windows.h>
#include <string>
#include <iostream>
#include "GL/glew.h"
#include "options.h"
#pragma comment(lib, "glew32s.lib")
#include "hooks.h"
#include <GL/gl.h>
#pragma comment(lib, "OpenGL32.lib")
#include <algorithm>

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        MessageBoxA(NULL, ("Shader compilation failed: " + std::string(infoLog)).c_str(), "Error", MB_ICONERROR);
    }
    return shader;
}

void createScrProgram(GLint& shaderProgram) {
    const char* vertShaderSource = R"glsl(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        void main() {
            gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
    )glsl";

    const char* fragShaderSource = R"glsl(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D screenTexture;
        void main() {
            FragColor = texture(screenTexture, TexCoord);
        }
    )glsl";

    GLuint vertShader = compileShader(GL_VERTEX_SHADER, vertShaderSource);
    GLuint fragShader = compileShader(GL_FRAGMENT_SHADER, fragShaderSource);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        MessageBoxA(NULL, ("Shader program linking failed: " + std::string(infoLog)).c_str(), "Error", MB_ICONERROR);
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
}

void createVAOquad(GLuint& vao, GLuint& vbo, GLuint &ebo) {
    const float quadVertices[] = {
            -1.0f,  1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 1.0f
    };
    const unsigned int indices2[] = {
        0, 1, 3,
        1, 2, 3
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices2), indices2, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

void PrintActiveUniforms(GLuint program) {
    GLint numUniforms = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numUniforms);

    std::cout << "Active uniforms in program " << program << ":\n";

    for (GLint i = 0; i < numUniforms; i++) {
        char name[256];
        GLsizei length;
        GLint size;
        GLenum type;

        glGetActiveUniform(program, i, sizeof(name), &length, &size, &type, name);
        GLint location = glGetUniformLocation(program, name);

        std::cout << "  [" << i << "] "
            << "Name: " << name
            << ", Type: " << type
            << ", Size: " << size
            << ", Location: " << location
            << std::endl;
    }
}

static GLuint chamsFBO = 0;
static GLuint chamsTexture = 0;
static GLuint quadVAO = 0, quadVBO = 0, quadEBO = 0;

static bool shouldBeCleaned = false;
static bool glewInitialized = false;

HGLRC chamsContext = 0;
HDC chamsHDC = 0;

bool canStart = false;

bool keyPress = false;

void WINAPI Hooks::hk_glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {

    if (!glewInitialized) {
        if (glewInit() != GLEW_OK) {
            MessageBoxA(NULL, "Failed to activate GLEW", "Error", MB_ICONERROR);
        }
        glewInitialized = true;
    }

    if (GetAsyncKeyState(options::bToggleChams) & 0x8000) {
        if (!keyPress) { 
            options::bChamsEnabled = !options::bChamsEnabled;
            keyPress = true;
        }
    }
    else keyPress = false;

    if (!options::bChamsEnabled) {
        fn_glDrawElements(mode, count, type, indices);
        return;
    }

    GLint currentProgram, activeTexture, textureBinding, arrayBufferBinding, elementArrayBufferBinding, vertexArrayBinding, originalFBO;
    GLint viewport[4];
    GLfloat clearColor[4], emissionColor[4];
    
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColor);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureBinding);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &arrayBufferBinding);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementArrayBufferBinding);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertexArrayBinding);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &originalFBO);

    if (chamsFBO == 0) {
        glGenFramebuffers(1, &chamsFBO);
        glGenTextures(1, &chamsTexture);
        glBindTexture(GL_TEXTURE_2D, chamsTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, options::monitorX, options::monitorY,
            0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (quadVAO == 0) {
        createVAOquad(quadVAO, quadVBO, quadEBO);
    }

    bool isReady = count > 3000 && count < 300000 && mode == 4; 

    chamsContext = wglGetCurrentContext();
    chamsHDC = wglGetCurrentDC();

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
            glUniform3f(location, 0.0f, 1.0f, 0.0f);
        }

        canStart = true;

        glBindFramebuffer(GL_FRAMEBUFFER, chamsFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, chamsTexture, 0);

        glViewport(0, 0, options::monitorX, options::monitorY);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            glBindFramebuffer(GL_FRAMEBUFFER, originalFBO);
        }

        fn_glDrawElements(mode, count, type, indices);

        glBindFramebuffer(GL_FRAMEBUFFER, originalFBO);
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

        glUseProgram(currentProgram);

         location = glGetUniformLocation(currentProgram, "emissionColor");
        glUniform3f(location, emissionColor[0], emissionColor[1], emissionColor[2]);

        glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);

        glActiveTexture(activeTexture);
        glBindTexture(GL_TEXTURE_2D, textureBinding);

        glBindBuffer(GL_ARRAY_BUFFER, arrayBufferBinding);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementArrayBufferBinding);
        glBindVertexArray(vertexArrayBinding);
    }
    
    THE_END:

    fn_glDrawElements(mode, count, type, indices);
}

static GLuint bypassTexture = -1;

void WINAPI Hooks::hk_glFlush() { 
    if (!glewInitialized || !canStart || !options::bToggleChams) {
        fn_glFlush();
        return;
    }

    if (wglGetCurrentContext() == chamsContext) {

        GLint orgFBO, vaoBinding, textureBinding, currentProgram, activeTexture;
        GLint viewport[4];

        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &orgFBO);
        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
        glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureBinding);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vaoBinding);

        if (bypassTexture == -1) {
            glGenTextures(1, &bypassTexture);
            glBindTexture(GL_TEXTURE_2D, bypassTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport[2], viewport[3],
                0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        glBindFramebuffer(GL_READ_FRAMEBUFFER, orgFBO);

        glBindTexture(GL_TEXTURE_2D, bypassTexture);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, viewport[2], viewport[3]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, chamsTexture);

        if (!glIsProgram(options::screenShaderProgram)) {
            createScrProgram(options::screenShaderProgram);
        }
        glUseProgram(options::screenShaderProgram);
        glUniform1i(glGetUniformLocation(options::screenShaderProgram, "screenTexture"), 0);

        glBindVertexArray(quadVAO);
        fn_glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, chamsFBO);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, orgFBO);
        glActiveTexture(activeTexture);
        glBindTexture(GL_TEXTURE_2D, textureBinding);
        glUseProgram(currentProgram);
        glBindVertexArray(vaoBinding);
    }

    canStart = false;

    if (options::debugEn) {

        options::debugEn = false;
        //saveScreenshot(options::monitorX, options::monitorY);
        //saveScreenshotUsingBitBlt(options::monitorX, options::monitorY);
    }

    fn_glFlush();
}

static GLuint bypassQuadVAO = -1, bypassQuadVBO = 0, bypassQuadEBO = 0;

void hideChams() {
    HGLRC currentContext = wglGetCurrentContext();
    HDC hdc = wglGetCurrentDC();

    bool isSameContext = currentContext == chamsContext;
    if (!isSameContext) wglMakeCurrent(chamsHDC, chamsContext);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bypassTexture);

    createScrProgram(options::bypassShaderProgram);
    glUseProgram(options::bypassShaderProgram);
    glUniform1i(glGetUniformLocation(options::bypassShaderProgram, "screenTexture"), 0);

    createVAOquad(bypassQuadVAO, bypassQuadVBO, bypassQuadEBO);
    glBindVertexArray(bypassQuadVAO);
    fn_glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    if (!isSameContext) wglMakeCurrent(hdc, currentContext);

    glFinish();
}

void WINAPI Hooks::hk_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* data) {
    if (!options::bToggleChams) {
        fn_glReadPixels(x, y, width, height, format, type, data);
        return;
    }

    if (!glewInitialized) {
        fn_glReadPixels(x, y, width, height, format, type, data);
        return;
    }

    hideChams();

    fn_glReadPixels(x, y, width, height, format, type, data);
}

void WINAPI Hooks::hk_BitBlt(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, DWORD rop) {
    if (!options::bToggleChams) {
        fn_BitBlt(hdc, x, y, cx, cy, hdcSrc, x1, y1, rop);
        return;
    }

    hideChams();

    SwapBuffers(wglGetCurrentDC());

    fn_BitBlt(hdc, x, y, cx, cy, hdcSrc, x1, y1, rop);
}