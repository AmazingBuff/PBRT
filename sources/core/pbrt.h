#include<vector>
#include<string>

namespace pbrt
{
    //forward declarations
    class Shape;
    class Aggregate;
    class Camera;
    class Sampler;
    class Filter;
    class Material;
    class Texture;
    class Medium;
    class Light;
    class Integrator;


    //global variables
    struct Options
    {
        int nThreads = 0;
        bool quickRender = false;
        bool quiet = false, verbose = false;
        std::string imageFile;
    };
}