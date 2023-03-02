#include"parameter.h"

namespace pbrt
{
    #define ADD_PARAM_TYPE(T, vec) \
    (vec).emplace_back(new ParamSetItem<T>(name, std::move(values), nValues))

    #define ERASE_PARAM_TYPE(vec)              \
    for (size_t i = 0; i < (vec).size(); i++)  \
    {                                          \
        if((vec)[i]->name == name)             \
        {                                      \
            (vec).erase((vec).begin() + i);    \
            return true;                       \
        }                                      \
        return false;                          \
    }

    #define LOOKUP_ONE(vec)                    \
    for(const auto& i : vec)                   \
    {                                          \
        if(i->name == name && i->nValues == 1) \
        {                                      \
            i->lookedUp = true;                \
            return i->values[0];               \
        }                                      \
    }                                          \
    return def

    #define LOOKUP_PTR(vec)                    \
    for(const auto& i : vec)                \
    {                                          \
        if(i->name == name)                    \
        {                                      \
            *nValues = i->nValues;             \
            i->lookedUp = true;                \
            return i->values.get();            \
        }                                      \
    }                                          \
    return nullptr           
    

    void ParamSet::AddFloat(const std::string& name, std::unique_ptr<Float[]> values, int nValues)
    {
        EraseFloat(name); //clear
        ADD_PARAM_TYPE(Float, floats);
    }
    void ParamSet::AddInt(const std::string& name, std::unique_ptr<int[]> values, int nValues)
    {
        EraseInt(name); //clear
        ADD_PARAM_TYPE(int, ints);
    }
    void ParamSet::AddBool(const std::string& name, std::unique_ptr<bool[]> values, int nValues)
    {
        EraseBool(name);
        ADD_PARAM_TYPE(bool, bools);
    }
    void ParamSet::AddPoint2f(const std::string& name, std::unique_ptr<Point2f[]> values, int nValues)
    {
        ErasePoint2f(name);
        ADD_PARAM_TYPE(Point2f, point2fs);
    }
    void ParamSet::AddVector2f(const std::string& name, std::unique_ptr<Vector2f[]> values, int nValues)
    {
        EraseVector2f(name);
        ADD_PARAM_TYPE(Vector2f, vector2fs);
    }
    void ParamSet::AddPoint3f(const std::string& name, std::unique_ptr<Point3f[]> values, int nValues)
    {
        ErasePoint3f(name);
        ADD_PARAM_TYPE(Point3f, point3fs);
    }
    void ParamSet::AddVector3f(const std::string& name, std::unique_ptr<Vector3f[]> values, int nValues)
    {
        EraseVector3f(name);
        ADD_PARAM_TYPE(Vector3f, vector3fs);
    }
    void ParamSet::AddNormal3f(const std::string& name, std::unique_ptr<Normal3f[]> values, int nValues)
    {
        EraseNormal3f(name);
        ADD_PARAM_TYPE(Normal3f, normals);
    }
    void ParamSet::AddString(const std::string& name, std::unique_ptr<std::string[]> values, int nValues)
    {
        EraseString(name);
        ADD_PARAM_TYPE(std::string, strings);
    }
    void ParamSet::AddTexture(const std::string& name, const std::string& value);
    void ParamSet::AddRGBSpectrum(const std::string& name, std::unique_ptr<Float[]> values, int nValues);
    void ParamSet::AddXYZSpectrum(const std::string& name, std::unique_ptr<Float[]> values, int nValues);
    void ParamSet::AddBlackBodySpectrum(const std::string& name, std::unique_ptr<Float[]> values, int nValues);
    void ParamSet::AddSampledSpcetrumFiles(const std::string& name, const char** names, int nValues);
    void ParamSet::AddSampledSpcetrum(const std::string& name, std::unique_ptr<Float[]> values, int nValues);

    bool ParamSet::EraseInt(const std::string& name)
    {
        ERASE_PARAM_TYPE(ints);
    }
    bool ParamSet::EraseBool(const std::string& name)
    {
        ERASE_PARAM_TYPE(bools);
    }
    bool ParamSet::EraseFloat(const std::string& name)
    {
        ERASE_PARAM_TYPE(floats);
    }
    bool ParamSet::ErasePoint2f(const std::string& name)
    {
        ERASE_PARAM_TYPE(point2fs);
    }
    bool ParamSet::EraseVector2f(const std::string& name)
    {
        ERASE_PARAM_TYPE(vector2fs);
    }
    bool ParamSet::ErasePoint3f(const std::string& name)
    {
        ERASE_PARAM_TYPE(point3fs);
    }
    bool ParamSet::EraseVector3f(const std::string& name)
    {
        ERASE_PARAM_TYPE(vector3fs);
    }
    bool ParamSet::EraseNormal3f(const std::string& name)
    {
        ERASE_PARAM_TYPE(normals);
    }
    bool ParamSet::EraseSpectrum(const std::string& name)
    {
        ERASE_PARAM_TYPE(spectra);
    }
    bool ParamSet::EraseString(const std::string& name)
    {
        ERASE_PARAM_TYPE(strings);
    }
    bool ParamSet::EraseTexture(const std::string& name)
    {
        ERASE_PARAM_TYPE(textures);
    }

