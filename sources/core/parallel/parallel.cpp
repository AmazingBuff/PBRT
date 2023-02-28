#include"parallel.h"
#include<thread>
#include<mutex>

namespace pbrt
{
    //parallel local definitions

    //worked threads
    static std::vector<std::thread> threads;
    static bool shutdownThreads = false;
    //thread_local just like static, is a thread-dependent qualifier
    //whose life-time is dependent on connected thread
    extern thread_local int ThreadIndex;
    //workList holds a pointer to the head of a list of parallel "for" loops
    class ParallelForLoop;
    static ParallelForLoop* workList = nullptr;
    static std::mutex workListMutex;
    //signal for waking up worker threads
    static std::condition_variable workListCondition;

    //perform loop for single thread
    class ParallelForLoop
    {
    public:
        ParallelForLoop(std::function<void(int)> func1D, int64_t maxIndex, int chunkSize, uint64_t profilerState)
        : function1D(func1D), maxIndex(maxIndex), chunkSize(chunkSize), profilerState(profilerState) { }
        //for 2D
        ParallelForLoop(const std::function<void(Point2i)>& func2D, const Point2i& count, uint64_t profilerState)
        : function2D(func2D), maxIndex(count.x * count.y), chunkSize(1), profilerState(profilerState)
        { xSum = count.x; }
        bool Finished() const { return nextIndex >= maxIndex && activeWorkers == 0; }
    public:
        std::function<void(int)> function1D;
        std::function<void(Point2i)> function2D;
        //2D image pixels count of x direction
        int xSum = -1; 
        //max count of computation required execution
        const int64_t maxIndex;
        //the computation of required exection of every single loop 
        const int chunkSize;
        const uint64_t profilerState;
        //the next loop index to be executed
        int64_t nextIndex = 0;
        //the amount of current worker threads which is running loop
        int activeWorkers = 0;
        ParallelForLoop* next = nullptr;
    };
    

    static void workerThreadFunc(int tIndex)
    {
        ThreadIndex = tIndex;
        std::unique_lock<std::mutex> lock(workListMutex);
        while(!shutdownThreads)
        {
            if(!workList)
            {
                //sleep until there are more tasks to run
                workListCondition.wait(lock);
            }
            else
            {
                //get work form worklist and run loop iterations
                ParallelForLoop& loop = *workList;
                //run a chunk of loop iterations
                //find the set of loop iterations to run next
                int64_t indexStart = loop.nextIndex;
                int64_t indexEnd = std::min(indexStart + loop.chunkSize, loop.maxIndex);
                //update loop to reflect iterations this thread will run
                loop.nextIndex = indexEnd;
                if(loop.nextIndex == loop.maxIndex)
                    workList = loop.next;
                loop.activeWorkers++;
                //run loop indices in [indexStart, indexEnd]
                lock.unlock();
                for(int index = indexStart; index < indexEnd; index ++)
                {
                    if(loop.function1D)
                        loop.function1D(index);
                    //handle other types of loops
                    else if(loop.function2D)
                        loop.function2D(Point2i(index % loop.xSum, index / loop.xSum));
                }
                lock.lock();
                //update loop to reflect completion of iterations
                loop.activeWorkers--;
                
                if(loop.Finished())
                    workListCondition.notify_all();
            }
        }
        //report thread statistics at work thread exit
        ReportThreadStats();
    }

