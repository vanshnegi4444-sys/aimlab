#pragma once

// WHY this file exists: on Windows, opengl32.lib only exposes OpenGL 1.1.
// Everything newer (shaders, VAOs, VBOs — the whole 3.3 core profile) must be
// fetched at runtime with a "get proc address" call. GLFW gives us
// glfwGetProcAddress for that. This is a minimal hand-written loader declaring
// EXACTLY the entry points our renderer + Dear ImGui's OpenGL3 backend use.
// (Alternatives like GLEW/glad would add another dependency; ~35 functions is
// small enough to own outright, and every line stays readable.)

#include <windows.h>   // for APIENTRY, and must come before GL/gl.h
#include <GL/gl.h>     // OpenGL 1.1 types + constants + 1.1 functions
#include <cstddef>     // ptrdiff_t

// ---- Types that MSVC's 1.1-era gl.h does not define -------------------------
typedef char      GLchar;     // used by shader info-log calls
typedef ptrdiff_t GLsizeiptr; // used by glBufferData

// ---- Constants from OpenGL >= 1.2 that MSVC's gl.h lacks --------------------
// Values are ABI-stable (fixed by the GL spec), guarded so a newer SDK header
// that already defines them will not cause redefinition errors.
#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER              0x8892
#endif
#ifndef GL_ELEMENT_ARRAY_BUFFER
#define GL_ELEMENT_ARRAY_BUFFER      0x8893
#endif
#ifndef GL_ARRAY_BUFFER_BINDING
#define GL_ARRAY_BUFFER_BINDING      0x8894
#endif
#ifndef GL_VERTEX_ARRAY_BINDING
#define GL_VERTEX_ARRAY_BINDING      0x85B5
#endif
#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW               0x88E4
#endif
#ifndef GL_DYNAMIC_DRAW
#define GL_DYNAMIC_DRAW              0x88E8
#endif
#ifndef GL_STREAM_DRAW
#define GL_STREAM_DRAW               0x88E0
#endif
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER             0x8B31
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER           0x8B30
#endif
#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS            0x8B81
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS               0x8B82
#endif
#ifndef GL_INFO_LOG_LENGTH
#define GL_INFO_LOG_LENGTH           0x8B84
#endif
#ifndef GL_CURRENT_PROGRAM
#define GL_CURRENT_PROGRAM           0x8B8D
#endif
#ifndef GL_TEXTURE0
#define GL_TEXTURE0                  0x84C0
#endif
#ifndef GL_ACTIVE_TEXTURE
#define GL_ACTIVE_TEXTURE            0x84E0
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE             0x812F
#endif
#ifndef GL_FUNC_ADD
#define GL_FUNC_ADD                  0x8006
#endif
#ifndef GL_BLEND_SRC_RGB
#define GL_BLEND_SRC_RGB             0x80C9
#endif
#ifndef GL_BLEND_DST_RGB
#define GL_BLEND_DST_RGB             0x80C8
#endif
#ifndef GL_BLEND_SRC_ALPHA
#define GL_BLEND_SRC_ALPHA           0x80CB
#endif
#ifndef GL_BLEND_DST_ALPHA
#define GL_BLEND_DST_ALPHA           0x80CA
#endif
#ifndef GL_BLEND_EQUATION_RGB
#define GL_BLEND_EQUATION_RGB        0x8009
#endif
#ifndef GL_BLEND_EQUATION_ALPHA
#define GL_BLEND_EQUATION_ALPHA      0x883D
#endif
#ifndef GL_SAMPLER_BINDING
#define GL_SAMPLER_BINDING           0x8919
#endif

