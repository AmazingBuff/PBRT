#include"api.h"
#include<map>

namespace pbrt
{
    constexpr int MaxTransforms      = 2;
    constexpr int StartTransformBits = 1 << 0;
    constexpr int EndTransformBits   = 1 << 1;
    constexpr int AllTransformBits   = (1 << MaxTransforms) - 1;

    struct TransformSet
    {
        Transform& operator[](int i)
        {
            return trans[i];
        }
        friend TransformSet Inverse(const TransformSet& ts)
        {
            TransformSet tInv;
            for(int i = 0; i < MaxTransforms; i++)
                tInv.trans[i] = Inverse(ts.trans[i]);
            return tInv;
        }
    private:
        Transform trans[MaxTransforms];
    };


    enum class APIState
    {
        //before pbrtInit() and after pbrtCleanup()
        Uninitialized,
        //outsize pbrtWorldBegin() and pbrtWorldEnd()
        OptionBlock,
        //insize pbrtWorldBegin() and pbrtWorldEnd()
        WorldBlock
    };
    static APIState currentApiState = APIState::Uninitialized;
    
    //module-local variable
    static TransformSet curTransform;
    static int activeTransformBits = AllTransformsBits;

    //a copy of coordinate system for further usage
    static std::map<std::string, TransformSet> namedCoordinateSystems;

    //verify APIState
    #define VERIFY_INITIALIZED(func)                                            \
    if(currentApiState == APIState::Uninitialized)                              \
    {                                                                           \
        Error("pbrtInit() must be before calling \"%s()\". Ignoring.", func);   \
        return;                                                                 \
    } else 

    #define VERIFY_OPTIONS(func)                                                                 \
    VERIFY_INITIALIZED(func)                                                                     \
    if(currentApiState == APIState::WorldBlock)                                                  \
    {                                                                                            \
        ERROR("Options cannot be set inside world block; \"%s\" not allowed. Ignoring.", func);  \
        return;                                                                                  \
    } else

    #define VERIFY_WORLD(func)                                                                           \
    VERIFY_INITIALIZED(func)                                                                             \
    if(currentApiState == APIState::OptionBlock)                                                         \
    {                                                                                                    \
        ERROR("Scene description must be set inside world block; \"%s\" not allowed. Ignoring.", func);  \
        return;                                                                                          \
    } else

    #define FOR_ACTIVE_TRANSFORMS(expr)     \
    for(int i = 0; i < MaxTransforms; i++)  \
    {                                       \
        if(activeTransformBits & (1 << i))  \
        {                                   \
            expr;                           \
        }                                   \
    }

    void pbrtInit(const Options& option)
    {
        PbrtOptions = option;
        //API initialization
        if(currentApiState != APIState::Uninitialized)
            Error("pbrtInit() has already been called.");
        currentApiState = APIState::OptionBlock;
        //general pbrt initialization
    }
    
    void pbrtCleanup()
    {
        //API cleanup
        if(currentApiState == APIState::Uninitialized)
            Error("pbrtCleanup() called without pbrtInit().");
        else if(currentApiState == APIState::WorldBlock)
            Error("pbrtCleanup() called while inside world block.");
        currentApiState = APIState::Uninitialized;
    }

    void pbrtIdentity()
    {
        VERIFY_INITIALIZED("Identity");
        FOR_ACTIVE_TRANSFORMS(curTransform[i] = Transform())
    }

    void pbrtTranslate(Float dx, Float dy, Float dz)
    {
        VERIFY_INITIALIZED("Translate");
        FOR_ACTIVE_TRANSFORMS(curTransform[i] = curTransform[i] * Translate(Vector3f(dx, dy, dz)))
    }

    void pbrtRotate(Float angle, Float axisX, Float axisY, Float axisZ)
    {
        VERIFY_INITIALIZED("Rotate");
        FOR_ACTIVE_TRANSFORMS(curTransform[i] = curTransform[i] * Rotate(angle, Vector3f(axisX, axisY, axisZ)))
    }

    
}
