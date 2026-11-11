#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif

#include "application/Application.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb.h"
#undef STB_IMAGE_IMPLEMENTATION

#include "dearimgui.h"
#include "entity/Mesh.h"
#include "logger/Logger.h"
#include "scene/Scene.h"
#include "scene/SceneDocument.h"
#include "scene/SceneManager.h"
#include "scene/SceneNode.h"
#include "shader/ShaderFactory.h"
#include "time/Time.h"
#include "utility/Camera.h"
#include "window/AssetPanel.h"
#include "window/BaseFrame.h"
#include "window/BaseWindow.h"
#include "window/ConsolePanel.h"
#include "window/MeshViewer.h"
#include "window/SceneControl.h"
#include "window/SceneWorld.h"
#include "window/SwitchableBaseWindow.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <cstdlib>
#include <iostream>

Application::Application()
    : showcase{
        {"pod.obj", "Pod", glm::vec3(0.53f, 0.83f, 0.62f)},
        {"deployment.obj", "Deployment", glm::vec3(0.44f, 0.63f, 1.0f)},
        {"replicaset.obj", "ReplicaSet", glm::vec3(0.60f, 0.75f, 1.0f)},
        {"statefulset.obj", "StatefulSet", glm::vec3(0.35f, 0.55f, 0.9f)},
        {"daemonset.obj", "DaemonSet", glm::vec3(1.0f, 0.76f, 0.35f)},
        {"job.obj", "Job", glm::vec3(1.0f, 0.60f, 0.40f)},
        {"cronjob.obj", "CronJob", glm::vec3(1.0f, 0.45f, 0.45f)},
        {"service.obj", "Service", glm::vec3(0.85f, 0.60f, 0.95f)},
        {"ingress.obj", "Ingress", glm::vec3(0.95f, 0.70f, 0.85f)},
        {"endpointslice.obj", "EndpointSlice", glm::vec3(0.70f, 0.85f, 0.70f)},
        {"configmap.obj", "ConfigMap", glm::vec3(1.0f, 0.92f, 0.60f)},
        {"secret.obj", "Secret", glm::vec3(0.95f, 0.75f, 0.60f)},
        {"storageclass.obj", "StorageClass", glm::vec3(0.88f, 0.78f, 0.62f)},
        {"crd.obj", "CRD", glm::vec3(0.75f, 0.65f, 0.95f)},
        {"namespace.obj", "Namespace", glm::vec3(0.90f, 0.92f, 0.95f)},
        {"networkpolicy.obj", "NetworkPolicy", glm::vec3(0.45f, 0.85f, 0.85f)},
        {"hpa.obj", "HPA", glm::vec3(0.98f, 0.82f, 0.45f)},
        {"quota.obj", "Quota", glm::vec3(1.0f, 0.70f, 0.75f)},
        {"user.obj", "User", glm::vec3(0.85f, 0.65f, 0.50f)},
        {"group.obj", "Group", glm::vec3(0.80f, 0.60f, 0.45f)},
        {"service-account.obj", "ServiceAccount", glm::vec3(0.50f, 0.80f, 0.75f)},
        {"psp.obj", "PSP", glm::vec3(0.70f, 0.75f, 0.80f)},
    }
{
}

Application::~Application()
{
    Shutdown();
}

int Application::Run()
{
    if (!Initialize()) {
        return EXIT_FAILURE;
    }

    while (!glfwWindowShouldClose(window)) {
        services.time->GetDelta();
        glfwPollEvents();
        RenderFrame();
        glfwSwapBuffers(window);
    }

    Shutdown();
    return EXIT_SUCCESS;
}

