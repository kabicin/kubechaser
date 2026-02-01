#ifndef MESHVIEWER_H
#define MESHVIEWER_H
#include "BaseWindow.h"
#include "entity/Mesh.h"
#include "utility/Reader.h"

class MeshViewer : public BaseWindow
{
private:
    std::shared_ptr<Camera> camera;
    static std::shared_ptr<Mesh> mesh;
    static double scroll_offset;
    static float scroll_speed;
    void handleInput();
    static int prev_x, prev_y, mouse_x, mouse_y;
    static bool clicked, holding, scaling, rotating, wireframe;
    void onClick(int x, int y);
    void onClick();

    glm::vec3 clickOffset;
    double lastT;
    void showGUI();

public:
    MeshViewer(int x, int y, int width, int height);

    static void SetMesh(std::shared_ptr<Mesh> mesh);
    void Render();
    static void KeyCallback(int action, int key);
    static void SetDownClick(int x, int y);
    static void SetDownClickPosition(int x, int y);
    static void SetReleaseClick();
    static void OnScroll(double yOffset);

    bool GetMouseClicked();
    glm::vec3 GetMouseLocation();
    
    void SetHandledClick(bool handled);
    void SetClickOffset(glm::vec3 offset);
    void SetLastT(double t);

    void SaveMeshJSON();
};
#endif
