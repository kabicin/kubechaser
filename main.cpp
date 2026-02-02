#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif
#include "window/BaseFrame.h"
#include "window/BaseWindow.h"
#include "window/SwitchableBaseWindow.h"
#include "scene/SceneManager.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "dearimgui.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

#include "controller/Controller.h"
// BaseWindow includes
#include "window/SceneBuilder.h"
#include "window/SceneControl.h"
#include "window/AssetPanel.h"
#include "window/ConsolePanel.h"
#include "window/SceneWorld.h"
#include "window/MeshViewer.h"

// Add logger
#include "logger/Logger.h"

// Add shaders
#include "shader/ShaderFactory.h"

// Add time
#include "time/Time.h"

#ifndef SRC_DIR
#define SRC_DIR "./src"
#endif

int screenWidth = 1500;
int screenHeight = 800;
std::vector<std::shared_ptr<BaseFrame>> windows;

void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    std::shared_ptr<Entity> entity = SceneBuilder::GetActiveEntity();
    SceneBuilder::OnScroll(entity, yoffset);
    MeshViewer::OnScroll(yoffset);
}

void notifyClickDownHandlers(int xpos, int ypos)
{
    SceneBuilder::SetDownClick(xpos, ypos);
    MeshViewer::SetDownClick(xpos, ypos);
}

void notifyClickReleaseHandlers()
{
    SceneBuilder::SetReleaseClick();
    MeshViewer::SetReleaseClick();
}

void notifyMousePositionHandlers(int x, int y)
{
    SceneBuilder::SetDownClickPosition(x, y);
    MeshViewer::SetDownClickPosition(x, y);
    SceneWorld::SetMousePosition(x, y);
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    notifyMousePositionHandlers(xpos, ypos);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            notifyClickDownHandlers(xpos, ypos);
        }
        else
        {
            notifyClickReleaseHandlers();
        }
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        SceneWorld::SetLookActive(action == GLFW_PRESS || action == GLFW_REPEAT);
    }
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
    screenWidth = width;
    screenHeight = height;
    windows[0]->Resize(0, 0, 250, screenHeight / 2);
    windows[1]->Resize(0, 0, screenWidth, screenHeight);
    windows[2]->Resize(screenWidth - 250, screenHeight / 4, 250, screenHeight / 2);
    windows[3]->Resize(screenWidth - 250, 0, 250, screenHeight / 2);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    MeshViewer::KeyCallback(action, key);
    SceneWorld::KeyCallback(action, key);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "KubeChaser", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwSetScrollCallback(window, mouse_scroll_callback);

    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetScrollCallback(window, mouse_scroll_callback);


    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    stbi_set_flip_vertically_on_load(true);

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL version supported " << version << std::endl;
    
    // Initialize Logger
    std::shared_ptr<Logger> logger = std::make_shared<Logger>();
    BaseWindow::AttachLogger(logger);

    // Initialize Shader Factory
    std::shared_ptr<ShaderFactory> factory = std::make_shared<ShaderFactory>();
    BaseWindow::AttachShaderFactory(factory);

    // Initialize Time
    std::shared_ptr<Time> time = std::make_shared<Time>();
    BaseWindow::AttachTime(time);

    std::shared_ptr<SceneControl> sceneControl = std::make_shared<SceneControl>(0, 0, 250, screenHeight / 2);
    windows.push_back(sceneControl);


    std::shared_ptr<SceneNode> root = std::make_shared<SceneNode>();
    std::shared_ptr<Mesh> rootMesh = std::make_shared<Mesh>("statefulset.obj");
    rootMesh->Translate(glm::vec3(0.0f, 1.0f, 0.0f));
    root->AddStaticObject(rootMesh);

    std::shared_ptr<Scene> scene = std::make_shared<Scene>(root);
    std::shared_ptr<Camera> worldCamera = std::make_shared<Camera>(screenWidth, screenHeight);
    std::shared_ptr<SceneWorld> sceneWorld = std::make_shared<SceneWorld>(0, 0, screenWidth, screenHeight);
    sceneWorld->AddCamera(worldCamera);
    std::shared_ptr<SceneManager> sceneManager = std::make_shared<SceneManager>();
    sceneManager->AddScene(SceneCoord{0, 0, 0}, scene);
    sceneManager->SetShowGrid(true, worldCamera);
    sceneWorld->AddSceneManager(sceneManager);


    // switch
    std::shared_ptr<SwitchableBaseWindow> windowSwitch = std::make_shared<SwitchableBaseWindow>(0, 0, screenWidth, screenHeight);
    windowSwitch->AddWindow(std::make_shared<MeshViewer>(0, 0, screenWidth, screenHeight));
    windowSwitch->SetActiveMeshViewer(0);
    windowSwitch->AddWindow(sceneWorld);
    windowSwitch->AttachSceneController(sceneControl);
    sceneControl->AttachSwitchableBaseWindow(windowSwitch);

    windows.push_back(windowSwitch);
    windows.push_back(std::make_shared<AssetPanel>(screenWidth - 250, screenHeight / 4, 250, screenHeight / 2));
    windows.push_back(std::make_shared<ConsolePanel>(screenWidth - 250, screenHeight / 2, 250, screenHeight / 2));

    // attach BaseWindows to controllers
    // Controller controller(window);
    // controller.AttachWindow(scenePlayer);
   
    const char* glsl_version = "#version 330";
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);
    
    bool scenePlayerOn = false;
    while (!glfwWindowShouldClose(window))
    {
        time->GetDelta();
        glfwPollEvents();
        
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // clear background
        glViewport(0, 0, screenWidth, screenHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // render windows
        for (int i = 0; i < windows.size(); i++)
        {
            if (!windows[i]->Hidden())
            {
                windows[i]->Render();
            }
        } 

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
    }
    
    // free memory
    windows.clear();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwTerminate();
    return 0;
}
