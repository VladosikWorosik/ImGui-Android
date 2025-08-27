# ImGui Android for AIDE Lite Mod, Android Studio

Minimal NativeActivity + OpenGL ES 3.0 app wired for Dear ImGui, using ndk-build. Works in AIDE Lite Mod and Android Studio.

## Setup in AIDE Lite Mod

1. Open this folder as an Android app project.
2. In Settings → Build & Run → Manage native code, ensure NDK is installed.
3. Copy Dear ImGui sources into `app/src/main/jni/imgui`:
   - Core: `imgui.h`, `imconfig.h` (optional), `imgui.cpp`, `imgui_draw.cpp`, `imgui_tables.cpp`, `imgui_widgets.cpp`, `imgui_demo.cpp`, `imgui_internal.h`, `imstb_rectpack.h`, `imstb_textedit.h`, `imstb_truetype.h`
   - Backends: `backends/imgui_impl_android.h`, `backends/imgui_impl_android.cpp`, `backends/imgui_impl_opengl3.h`, `backends/imgui_impl_opengl3.cpp`
4. Build and run. You should see a window with a checkbox and optionally the ImGui demo.

## Notes
- Requires GLES 3.0 device (API 18+). ABIs set to `armeabi-v7a` and `arm64-v8a`.
- Files to adjust: `Application.mk` (ABIs), `Android.mk` (sources), `main.cpp` (your UI).
- If using Android Studio, Gradle files are included; enable externalNativeBuild (ndkBuild).
