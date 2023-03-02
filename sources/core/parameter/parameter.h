#pragma once
#include"../pbrt.h"
#include<map>

namespace pbrt
{
    template<typename T>
    struct ParamSetItem
    {
    public:
        ParamSetItem(const std::string& name, std::unique_ptr<T[]> values, int nValues = 1)
        : name(name), values(std::move(values)), nValues(nValues) { }

        const std::string name;
        const std::unique_ptr<T[]> values;
        const int nValues;
        //mutable means that the corresponding value is changable while in const function
        mutable bool lookedUp = false;
    };

    class ParamSet
    {
    public:
        void AddFloat(const std::string& name, std::unique_ptr<Float[]> values, int nValues);
        void AddInt(const std::string& name, std::unique_ptr<int[]> values, int nValues);
        void AddBool(const std::string& name, std::unique_ptr<bool[]> values, int nValues);
        void AddPoint2f(const std::string& name, std::unique_ptr<Point2f[]> values, int nValues);
        void AddVector2f(const std::string& name, std::unique_ptr<Vector2f[]> values, int nValues);
        void AddPoint3f(const std::string& name, std::unique_ptr<Point3f[]> values, int nValues);
        void AddVector3f(const std::string& name, std::unique_ptr<Vector3f[]> values, int nValues);
        void AddNormal3f(const std::string& name, std::unique_ptr<Normal3f[]> values, int nValues);
        void AddString(const std::string& name, std::unique_ptr<std::string[]> values, int nValues);
        void AddTexture(const std::string& name, const std::string& value);
        void AddRGBSpectrum(const std::string& name, std::unique_ptr<Float[]> values, int nValues);
        void AddXYZSpectrum(const std::string& name, std::unique_ptr<Float[]> values, int nValues);
        void AddBlackBodySpectrum(const std::string& name, std::unique_ptr<Float[]> values, int nValues);
        void AddSampledSpcetrumFiles(const std::string& name, const char** names, int nValues);
        void AddSampledSpcetrum(const std::string& name, std::unique_ptr<Float[]> values, int nValues);

        bool EraseInt(const std::string& name);
        bool EraseBool(const std::string& name);
        bool EraseFloat(const std::string& name);
        bool ErasePoint2f(const std::string& name);
        bool EraseVector2f(const std::string& name);
        bool ErasePoint3f(const std::string& name);
        bool EraseVector3f(const std::string& name);
        bool EraseNormal3f(const std::string& name);
        bool EraseSpectrum(const std::string& name);
        bool EraseString(const std::string& name);
        bool EraseTexture(const std::string& name);

        //return the value of specified parameter, if not, return default value
        Float FindOneFloat(const std::string& name, Float def) const;
        int FindOneInt(const std::string& name, int def) const;
        bool FindOneBool(const std::string& name, bool def) const;
        Point2f FindOnePoint2f(const std::string& name, const Point2f& def) const;
        Vector2f FindOneVector2f(const std::string& name, const Vector2f& def) const;
        Point3f FindOnePoint3f(const std::string& name, const Point3f& def) const;
        Vector3f FindOneVector3f(const std::string& name, const Vector3f& def) const;
        Normal3f FindOneNormal3f(const std::string& name, const Normal3f& def) const;
        Spectrum FindOneSpectrum(const std::string& name, const Spectrum& def) const;
        std::string FindOneString(const std::string& name, const std::string& def = "") const;
        std::string FindOneFilename(const std::string& name, const std::string& def = "") const;
        std::string FindTexture(const std::string& name, const std::string& def = "") const;

        const Float* FindFloat(const std::string& name, int* nValues) const;
        const int* FindInt(const std::string& name, int* nValues) const;
        const bool* FindBool(const std::string& name, int* nValues) const;
        const Point2f* FindPoint2f(const std::string& name, int* nValues) const;
        const Vector2f* FindVector2f(const std::string& name, int* nValues) const;
        const Point3f* FindPoint3f(const std::string& name, int* nValues) const;
        const Vector3f* FindVector3f(const std::string& name, int* nValues) const;
        const Normal3f* FindNormal3f(const std::string& name, int* nValues) const;
        const Spectrum* FindSpectrum(const std::string& name, int* nValues) const;
        const std::string* FindString(const std::string& name, int* nValues) const;

        //report the parameters that are never looked up
        void ReportUnused() const;
        void Clear();
    private:
        std::vector<std::shared_ptr<ParamSetItem<bool>>> bools;
        std::vector<std::shared_ptr<ParamSetItem<int>>> ints;
        std::vector<std::shared_ptr<ParamSetItem<float>>> floats;
        std::vector<std::shared_ptr<ParamSetItem<Point2f>>> point2fs;
        std::vector<std::shared_ptr<ParamSetItem<Vector2f>>> vector2fs;
        std::vector<std::shared_ptr<ParamSetItem<Point3f>>> point3fs;
        std::vector<std::shared_ptr<ParamSetItem<Vector3f>>> vector3fs;
        std::vector<std::shared_ptr<ParamSetItem<Normal3f>>> normals;
        std::vector<std::shared_ptr<ParamSetItem<Spectrum>>> spectra;
        std::vector<std::shared_ptr<ParamSetItem<std::string>>> strings;
        std::vector<std::shared_ptr<ParamSetItem<std::string>>> textures;
    };


    //specific parameters for texture, notify the order of access
    class TextureParams
    {
    public:
        TextureParams(const ParamSet& geometryParams, const ParamSet& materialParams,
        std::map<std::string, std::shared_ptr<Texture<Float>>>& floatTextures, 
        std::map<std::string, std::shared_ptr<Texture<Spectrum>>>& sepctrumTextures)
        : geometryParams(geometryParams), materialParams(materialParams), 
          floatTextures(floatTextures), sepctrumTextures(sepctrumTextures) { }
        
        //get spectrum texture from decription, if not be found, return default
        std::shared_ptr<Texture<Spectrum>> GetSpectrumTexture(const std::string& name, const Spectrum& def) const;

        //find specific value with the order: geometryParams, then materialParams
        Float FindFloat(const std::string& name, Float def) const
        {
            return geometryParams.FindOneFloat(name, materialParams.FindOneFloat(name, def));
        }
    private:
        std::map<std::string, std::shared_ptr<Texture<Float>>>& floatTextures;
        std::map<std::string, std::shared_ptr<Texture<Spectrum>>>& sepctrumTextures;
        const ParamSet& geometryParams;
        const ParamSet& materialParams;
    };
}