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

        //while current transform is not equal to next transform
        bool IsAnimated() const
        {
            for(int i = 0; i < MaxTransforms; i++)
            {
                if(trans[i] != trans[i + 1])
                    return true;
            }
            return false;
        }
    private:
        Transform trans[MaxTransforms];
    };


    //render option for storing something had been set
    struct RenderOptions
    {
        //transform matrix
        Float transformStartTime = 0;
        Float transformEndTime = 1;
        //pixel filter
        std::string FilterName = "box";
        ParamSet FilterParams;
        //camera
        std::string CameraName = "perspective";
        ParamSet CameraParams;
        TransformSet CameraToWorld;
        //medium
        std::map<std::string, std::shared_ptr<Medium>> namedMedia;
    };

    struct GraphicsState
    {

    }


    enum class APIState
    {
        //before pbrtInit() and after pbrtCleanup()
        Uninitialized,
        //outsize pbrtWorldBegin() and pbrtWorldEnd()
        OptionBlock,
        //insize pbrtWorldBegin() and pbrtWorldEnd()
        WorldBlock
    };

    //api static variable
    static APIState currentApiState = APIState::Uninitialized;
    
    //module-local variable
    static TransformSet curTransform;
    static int activeTransformBits = AllTransformsBits;

    //a copy of coordinate system for further usage
    static std::map<std::string, TransformSet> namedCoordinateSystems;

    //render option
    static std::unique_ptr<RenderOptions> renderOptions;

    //verify APIState
    #define VERIFY_INITIALIZED(func)                                            \
    if(currentApiState == APIState::Uninitialized)                              \
    {                                                                           \
        Error("pbrtInit() must be before calling \"%s()\". Ignoring.", func);   \
        return;                                                                 \
    } else /*swallow trailing semicolon*/

    #define VERIFY_OPTIONS(func)                                                                 \
    VERIFY_INITIALIZED(func)                                                                     \
    if(currentApiState == APIState::WorldBlock)                                                  \
    {                                                                                            \
        ERROR("Options cannot be set inside world block; \"%s\" not allowed. Ignoring.", func);  \
        return;                                                                                  \
    } else /*swallow trailing semicolon*/

    #define VERIFY_WORLD(func)                                                                           \
    VERIFY_INITIALIZED(func)                                                                             \
    if(currentApiState == APIState::OptionBlock)                                                         \
    {                                                                                                    \
        ERROR("Scene description must be set inside world block; \"%s\" not allowed. Ignoring.", func);  \
        return;                                                                                          \
    } else /*swallow trailing semicolon*/

    #define FOR_ACTIVE_TRANSFORMS(expr)     \
    for(int i = 0; i < MaxTransforms; i++)  \
    {                                       \
        if(activeTransformBits & (1 << i))  \
        {                                   \
            expr;                           \
        }                                   \
    }

    #define WARN_IF_ANIMATED_TRANSFORM(func)                              \
    do {                                                                  \
        if(curTransform.IsAnimated())                                     \
            Warning("Animated transformations set: ignoring for "\%s"\ "  \
            "and using the start transform only", func);                  \
    } while(false) /*swallow trailing semicolon*/



    void pbrtInit(const Options& option)
    {
        PbrtOptions = option;
        //API initialization
        if(currentApiState != APIState::Uninitialized)
            Error("pbrtInit() has already been called.");
        currentApiState = APIState::OptionBlock;
        renderOptions.reset(new RenderOptions);
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
        renderOptions.reset(nullptr);
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

    void pbrtCoordinateSystem(const std::string& name)
    {
        VERIFY_INITIALIZED("CoordinateSystem");
        namedCoordinateSystems[name] = curTransform;
    }

    void pbrtCoordSysTransform(const std::string& name)
    {
        VERIFY_INITIALIZED("CoordSysTransform");
        if(namedCoordinateSystems.find(name) != namedCoordinateSystems.end())
            curTransform = namedCoordinateSystems[name];
        else
            Warning("Couldn't find named coordinate system \"%s\"", name.c_str());
    }

    void pbrtActiveTransformAll()
    {
        activeTransformBits = AllTransformBits;
    }
    void pbrtActiveTransformEndTime()
    {
        activeTransformBits = EndTransformBits;
    }
    void pbrtActiveTransformStartTime()
    {
        activeTransformBits = StartTransformBits;
    }

    void pbrtTransformTimes(Float start, Float end)
    {
        VERIFY_OPTIONS("TransformTimes");
        renderOptions->transformStartTime = start;
        renderOptions->transformEndTime = end;
    }

    void pbrtPixelFilter(const std::string& name, const ParamSet& params)
    {
        VERIFY_OPTIONS("PixelFilter");
        renderOptions->FilterName = name;
        renderOptions->FilterParams = params;
    }

    void pbrtCamera(const std::string& name, const ParamSet& params)
    {
        VERIFY_OPTIONS("Camera");
        renderOptions->CameraName = name;
        renderOptions->CameraParams = params;
        //set transform and save
        renderOptions->CameraToWorld = Inverse(curTransform);
        namedCoordinateSystems["camera"] = renderOptions->CameraToWorld;
    }

    void pbrtMakeNamedMedium(const std::string& name, const ParamSet& params)
    {

    }

    void pbrtMediumInterface(const std::string& insideName, const std::string& outsideName)
    {
        VERIFY_INITIALIZED("MediumInterface");
        graphicsState.currentInsideMedium = insideName;
        graphicsState.currentOutsideMedium = outsideName;
    }
}