// ---- Function pointers for everything NEWER than OpenGL 1.1 ----------------
// Functions that ARE in gl.h (glClear, glViewport, glBindTexture, ... come
// from opengl32.lib) are deliberately NOT redeclared here — doing so would
// be a compile error. Each pointer below is filled by LoadOpenGLFunctions().
typedef void     (APIENTRY *PFNGLACTIVETEXTUREPROC)(GLenum texture);
typedef void     (APIENTRY *PFNGLBLENDEQUATIONPROC)(GLenum mode);
typedef void     (APIENTRY *PFNGLBLENDFUNCSEPARATEPROC)(GLenum sfRGB, GLenum dfRGB, GLenum sfA, GLenum dfA);
typedef void     (APIENTRY *PFNGLGENBUFFERSPROC)(GLsizei n, GLuint* buffers);
typedef void     (APIENTRY *PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void     (APIENTRY *PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
typedef void     (APIENTRY *PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint* buffers);
typedef void     (APIENTRY *PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint* arrays);
typedef void     (APIENTRY *PFNGLBINDVERTEXARRAYPROC)(GLuint array);
typedef void     (APIENTRY *PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint* arrays);
typedef void     (APIENTRY *PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef void     (APIENTRY *PFNGLDISABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef void     (APIENTRY *PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean norm, GLsizei stride, const void* ptr);
typedef GLuint   (APIENTRY *PFNGLCREATESHADERPROC)(GLenum type);
typedef void     (APIENTRY *PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar* const* str, const GLint* len);
typedef void     (APIENTRY *PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void     (APIENTRY *PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint* params);
typedef void     (APIENTRY *PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei maxLen, GLsizei* len, GLchar* log);
typedef void     (APIENTRY *PFNGLDELETESHADERPROC)(GLuint shader);
typedef GLuint   (APIENTRY *PFNGLCREATEPROGRAMPROC)(void);
typedef void     (APIENTRY *PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void     (APIENTRY *PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void     (APIENTRY *PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint* params);
typedef void     (APIENTRY *PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei maxLen, GLsizei* len, GLchar* log);
typedef void     (APIENTRY *PFNGLDETACHSHADERPROC)(GLuint program, GLuint shader);
typedef void     (APIENTRY *PFNGLDELETEPROGRAMPROC)(GLuint program);
typedef void     (APIENTRY *PFNGLUSEPROGRAMPROC)(GLuint program);
typedef GLint    (APIENTRY *PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar* name);
typedef void     (APIENTRY *PFNGLUNIFORM1IPROC)(GLint loc, GLint v0);
typedef void     (APIENTRY *PFNGLUNIFORM1FPROC)(GLint loc, GLfloat v0);
typedef void     (APIENTRY *PFNGLUNIFORM2FPROC)(GLint loc, GLfloat v0, GLfloat v1);
typedef void     (APIENTRY *PFNGLUNIFORM3FPROC)(GLint loc, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void     (APIENTRY *PFNGLUNIFORM4FPROC)(GLint loc, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void     (APIENTRY *PFNGLUNIFORMMATRIX4FVPROC)(GLint loc, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void     (APIENTRY *PFNGLBINDSAMPLERPROC)(GLuint unit, GLuint sampler);

extern PFNGLACTIVETEXTUREPROC      glActiveTexture;
extern PFNGLBLENDEQUATIONPROC      glBlendEquation;
extern PFNGLBLENDFUNCSEPARATEPROC  glBlendFuncSeparate;
extern PFNGLGENBUFFERSPROC         glGenBuffers;
extern PFNGLBINDBUFFERPROC         glBindBuffer;
extern PFNGLBUFFERDATAPROC         glBufferData;
extern PFNGLDELETEBUFFERSPROC      glDeleteBuffers;
extern PFNGLGENVERTEXARRAYSPROC    glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC    glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC  glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC      glVertexAttribPointer;
extern PFNGLCREATESHADERPROC       glCreateShader;
extern PFNGLSHADERSOURCEPROC       glShaderSource;
extern PFNGLCOMPILESHADERPROC      glCompileShader;
extern PFNGLGETSHADERIVPROC        glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC   glGetShaderInfoLog;
extern PFNGLDELETESHADERPROC       glDeleteShader;
extern PFNGLCREATEPROGRAMPROC      glCreateProgram;
extern PFNGLATTACHSHADERPROC       glAttachShader;
extern PFNGLLINKPROGRAMPROC        glLinkProgram;
extern PFNGLGETPROGRAMIVPROC       glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC  glGetProgramInfoLog;
extern PFNGLDETACHSHADERPROC       glDetachShader;
extern PFNGLDELETEPROGRAMPROC      glDeleteProgram;
extern PFNGLUSEPROGRAMPROC         glUseProgram;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORM1IPROC          glUniform1i;
extern PFNGLUNIFORM1FPROC          glUniform1f;
extern PFNGLUNIFORM2FPROC          glUniform2f;
extern PFNGLUNIFORM3FPROC          glUniform3f;
extern PFNGLUNIFORM4FPROC          glUniform4f;
extern PFNGLUNIFORMMATRIX4FVPROC   glUniformMatrix4fv;
extern PFNGLBINDSAMPLERPROC        glBindSampler;

// Loads every pointer above via glfwGetProcAddress.
// MUST be called once, after the OpenGL context is created AND made current
// (glfwMakeContextCurrent). Returns false if any pointer failed to load —
// that means the driver did not really give us a 3.3 core context.
bool LoadOpenGLFunctions();
