// nexus_vk_render.c — NexusStub EGL/GL com eglGetProcAddress redirecionando para libvulkan.so real
// v3 — Versão limpa e compilável para NVKL

#include <jni.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <stdint.h>

#define TAG "NexusVKRender"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// ═══════════════════════════════════════════════════════════════════════
// Vulkan types (minimalistas, sem depender do vulkan.h do NDK)
// ═══════════════════════════════════════════════════════════════════════
typedef struct VkInstance_T* VkInstance;
typedef uint64_t VkSurfaceKHR;
typedef int32_t  VkResult;
typedef uint32_t VkStructureType;
typedef void*    PFN_vkVoidFunction;

#define VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR 1000008000
#define VK_SUCCESS                    0
#define VK_ERROR_EXTENSION_NOT_PRESENT (-9)
#define VK_ERROR_INITIALIZATION_FAILED (-3)

typedef struct {
    VkStructureType sType;
    void*           pNext;
    int             flags;
    void*           window;  // ANativeWindow*
} VkAndroidSurfaceCreateInfoKHR;

typedef VkResult (*PFN_vkCreateAndroidSurfaceKHR)(
    VkInstance,
    const VkAndroidSurfaceCreateInfoKHR*,
    const void*,
    VkSurfaceKHR*
);

typedef PFN_vkVoidFunction (*PFN_vkGetInstanceProcAddr_t)(VkInstance, const char*);

// ═══════════════════════════════════════════════════════════════════════
// INIT — carrega libvulkan.so real ao carregar a lib
// ═══════════════════════════════════════════════════════════════════════
static void*                      libvulkan_handle = NULL;
static PFN_vkGetInstanceProcAddr_t g_vkGetInstanceProcAddr = NULL;
static PFN_vkCreateAndroidSurfaceKHR g_vkCreateAndroidSurfaceKHR = NULL;

__attribute__((constructor))
static void nexus_stub_init(void) {
    libvulkan_handle = dlopen("libvulkan.so", RTLD_NOW | RTLD_GLOBAL);
    if (libvulkan_handle) {
        LOGI("libvulkan.so carregada: %p", libvulkan_handle);
        g_vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr_t)
            dlsym(libvulkan_handle, "vkGetInstanceProcAddr");
        g_vkCreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR)
            dlsym(libvulkan_handle, "vkCreateAndroidSurfaceKHR");
        LOGI("vkGetInstanceProcAddr: %p", (void*)g_vkGetInstanceProcAddr);
        LOGI("vkCreateAndroidSurfaceKHR(direct): %p", (void*)g_vkCreateAndroidSurfaceKHR);
    } else {
        LOGE("FALHOU ao carregar libvulkan.so: %s", dlerror());
    }
}

// ═══════════════════════════════════════════════════════════════════════
// EGL STUBS
// ═══════════════════════════════════════════════════════════════════════
EGLDisplay eglGetDisplay(EGLNativeDisplayType d)             { (void)d; return (EGLDisplay)1; }

EGLBoolean eglInitialize(EGLDisplay d, EGLint *maj, EGLint *min) {
    (void)d;
    if (maj) *maj = 1;
    if (min) *min = 5;
    return EGL_TRUE;
}

EGLBoolean eglBindAPI(EGLenum api)                           { (void)api; return EGL_TRUE; }

EGLBoolean eglChooseConfig(EGLDisplay d, const EGLint *attribs,
                            EGLConfig *configs, EGLint size, EGLint *count) {
    (void)d; (void)attribs;
    if (configs && size > 0) configs[0] = (EGLConfig)1;
    if (count) *count = 1;
    return EGL_TRUE;
}

EGLSurface eglCreateWindowSurface(EGLDisplay d, EGLConfig c,
                                   EGLNativeWindowType w, const EGLint *a)
    { (void)d;(void)c;(void)w;(void)a; return (EGLSurface)1; }
EGLSurface eglCreatePbufferSurface(EGLDisplay d, EGLConfig c, const EGLint *a)
    { (void)d;(void)c;(void)a; return (EGLSurface)1; }
