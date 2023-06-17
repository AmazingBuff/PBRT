#pragma once
#include"core/pbrt.h"
#include"core/scene/scene.h"

namespace pbrt
{
    class Integrator
    {
    public:
        virtual void Render(const Scene& scene) = 0;
    };


    class SamplerIntegrator : public Integrator
    {
    public:
        SamplerIntegrator(std::shared_ptr<const Camera> camera, std::shared_ptr<Sampler> sampler)
            : camera(camera), sampler(sampler) { }
        //preprocess
        virtual void Preprocess(const Scene& scene, Sampler& sampler) { }
        //render
        void Render(const Scene& scene);
        //Li
        virtual Spectrum Li(const RayDifferential& ray, const Scene& scene, Sampler& sampler, MemoryArena& arena, uint32_t depth = 0) const = 0;
    protected:
        //protected data
        std::shared_ptr<const Camera> camera;
    private:
        //private data
        std::shared_ptr<Sampler> sampler;
    }
}