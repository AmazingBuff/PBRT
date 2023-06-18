#include"bvh.h"
#include"core/memory/memory.h"

namespace pbrt
{
    //record the information of the primitive
    struct BVHPrimitiveInfo
    {
        BVHPrimitiveInfo(size_t primitiveIndex, const Bounds3f& bounds)
        : primitiveIndex(primitiveIndex), bounds(bounds), centroid(0.5f * bounds.pMin + 0.5f * bounds.pMax) { }
        size_t primitiveIndex;
        Bounds3f bounds;
        Point3f centroid;    
    };

    struct BVHBuildNode
    {
        //initialize leaf node which has no children
        void InitLeaf(int firstPrimOffset, int nPrimitives, const Bounds3f& bounds)
        {
            this->firstPrimOffset = firstPrimOffset;
            this->nPrimitives = nPrimitives;
            this->bounds = bounds;
            left = right = nullptr;
        }
        //initialize interior node which has two children
        void InitInterior(int splitAxis, BVHBuildNode* left, BVHBuildNode* right)
        {
            this->left = left;
            this->right = right;
            this->splitAxis = splitAxis;
            bounds = Union(left->bounds, right->bounds);
            nPrimitives = 0;
        }
        Bounds3f bounds;
        BVHBuildNode* left;
        BVHBuildNode* right;
        int splitAxis;
        int firstPrimOffset;
        int nPrimitives;
    };


    BVHAccel::BVHAccel(std::vector<std::shared_ptr<Primitive>>& primitives, int maxPrimsInNode, SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), primitives(primitives), splitMethod(splitMethod)
    {
        if(primitives.empty())
            return;
        //build BVH from primitives
        //initialize primitiveInfo array for primitives
        std::vector<BVHPrimitiveInfo> primitiveInfo(primitives.size());
        for (size_t i = 0; i < primitives.size(); i++)
            primitiveInfo[i] = { i, primitives[i]->WorldBound()};
        //build BVH tree for primitives using primitiveInfo
        MemoryArena arena(1024 * 1024);
        int totalNodes = 0;
        std::vector<std::shared_ptr<Primitive>> orderedPrimitives;
        BVHBuildNode* root;
        if(splitMethod == SplitMethod::HLBVH)
            root = HLBVHBuild(arena, primitiveInfo, 0, primitives.size(), &totalNodes, orderedPrimitives);
        else
            root = recursiveBuild(arena, primitiveInfo, 0, primitives.size(), &totalNodes, orderedPrimitives);
        primitives.swap(orderedPrimitives);
        //compute representation of depth-first traversal of BVH tree
    }
}