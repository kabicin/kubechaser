#ifndef MESHVIEWER_H
#define MESHVIEWER_H
#include "BaseWindow.h"
#include "entity/Mesh.h"
#include "utility/Reader.h"

class MeshViewer : public BaseWindow
{
private:
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Mesh> mesh;
    double scroll_offset = 0.0;
    float scroll_speed = 3.0f;
    void handleInput();
    int prev_x = 0;
    int prev_y = 0;
    int mouse_x = 0;
    int mouse_y = 0;
    bool clicked = false;
    bool holding = false;
    bool scaling = false;
    bool rotating = false;

    bool wireframe = true;
    bool tesselate = false;
    bool normal = false;
    bool texture = true;
    bool lighting = true;
    bool dir_light = true;
    bool point_light = true;
    bool spot_light = true;
    bool blinn = false;
    float main_rgb[3] = {0.8f, 0.8f, 0.8f};

    void onClick(int x, int y);
    void onClick();

    glm::vec3 clickOffset;
    double lastT;
    void showGUI();

public:
    MeshViewer(const WindowServices& services, int x, int y, int width, int height);

    void SetMesh(std::shared_ptr<Mesh> mesh);
    void Render() override;
    void KeyCallback(int action, int key);
    void SetDownClick(int x, int y);
    void SetDownClickPosition(int x, int y);
    void SetReleaseClick();
    void OnScroll(double yOffset) override;

    void OnCursorPosition(double xpos, double ypos) override;
    void OnMouseButton(int button, int action, double xpos, double ypos) override;
    void OnKey(int key, int action) override;
    void OnDeactivate() override;

    bool GetMouseClicked();
    glm::vec3 GetMouseLocation();
    
    void SetHandledClick(bool handled);
    void SetClickOffset(glm::vec3 offset);
    void SetLastT(double t);

    void SaveMeshJSON();
};
#endif