EGLContext eglCreateContext(EGLDisplay d, EGLConfig c, EGLContext s, const EGLint *a)
    { (void)d;(void)c;(void)s;(void)a; return (EGLContext)1; }
EGLBoolean eglMakeCurrent(EGLDisplay d, EGLSurface draw, EGLSurface read, EGLContext ctx)
    { (void)d;(void)draw;(void)read;(void)ctx; return EGL_TRUE; }
EGLBoolean eglSwapBuffers(EGLDisplay d, EGLSurface s)
    { (void)d;(void)s; return EGL_TRUE; }
EGLBoolean eglDestroyContext(EGLDisplay d, EGLContext c)
    { (void)d;(void)c; return EGL_TRUE; }
EGLBoolean eglDestroySurface(EGLDisplay d, EGLSurface s)
    { (void)d;(void)s; return EGL_TRUE; }
EGLBoolean eglTerminate(EGLDisplay d)
    { (void)d; return EGL_TRUE; }
EGLint     eglGetError(void)
    { return EGL_SUCCESS; }
EGLBoolean eglSwapInterval(EGLDisplay d, EGLint i)
    { (void)d;(void)i; return EGL_TRUE; }

EGLBoolean eglQuerySurface(EGLDisplay d, EGLSurface s, EGLint attribute, EGLint *value) {
    (void)d;(void)s;
    if (!value) return EGL_TRUE;
    const char* env_w = getenv("AWTSTUB_WIDTH");
    const char* env_h = getenv("AWTSTUB_HEIGHT");
    switch (attribute) {
        case EGL_WIDTH:  *value = env_w ? atoi(env_w) : 2408; break;
        case EGL_HEIGHT: *value = env_h ? atoi(env_h) : 1080; break;
        default:         *value = 0; break;
    }
    return EGL_TRUE;
}

EGLBoolean   eglQueryContext(EGLDisplay d, EGLContext c, EGLint a, EGLint *v)
    { (void)d;(void)c;(void)a; if(v) *v=0; return EGL_TRUE; }
const char*  eglQueryString(EGLDisplay d, EGLint n)
    { (void)d;(void)n; return ""; }
EGLContext   eglGetCurrentContext(void)          { return EGL_NO_CONTEXT; }
EGLSurface   eglGetCurrentSurface(EGLint which)  { (void)which; return EGL_NO_SURFACE; }
EGLDisplay   eglGetCurrentDisplay(void)          { return EGL_NO_DISPLAY; }

__eglMustCastToProperFunctionPointerType eglGetProcAddress(const char *name) {
    if (!name) return NULL;
    if (libvulkan_handle) {
        void* sym = dlsym(libvulkan_handle, name);
        if (sym) {
            LOGI("eglGetProcAddress(\"%s\") -> libvulkan: %p", name, sym);
            return (__eglMustCastToProperFunctionPointerType)sym;
        }
    }
    void* sym = dlsym(RTLD_DEFAULT, name);
    if (sym) {
        LOGI("eglGetProcAddress(\"%s\") -> RTLD_DEFAULT: %p", name, sym);
        return (__eglMustCastToProperFunctionPointerType)sym;
    }
    LOGE("eglGetProcAddress(\"%s\") -> NULL", name);
    return NULL;
}

// ═══════════════════════════════════════════════════════════════════════
// GL strings — reporta OpenGL 4.6 para passar as verificações do Minecraft
// ═══════════════════════════════════════════════════════════════════════
const GLubyte* glGetString(GLenum n) {
    switch(n) {
        case GL_VENDOR:                    return (GLubyte*)"ARM";
        case GL_RENDERER:                  return (GLubyte*)"Mali-G52 MC2";
        case GL_VERSION:                   return (GLubyte*)"4.6.0 NexusStub";
        case GL_SHADING_LANGUAGE_VERSION:  return (GLubyte*)"4.60 NexusStub";
        case GL_EXTENSIONS:                return (GLubyte*)"";
        default:                           return (GLubyte*)"";
    }
}

const GLubyte* glGetStringi(GLenum name, GLuint index) {
    (void)name;(void)index;
    return (GLubyte*)"";
}

