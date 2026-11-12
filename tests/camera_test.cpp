#include "support/TestSupport.h"
#include "utility/Camera.h"

using test::approx;
using test::expect;

namespace {

bool approxVec(const glm::vec3& a, const glm::vec3& b, float eps = 1e-4f)
{
    return approx(a.x, b.x, eps) && approx(a.y, b.y, eps) && approx(a.z, b.z, eps);
}

} // namespace

int main()
{
    // Defaults: starts at (0,0,5) looking down -Z.
    {
        Camera camera(800, 600);
        expect(approxVec(camera.GetCameraPos(), glm::vec3(0, 0, 5)), "initial position");
        expect(approxVec(camera.GetLookDirection(), glm::vec3(0, 0, -1)), "initial look direction");
        expect(camera.GetMoveSpeed() == Camera::DEFAULT_MOVE_SPEED, "default move speed");

        // The origin is straight ahead, 5 units in front of the camera.
        glm::vec4 eye = camera.GetView() * glm::vec4(0, 0, 0, 1);
        expect(approxVec(glm::vec3(eye), glm::vec3(0, 0, -5)), "view puts origin 5 units ahead");
    }

    // Movement scales with speed and elapsed time.
    {
        Camera camera(800, 600);
        camera.SetMoveSpeed(10.0f);
        camera.SetForward();
        camera.UpdateFrame(0.5);
        expect(approxVec(camera.GetCameraPos(), glm::vec3(0, 0, 0)), "forward 10 u/s for 0.5 s");

        camera.SetCenter();
        camera.UpdateFrame(1.0);
        expect(approxVec(camera.GetCameraPos(), glm::vec3(0, 0, 0)), "SetCenter stops movement");

        // Forward + right is normalized, so diagonal speed equals straight speed.
        camera.SetForward();
        camera.SetRight();
        camera.UpdateFrame(1.0);
        expect(approx(glm::length(camera.GetCameraPos()), 10.0f), "diagonal movement is normalized");
        expect(camera.GetCameraPos().x > 0.0f && camera.GetCameraPos().z < 0.0f, "forward-right heads +X/-Z");

        camera.ResetCamPos();
        expect(approxVec(camera.GetCameraPos(), glm::vec3(0, 0, 5)), "ResetCamPos restores position");

        camera.SetMoveSpeed(-3.0f);
        expect(camera.GetMoveSpeed() == 0.0f, "negative speed clamps to zero");
    }

    // Vertical clamping.
    {
        Camera camera(800, 600);
        camera.SetDown();
        camera.UpdateFrame(1.0);
        expect(camera.GetCameraPos().y < 0.0f, "moved below ground");
        camera.ClampMinY(1.0f);
        expect(approx(camera.GetCameraPos().y, 1.0f), "ClampMinY raises camera");
        camera.ClampMinY(-100.0f);
        expect(approx(camera.GetCameraPos().y, 1.0f), "ClampMinY leaves higher camera alone");
    }

    // Look direction.
    {
        Camera camera(800, 600);
        camera.SetLookDirection(glm::vec3(3, 0, 0));
        expect(approxVec(camera.GetLookDirection(), glm::vec3(1, 0, 0)), "SetLookDirection normalizes");
        camera.SetLookDirection(glm::vec3(0, 0, 0));
        expect(approxVec(camera.GetLookDirection(), glm::vec3(1, 0, 0)), "zero look direction is ignored");
    }

    // Picking rays: the screen centre maps to the look direction, and the
    // left half of the screen maps to -X.
    {
        Camera camera(800, 600);
        Ray centre = camera.GetRay(400, 300);
        expect(approx(centre.eye.z(), 5.0), "ray starts at camera");
        expect(approx(centre.direction.x(), 0.0) && approx(centre.direction.y(), 0.0)
                   && approx(centre.direction.z(), -1.0),
               "centre ray looks down -Z");
        expect(camera.GetDirection(100, 300).x < 0.0f, "left of centre points -X");
        expect(camera.GetDirection(400, 100).y > 0.0f, "above centre points +Y");

        // Resize keeps the centre ray centred.
        camera.Resize(200, 100);
        expect(approxVec(camera.GetDirection(100, 50), glm::vec3(0, 0, -1)), "centre ray after resize");
        camera.Resize(200, 0); // must not divide by zero
        expect(approxVec(camera.GetDirection(100, 50), glm::vec3(0, 0, -1)), "zero height resize ignored");
    }

    // Model matrix: local scale/translate/rotate, then the whole-model offset
    // in world units regardless of scale.
    {
        Camera camera;
        CameraAttributes ca(glm::vec3(1, 0, 0), glm::vec3(2, 2, 2), 0, 0, 0);
        camera.cma.translate = glm::vec3(10, 0, 0);
        glm::vec4 origin = camera.GenerateModel(ca) * glm::vec4(0, 0, 0, 1);
        expect(approxVec(glm::vec3(origin), glm::vec3(12, 0, 0)), "scale, translate and model offset");
        expect(camera.GetModel() == camera.GenerateModel(ca), "GetModel returns last model");

        camera.cma.translate = glm::vec3(0);
        CameraAttributes rotated(glm::vec3(0), glm::vec3(1), 0, 90, 0);
        glm::vec4 x = camera.GenerateModel(rotated) * glm::vec4(1, 0, 0, 1);
        expect(approxVec(glm::vec3(x), glm::vec3(0, 0, -1)), "rotate 90 about Y maps +X to -Z");
    }

    return test::finish();
}
