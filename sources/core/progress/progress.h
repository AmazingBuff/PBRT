#pragma once
#include"core/pbrt.h"
#include<chrono>
#include<atomic>
#include<thread>

namespace pbrt
{
    //Progress for feedback with showing rendering progress
    class Progress
    {
    public:
        Progress(int64_t totalWork, const std::string& title);
        ~Progress();

        //work done count
        void Update(int64_t num = 1)
        {
            if(num == 0 || PbrtOptions.quiet)
                return;
            workDone += num;
        }
    private:
        //private method
        void PrintBar();

        //private data
        const int64_t totalWork;
        const std::string title;
        const std::chrono::system_clock::time_point startTime;
        std::atomic<int64_t> workDone;
        std::atomic<bool> exitThread;
        std::thread updateThread;
    }
}