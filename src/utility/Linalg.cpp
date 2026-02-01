#include "utility/Linalg.h"
#include <iostream>

glm::vec3 Linalg::ProjectUntilZ(const Ray& ray, float z, float original_z)
{
	float t = (z - ray.eye.z()) / ray.direction.z();
	Eigen::Vector3d proj = ray.eye + t * ray.direction;
	return glm::vec3(proj.x(), proj.y(), original_z);
}

bool Linalg::RayBaryIntersect(const Ray& ray, const Eigen::Vector3d& pivot, 
    const Eigen::Vector3d& t1, const Eigen::Vector3d& t2, Eigen::Vector3d& n, double& t)
{
    // barycentric coordinate test for intersection
    Eigen::Vector3d temp_n = t1.cross(t2);
    if (temp_n.dot(ray.direction) == 0) return false;
    Eigen::Matrix3d m;
    m.col(0) = t1;
    m.col(1) = t2;
    m.col(2) = -ray.direction;
    Eigen::Vector3d values = m.colPivHouseholderQr().solve(ray.eye - pivot);
    double alpha = values(0), gamma = values(1);
    t = values(2);
    if (t > 0 && alpha >= 0 && gamma >= 0 && alpha + gamma <= 1)
    {
        // find surface normal
        Eigen::Vector3d n = t1.cross(t2);
        if (n.dot(ray.direction) < 0)
            n = t1.cross(t2);
        n = n / n.norm();
        std::cout << "Ray-Bary Intersect: t=" << t << " n: (" << n(0) << "," << n(1) << "," << n(2) << ")"  << std::endl;
        return true;
    }
    return false;
}

bool Linalg::RayPlaneIntersect(const Ray& ray, const Plane3& plane, Eigen::Vector3d& poi, Eigen::Vector3d& n, double& t)
{
    Eigen::Vector3d t1 = plane.point2 - plane.point1;
    Eigen::Vector3d t2 = plane.point3 - plane.point1;
    Eigen::Vector3d offset = ray.eye - plane.point1;
    Eigen::Vector3d normal = t1.cross(t2);
    double denom = -ray.direction.dot(normal);
    if (denom == 0) return false;
    // double u = (t2.cross(-ray.direction)).dot(offset) / denom;
    // double v = ((-ray.direction).cross(t1)).dot(offset) / denom;
    t = normal.dot(offset) / denom;
    n = normal / normal.norm();
    poi = ray.eye + t * ray.direction;
    return true;
}

bool Linalg::RayPlaneIntersect(const Ray& ray, const Plane2& plane, Eigen::Vector3d& poi, Eigen::Vector3d& n, double& t)
{
    Eigen::Vector3d offset = ray.eye - plane.point;
    Eigen::Vector3d normal = plane.normal;
    double denom = -ray.direction.dot(normal);
    if (denom == 0) return false;
    t = normal.dot(offset) / denom;
    n = normal / normal.norm();
    poi = ray.eye + t * ray.direction;
    return true;
}

Eigen::Vector3d Linalg::ProjVonU(const Eigen::Vector3d& u, const Eigen::Vector3d& v)
{
    return (u.dot(v) / u.squaredNorm()) * u;
}

Eigen::Vector3d Linalg::GLM2Eigen(const glm::vec3& vec)
{
    return Eigen::Vector3d(vec.x, vec.y, vec.z);
}

glm::vec3 Linalg::Eigen2GLM(const Eigen::Vector3d& vec)
{
    return glm::vec3(vec(0), vec(1), vec(2));
}