# ImGui Cross-Platform Starter (Hello World)

A minimal skeleton for building **Dear ImGui** applications that compile and run **both natively** (desktop) and on the **web** (WebAssembly via Emscripten), using a single shared codebase.

Inspired by the architecture and patterns used in the [Tracy Profiler](https://github.com/wolfpld/tracy) (which uses ImGui for its high-performance GUI) and modern cross-platform ImGui + GLFW + Emscripten setups.

## Features

- **Single source code** for native + web
- Uses **GLFW** + **OpenGL 3** (native) / **WebGL 2** (web) via Emscripten's `-sUSE_GLFW=3`
- Clean immediate-mode GUI with demo window included
- Easy to extend into a full tool/profiler/editor
- Modern CMake with FetchContent (downloads ImGui + GLFW automatically)
- **Mobile-friendly web build** (Phase 1 scaling implemented)
- VSync enabled, dark theme, keyboard/gamepad nav

## Project Structure

```
imgui-crossplatform-starter/
├── .github/
│   └── workflows/
│       └── ci.yml        # GitHub Actions CI (native + web builds + Pages deploy)
├── web/
│   └── shell.html      # Custom Emscripten shell (mobile responsive + high-DPI)
├── CMakeLists.txt
├── README.md
├── scripts/
│   └── test-build.sh     # Local build & smoke test helper
├── src/
│   └── main.cpp          # All your GUI code lives here (shared!)
└── (build*/ generated)
```

## Prerequisites

### For Native Desktop
- C++17 compiler (GCC, Clang, MSVC)
- CMake >= 3.16
- Internet connection (for FetchContent on first configure)

### For Web (Emscripten)
- [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) installed and activated (`emsdk activate`)
- Same CMake

## Build Instructions

### 1. Native Desktop (Linux / macOS / Windows)

```bash
mkdir build-native && cd build-native
cmake ..
cmake --build . --config Release -j
./app          # or app.exe on Windows
```

The executable will open a window with the hello world GUI + full ImGui demo.

### 2. WebAssembly (runs in any modern browser)

```bash
# Activate emsdk first if needed:
# source ~/emsdk/emsdk_env.sh

mkdir build-web && cd build-web
emcmake cmake ..
cmake --build . --config Release -j

# Serve the files (any static HTTP server)
python3 -m http.server 8080
# or: npx serve .

# Open http://localhost:8080/index.html in your browser
```

You should see the same GUI running inside the browser canvas. It feels native!

**Mobile note**: The web version now includes Phase 1 mobile scaling (high-DPI font & style scaling, larger touch targets, proper framebuffer handling).

## Continuous Integration & GitHub Pages (GitHub Actions)

This repository includes a ready-to-use GitHub Actions workflow (`.github/workflows/ci.yml`) that:

- Builds the **native Linux** version on every push/PR
- Builds the **WebAssembly** version using Emscripten
- Runs a headless smoke test for the native app (using `xvfb`)
- **Deploys the web build automatically to GitHub Pages** (on push to `main`/`master`)

**Live Demo URL** (after first deployment):  
**https://fractotally.github.io/imgui-crossplatform-starter/** (replace with your username/repo name)

### One-time Setup
1. Push the changes (including the new workflow).
2. Go to **Repository Settings → Pages**.
3. Under **Source**, select **GitHub Actions** and save.
4. The `deploy-web` job will publish the built WASM app on the next successful push to `main`.

### Workflow Features
- `mymindstorm/setup-emsdk` for Emscripten
- Ninja for fast builds
- Artifacts for manual download
- Secure GitHub Pages deployment

View/edit the workflow: [`.github/workflows/ci.yml`](.github/workflows/ci.yml)

To customize (add Windows/macOS jobs, caching, etc.), edit the YAML.

## How It Works (Architecture)

- **GLFW** abstracts window creation, input, and OpenGL context for both platforms.
  - Native: real desktop window + OpenGL 3.3 Core
  - Web: Emscripten translates GLFW calls → HTML5 canvas + WebGL2 events
- **ImGui backends** (`imgui_impl_glfw` + `imgui_impl_opengl3`) are the same files for both.
- **Main loop difference** (only place with `#ifdef __EMSCRIPTEN__`):
  - Native: classic `while (!glfwWindowShouldClose()) { ... }`
  - Web: `emscripten_set_main_loop_arg(...)` so the browser can control animation frames and stay responsive.
- All your actual UI code (`ImGui::Begin`, buttons, plots, etc.) is 100% shared and platform-agnostic.

This is the same pattern that makes Tracy's profiler GUI able to target both desktop and WASM web viewer.

## Mobile Improvements (Phase 1 Implemented)

- High-DPI scaling via `devicePixelRatio`
- `ScaleAllSizes()` + larger `FramePadding` / `ItemSpacing`
- Increased `TouchExtraPadding` for finger-friendly targets
- Proper `DisplaySize` + `DisplayFramebufferScale` updated every frame
- Improved responsive shell with better resize/orientation handling
- `NoMouseCursorChange` on web

Future phases can add even more mobile-specific behavior.

## Customizing / Next Steps

1. **Add your own windows** inside `main_loop()` in `src/main.cpp`.
2. **Custom fonts**: 
   - Native: load with `io.Fonts->AddFontFromFileTTF(...)`
   - Web: use `--preload-file fonts/Roboto.ttf@/fonts` in `target_link_options` and load from virtual FS.
3. **Add more libraries**: ImPlot, ImGuiFileDialog, etc. — just add their sources to CMake.
4. **Theming / DPI**: Tracy does advanced font handling + DPI scaling — easy to add here.
5. **Production polish**:
   - Disable `.ini` saving on web (`io.IniFilename = nullptr;`)
   - Handle canvas resize properly (the current code does basic framebuffer size)
   - Add error callbacks, etc.

## Why This Approach? (Lessons from Tracy)

Tracy's profiler GUI is a masterclass in high-performance immediate-mode UIs:
- Rebuild UI every frame (no retained widget state hell)
- Separate heavy data processing from rendering when possible
- Works great on web because everything is compiled to efficient WASM + WebGL

This starter gives you the boilerplate so you can focus on building your tool's actual functionality (timelines, plots, memory views, node editors, etc.).

## Troubleshooting

- **Web build fails with GLFW/WebGL errors**: Make sure you used `emcmake cmake` and have a recent Emscripten.
- **Fonts look bad on web**: Add font preloading or use embedded font (ImGui has `ProggyClean` etc. built-in).
- **HiDPI / scaling issues**: The code now automatically scales using `devicePixelRatio`.
- **Slow on web**: Profile with browser devtools + Emscripten `--profiling` flags. Tracy itself proves complex UIs can run great at 60fps in WASM.

## License

This skeleton is provided under the MIT License (same as Dear ImGui). Feel free to use it as the foundation for your own projects.

Happy building! If you create something cool with it, consider sharing back improvements. 

Built as a practical starting point inspired by real-world production use in profilers like Tracy.