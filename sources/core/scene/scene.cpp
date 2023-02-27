#include"scene.h"

namespace pbrt
{
    Scene::Scene(std::shared_ptr<Primitive> aggregate, 
        const std::vector<std::shared_ptr<Light>>& lights)
        : lights(lights), aggregate(aggregate)
        {
            worldBound = aggregate->WorldBound();
            //light initialize
            for(const std::shared_ptr<Light>& light : lights)
                light->Preprocess(*this);
        }
}