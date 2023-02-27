#pragma once
#include"pbrt.h"
#include"scene/scene.h"

namespace pbrt
{
    class Integrator
    {
    public:
        virtual void Render(const Scene& scene) = 0;
    };
}