// ═══════════════════════════════════════════════════════════════════════
// glGetIntegerv — valores críticos; default retorna 1 (NUNCA 0)
// ═══════════════════════════════════════════════════════════════════════
void glGetIntegerv(GLenum pn, GLint *d) {
    if (!d) return;
    switch (pn) {
        case 0x0D33: *d = 4096; break;  // GL_MAX_TEXTURE_SIZE
        case 0x8073: *d = 256;  break;  // GL_MAX_3D_TEXTURE_SIZE
        case 0x8069: *d = 4096; break;  // GL_MAX_CUBE_MAP_TEXTURE_SIZE
        case 0x8A2B: *d = 256;  break;  // GL_MAX_ARRAY_TEXTURE_LAYERS
        case 0x84FF: *d = 4096; break;  // GL_MAX_RECTANGLE_TEXTURE_SIZE
        case 0x84E8: *d = 4;    break;  // GL_MAX_TEXTURE_UNITS
        case 0x8872: *d = 16;   break;  // GL_MAX_TEXTURE_IMAGE_UNITS
        case 0x8B4C: *d = 16;   break;  // GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS
        case 0x8B4D: *d = 32;   break;  // GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS
        case 0x8869: *d = 16;   break;  // GL_MAX_VERTEX_ATTRIBS
        case 0x8B4A: *d = 4096; break;  // GL_MAX_VERTEX_UNIFORM_COMPONENTS
        case 0x8B49: *d = 1024; break;  // GL_MAX_FRAGMENT_UNIFORM_COMPONENTS
        case 0x8DFD: *d = 128;  break;  // GL_MAX_VERTEX_UNIFORM_BLOCKS
        case 0x8A2E: *d = 128;  break;  // GL_MAX_FRAGMENT_UNIFORM_BLOCKS
        case 0x8A2D: *d = 128;  break;  // GL_MAX_COMBINED_UNIFORM_BLOCKS
        case 0x8A29: *d = 256;  break;  // GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT — CRÍTICO
        case 0x8CDF: *d = 8;    break;  // GL_MAX_COLOR_ATTACHMENTS
        case 0x8D57: *d = 4;    break;  // GL_MAX_SAMPLES
        case 0x84E9: *d = 8;    break;  // GL_MAX_DRAW_BUFFERS
        case 0x8B8C: *d = 1024; break;  // GL_MAX_VARYING_FLOATS
        case 0x0B72: *d = 8;    break;  // GL_MAX_LIGHTS
        case 0x0D56: *d = 24;   break;  // GL_DEPTH_BITS
        case 0x0D57: *d = 8;    break;  // GL_STENCIL_BITS
        case 0x821B: *d = 4;    break;  // GL_MAJOR_VERSION
        case 0x821C: *d = 6;    break;  // GL_MINOR_VERSION
        case 0x0D36: *d = 32;   break;  // GL_MAX_MODELVIEW_STACK_DEPTH
        case 0x0D38: *d = 32;   break;  // GL_MAX_PROJECTION_STACK_DEPTH
        case 0x0D39: *d = 32;   break;  // GL_MAX_TEXTURE_STACK_DEPTH
        default:     *d = 1;    break;  // NUNCA retornar 0
    }
}

