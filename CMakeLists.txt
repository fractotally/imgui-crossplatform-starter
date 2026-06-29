cmake_minimum_required(VERSION 3.16)
project(imgui-crossplatform-starter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(USE_FETCHCONTENT "Use FetchContent to download dependencies" ON)

if(USE_FETCHCONTENT)
    include(FetchContent)
endif()

# Dear ImGui
if(USE_FETCHCONTENT)
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.91.0
    )
    FetchContent_MakeAvailable(imgui)
else()
    # User can set IMGUI_DIR or have it as submodule
    if(NOT DEFINED IMGUI_DIR)
        set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
    endif()
endif()

set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC 
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# Platform specific setup
if(EMSCRIPTEN)
    message(STATUS "Configuring for Emscripten/WebAssembly")
    
    add_executable(app src/main.cpp)
    set_target_properties(app PROPERTIES 
        OUTPUT_NAME "index" 
        SUFFIX ".html"
    )
    
    target_link_libraries(app PRIVATE imgui)
    
    # Emscripten flags (USE_GLFW provides GLFW, MAX_WEBGL_VERSION=2 enables WebGL2)
    target_compile_options(app PRIVATE 
        "-sUSE_GLFW=3" 
        "-sMAX_WEBGL_VERSION=2"
    )
    target_link_options(app PRIVATE
        "-sUSE_GLFW=3"
        "-sMAX_WEBGL_VERSION=2"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sINITIAL_MEMORY=67108864"
        "-sWASM=1"
        "--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html"
        # For debugging: uncomment
        # "-gsource-map"
        # "-sASSERTIONS=1"
    )
    
    # Example for preloading assets (fonts, etc.)
    # target_link_options(app PRIVATE "--preload-file assets@/assets")
    
else()
    message(STATUS "Configuring for native desktop")
    
    # Optional GLAD for robust OpenGL function loading in Core Profile
    if(USE_FETCHCONTENT)
        FetchContent_Declare(
            glad
            GIT_REPOSITORY https://github.com/Dav1dde/glad.git
            GIT_TAG v2.0.8
        )
        # Configure GLAD for OpenGL 3.3 Core
        set(GLAD_API "gl:core=3.3" CACHE STRING "" FORCE)
        FetchContent_MakeAvailable(glad)
    endif()
    
    # GLFW via FetchContent for easy native build (no system install required)
    if(USE_FETCHCONTENT)
        FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.4
        )
        FetchContent_MakeAvailable(glfw)
    else()
        find_package(glfw3 REQUIRED)
    endif()
    
    find_package(OpenGL REQUIRED)
    
    add_executable(app src/main.cpp)
    
    target_link_libraries(app PRIVATE 
        imgui 
        glfw 
        OpenGL::GL
    )
    
    if(TARGET glad)
        target_link_libraries(app PRIVATE glad)
        target_include_directories(app PRIVATE ${glad_SOURCE_DIR}/include)
    endif()
    
    if(WIN32)
        target_link_libraries(app PRIVATE opengl32)
    endif()
    
    # For macOS, usually fine; add "-framework OpenGL" if needed in future
endif()

# Common include for main (backends headers)
target_include_directories(app PRIVATE ${imgui_SOURCE_DIR}/backends)

# Copy any assets or note in README
# For production, consider embedding fonts with ImGui::GetIO().Fonts->AddFontFromMemoryTTF or preload.