    void ParallelFor(const std::function<void(int)>& func, int count, int chunkSize)
    {
        //run iteartions immediately if not using multi-threads or count is small
        if(PbrtOptions.nThreads == 1 || count < chunkSize)
        {
            for(int i = 0; i < count; i++)
                func(i);
            return;
        }
        //launch work threads if needed
        if(threads.size() == 0)
        {
            ThreadIndex = 0;
            //except for current execution thread
            for(int i = 0; i < NumSystemCores() - 1; i++)
                threads.push_back(std::thread(workerThreadFunc, i + 1));
        }
        //create and enqueue ParallerForLoop for this loop
        ParallelForLoop loop(func, count, chunkSize, CurrentProfilerState());
        workListMutex.lock();
        loop.next = workList;
        workList = &loop;
        workListMutex.unlock();
        //notify worker threads of work to be done
        std::unique_lock<std::mutex> lock(workListMutex);
        workListCondition.notify_all();
        //help out with parallel loop iterations in current thread
        while(!loop.Finished())
        {
            //run a chunk of loop iterations
            //find the set of loop iterations to run next
            int64_t indexStart = loop.nextIndex;
            int64_t indexEnd = std::min(indexStart + loop.chunkSize, loop.maxIndex);
            //update loop to reflect iterations this thread will run
            loop.nextIndex = indexEnd;
            if(loop.nextIndex == loop.maxIndex)
                workList = loop.next;
            loop.activeWorkers++;
            //run loop indices in [indexStart, indexEnd]
            lock.unlock();
            for(int index = indexStart; index < indexEnd; index ++)
            {
                if(loop.function1D)
                    loop.function1D(index);
                //handle other types of loops
                else if(loop.function2D)
                    loop.function2D(Point2i(index % loop.xSum, index / loop.xSum));
            }
            lock.lock();
            //update loop to reflect completion of iterations
            loop.activeWorkers--;
        }
    }

    void ParallelFor2D(std::function<void(Point2i)>& func, const Point2i& count)
    {
        //run iteartions immediately if not using multi-threads or count is small
        if(PbrtOptions.nThreads == 1 || count.x * count.y <= 1)
        {
            for(int y = 0; y < count.y; y++)
            {
                for(int x = 0; x < count.x; x++)
                    func(Point2i(x, y));
            }
            return;
        }
        //launch work threads if needed
        if(threads.size() == 0)
        {
            ThreadIndex = 0;
            //except for current execution thread
            for(int i = 0; i < NumSystemCores() - 1; i++)
                threads.push_back(std::thread(workerThreadFunc, i + 1));
        }
        //create and enqueue ParallerForLoop for this loop
        ParallelForLoop loop(func, count, CurrentProfilerState());
        {
            std::lock_guard<std::mutex> lock(workListMutex);
            loop.next = workList;
            workList = &loop;
        }
        //notify worker threads of work to be done
        std::unique_lock<std::mutex> lock(workListMutex);
        workListCondition.notify_all();
        //help out with parallel loop iterations in current thread
        while(!loop.Finished())
        {
            //run a chunk of loop iterations
            //find the set of loop iterations to run next
            int64_t indexStart = loop.nextIndex;
            int64_t indexEnd = std::min(indexStart + loop.chunkSize, loop.maxIndex);
            //update loop to reflect iterations this thread will run
            loop.nextIndex = indexEnd;
            if(loop.nextIndex == loop.maxIndex)
                workList = loop.next;
            loop.activeWorkers++;
            //run loop indices in [indexStart, indexEnd]
            lock.unlock();
            for(int index = indexStart; index < indexEnd; index ++)
            {
                if(loop.function1D)
                    loop.function1D(index);
                //handle other types of loops
                else if(loop.function2D)
                    loop.function2D(Point2i(index % loop.xSum, index / loop.xSum));
            }
            lock.lock();
            //update loop to reflect completion of iterations
            loop.activeWorkers--;
        }
    }

    int MaxThreadIndex()
    {
        if(PbrtOptions.nThreads != 1)
        {
            //launch worker threads if needed
            if(threads.size() == 0)
            {
                ThreadIndex = 0;
                //except for current execution thread
                for(int i = 0; i < NumSystemCores() - 1; i++)
                    threads.push_back(std::thread(workerThreadFunc, i + 1));
            }
        }
        return 1 + threads.size();
    }

    int NumSystemCores() 
    {
        return std::max(1u, std::thread::hardware_concurrency());
    }

}