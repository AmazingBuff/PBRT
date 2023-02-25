#include"../pbrt.h"
#include"memory"

namespace pbrt
{
    class Primitive;
    
    class Scene
    {
    public:
        //public method
        Scene(std::shared_ptr<Primitive> aggregate, 
        const std::vector<std::shared_ptr<Light>>& lights);
    
        //public data
        std::vector<std::shared_ptr<Light>> lights;
    private:
        //private data
        std::shared_ptr<Primitive> aggregate;
    };
}