    Float ParamSet::FindOneFloat(const std::string& name, Float def) const
    {
        LOOKUP_ONE(floats);
    }
    int ParamSet::FindOneInt(const std::string& name, int def) const
    {
        LOOKUP_ONE(ints);
    }
    bool ParamSet::FindOneBool(const std::string& name, bool def) const
    {
        LOOKUP_ONE(bools);
    }
    Point2f ParamSet::FindOnePoint2f(const std::string& name, const Point2f& def) const
    {
        LOOKUP_ONE(point2fs);
    }
    Vector2f ParamSet::FindOneVector2f(const std::string& name, const Vector2f& def) const
    {
        LOOKUP_ONE(vector2fs);
    }
    Point3f ParamSet::FindOnePoint3f(const std::string& name, const Point3f& def) const
    {
        LOOKUP_ONE(point3fs);
    }
    Vector3f ParamSet::FindOneVector3f(const std::string& name, const Vector3f& def) const
    {
        LOOKUP_ONE(vector3fs);
    }
    Normal3f ParamSet::FindOneNormal3f(const std::string& name, const Normal3f& def) const
    {
        LOOKUP_ONE(normals);
    }
    Spectrum ParamSet::FindOneSpectrum(const std::string& name, const Spectrum& def) const
    {
        LOOKUP_ONE(spectra);
    }
    std::string ParamSet::FindOneString(const std::string& name, const std::string& def) const
    {
        LOOKUP_ONE(strings);
    }
    std::string ParamSet::FindTexture(const std::string& name, const std::string& def) const
    {
        LOOKUP_ONE(textures);
    }
    std::string ParamSet::FindOneFilename(const std::string& name, const std::string& def) const
    {
        std::string filename = FindOneString(name, "");
        if(filename == "")
            return def;
        filename = AbsolutePath(ResolveFilename(filename));
        return filename;
    }

    const Float* ParamSet::FindFloat(const std::string& name, int* nValues) const
    {
        LOOKUP_PTR(floats);
    }
    const int* ParamSet::FindInt(const std::string& name, int* nValues) const
    {
        LOOKUP_PTR(ints);
    }
    const bool* ParamSet::FindBool(const std::string& name, int* nValues) const
    {
        LOOKUP_PTR(bools);
    }
    const Point2f* ParamSet::FindPoint2f(const std::string& name, int* nValues) const
    {
        LOOKUP_PTR(point2fs);
    }
    const Vector2f* ParamSet::FindVector2f(const std::string& name, int* nValues) const
    {
        LOOKUP_PTR(vector2fs);
    }
    const Point3f* ParamSet::FindPoint3f(const std::string& name, int* nValues) const
    {
        LOOKUP_PTR(point3fs);
    }
    const Vector3f* ParamSet::FindVector3f(const std::string& name, int* nValues) const
    {
        LOOKUP_PTR(vector3fs);
    }
    const Normal3f* ParamSet::FindNormal3f(const std::string& name, int* nValues) const
    {
        LOOKUP_PTR(normals);
    }
    const Spectrum* ParamSet::FindSpectrum(const std::string& name, int* nValues) const
    {
        LOOKUP_PTR(spectra);
    }
    const std::string* ParamSet::FindString(const std::string& name, int* nValues) const
    {
        LOOKUP_PTR(strings);
    }

    void ParamSet::ReportUnused() const
    {
        #define CHECK_UNUSED(value)                                          \
        for (size_t i = 0; i < (value).size(); i++)                          \
        {                                                                    \
            if(!(value)[i]->lookedUp)                                        \
            Warning("Parameter \"%s\" not used", (value)[i]->name.c_str());  \
        }
        CHECK_UNUSED(ints);
        CHECK_UNUSED(bools);
        CHECK_UNUSED(floats);
        CHECK_UNUSED(point2fs);
        CHECK_UNUSED(vector2fs);
        CHECK_UNUSED(point3fs);
        CHECK_UNUSED(vector3fs);
        CHECK_UNUSED(normals);
        CHECK_UNUSED(spectra);
        CHECK_UNUSED(strings);
        CHECK_UNUSED(textures);
        #undef CHECK_UNUSED
    }

    void ParamSet::Clear()
    {
        #define DEL_PARAMS(value) (value).erase((value).begin(), (value).end())
        DEL_PARAMS(ints);
        DEL_PARAMS(bools);
        DEL_PARAMS(floats);
        DEL_PARAMS(point2fs);
        DEL_PARAMS(vector2fs);
        DEL_PARAMS(point3fs);
        DEL_PARAMS(vector3fs);
        DEL_PARAMS(normals);
        DEL_PARAMS(spectra);
        DEL_PARAMS(strings);
        DEL_PARAMS(textures);
        #undef DEL_PARAMS
    }

    std::shared_ptr<Texture<Spectrum>> TextureParams::GetSpectrumTexture(
        const std::string& name, const Spectrum& def) const
    {
        //notify the order of search
        std::string requiredName = geometryParams.FindTexture(name);
        if(requiredName == "")
            requiredName = materialParams.FindTexture(name);
        else
        {
            if(sepctrumTextures.find(requiredName) != sepctrumTextures.end())
                return sepctrumTextures[requiredName];
            else
                Error("Countn't find spectrum texture named \"%s\" " 
                "for parameter \"%s\"", requiredName.c_str(), name.c_str());
        }
        Spectrum value = materialParams.FindOneSpectrum(name, def);
        value = geometryParams.FindOneSpectrum(name, value);
        return std::make_shared<ConstantTexture<Spectrum>>(value);
    }
}