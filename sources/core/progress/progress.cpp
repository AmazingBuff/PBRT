#include"progress.h"

namespace pbrt
{
   Progress::Progress(int64_t totalWork, const std::string& title)
   : totalWork(std::max((int64_t)1, totalWork)), title(title),
   startTime(std::chrono::system_clock::now())
   {
       workDone = 0;
       exitThread = false;
   } 
}