// ═══════════════════════════════════════════════════════════════════════
// GL STUBS — no-op
// ═══════════════════════════════════════════════════════════════════════
void glActiveTexture(GLenum t) {(void)t;}
void glAttachShader(GLuint p, GLuint s) {(void)p;(void)s;}
void glBindAttribLocation(GLuint p, GLuint i, const GLchar *n) {(void)p;(void)i;(void)n;}
void glBindBuffer(GLenum t, GLuint b) {(void)t;(void)b;}
void glBindFramebuffer(GLenum t, GLuint f) {(void)t;(void)f;}
void glBindRenderbuffer(GLenum t, GLuint r) {(void)t;(void)r;}
void glBindTexture(GLenum t, GLuint tex) {(void)t;(void)tex;}
void glBlendColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {(void)r;(void)g;(void)b;(void)a;}
void glBlendEquation(GLenum m) {(void)m;}
void glBlendEquationSeparate(GLenum r, GLenum a) {(void)r;(void)a;}
void glBlendFunc(GLenum s, GLuint d) {(void)s;(void)d;}
void glBlendFuncSeparate(GLenum sr, GLenum dr, GLenum sa, GLenum da) {(void)sr;(void)dr;(void)sa;(void)da;}
void glBufferData(GLenum t, GLsizeiptr s, const void *d, GLenum u) {(void)t;(void)s;(void)d;(void)u;}
void glBufferSubData(GLenum t, GLintptr o, GLsizeiptr s, const void *d) {(void)t;(void)o;(void)s;(void)d;}
GLenum glCheckFramebufferStatus(GLenum t) {(void)t; return GL_FRAMEBUFFER_COMPLETE;}
void glClear(GLbitfield m) {(void)m;}
void glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {(void)r;(void)g;(void)b;(void)a;}
void glClearDepthf(GLfloat d) {(void)d;}
void glClearStencil(GLint s) {(void)s;}
void glColorMask(GLboolean r, GLboolean g, GLboolean b, GLboolean a) {(void)r;(void)g;(void)b;(void)a;}
void glCompileShader(GLuint s) {(void)s;}
void glCompressedTexImage2D(GLenum t, GLint l, GLenum f, GLsizei w, GLsizei h,
                             GLint b, GLsizei s, const void *d) {(void)t;(void)l;(void)f;(void)w;(void)h;(void)b;(void)s;(void)d;}
void glCompressedTexSubImage2D(GLenum t, GLint l, GLint x, GLint y,
                                GLsizei w, GLsizei h, GLenum f, GLsizei s, const void *d)
    {(void)t;(void)l;(void)x;(void)y;(void)w;(void)h;(void)f;(void)s;(void)d;}
void glCopyTexImage2D(GLenum t, GLint l, GLenum f, GLint x, GLint y,
                       GLsizei w, GLsizei h, GLint b)
    {(void)t;(void)l;(void)f;(void)x;(void)y;(void)w;(void)h;(void)b;}
void glCopyTexSubImage2D(GLenum t, GLint l, GLint xo, GLint yo,
                          GLint x, GLint y, GLsizei w, GLsizei h)
    {(void)t;(void)l;(void)xo;(void)yo;(void)x;(void)y;(void)w;(void)h;}
GLuint glCreateProgram(void)    { return 1; }
GLuint glCreateShader(GLenum t) { (void)t; return 1; }
void glCullFace(GLenum m) {(void)m;}
void glDeleteBuffers(GLsizei n, const GLuint *b) {(void)n;(void)b;}
void glDeleteFramebuffers(GLsizei n, const GLuint *f) {(void)n;(void)f;}
void glDeleteProgram(GLuint p) {(void)p;}
void glDeleteRenderbuffers(GLsizei n, const GLuint *r) {(void)n;(void)r;}
void glDeleteShader(GLuint s) {(void)s;}
void glDeleteTextures(GLsizei n, const GLuint *t) {(void)n;(void)t;}
void glDepthFunc(GLenum f) {(void)f;}
void glDepthMask(GLboolean f) {(void)f;}
void glDepthRangef(GLfloat n, GLfloat fa) {(void)n;(void)fa;}
void glDetachShader(GLuint p, GLuint s) {(void)p;(void)s;}
void glDisable(GLenum c) {(void)c;}
void glDisableVertexAttribArray(GLuint i) {(void)i;}
void glDrawArrays(GLenum m, GLint f, GLsizei c) {(void)m;(void)f;(void)c;}
void glDrawElements(GLenum m, GLsizei c, GLenum t, const void *i) {(void)m;(void)c;(void)t;(void)i;}
void glEnable(GLenum c) {(void)c;}
void glEnableVertexAttribArray(GLuint i) {(void)i;}
void glFinish(void) {}
void glFlush(void) {}
void glFramebufferRenderbuffer(GLenum t, GLenum a, GLenum rt, GLuint r) {(void)t;(void)a;(void)rt;(void)r;}
void glFramebufferTexture2D(GLenum t, GLenum a, GLenum tt, GLuint tex, GLint l)
    {(void)t;(void)a;(void)tt;(void)tex;(void)l;}
