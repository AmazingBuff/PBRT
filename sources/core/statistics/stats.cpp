#include"stats.h"
#include<mutex>

namespace pbrt
{
    std::vector<std::function<void(StatsAccumulator&)>>* StatRegisterer::funcs;
    static StatsAccumulator statsAccumulator;

    void StatRegisterer::CallCallbacks(StatsAccumulator& accum)
    {
        for(auto func : *funcs)
            func(accum);
    }

    void ReportThreadStats()
    {
        //add a lock to avoid other thread updating the StatsAccumulator
        static std::mutex mutex;
        std::lock_guard<std::mutex> lock(mutex);
        StatRegisterer::CallCallbacks(statsAccumulator);
    }

    void InitProfiler()
    {
        
    }
}