#include"../pbrt.h"
#include"memory"

namespace pbrt
{
    class Primitive;
    class Bound3f;
    class Ray;
    
    class Scene
    {
    public:
        //public method
        Scene(std::shared_ptr<Primitive> aggregate, 
        const std::vector<std::shared_ptr<Light>>& lights);
        const Bound3f& WorldBound() const { return worldBound; }
        //intersect with ray, and return the information about the frist intersection point
        bool Intersect(const Ray& ray, SurfaceInteraction* pSurfaceInter) const 
        { return aggregate->Intersect(ray, pSurfaceInter); }
        //intersect with ray, just return the bool
        bool IntersectP(const Ray& ray) const { return aggregate->IntersectP(ray);}

        //public data
        std::vector<std::shared_ptr<Light>> lights;
    private:
        //private data
        std::shared_ptr<Primitive> aggregate;
        //bounding box
        Bound3f worldBound;
    };
}
