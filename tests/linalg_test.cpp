#include "utility/Linalg.h"

#include <cmath>
#include <iostream>

namespace {

constexpr double epsilon = 1e-9;

bool approximatelyEqual(double lhs, double rhs)
{
    return std::abs(lhs - rhs) < epsilon;
}

bool approximatelyEqual(const Eigen::Vector3d& lhs, const Eigen::Vector3d& rhs)
{
    return (lhs - rhs).norm() < epsilon;
}

} // namespace

int main()
{
    const glm::vec3 glmVector(1.0f, -2.0f, 3.5f);
    const Eigen::Vector3d eigenVector = Linalg::GLM2Eigen(glmVector);
    const glm::vec3 roundTrip = Linalg::Eigen2GLM(eigenVector);

    if (!approximatelyEqual(roundTrip.x, 1.0)
        || !approximatelyEqual(roundTrip.y, -2.0)
        || !approximatelyEqual(roundTrip.z, 3.5)) {
        std::cerr << "GLM/Eigen vector conversion did not round-trip.\n";
        return 1;
    }

    const Ray ray(Eigen::Vector3d(0.0, 0.0, 1.0), Eigen::Vector3d(0.0, 0.0, -1.0));
    const Plane2 plane(Eigen::Vector3d(0.0, 0.0, 0.0), Eigen::Vector3d(0.0, 0.0, 1.0));
    Eigen::Vector3d intersection;
    Eigen::Vector3d normal;
    double distance = 0.0;

    if (!Linalg::RayPlaneIntersect(ray, plane, intersection, normal, distance)) {
        std::cerr << "Expected the ray to intersect the plane.\n";
        return 1;
    }
    if (!approximatelyEqual(intersection, Eigen::Vector3d::Zero())
        || !approximatelyEqual(normal, Eigen::Vector3d(0.0, 0.0, 1.0))
        || !approximatelyEqual(distance, 1.0)) {
        std::cerr << "Ray-plane intersection returned an unexpected result.\n";
        return 1;
    }

    const Eigen::Vector3d projection = Linalg::ProjVonU(
        Eigen::Vector3d(1.0, 0.0, 0.0),
        Eigen::Vector3d(2.0, 3.0, 0.0));
    if (!approximatelyEqual(projection, Eigen::Vector3d(2.0, 0.0, 0.0))) {
        std::cerr << "Vector projection returned an unexpected result.\n";
        return 1;
    }

    return 0;
}
