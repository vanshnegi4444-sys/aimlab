#include "gl_loader.h"
#include <GLFW/glfw3.h> // only for glfwGetProcAddress
#include <cstdio>

// One definition for every extern declared in the header.
PFNGLACTIVETEXTUREPROC      glActiveTexture      = nullptr;
PFNGLBLENDEQUATIONPROC      glBlendEquation      = nullptr;
PFNGLBLENDFUNCSEPARATEPROC  glBlendFuncSeparate  = nullptr;
PFNGLGENBUFFERSPROC         glGenBuffers         = nullptr;
PFNGLBINDBUFFERPROC         glBindBuffer         = nullptr;
PFNGLBUFFERDATAPROC         glBufferData         = nullptr;
PFNGLDELETEBUFFERSPROC      glDeleteBuffers      = nullptr;
PFNGLGENVERTEXARRAYSPROC    glGenVertexArrays    = nullptr;
PFNGLBINDVERTEXARRAYPROC    glBindVertexArray    = nullptr;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC   glEnableVertexAttribArray  = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC  glDisableVertexAttribArray = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC       glVertexAttribPointer      = nullptr;
PFNGLCREATESHADERPROC       glCreateShader       = nullptr;
PFNGLSHADERSOURCEPROC       glShaderSource       = nullptr;
PFNGLCOMPILESHADERPROC      glCompileShader      = nullptr;
PFNGLGETSHADERIVPROC        glGetShaderiv        = nullptr;
PFNGLGETSHADERINFOLOGPROC   glGetShaderInfoLog   = nullptr;
PFNGLDELETESHADERPROC       glDeleteShader       = nullptr;
PFNGLCREATEPROGRAMPROC      glCreateProgram      = nullptr;
PFNGLATTACHSHADERPROC       glAttachShader       = nullptr;
PFNGLLINKPROGRAMPROC        glLinkProgram        = nullptr;
PFNGLGETPROGRAMIVPROC       glGetProgramiv       = nullptr;
PFNGLGETPROGRAMINFOLOGPROC  glGetProgramInfoLog  = nullptr;
PFNGLDETACHSHADERPROC       glDetachShader       = nullptr;
PFNGLDELETEPROGRAMPROC      glDeleteProgram      = nullptr;
PFNGLUSEPROGRAMPROC         glUseProgram         = nullptr;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
PFNGLUNIFORM1IPROC          glUniform1i          = nullptr;
PFNGLUNIFORM1FPROC          glUniform1f          = nullptr;
PFNGLUNIFORM2FPROC          glUniform2f          = nullptr;
PFNGLUNIFORM3FPROC          glUniform3f          = nullptr;
PFNGLUNIFORM4FPROC          glUniform4f          = nullptr;
PFNGLUNIFORMMATRIX4FVPROC   glUniformMatrix4fv   = nullptr;
PFNGLBINDSAMPLERPROC        glBindSampler        = nullptr;

bool LoadOpenGLFunctions() {
    // WHY a macro: 35 near-identical lines would hide a typo. The macro stringifies
    // the name (#name) for glfwGetProcAddress and assigns the typed pointer.
    // reinterpret_cast from GLFWglproc is the standard way to do this.
    bool ok = true;
#define LOAD_GL(name)                                                        \
    do {                                                                     \
        name = reinterpret_cast<decltype(name)>(glfwGetProcAddress(#name));   \
        if (!name) {                                                         \
            std::printf("[gl_loader] FAILED to load %s\n", #name);            \
            ok = false;                                                      \
        }                                                                    \
    } while (0)

    LOAD_GL(glActiveTexture);
    LOAD_GL(glBlendEquation);
    LOAD_GL(glBlendFuncSeparate);
    LOAD_GL(glGenBuffers);
    LOAD_GL(glBindBuffer);
    LOAD_GL(glBufferData);
    LOAD_GL(glDeleteBuffers);
    LOAD_GL(glGenVertexArrays);
    LOAD_GL(glBindVertexArray);
    LOAD_GL(glDeleteVertexArrays);
    LOAD_GL(glEnableVertexAttribArray);
    LOAD_GL(glDisableVertexAttribArray);
    LOAD_GL(glVertexAttribPointer);
    LOAD_GL(glCreateShader);
    LOAD_GL(glShaderSource);
    LOAD_GL(glCompileShader);
    LOAD_GL(glGetShaderiv);
    LOAD_GL(glGetShaderInfoLog);
    LOAD_GL(glDeleteShader);
    LOAD_GL(glCreateProgram);
    LOAD_GL(glAttachShader);
    LOAD_GL(glLinkProgram);
    LOAD_GL(glGetProgramiv);
    LOAD_GL(glGetProgramInfoLog);
    LOAD_GL(glDetachShader);
    LOAD_GL(glDeleteProgram);
    LOAD_GL(glUseProgram);
    LOAD_GL(glGetUniformLocation);
    LOAD_GL(glUniform1i);
    LOAD_GL(glUniform1f);
    LOAD_GL(glUniform2f);
    LOAD_GL(glUniform3f);
    LOAD_GL(glUniform4f);
    LOAD_GL(glUniformMatrix4fv);
    LOAD_GL(glBindSampler);
#undef LOAD_GL
    return ok;
}