bool Application::Initialize()
{
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    glfwInitialized = true;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(screenWidth, screenHeight, "KubeChaser", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    InstallCallbacks();

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    stbi_set_flip_vertically_on_load(true);

    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL version supported " << glGetString(GL_VERSION) << std::endl;

    InitializeServices();
    InitializeWindows();
    return InitializeImGui();
}

void Application::InitializeServices()
{
    services.logger = std::make_shared<Logger>();
    services.shaderFactory = std::make_shared<ShaderFactory>();
    services.time = std::make_shared<Time>();
}

void Application::InitializeWindows()
{
    auto sceneControl = std::make_shared<SceneControl>(services, sceneDocument, 0, 0, 250, screenHeight / 2);
    windows.push_back(sceneControl);

    auto root = std::make_shared<SceneNode>();

    const int numCols = 6;
    const float spacingX = 4.0f;
    const float spacingZ = 4.0f;
    for (std::size_t i = 0; i < showcase.size(); ++i) {
        auto mesh = std::make_shared<Mesh>(showcase[i].filename);
        const int col = static_cast<int>(i % numCols);
        const int row = static_cast<int>(i / numCols);
        const float posX = (col - (numCols - 1) * 0.5f) * spacingX;
        const float posZ = (row - 1.5f) * spacingZ;
        mesh->Translate(glm::vec3(posX, 1.0f, posZ));
        mesh->da.color = true;
        mesh->da.useVertexColor = false;
        mesh->da.solidColor = showcase[i].color;
        mesh->da.materialAttributes.diffuse = showcase[i].color;
        root->AddStaticObject(mesh);
    }

    auto scene = std::make_shared<Scene>(root);
    auto worldCamera = std::make_shared<Camera>(screenWidth, screenHeight);
    sceneWorld = std::make_shared<SceneWorld>(services, 0, 0, screenWidth, screenHeight);
    sceneWorld->AddCamera(worldCamera);

    auto sceneManager = std::make_shared<SceneManager>();
    sceneManager->AddScene(SceneCoord{0, 0, 0}, scene);
    sceneManager->SetShowGrid(true, worldCamera);
    sceneManager->SetActiveRadius(40);
    sceneWorld->AddSceneManager(sceneManager);

    meshViewer = std::make_shared<MeshViewer>(services, 0, 0, screenWidth, screenHeight);
    windowSwitch = std::make_shared<SwitchableBaseWindow>(
        0, 0, screenWidth, screenHeight, meshViewer, sceneWorld);
    sceneControl->SetModeChangeCallback([this](SwitchableBaseWindow::WindowMode mode) {
        if (windowSwitch) {
            windowSwitch->SetActiveMode(mode);
        }
    });
    sceneControl->AttachSwitchableBaseWindow(windowSwitch);

    windows.push_back(windowSwitch);
    windows.push_back(std::make_shared<AssetPanel>(services, sceneDocument, screenWidth - 250, screenHeight / 4, 250, screenHeight / 2));
    windows.push_back(std::make_shared<ConsolePanel>(services, screenWidth - 250, screenHeight / 2, 250, screenHeight / 2));

    auto initialPod = std::make_shared<Mesh>("pod.obj");
    initialPod->da.solidColor = glm::vec3(0.53f, 0.83f, 0.62f);
    initialPod->da.materialAttributes.diffuse = initialPod->da.solidColor;
    initialPod->da.color = true;
    initialPod->da.useVertexColor = false;
    meshViewer->SetMesh(initialPod);
}

bool Application::InitializeImGui()
{
    constexpr const char* glslVersion = "#version 330";

    ImGui::CreateContext();
    imguiContextCreated = true;
    ImGui::StyleColorsClassic();

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        std::cerr << "Failed to initialize the ImGui GLFW backend" << std::endl;
        return false;
    }
    imguiGlfwInitialized = true;

    if (!ImGui_ImplOpenGL3_Init(glslVersion)) {
        std::cerr << "Failed to initialize the ImGui OpenGL backend" << std::endl;
        return false;
    }
    imguiOpenGLInitialized = true;
    return true;
}

void Application::InstallCallbacks()
{
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetCursorPosCallback(window, CursorPositionCallback);
    glfwSetWindowSizeCallback(window, WindowSizeCallback);
}

