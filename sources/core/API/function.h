#include"../pbrt.h"

namespace pbrt
{
    Options PbrtOptions;
    
    //pbrt renderer initialize
    void pbrtInit(const Options& option);
    
    //pbrt renderer cleanup
    void pbrtCleanup();
}
