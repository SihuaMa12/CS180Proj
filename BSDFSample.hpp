#ifndef BSDFSAM_H
#define BSDFSAM_H
#include <Vector.hpp>


enum REFT {LAMB, PERR};

class BSDFSample{
public:
    BSDFSample(REFT reftype);
    Vector3f sample_f();

private:
    REFT refType;

}

#endif