void Application::RenderFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    glViewport(0, 0, screenWidth, screenHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for (const auto& frame : windows) {
        if (!frame->Hidden()) {
            frame->Render();
        }
    }

    RenderShowcase();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::RenderShowcase()
{
    ImGui::SetNextWindowPos(ImVec2(10, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(320, 380), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("OBJ Model Showcase")) {
        ImGui::TextWrapped("Click any OBJ model below to preview it in the Mesh Viewer, or open Scene World from Scene Control to see all 22 on the grid!");
        ImGui::Separator();
        for (std::size_t i = 0; i < showcase.size(); ++i) {
            ImGui::PushID(static_cast<int>(i));
            ImGui::PushStyleColor(
                ImGuiCol_Text,
                ImVec4(showcase[i].color.x, showcase[i].color.y, showcase[i].color.z, 1.0f));
            if (ImGui::Button(showcase[i].title.c_str(), ImVec2(-1, 0))) {
                auto selectedMesh = std::make_shared<Mesh>(showcase[i].filename);
                selectedMesh->da.solidColor = showcase[i].color;
                selectedMesh->da.materialAttributes.diffuse = showcase[i].color;
                selectedMesh->da.color = true;
                selectedMesh->da.useVertexColor = false;
                meshViewer->SetMesh(selectedMesh);
                services.logger->LogMessage("Showcasing OBJ model: " + showcase[i].filename);
            }
            ImGui::PopStyleColor();
            ImGui::PopID();
        }
    }
    ImGui::End();
}

void Application::Shutdown()
{
    if (window != nullptr) {
        glfwMakeContextCurrent(window);
    }

    windows.clear();
    windowSwitch.reset();
    meshViewer.reset();
    sceneWorld.reset();
    sceneDocument.Clear();

    services.shaderFactory.reset();
    services.time.reset();
    services.logger.reset();

    if (imguiOpenGLInitialized) {
        ImGui_ImplOpenGL3_Shutdown();
        imguiOpenGLInitialized = false;
    }
    if (imguiGlfwInitialized) {
        ImGui_ImplGlfw_Shutdown();
        imguiGlfwInitialized = false;
    }
    if (imguiContextCreated) {
        ImGui::DestroyContext();
        imguiContextCreated = false;
    }

    if (window != nullptr) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    if (glfwInitialized) {
        glfwTerminate();
        glfwInitialized = false;
    }
}

void Application::OnCursorPosition(double xpos, double ypos)
{
    if (windowSwitch) {
        windowSwitch->OnCursorPosition(xpos, ypos);
    }
}

void Application::OnKey(int key, int action)
{
    if (imguiGlfwInitialized && ImGui::GetIO().WantCaptureKeyboard) {
        return;
    }
    if (windowSwitch) {
        windowSwitch->OnKey(key, action);
    }
}

void Application::OnMouseButton(int button, int action)
{
    if (imguiGlfwInitialized && ImGui::GetIO().WantCaptureMouse) {
        return;
    }
    if (windowSwitch) {
        double xpos = 0.0;
        double ypos = 0.0;
        glfwGetCursorPos(window, &xpos, &ypos);
        windowSwitch->OnMouseButton(button, action, xpos, ypos);
    }
}

void Application::OnScroll(double yoffset)
{
    if (imguiGlfwInitialized && ImGui::GetIO().WantCaptureMouse) {
        return;
    }
    if (windowSwitch) {
        windowSwitch->OnScroll(yoffset);
    }
}

void Application::OnWindowSize(int width, int height)
{
    screenWidth = width;
    screenHeight = height;

    if (windows.size() < 4) {
        return;
    }
    windows[0]->Resize(0, 0, 250, screenHeight / 2);
    windows[1]->Resize(0, 0, screenWidth, screenHeight);
    windows[2]->Resize(screenWidth - 250, screenHeight / 4, 250, screenHeight / 2);
    windows[3]->Resize(screenWidth - 250, 0, 250, screenHeight / 2);
}

Application* Application::FromWindow(GLFWwindow* glfwWindow)
{
    return static_cast<Application*>(glfwGetWindowUserPointer(glfwWindow));
}

void Application::CursorPositionCallback(GLFWwindow* glfwWindow, double xpos, double ypos)
{
    if (auto* app = FromWindow(glfwWindow)) {
        app->OnCursorPosition(xpos, ypos);
    }
}

void Application::KeyCallback(GLFWwindow* glfwWindow, int key, int, int action, int)
{
    if (auto* app = FromWindow(glfwWindow)) {
        app->OnKey(key, action);
    }
}

void Application::MouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int)
{
    if (auto* app = FromWindow(glfwWindow)) {
        app->OnMouseButton(button, action);
    }
}

void Application::ScrollCallback(GLFWwindow* glfwWindow, double, double yoffset)
{
    if (auto* app = FromWindow(glfwWindow)) {
        app->OnScroll(yoffset);
    }
}

void Application::WindowSizeCallback(GLFWwindow* glfwWindow, int width, int height)
{
    if (auto* app = FromWindow(glfwWindow)) {
        app->OnWindowSize(width, height);
    }
}
