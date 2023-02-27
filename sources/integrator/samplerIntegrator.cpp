#include"samplerIntegrator.h"

namespace pbrt
{
    void SamplerIntegrator::Render(const Scene& scene)
    {
        Preprocess(scene, *sampler);
        //render image tiles in parallel
        //compute number of tiles, nTiles, to use for parallel rendering
        Bounds2i sampleBounds = camera->film->GetSampleBounds();
        Vector2i sampleExtent = sampleBounds.Diagonal();
        ParallelFor2D([&](Point2i tile){
            //render section of image corresponding to tile          
        }, nTiles);
    }
}