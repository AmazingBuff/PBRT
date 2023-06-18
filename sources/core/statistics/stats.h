#pragma once
#include"core/pbrt.h"
#include<map>
#include<functional>

namespace pbrt
{
    //statistics counters
    class StatsAccumulator
    {
    public:
        void ReportCounter(const std::string& name, int64_t value) { counters[name] += value; }    
    private:
        std::map<std::string, int64_t> counters;
    };

    class StatRegisterer
    {
    public:
        StatRegisterer(std::function<void(StatsAccumulator&)> func)
        {
            if(!funcs)
                funcs = new std::vector<std::function<void(StatsAccumulator&)>>;
            funcs->push_back(func);
        }
        //for counters
        static void CallCallbacks(StatsAccumulator& accum);
    private:
        //StatRegisterer run first while StatsAccumulator don't run, so a pointer is neccessary
        static std::vector<std::function<void(StatsAccumulator&)>>* funcs;  
    };

    #define STAT_COUNTER(title, variable)                              \
    static thread_local int64_t variable;                              \
    static void STATS_FUNC##variable(StatsAccumulator& accumulartor)   \
    {                                                                  \
        accumulartor.ReportCounter(title, variable);                   \
        variable = 0;                                                  \
    }                                                                  \
    static StatRegisterer STATS_REG##variable(STATS_FUNC##variable)

    void ReportThreadStats();

    //profiler
    //a enumerant specify the phase of execution
    //which is implicitly defined by the order of the sort of excution 
    enum class Profiler
    {
        SceneConstruction,
        AccelConstruction,
        TextureLoading,
        MipMapCreation,

        IntegratorRender,
        SamplerIntegratorLi,
        SPPMCameraPass,
        SPPMGridConstruction,
        SPPMPhotonPass,
        SPPMStatsUpdate,
        BDPTGenerateSubpath,
        BDPTConnectSubpaths,
        LightDistribLookup,
        LightDistribSpinWait,
        LightDistribCreation,
        DirectLighting,
        BSDFEvaluation,
        BSDFSampling,
        BSDFPdf,
        BSSRDFEvaluation,
        BSSRDFSampling,
        AccelIntersect,
        AccelIntersectP,
        LightSample,
        LightPdf,
        MediumSample,
        MediumTr,
        TriIntersect,
        TriIntersectP,
        CurveIntersect,
        CurveIntersectP,
        ShapeIntersect,
        ShapeIntersectP,
        ComputeScatteringFuncs,
        GenerateCameraRay,
        MergeFilmTile,
        SplatFilm,
        AddFilmSample,
        StartPixel,
        GetSample,
        TexFiltTrilerp,
        TexFiltEWA,
        TexFiltPtex,
        NumProfCategories
    };

    extern thread_local uint64_t ProfilerState;
    inline uint64_t CurrentProfilerState() { return ProfilerState; }

    class ProfilePhase
    {
    public:
        ProfilePhase(Profiler phase)
        {
            categoryBit = (1ull << (int)phase);
            //notify high-level resursion and current resursion
            reset = (ProfilerState & categoryBit) == 0;
            ProfilerState |= categoryBit;
        }
        ~ProfilePhase()
        {
            if(reset)
                ProfilerState &= ~categoryBit;
        }
    private:
        ProfilePhase(const ProfilePhase&) = delete;
        ProfilePhase& operator=(const ProfilePhase&) = delete;
        //reset for resursion(such as computing Li)
        bool reset;
        //store the bit of corresponding current phase
        uint64_t categoryBit;
    };

    void InitProfiler();
}