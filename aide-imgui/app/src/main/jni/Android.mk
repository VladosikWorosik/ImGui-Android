LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := imgui_native
LOCAL_SRC_FILES := \
	main.cpp \
	imgui/imgui.cpp \
	imgui/imgui_draw.cpp \
	imgui/imgui_widgets.cpp \
	imgui/imgui_tables.cpp \
	imgui/imgui_demo.cpp \
	imgui/backends/imgui_impl_android.cpp \
	imgui/backends/imgui_impl_opengl3.cpp

LOCAL_CPPFLAGS += -DGL_GLEXT_PROTOTYPES -DIMGUI_IMPL_OPENGL_ES3 -std=c++17 -fexceptions -frtti
LOCAL_LDLIBS := -landroid -llog -lEGL -lGLESv3
LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)

