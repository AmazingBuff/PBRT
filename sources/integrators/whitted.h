#pragma once

#include"core/integrator/integrator.h"

namespace pbrt
{
    class WhittedIntegrator : public SamplerIntegrator
    {
    public:
        WhittedIntegrator(std::shared_ptr<const Camera> camera, std::shared_ptr<Sampler> sampler, uint32_t maxDepth = 5)
            : SamplerIntegrator(camera, sampler), maxDepth(maxDepth) {} 
        Spectrum Li(const RayDifferential& ray, const Scene& scene, Sampler& sampler, MemoryArena& arena, uint32_t depth) const override;
    private:
        const uint32_t maxDepth;
    };
    
    
    
}