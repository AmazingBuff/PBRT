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

    //copy current transform into namedCoordinateSystems
    void pbrtCoordinateSystem(const std::string& name);

    //make current transform get form namedCoordinateSystems
    void pbrtCoordSysTransform(const std::string& name);

    //set transform bits to active
    void pbrtActiveTransformAll();
    void pbrtActiveTransformEndTime();
    void pbrtActiveTransformStartTime();

    //the defined time of two current transform matrices
    void pbrtTransformTimes(Float start, Float end);

    //set which kind of Filter to be used for filtering image samples
    void pbrtPixelFilter(const std::string& name, const ParamSet& params);

    //set which kind of Film to be used
    void pbrtFilm(const std::string& type, const ParamSet& params);

    //set which kind of Sampler to be used
    void pbrtSampler(const std::string& name, const ParamSet& params);

    //set which kind of Accelerator to be used
    void pbrtAccelerator(const std::string& name, const ParamSet& params);

    //set which kind of Integrator to be used
    void pbrtIntegrator(const std::string& name, const ParamSet& params);

    //set the variable of the camera to be used
    void pbrtCamera(const std::string& name, const ParamSet& params);

    //set the type of participation media
    void pbrtMakeNamedMedium(const std::string& name, const ParamSet& params);

    //set the two side types of specified surface
    void pbrtMediumInterface(const std::string& insideName, const std::string& outsideName);
}
