#pragma once
#include"core/pbrt.h"

namespace pbrt
{
    class BVHAccel
    {
    public:
        //the algorithms that can be specify to create BVH tree
        enum class SplitMethod
        {
            SAH,
            HLBVH,
            Middle,
            EqualCounts
        };

        BVHAccel(std::vector<std::shared_ptr<Primitive>>& primitives, int maxPrimsInNode, SplitMethod splitMethod);
    private:
        const int maxPrimsInNode;
        const SplitMethod splitMethod;
        std::vector<std::shared_ptr<Primitive>> primitives;
    }
    


}
