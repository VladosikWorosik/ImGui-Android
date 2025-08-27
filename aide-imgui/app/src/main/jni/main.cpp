#include <android/native_activity.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_android.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#define LOG_TAG "ImGuiNative"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

struct RendererState {
	EGLDisplay display = EGL_NO_DISPLAY;
	EGLSurface surface = EGL_NO_SURFACE;
	EGLContext context = EGL_NO_CONTEXT;
	int32_t width = 0;
	int32_t height = 0;
	bool imguiInitialized = false;
};

static int32_t HandleInput(struct android_app* app, AInputEvent* event) {
	return ImGui_ImplAndroid_HandleInputEvent(event);
}

static void TerminateDisplay(RendererState* state) {
	if (state->imguiInitialized) {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplAndroid_Shutdown();
		ImGui::DestroyContext();
		state->imguiInitialized = false;
	}
	if (state->display != EGL_NO_DISPLAY) {
		if (state->surface != EGL_NO_SURFACE) {
			eglDestroySurface(state->display, state->surface);
			state->surface = EGL_NO_SURFACE;
		}
		if (state->context != EGL_NO_CONTEXT) {
			eglDestroyContext(state->display, state->context);
			state->context = EGL_NO_CONTEXT;
		}
		eglTerminate(state->display);
		state->display = EGL_NO_DISPLAY;
	}
}

static bool InitDisplay(struct android_app* app, RendererState* state) {
	const EGLint cfgAttribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_DEPTH_SIZE, 16,
		EGL_STENCIL_SIZE, 8,
		EGL_NONE
	};

	EGLint numConfigs;
	EGLConfig config;
	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (display == EGL_NO_DISPLAY) {
		LOGE("eglGetDisplay failed");
		return false;
	}
	if (!eglInitialize(display, 0, 0)) {
		LOGE("eglInitialize failed");
		return false;
	}

	if (!eglChooseConfig(display, cfgAttribs, &config, 1, &numConfigs)) {
		LOGE("eglChooseConfig failed");
		return false;
	}

	EGLint format;
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	const EGLint ctxAttribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 3,
		EGL_NONE
	};

	EGLSurface surface = eglCreateWindowSurface(display, config, app->window, nullptr);
	if (surface == EGL_NO_SURFACE) {
		LOGE("eglCreateWindowSurface failed");
		return false;
	}
	EGLContext context = eglCreateContext(display, config, nullptr, ctxAttribs);
	if (context == EGL_NO_CONTEXT) {
		LOGE("eglCreateContext failed");
		return false;
	}
	if (!eglMakeCurrent(display, surface, surface, context)) {
		LOGE("eglMakeCurrent failed");
		return false;
	}

	state->display = display;
	state->surface = surface;
	state->context = context;

	EGLint w, h;
	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);
	state->width = w;
	state->height = h;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	ImGui::StyleColorsDark();

	ImGui_ImplAndroid_Init(app->window);
	ImGui_ImplOpenGL3_Init("#version 300 es");
	state->imguiInitialized = true;

	return true;
}

static void DrawFrame(RendererState* state) {
	if (!state->imguiInitialized) return;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplAndroid_NewFrame();
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)state->width, (float)state->height);
	ImGui::NewFrame();

	// Example window
	static bool show_demo = true;
	ImGui::Begin("Hello, ImGui on Android");
	ImGui::Text("GLES3 + NativeActivity");
	ImGui::Checkbox("Show Demo", &show_demo);
	ImGui::End();
	if (show_demo)
		ImGui::ShowDemoWindow(&show_demo);

	ImGui::Render();
	glViewport(0, 0, state->width, state->height);
	glClearColor(0.10f, 0.18f, 0.24f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	eglSwapBuffers(state->display, state->surface);
}

static void HandleCmd(struct android_app* app, int32_t cmd) {
	RendererState* state = (RendererState*)app->userData;
	switch (cmd) {
		case APP_CMD_INIT_WINDOW:
			if (app->window != nullptr) {
				InitDisplay(app, state);
			}
			break;
		case APP_CMD_TERM_WINDOW:
			TerminateDisplay(state);
			break;
		case APP_CMD_CONFIG_CHANGED:
			if (state->display != EGL_NO_DISPLAY && state->surface != EGL_NO_SURFACE) {
				EGLint w, h;
				eglQuerySurface(state->display, state->surface, EGL_WIDTH, &w);
				eglQuerySurface(state->display, state->surface, EGL_HEIGHT, &h);
				state->width = w;
				state->height = h;
			}
			break;
	}
}

void android_main(struct android_app* app) {
	app->onAppCmd = HandleCmd;
	app->onInputEvent = HandleInput;
	RendererState state;
	app->userData = &state;

	int events;
	android_poll_source* source;
	while (true) {
		while (ALooper_pollAll(state.imguiInitialized ? 0 : -1, nullptr, &events, (void**)&source) >= 0) {
			if (source) source->process(app, source);
			if (app->destroyRequested != 0) {
				TerminateDisplay(&state);
				return;
			}
		}
		DrawFrame(&state);
	}
}

