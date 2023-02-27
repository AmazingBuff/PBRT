#pragma once
#include"../core/pbrt.h"
#include"../core/integrator.h"

namespace pbrt
{
    class SamplerIntegrator : public Integrator
    {
    public:
        SamplerIntegrator(std::shared_ptr<const Camera> camera, std::shared_ptr<Sampler> sampler)
        : camera(camera), sampler(sampler) { }
        //preprocess
        virtual void Preprocess(const Scene& scene, Sampler& sampler) { }
        //render
        void Render(const Scene& scene);
    protected:
        //protected data
        std::shared_ptr<const Camera> camera;
    private:
        //private data
        std::shared_ptr<Sampler> sampler;
    }
}