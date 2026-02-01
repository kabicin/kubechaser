#ifndef LINALG_H
#define LINALG_H
#include <Eigen/Dense>
#include "glm.h"
#include <math.h>
struct Plane3
{
	Eigen::Vector3d point1, point2, point3;
	Plane3(Eigen::Vector3d point1, Eigen::Vector3d point2, Eigen::Vector3d point3)
        : point1(point1), point2(point2), point3(point3) {}
	Plane3(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3)
        : point1(Eigen::Vector3d(point1.x, point1.y, point1.z)), 
        point2(Eigen::Vector3d(point2.x, point2.y, point2.z)), 
        point3(Eigen::Vector3d(point3.x, point3.y, point3.z)) {}
};

struct Plane2
{
	Eigen::Vector3d normal, point;
	Plane2(Eigen::Vector3d point, Eigen::Vector3d normal)
        : point(point), normal(normal) {}
	Plane2(glm::vec3 point, glm::vec3 normal)
        : point(Eigen::Vector3d(point.x, point.y, point.z)), normal(Eigen::Vector3d(normal.x, normal.y, normal.z)) {}
};

struct Ray
{
    Eigen::Vector3d eye, direction;
    Ray(Eigen::Vector3d eye, Eigen::Vector3d direction)
        : eye(eye), direction(direction) {}
    Ray(glm::vec3 eye, glm::vec3 direction)
        : eye(Eigen::Vector3d(eye.x, eye.y, eye.z)), direction(Eigen::Vector3d(direction.x, direction.y, direction.z)) {}
    Eigen::Vector3d Sub(const Ray& other, double t)
    {
        return (eye + t * direction) - (other.eye + t * other.direction);
    }
    double Angle(const Ray& other)
    {
        Eigen::Vector3d nd = direction / direction.norm();
        Eigen::Vector3d ond = other.direction / other.direction.norm();
        return acos(nd.dot(ond));
    }
    Eigen::Vector3d Point(double t)
    {
        return eye + t * direction;
    }
};

struct Vector
{
    Eigen::Vector3d tip, tail;
    Vector(Eigen::Vector3d tip, Eigen::Vector3d tail)
        : tip(tip), tail(tail) {}
    Vector(glm::vec3 tip, glm::vec3 tail)
        : tip(Eigen::Vector3d(tip.x, tip.y, tip.z)), tail(Eigen::Vector3d(tail.x, tail.y, tail.z)) {}
};

class Linalg
{
public:
    // helpers
    static Eigen::Vector3d GLM2Eigen(const glm::vec3& vec);
    static glm::vec3 Eigen2GLM(const Eigen::Vector3d& vec);

    // projection
	static glm::vec3 ProjectUntilZ(const Ray& ray, float z, float original_z);
    static Eigen::Vector3d ProjVonU(const Eigen::Vector3d& u, const Eigen::Vector3d& v);
    
    // ray-* intersection
    static bool RayBaryIntersect(const Ray& ray, const Eigen::Vector3d& pivot, 
        const Eigen::Vector3d& t1, const Eigen::Vector3d& t2, Eigen::Vector3d& n, double& t);
    static bool RayPlaneIntersect(const Ray& ray, const Plane3& plane, Eigen::Vector3d& poi, Eigen::Vector3d& n, double& t);
    static bool RayPlaneIntersect(const Ray& ray, const Plane2& plane, Eigen::Vector3d& poi, Eigen::Vector3d& n, double& t);
};
#endif