void glFrontFace(GLenum m) {(void)m;}
void glGenBuffers(GLsizei n, GLuint *b)      { for(int i=0;i<n;i++) b[i]=i+1; }
void glGenerateMipmap(GLenum t) {(void)t;}
void glGenFramebuffers(GLsizei n, GLuint *f) { for(int i=0;i<n;i++) f[i]=i+1; }
void glGenRenderbuffers(GLsizei n, GLuint *r){ for(int i=0;i<n;i++) r[i]=i+1; }
void glGenTextures(GLsizei n, GLuint *t)     { for(int i=0;i<n;i++) t[i]=i+1; }
void glGetActiveAttrib(GLuint p, GLuint i, GLsizei b, GLsizei *l,
                        GLint *s, GLenum *t, GLchar *n)
    {(void)p;(void)i;(void)b;(void)l;(void)s;(void)t;(void)n;}
void glGetActiveUniform(GLuint p, GLuint i, GLsizei b, GLsizei *l,
                         GLint *s, GLenum *t, GLchar *n)
    {(void)p;(void)i;(void)b;(void)l;(void)s;(void)t;(void)n;}
void glGetAttachedShaders(GLuint p, GLsizei m, GLsizei *c, GLuint *s) {(void)p;(void)m;(void)c;(void)s;}
GLint  glGetAttribLocation(GLuint p, const GLchar *n)  {(void)p;(void)n; return 0;}
void   glGetBooleanv(GLenum pn, GLboolean *d)          {(void)pn; if(d) *d=GL_FALSE;}
void   glGetBufferParameteriv(GLenum t, GLenum v, GLint *d) {(void)t;(void)v; if(d) *d=0;}
GLenum glGetError(void)                                { return GL_NO_ERROR; }
void   glGetFloatv(GLenum pn, GLfloat *d)              {(void)pn; if(d) *d=0.0f;}
void   glGetFramebufferAttachmentParameteriv(GLenum t, GLenum a, GLenum pn, GLint *p)
    {(void)t;(void)a;(void)pn; if(p) *p=0;}
void   glGetProgramiv(GLuint p, GLenum pn, GLint *pa)  {(void)p;(void)pn; if(pa) *pa=GL_TRUE;}
void   glGetProgramInfoLog(GLuint p, GLsizei m, GLsizei *l, GLchar *i)
    {(void)p;(void)m;(void)l; if(i&&m>0) i[0]=0;}
void   glGetRenderbufferParameteriv(GLenum t, GLenum pn, GLint *p) {(void)t;(void)pn; if(p) *p=0;}
void   glGetShaderiv(GLuint s, GLenum pn, GLint *p)    {(void)s;(void)pn; if(p) *p=GL_TRUE;}
void   glGetShaderInfoLog(GLuint s, GLsizei m, GLsizei *l, GLchar *i)
    {(void)s;(void)m;(void)l; if(i&&m>0) i[0]=0;}
void   glGetShaderPrecisionFormat(GLenum st, GLenum pt, GLint *r, GLint *p)
    {(void)st;(void)pt;(void)r;(void)p;}
void   glGetShaderSource(GLuint s, GLsizei b, GLsizei *l, GLchar *src)
    {(void)s;(void)b;(void)l; if(src&&b>0) src[0]=0;}
