#pragma once
#include"../pbrt.h"

namespace pbrt
{
    Options PbrtOptions;
    
    class Transform;
    class Vector3f;

    //pbrt renderer initialize
    void pbrtInit(const Options& option);
    
    //pbrt renderer cleanup
    void pbrtCleanup();

    //initialize the transformation to identity transform
    void pbrtIdentity();

    //translate for current transformation
    void pbrtTranslate(Float dx, Float dy, Float dz);

    //ratate for current tranformation
    void pbrtRotate(Float angle, Float axisX, Float axisY, Float axisZ);

    //scale for current tranformation
    void pbrtScale(Float sx, Float sy, Float sz);

    //obtain lookat matrix
    void pbrtLookAt(Float ex, Float ey, Float ez,
                    Float lx, Float ly, Float lz,
                    Float ux, Float uy, Float uz);
    
    //concat transform with postmultiply
    void pbrtConcatTransform(Float transform[16]);

    //replace the active CTM
    void pbrtTransform(Float transform[16]);

    //copy current transform into namedCoordinateSystem
    void pbrtCoordinateSystem(const std::string& name);

    //
    void pbrtCoordSysTransform(const std::string& name);
}
