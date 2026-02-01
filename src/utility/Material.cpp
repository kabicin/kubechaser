#include "utility/Material.h"

std::ostream& operator<<(std::ostream& o, const Material& m)
{
    o << "Material: " << m.name << std::endl;
    o << "\tKa: (" << m.Ka.x <<  ", " << m.Ka.y << ", " << m.Ka.z << ")" << std::endl;
    o << "\tKd: (" << m.Kd.x <<  ", " << m.Kd.y << ", " << m.Kd.z << ")"<< std::endl;
    o << "\tKs: (" << m.Ks.x <<  ", " << m.Ks.y << ", " << m.Ks.z << ")" << std::endl;
    o << "\tNs: " << m.Ns << std::endl;
    return o;
}