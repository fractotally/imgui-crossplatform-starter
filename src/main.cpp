#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <cstdio>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Global state
static GLFWwindow* g_window = nullptr;
static bool g_show_demo_window = true;
static bool g_show_hello_window = true;
static int g_click_counter = 0;
static float g_slider_value = 0.5f;
static ImVec4 g_clear_color = ImVec4(0.1f, 0.1f, 0.15f, 1.00f);

void main_loop(void* arg) {
    (void)arg;

    // Poll events (on Emscripten with GLFW this processes input)
    glfwPollEvents();

    // === Update display size every frame (important for resize + mobile) ===
    int display_w, display_h;
    glfwGetFramebufferSize(g_window, &display_w, &display_h);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)display_w, (float)display_h);

    // Framebuffer scale is already set globally via ui_scale above,
    // but we can refresh it here if needed on resize
#ifdef __EMSCRIPTEN__
    float current_scale = EM_ASM_DOUBLE({ return window.devicePixelRatio || 1.0; });
    io.DisplayFramebufferScale = ImVec2(current_scale, current_scale);
#endif

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 1. Show the big demo window (most of the sample code is in ImGui::ShowDemoWindow()!)
    if (g_show_demo_window)
        ImGui::ShowDemoWindow(&g_show_demo_window);

    // 2. Show a simple "Hello, World" custom window
    if (g_show_hello_window) {
        ImGui::Begin("ImGui Cross-Platform Hello World", &g_show_hello_window, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Text("This is a skeleton GUI that runs natively (GLFW + OpenGL) and on the web (Emscripten + WebGL2).");
        ImGui::Text("Inspired by Tracy Profiler's approach to high-performance immediate mode UIs.");
        ImGui::Separator();

        ImGui::Text("Build once, run everywhere!");
        ImGui::Spacing();

        if (ImGui::Button("Click me!")) {
            g_click_counter++;
        }
        ImGui::SameLine();
        ImGui::Text("Clicks: %d", g_click_counter);

        ImGui::SliderFloat("Float slider", &g_slider_value, 0.0f, 1.0f, "%.2f");

        if (ImGui::ColorEdit3("Clear color", (float*)&g_clear_color)) {
            // color changed
        }

        ImGui::Spacing();
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                    1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::Separator();
        ImGui::TextWrapped("Tip: Toggle the Demo Window checkbox to explore hundreds of ImGui widgets and examples. "
                           "All code is shared between native and web builds. Perfect starting point for tools, profilers, or editors.");

        ImGui::End();
    }

    // 3. Simple control window
    {
        ImGui::Begin("Controls");
        ImGui::Checkbox("Show Demo Window", &g_show_demo_window);
        ImGui::Checkbox("Show Hello Window", &g_show_hello_window);
        ImGui::End();
    }

    // Rendering
    ImGui::Render();
    glViewport(0, 0, display_w, display_h);
    glClearColor(g_clear_color.x * g_clear_color.w, g_clear_color.y * g_clear_color.w, g_clear_color.z * g_clear_color.w, g_clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(g_window);
}

int main(int, char**) {
    // Setup GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
    }

    // Decide GL+GLSL versions
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

#ifdef __EMSCRIPTEN__
    // For WebGL 2 / GLES3
    glsl_version = "#version 300 es";
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    // Create window with graphics context
    g_window = glfwCreateWindow(1280, 720, "ImGui Cross-Platform Starter (Hello World)", nullptr, nullptr);
    if (g_window == nullptr) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(g_window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize GLAD for native OpenGL Core Profile function loading (Emscripten provides its own)
#ifndef __EMSCRIPTEN__
#if __has_include(<glad/glad.h>)
#include <glad/glad.h>
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        glfwTerminate();
        return 1;
    }
#endif
#endif

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.IniFilename = nullptr; // Disable .ini file for web/demo

    // === Mobile / High-DPI Scaling (Phase 1) ===
    float ui_scale = 1.0f;

#ifdef __EMSCRIPTEN__
    // Get device pixel ratio from browser for proper high-DPI scaling on mobile
    ui_scale = EM_ASM_DOUBLE({
        return window.devicePixelRatio || 1.0;
    });

    // Disable mouse cursor changes on touch devices (looks weird)
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    // Increase touch target size significantly for fingers
    io.TouchExtraPadding = ImVec2(12.0f, 12.0f);
#else
    // Desktop: you can detect monitor DPI here if desired
    ui_scale = 1.0f;
#endif

    // Apply global scaling
    io.FontGlobalScale = ui_scale;
    ImGui::GetStyle().ScaleAllSizes(ui_scale);

    // Make buttons and interactive elements more touch-friendly by default
    ImGuiStyle& style = ImGui::GetStyle();
    style.FramePadding = ImVec2(style.FramePadding.x * 1.5f, style.FramePadding.y * 1.5f);
    style.ItemSpacing  = ImVec2(style.ItemSpacing.x * 1.3f, style.ItemSpacing.y * 1.3f);

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(g_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts (optional: add custom fonts here)
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("fonts/Roboto-Regular.ttf", 16.0f); // would need preload on web

    // Main loop
#ifdef __EMSCRIPTEN__
    // Emscripten main loop - browser controls the frame rate
    emscripten_set_main_loop_arg(main_loop, nullptr, 0, true);
#else
    // Native main loop
    while (!glfwWindowShouldClose(g_window)) {
        main_loop(nullptr);
    }
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(g_window);
    glfwTerminate();

    return 0;
}