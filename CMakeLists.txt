cmake_minimum_required(VERSION 3.20)

project(kubechaser LANGUAGES C CXX)

include(CTest)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(KUBECHASER_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(KUBECHASER_THIRD_PARTY_DIR "${KUBECHASER_ROOT}/third_party")

option(KUBECHASER_BUILD_APP "Build the KubeChaser desktop application" ON)
option(KUBECHASER_WARNINGS_AS_ERRORS "Treat compiler warnings as errors in KubeChaser targets" OFF)

# Warning flags for KubeChaser's own targets. Linked PRIVATE so they never
# propagate to third-party code; third-party headers are included as SYSTEM.
add_library(kubechaser_warnings INTERFACE)
add_library(kubechaser::warnings ALIAS kubechaser_warnings)
if(MSVC)
    target_compile_options(kubechaser_warnings INTERFACE /W4 /permissive-)
    if(KUBECHASER_WARNINGS_AS_ERRORS)
        target_compile_options(kubechaser_warnings INTERFACE /WX)
    endif()
else()
    target_compile_options(kubechaser_warnings INTERFACE -Wall -Wextra -Wpedantic)
    if(KUBECHASER_WARNINGS_AS_ERRORS)
        target_compile_options(kubechaser_warnings INTERFACE -Werror)
    endif()
endif()

# Mark a third-party target's usage-requirement include directories as SYSTEM
# so its headers do not trigger warnings in KubeChaser code.
function(kubechaser_mark_system_includes target)
    get_target_property(_aliased ${target} ALIASED_TARGET)
    if(_aliased)
        set(target ${_aliased})
    endif()
    get_target_property(_dirs ${target} INTERFACE_INCLUDE_DIRECTORIES)
    if(_dirs)
        set_property(TARGET ${target} APPEND PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${_dirs})
    endif()
endfunction()

set(GLM_BUILD_LIBRARY OFF CACHE BOOL "Build GLM as a compiled library" FORCE)
set(GLM_BUILD_TESTS OFF CACHE BOOL "Build GLM tests" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "Generate GLM install rules" FORCE)
add_subdirectory("${KUBECHASER_THIRD_PARTY_DIR}/glm" glm EXCLUDE_FROM_ALL)
# glm::glm gets its include directory from glm::glm-header-only.
kubechaser_mark_system_includes(glm::glm-header-only)

add_library(kubechaser_core STATIC
    src/logger/Logger.cpp
    src/shader/DrawAttributes.cpp
    src/utility/Json.cpp
    src/utility/Linalg.cpp
    src/utility/Material.cpp
    src/utility/Reader.cpp
)
add_library(kubechaser::core ALIAS kubechaser_core)

target_compile_features(kubechaser_core PUBLIC cxx_std_17)
target_compile_definitions(kubechaser_core
    PUBLIC GLM_ENABLE_EXPERIMENTAL
    PRIVATE ASSET_DIR="${KUBECHASER_ROOT}/assets"
)
target_include_directories(kubechaser_core PUBLIC
    "$<BUILD_INTERFACE:${KUBECHASER_ROOT}/include>"
)
target_include_directories(kubechaser_core SYSTEM PUBLIC
    "$<BUILD_INTERFACE:${KUBECHASER_THIRD_PARTY_DIR}/eigen>"
)
target_link_libraries(kubechaser_core
    PUBLIC glm::glm
    PRIVATE kubechaser::warnings
)

if(KUBECHASER_BUILD_APP)
    find_package(OpenGL REQUIRED)

    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build GLFW examples" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build GLFW tests" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build GLFW documentation" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "Install GLFW" FORCE)
    add_subdirectory("${KUBECHASER_THIRD_PARTY_DIR}/glfw" glfw)

    add_subdirectory("${KUBECHASER_THIRD_PARTY_DIR}/glad" glad)
    add_subdirectory("${KUBECHASER_THIRD_PARTY_DIR}/imgui" imgui EXCLUDE_FROM_ALL)
    foreach(_third_party_target glfw glad imgui)
        kubechaser_mark_system_includes(${_third_party_target})
    endforeach()

    set(KUBECHASER_APPLICATION_SOURCES
        src/application/Application.cpp
        src/entity/Entity.cpp
        src/entity/Grid.cpp
        src/entity/Mesh.cpp
        src/entity/Quad.cpp
        src/entity/Triangle.cpp
        src/scene/Scene.cpp
        src/scene/SceneDocument.cpp
        src/scene/SceneManager.cpp
        src/scene/SceneNode.cpp
        src/shader/ShaderFactory.cpp
        src/time/Time.cpp
        src/utility/Camera.cpp
        src/window/AssetPanel.cpp
        src/window/BaseFrame.cpp
        src/window/BaseWindow.cpp
        src/window/ConsolePanel.cpp
        src/window/MeshViewer.cpp
        src/window/SceneControl.cpp
        src/window/SceneWorld.cpp
        src/window/SwitchableBaseWindow.cpp
    )

    add_executable(kubechaser main.cpp ${KUBECHASER_APPLICATION_SOURCES})

    target_compile_features(kubechaser PRIVATE cxx_std_17)
    target_compile_definitions(kubechaser PRIVATE
        ASSET_DIR="${KUBECHASER_ROOT}/assets"
        SRC_DIR="${KUBECHASER_ROOT}/src"
        OUTPUT_DIR="${KUBECHASER_ROOT}/output"
    )
    target_include_directories(kubechaser PRIVATE "${KUBECHASER_ROOT}/include")
    target_include_directories(kubechaser SYSTEM PRIVATE
        "${KUBECHASER_THIRD_PARTY_DIR}/stb_image/include"
    )
    target_link_libraries(kubechaser PRIVATE
        kubechaser::core
        glfw
        OpenGL::GL
        glad
        imgui
        kubechaser::warnings
    )
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