void   glGetTexParameterfv(GLenum t, GLenum pn, GLfloat *p) {(void)t;(void)pn; if(p) *p=0.0f;}
void   glGetTexParameteriv(GLenum t, GLenum pn, GLint *p)   {(void)t;(void)pn; if(p) *p=0;}
void   glGetUniformfv(GLuint p, GLint l, GLfloat *pa) {(void)p;(void)l;(void)pa;}
void   glGetUniformiv(GLuint p, GLint l, GLint *pa)   {(void)p;(void)l; if(pa) *pa=0;}
GLint  glGetUniformLocation(GLuint p, const GLchar *n) {(void)p;(void)n; return 0;}
void   glGetVertexAttribfv(GLuint i, GLenum pn, GLfloat *p) {(void)i;(void)pn; if(p) *p=0.0f;}
void   glGetVertexAttribiv(GLuint i, GLenum pn, GLint *p)   {(void)i;(void)pn; if(p) *p=0;}
void   glGetVertexAttribPointerv(GLuint i, GLenum pn, void **p) {(void)i;(void)pn;(void)p;}
void   glHint(GLenum t, GLenum m) {(void)t;(void)m;}
GLboolean glIsBuffer(GLuint b)      {(void)b; return GL_FALSE;}
GLboolean glIsEnabled(GLenum c)     {(void)c; return GL_FALSE;}
GLboolean glIsFramebuffer(GLuint f) {(void)f; return GL_FALSE;}
GLboolean glIsProgram(GLuint p)     {(void)p; return GL_FALSE;}
GLboolean glIsRenderbuffer(GLuint r){(void)r; return GL_FALSE;}
GLboolean glIsShader(GLuint s)      {(void)s; return GL_FALSE;}
GLboolean glIsTexture(GLuint t)     {(void)t; return GL_FALSE;}
void glLineWidth(GLfloat w) {(void)w;}
void glLinkProgram(GLuint p) {(void)p;}
void glPixelStorei(GLenum pn, GLint p) {(void)pn;(void)p;}
void glPolygonOffset(GLfloat f, GLfloat u) {(void)f;(void)u;}
void glReadPixels(GLint x, GLint y, GLsizei w, GLsizei h,
                  GLenum f, GLenum t, void *pixels)
    {(void)x;(void)y;(void)w;(void)h;(void)f;(void)t;(void)pixels;}
void glReleaseShaderCompiler(void) {}
void glRenderbufferStorage(GLenum t, GLenum f, GLsizei w, GLsizei h)
    {(void)t;(void)f;(void)w;(void)h;}
void glSampleCoverage(GLfloat v, GLboolean i) {(void)v;(void)i;}
void glScissor(GLint x, GLint y, GLsizei w, GLsizei h) {(void)x;(void)y;(void)w;(void)h;}
void glShaderBinary(GLsizei c, const GLuint *s, GLenum f, const void *b, GLsizei l)
    {(void)c;(void)s;(void)f;(void)b;(void)l;}
void glShaderSource(GLuint s, GLsizei c, const GLchar *const*st, const GLint *l)
    {(void)s;(void)c;(void)st;(void)l;}
void glStencilFunc(GLenum f, GLint ref, GLuint m) {(void)f;(void)ref;(void)m;}
void glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
    {(void)face;(void)func;(void)ref;(void)mask;}
void glStencilMask(GLuint m) {(void)m;}
void glStencilMaskSeparate(GLenum f, GLuint m) {(void)f;(void)m;}
void glStencilOp(GLenum sf, GLenum df, GLenum dp) {(void)sf;(void)df;(void)dp;}
void glStencilOpSeparate(GLenum f, GLenum sf, GLenum df, GLenum dp)
    {(void)f;(void)sf;(void)df;(void)dp;}
void glTexImage2D(GLenum target, GLint level, GLint internalformat,
                   GLsizei width, GLsizei height, GLint border,
                   GLenum format, GLenum type, const void *pixels)
    {(void)target;(void)level;(void)internalformat;(void)width;(void)height;
     (void)border;(void)format;(void)type;(void)pixels;}
void glTexParameterf(GLenum t, GLenum pn, GLfloat p) {(void)t;(void)pn;(void)p;}
void glTexParameterfv(GLenum t, GLenum pn, const GLfloat *pa) {(void)t;(void)pn;(void)pa;}
void glTexParameteri(GLenum t, GLenum pn, GLint p) {(void)t;(void)pn;(void)p;}
void glTexParameteriv(GLenum t, GLenum pn, const GLint *pa) {(void)t;(void)pn;(void)pa;}
void glTexSubImage2D(GLenum t, GLint l, GLint xo, GLint yo,
                     GLsizei w, GLsizei h, GLenum f, GLenum tp, const void *pixels)
    {(void)t;(void)l;(void)xo;(void)yo;(void)w;(void)h;(void)f;(void)tp;(void)pixels;}
