#include"api.h"
#include"parameter/parameter.h"

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
        //method
        Scene* MakeScene()
        {
            std::shared_ptr<Primitive> accelrator = MakeAccelerator(AccelratorName, primitives, AccelratorParams);
            if(!accelrator)
                accelrator = std::make_shared<BVHAccel>(primitives);
            Scene* scene = new Scene(accelrator, lights);
            //erase primitives and lights from RenderOptions
            primitives.erase(primitives.begin(), primitives.end());
            lights.erase(lights.begin(), lights.end());
            return scene;
        }
        Integrator* MakeIntegrator() const
        {

        }
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
        //light
        std::vector<std::shared_ptr<Light>> lights;
        //shape
        std::vector<std::shared_ptr<Primitive>> primitives;
        //object instance
        std::map<std::string, std::vector<std::shared_ptr<Primitive>>> instances;
        std::vector<std::shared_ptr<Primitive>>* currentInstance = nullptr;
    };

    struct GraphicsState
    {
        //method
        std::shared_ptr<Material> GetMaterialFromShape(const ParamSet& params)
        {

        }
        //create a medium interface based on "inside" and "outside" media established with pbrtMediumInterface()
        MediumInterface CreateMediumInterface()
        {

        }
        //texture
        std::map<std::string, std::shared_ptr<Texture<Float>>> floatTextures;
        std::map<std::string, std::shared_ptr<Texture<Spectrum>>> spectrumTextures;
        //current material
        ParamSet materialParams;
        std::string material = "matte";
        std::map<std::string, std::shared_ptr<Material>> namedMaterials;
        std::string currentNamedMateril;
        //area light
        ParamSet areaLightParams;
        std::string areaLight;
    };

    class TransformCache
    {
        public:

        private:
        std::vector<Transform*> hashTable;
        int hashTableOccupancy;
        MemoryArena arena;
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

    //api static variable
    static APIState currentApiState = APIState::Uninitialized;
    
    //module-local variable
    static TransformSet curTransform;
    static int activeTransformBits = AllTransformsBits;

    //a copy of coordinate system for further usage
    static std::map<std::string, TransformSet> namedCoordinateSystems;

    //render option
    static std::unique_ptr<RenderOptions> renderOptions;

    //graphics state
    static GraphicsState graphicsState;
    static std::vector<GraphicsState> pushedGraphicsStates;
    static std::vector<TransformSet> pushedTransforms;
    static std::vector<uint32_t> pushedActiveTransformBits;

    //transform cache
    static TransformCache transformCache;



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
        Error("Options cannot be set inside world block; \"%s\" not allowed. Ignoring.", func);  \
        return;                                                                                  \
    } else /*swallow trailing semicolon*/

    #define VERIFY_WORLD(func)                                                                           \
    VERIFY_INITIALIZED(func)                                                                             \
    if(currentApiState == APIState::OptionBlock)                                                         \
    {                                                                                                    \
        Error("Scene description must be set inside world block; \"%s\" not allowed. Ignoring.", func);  \
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


    //object creation function
    std::vector<std::shared_ptr<Shape>> MakeShapes(const std::string& name, const Transform* ObjectToWorld,
    const Transform* WorldToObject, bool reverseOrientation, const ParamSet& paramSet)
    {
        std::vector<std::shared_ptr<Shape>> shapes;
        std::shared_ptr<Shape> shape;
        if(name == "sphere")
            shape = CreateSphereShape(ObjectToWorld, WorldToObject, reverseOrientation, paramSet);
        else if(name == "cylinder")
            shape = CreateCylinderShape(ObjectToWorld, WorldToObject, reverseOrientation, paramSet);
        else if(name == "disk")
            shape = CreateDiskShape(ObjectToWorld, WorldToObject, reverseOrientation, paramSet);   
    }

    //material creation function
    std::shared_ptr<Material> MakeMaterial(const std::string& name, const TextureParams& texParams)
    {

    }


    void pbrtInit(const Options& option)
    {
        PbrtOptions = option;
        //API initialization
        if(currentApiState != APIState::Uninitialized)
            Error("pbrtInit() has already been called.");
        currentApiState = APIState::OptionBlock;
        renderOptions.reset(new RenderOptions);
        graphicsState = GraphicsState();
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

    void pbrtWorldBegin()
    {
        VERIFY_OPTIONS("WorldBegin");
        currentApiState = APIState::WorldBlock;
        //reset the current transform to identity matrices and enable all
        for(uint32_t i = 0; i < MaxTransforms; i++)
            curTransform[i] = Transform();
        activeTransformBits = AllTransformBits;
        namedCoordinateSystems["World"] = curTransform;
    }

    void pbrtAttributeBegin()
    {
        VERIFY_WORLD("AttributeBegin");
        pushedGraphicsStates.push_back(graphicsState);
        pushedTransforms.push_back(curTransform);
        pushedActiveTransformBits.push_back(activeTransformBits);
    }

    void pbrtAttributeEnd()
    {
        VERIFY_WORLD("AttributeEnd");
        if(!pushedGraphicsStates.empty())
        {
            Error("Unmatched pbrtAttributeEnd() encountered. Ignoring it.");
            return;
        }
        graphicsState = pushedGraphicsStates.back();
        pushedGraphicsStates.pop_back();
        curTransform = pushedTransforms.back();
        pushedTransforms.pop_back();
        activeTransformBits = pushedActiveTransformBits.back();
        pushedActiveTransformBits.pop_back();
    }

    void pbrtTransformBegin()
    {
        VERIFY_WORLD("TransformBegin");
        pushedTransforms.push_back(curTransform);
        pushedActiveTransformBits.push_back(activeTransformBits);
    }

    void pbrtTransformEnd()
    {
        VERIFY_WORLD("TransformEnd");
        if(!pushedTransforms.empty())
        {
            Error("Unmatched pbrtTransformEnd() encountered. Ignoring it.");
            return;
        }
        curTransform = pushedTransforms.back();
        pushedTransforms.pop_back();
        activeTransformBits = pushedActiveTransformBits.back();
        pushedActiveTransformBits.pop_back();
    }

    void pbrtTexture(const std::string& name, const std::string& type, const std::string& texName, const ParamSet& params)
    {
        VERIFY_WORLD("Texture");
        TextureParams texParams(params, params, graphicsState.floatTextures, graphicsState.spectrumTextures);
        if(type == "float")
        {
            //create Float texture and store in floatTextures
            if(graphicsState.floatTextures.find(name) != graphicsState.floatTextures.end())
                Warning("Texture \"%s\" being redefined", name.c_str());
            WARN_IF_ANIMATED_TRANSFORM("Texture");
            std::shared_ptr<Texture<Float>> floatTexture = MakeFloatTexture(texName, curTransform[0], texParams);
            if(floatTexture)
                graphicsState.floatTextures[name] = floatTexture;
        }
        else if(type == "color" || type == "spectrum")
        {
            //create color texture and store in spectrumTextures
            if(graphicsState.spectrumTextures.find(name) != graphicsState.spectrumTextures.end())
                Warning("Texture \"%s\" being redefined", name.c_str());
            WARN_IF_ANIMATED_TRANSFORM("Texture");
            std::shared_ptr<Texture<Float>> spectrumTexture = MakeSpectrumTexture(texName, curTransform[0], texParams);
            if(spectrumTexture)
                graphicsState.spectrumTextures[name] = spectrumTexture;
        }
        else
            Error("Texture type \"%s\" unknown.", type.c_str());
    }

    void pbrtMaterial(const std::string& name, const ParamSet& params)
    {

    }

    void pbrtLightSource(const std::string& name, const ParamSet& params)
    {
        VERIFY_WORLD("LightSource");
        WARN_IF_ANIMATED_TRANSFORM("LightSource");
        MediumInterface interface = graphicsState.CreateMediumInterface();
        std::shared_ptr<Light> light = MakeLight(name, params, curTransform[0], interface);
        if(light)
            renderOptions->lights.push_back(light);
        else
            Error("LightSource: light type \"%s\" unknown.", name.c_str());
    }

    void pbrtAreaLightSource(const std::string& name, const ParamSet& params)
    {
        VERIFY_WORLD("AreaLightSource");
        graphicsState.areaLight = name;
        graphicsState.areaLightParams = params;
    }

    void pbrtShape(const std::string& name, const ParamSet& params)
    {
        VERIFY_WORLD("Shape");
        std::vector<std::shared_ptr<Primitive>> primitives;
        std::vector<std::shared_ptr<AreaLight>> areaLights;
        if(!curTransform.IsAnimated())
        {
            //initialize primitives and areaLights for static shape
            //create shapes for shape name
            Transform* ObjToWorld, *WorldToObj;
            transformCache.Lookup(curTransform[0], &ObjToWorld, &WorldToObj);
            std::vector<std::shared_ptr<Shape>> shapes = MakeShapes(name, ObjToWorld, WorldToObj, graphicsState.reverseOrientation, params);
            if(shapes.empty())
                return;
            std::shared_ptr<Material> material = graphicsState.GetMaterialFromShape(params);
            params.ReportUnused();
            MediumInterface interface = graphicsState.CreateMediumInterface();
            for(auto shape : shapes)
            {
                //possibly create area light for shape
                std::shared_ptr<AreaLight> area;
                if(graphicsState.areaLight != "")
                {
                    area = MakeAreaLight(graphicsState.areaLight, curTransform[0], interface, graphicsState.areaLightParams, shape);
                    areaLights.push_back(area);
                }
                //GeometricPrimitives stand for static shape
                primitives.push_back(std::make_shared<GeometricPrimitive>(shape, material, area, interface));
            }
        }
        else
        {
            //initialize primitives and areaLights for animated shape
            //create initial shape or shapes for animated shape
            if(graphicsState.areaLight != "")
                //don't support animated area light source
                Warning("Ignoring currently set area light when creating animated shape");
            Transform* identity;
            transformCache.Lookup(Transform(), &identity, nullptr);
            std::vector<std::shared_ptr<Shape>> shapes = MakeShapes(name, identity, identity, graphicsState.reverseOrientation, params);
            if(shapes.empty())
                return;
            //create GeometricPrimitive for animated shape
            std::shared_ptr<Material> material = graphicsState.GetMaterialFromShape(params);
            params.ReportUnused();
            MediumInterface interface = graphicsState.CreateMediumInterface();
            for(auto shape : shapes)
                primitives.push_back(std::make_shared<GeometricPrimitive>(shape, material, nullptr, interface));
            //create single TransformedPrimitive for primitives
            //get animatedObjectToWor ld transform for shape
            Transform* ObjToWorld[2];
            transformCache.Lookup(curTransform[0], &ObjToWorld[0], nullptr);
            transformCache.Lookup(curTransform[0], &ObjToWorld[1], nullptr);
            AnimatedTransform animatedObjectToWorld(ObjToWorld[0], renderOptions->transformStartTime, ObjToWorld[1], renderOptions->transformEndTime);
            if(primitives.size() > 1)
            {
                std::shared_ptr<Primitive> bvh = std::make_shared<BVHAccel>(primitives);
                primitives.clear();
                primitives.push_back(bvh);
            }
            primitives[0] = std::make_shared<TransformedPrimitive>(primitives[0], animatedObjectToWorld);
        }
        //add primitives and areaLights to scene or current intance
        if(renderOptions->currentInstance)
        {
            if(!areaLights.empty())
                Warning("Area lights not supported with object instancing");
            renderOptions->currentInstance->insert(renderOptions->currentInstance->end(), primitives.begin(), primitives.end());
        }
        else
        {
            renderOptions->primitives.insert(renderOptions->primitives.end(), primitives.begin(), primitives.end());
            if(!areaLights.empty())
                renderOptions->lights.insert(renderOptions->lights.end(), areaLights.begin(), areaLights.end());
        }
    }

    void pbrtObjectBegin(const std::string& name)
    {
        pbrtAttributeBegin();
        if(renderOptions->currentInstance)
            Error("ObjectBegin called inside of instance definition");
        renderOptions->instances[name] = std::vector<std::shared_ptr<Primitive>>();
        renderOptions->currentInstance = &renderOptions->instances[name];
    }

    void pbrtObjectEnd()
    {
        VERIFY_WORLD("ObjectEnd");
        if(!renderOptions->currentInstance)
            Error("objectEnd called outside of instance definition");
        renderOptions->currentInstance = nullptr;
        pbrtAttributeEnd();
    }

    void pbrtObjectInstance(const std::string& name)
    {
        VERIFY_WORLD("ObjectInstance");
        //perform object instance error checking

        std::vector<std::shared_ptr<Primitive>>& instance = renderOptions->instances[name];
        if(instance.empty())
            return;
        if(instance.size() > 1)
        {
            //create aggregate for instace Primitives
            std::shared_ptr<Primitive> accel(MakeAccelerator(renderOptions->AcceleratorName, 
            instance, renderOptions->AcceleratorParams));
            if(!accel)
                accel = std::make_shared<BVHAccel>(instance);
            instance.erase(instance.begin(), instance.end());
            instance.push_back(accel);
        }
        //create animatedInstanceToWorld transform for instance
        Transform* InstanceToWorld[2];
        transformCache.Lookup(curTransform[0], &InstanceToWorld[0], nullptr);
        transformCache.Lookup(curTransform[0], &InstanceToWorld[1], nullptr);
        AnimatedTransform animatedInstanceToWorld(InstanceToWorld[0], 
        renderOptions->transformStartTime, InstanceToWorld[1], renderOptions->transformEndTime);
        std::shared_ptr<Primitive> primitive(std::make_shared<TransformedPrimitive>(instance[0], animatedInstanceToWorld));
        renderOptions->primitives.push_back(primitive);
    }

    void pbrtWorldEnd()
    {
        VERIFY_WORLD("WorldEnd");
        //ensure there are no pushed graphics states
        while(pushedGraphicsStates.size())
        {
            Warning("Missing end to pbrtAttributeBegin()");
            pushedGraphicsStates.pop_back();
            pushedTransforms.pop_back();
        }
        while(pushedTransforms.size())
        {
            Warning("Missing end to pbrtTransformBegin()");
            pushedTransforms.pop_back();
        }
        //create scene and render
        std::unique_ptr<Integrator> integrator(renderOptions->MakeIntegrator());
        std::unique_ptr<Scene> scene(renderOptions->MakeScene());
        if(scene && integrator)
            integrator->Render(*scene);
        TerminateWorkerThreads();
        //clean up after rendering
        currentApiState = APIState::OptionBlock;
        ReportThreadStats();
        if(!PbrtOptions.quiet)
        {
            PrintStats(stdout);
            ReportProfilerResults(stdout);
        }
        for(int i = 0; i < MaxTransforms; i++)
            curTransform[i] = Transform();
        activeTransformBits = AllTransformBits;
        namedCoordinateSystems.erase(namedCoordinateSystems.begin(), namedCoordinateSystems.end());
    }
}
