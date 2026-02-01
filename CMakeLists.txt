cmake_minimum_required(VERSION 3.6)
set(ROOT "${CMAKE_CURRENT_LIST_DIR}")
project(kubechaser)

include_directories("include/")
file(GLOB_RECURSE SOURCES
	"src/*.cpp"
)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX17)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
elseif(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  message(STATUS "${CMAKE_CXX_COMPILER} has no C++0x support.")
endif()

find_package(OpenGL REQUIRED)

set(THIRD_PARTY_DIR "${ROOT}/third_party")
if(NOT TARGET glfw)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL " " FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL " " FORCE)
  set(GLFW_BUILD_DOCS OFF CACHE BOOL " " FORCE)
  set(GLFW_INSTALL OFF CACHE BOOL " " FORCE)
  add_subdirectory(${THIRD_PARTY_DIR}/glfw glfw)
endif()

add_subdirectory(${THIRD_PARTY_DIR}/glad glad)
add_subdirectory(${THIRD_PARTY_DIR}/glm glm EXCLUDE_FROM_ALL)
add_subdirectory(${THIRD_PARTY_DIR}/imgui imgui EXCLUDE_FROM_ALL)

add_executable(${PROJECT_NAME} ${SOURCES} "main.cpp")
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${THIRD_PARTY_DIR}/eigen/)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${THIRD_PARTY_DIR}/glm/glm)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${THIRD_PARTY_DIR}/stb_image/include)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw ${OPENGL_gl_LIBRARY} glad imgui)

target_compile_definitions(${PROJECT_NAME} PRIVATE GLM_ENABLE_EXPERIMENTAL)
target_compile_definitions(${PROJECT_NAME} PRIVATE ASSET_DIR="${ROOT}/assets")
target_compile_definitions(${PROJECT_NAME} PRIVATE SRC_DIR="${ROOT}/src")