void glUniform1f(GLint l, GLfloat v0) {(void)l;(void)v0;}
void glUniform1fv(GLint l, GLsizei c, const GLfloat *v) {(void)l;(void)c;(void)v;}
void glUniform1i(GLint l, GLint v0) {(void)l;(void)v0;}
void glUniform1iv(GLint l, GLsizei c, const GLint *v) {(void)l;(void)c;(void)v;}
void glUniform2f(GLint l, GLfloat v0, GLfloat v1) {(void)l;(void)v0;(void)v1;}
void glUniform2fv(GLint l, GLsizei c, const GLfloat *v) {(void)l;(void)c;(void)v;}
void glUniform2i(GLint l, GLint v0, GLint v1) {(void)l;(void)v0;(void)v1;}
void glUniform2iv(GLint l, GLsizei c, const GLint *v) {(void)l;(void)c;(void)v;}
void glUniform3f(GLint l, GLfloat v0, GLfloat v1, GLfloat v2) {(void)l;(void)v0;(void)v1;(void)v2;}
void glUniform3fv(GLint l, GLsizei c, const GLfloat *v) {(void)l;(void)c;(void)v;}
void glUniform3i(GLint l, GLint v0, GLint v1, GLint v2) {(void)l;(void)v0;(void)v1;(void)v2;}
void glUniform3iv(GLint l, GLsizei c, const GLint *v) {(void)l;(void)c;(void)v;}
void glUniform4f(GLint l, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
    {(void)l;(void)v0;(void)v1;(void)v2;(void)v3;}
void glUniform4fv(GLint l, GLsizei c, const GLfloat *v) {(void)l;(void)c;(void)v;}
void glUniform4i(GLint l, GLint v0, GLint v1, GLint v2, GLint v3)
    {(void)l;(void)v0;(void)v1;(void)v2;(void)v3;}
void glUniform4iv(GLint l, GLsizei c, const GLint *v) {(void)l;(void)c;(void)v;}
void glUniformMatrix2fv(GLint l, GLsizei c, GLboolean tr, const GLfloat *v)
    {(void)l;(void)c;(void)tr;(void)v;}
void glUniformMatrix3fv(GLint l, GLsizei c, GLboolean tr, const GLfloat *v)
    {(void)l;(void)c;(void)tr;(void)v;}
void glUniformMatrix4fv(GLint l, GLsizei c, GLboolean tr, const GLfloat *v)
    {(void)l;(void)c;(void)tr;(void)v;}
void glUseProgram(GLuint p) {(void)p;}
void glValidateProgram(GLuint p) {(void)p;}
void glVertexAttrib1f(GLuint i, GLfloat v0) {(void)i;(void)v0;}
void glVertexAttrib1fv(GLuint i, const GLfloat *v) {(void)i;(void)v;}
void glVertexAttrib2f(GLuint i, GLfloat v0, GLfloat v1) {(void)i;(void)v0;(void)v1;}
void glVertexAttrib2fv(GLuint i, const GLfloat *v) {(void)i;(void)v;}
void glVertexAttrib3f(GLuint i, GLfloat v0, GLfloat v1, GLfloat v2) {(void)i;(void)v0;(void)v1;(void)v2;}
void glVertexAttrib3fv(GLuint i, const GLfloat *v) {(void)i;(void)v;}
void glVertexAttrib4f(GLuint i, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
    {(void)i;(void)v0;(void)v1;(void)v2;(void)v3;}
void glVertexAttrib4fv(GLuint i, const GLfloat *v) {(void)i;(void)v;}
void glVertexAttribPointer(GLuint i, GLint s, GLenum t, GLboolean n,
                            GLsizei st, const void *p)
    {(void)i;(void)s;(void)t;(void)n;(void)st;(void)p;}
void glViewport(GLint x, GLint y, GLsizei w, GLsizei h) {(void)x;(void)y;(void)w;(void)h;}

// ═══════════════════════════════════════════════════════════════════════
// GLFW stubs — libpojavexec.so é o stub GLFW real; estes são fallback
// ═══════════════════════════════════════════════════════════════════════
int glfwInit(void) { LOGI("glfwInit() stub"); return 1; }
void glfwTerminate(void) { LOGI("glfwTerminate() stub"); }
const char* glfwGetVersionString(void) { return "3.3.3 NexusStub"; }
int glfwInitVulkanLoader(void *loader) { (void)loader; return 1; }
int glfwGetPlatform(void) { return 0x00040000; /* GLFW_PLATFORM_ANDROID */ }
int glfwPlatformSupported(int platform) { (void)platform; return 1; }
long glfwCreateWindow(int w, int h, const char *title, long monitor, long window) {
    (void)w;(void)h;(void)title;(void)monitor;(void)window;
    LOGI("glfwCreateWindow() stub");
    return 1L;
}
void glfwDestroyWindow(long window) { (void)window; }
int  glfwWindowShouldClose(long window) { (void)window; return 0; }
void glfwSetWindowShouldClose(long window, int v) { (void)window;(void)v; }
int  glfwCreateWindowSurface(long instance, long window, long allocator, long *surface) {
    (void)instance;(void)window;(void)allocator;
    LOGI("glfwCreateWindowSurface() stub");
    if (surface) *surface = 0x12345678L;
    return 0;
}

// ═══════════════════════════════════════════════════════════════════════
// JNI — nativeCreateAndroidSurface para o VulkanMod
// Chamado por AndroidSurfaceCreator.java via JNI
// ═══════════════════════════════════════════════════════════════════════
JNIEXPORT jint JNICALL
Java_net_vulkanmod_vulkan_AndroidSurfaceCreator_nativeCreateAndroidSurface(
    JNIEnv *env, jclass clazz,
    jlong instance, jlong createInfo, jlong pAllocator, jlong pSurface)
{
    (void)env;(void)clazz;
    LOGI("=== nativeCreateAndroidSurface ===");
    LOGI("  instance:   0x%lX", (long)instance);
    LOGI("  createInfo: 0x%lX", (long)createInfo);

    if (createInfo == 0) {
        LOGE("createInfo pointer is NULL!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Carrega vkCreateAndroidSurfaceKHR via vkGetInstanceProcAddr se necessário
    if (!g_vkCreateAndroidSurfaceKHR && g_vkGetInstanceProcAddr) {
        g_vkCreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR)
            g_vkGetInstanceProcAddr((VkInstance)(uintptr_t)instance,
                                    "vkCreateAndroidSurfaceKHR");
        if (g_vkCreateAndroidSurfaceKHR) {
            LOGI("vkCreateAndroidSurfaceKHR via vkGetInstanceProcAddr: %p",
                 (void*)g_vkCreateAndroidSurfaceKHR);
        }
    }

    if (!g_vkCreateAndroidSurfaceKHR) {
        LOGE("vkCreateAndroidSurfaceKHR não disponível!");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    // Extrai o ANativeWindow do struct createInfo
    // VkAndroidSurfaceCreateInfoKHR: sType(4)+pad(4)+pNext(8)+flags(4)+pad(4)+window(8) = offset 24
    void* nativeWindow = *(void**)((uintptr_t)createInfo + 24);
    LOGI("  nativeWindow: %p", nativeWindow);

    if (!nativeWindow) {
        LOGE("ANativeWindow é NULL — surface não criada!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkAndroidSurfaceCreateInfoKHR* info = (VkAndroidSurfaceCreateInfoKHR*)(uintptr_t)createInfo;
    VkSurfaceKHR surface = 0;

    VkResult result = g_vkCreateAndroidSurfaceKHR(
        (VkInstance)(uintptr_t)instance,
        info,
        (const void*)(uintptr_t)pAllocator,
        &surface
    );

    if (result == VK_SUCCESS) {
        *(jlong*)(uintptr_t)pSurface = (jlong)surface;
        LOGI("✅ vkCreateAndroidSurfaceKHR OK: 0x%lX", (long)surface);
    } else {
        LOGE("❌ vkCreateAndroidSurfaceKHR falhou: %d", result);
    }

    return (jint)result;
}

// ═══════════════════════════════════════════════════════════════════════
// JNI_OnLoad
// ═══════════════════════════════════════════════════════════════════════
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    (void)vm;(void)reserved;
    LOGI("JNI_OnLoad: Nexus_VK_Render carregado");
    return JNI_VERSION_1_6;
}
