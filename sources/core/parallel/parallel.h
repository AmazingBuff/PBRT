#pragma once
#include"../pbrt.h"
#include<functional>

namespace pbrt
{
    //atomic Float for multi-thread
    class AtomicFloat
    {
    public:
        explicit AtomicFloat(Float value = 0)
        { bits = FloatToBits(value); }

        //implicit conversion AtomicFloat to Float-type
        operator Float() const { return BitsToFloat(bits); }
        Float operator=(Float value) 
        { bits = FloatToBits(value); return value; }
        //AtomicFloat addi  tion
        void Add(Float value)
        {
            #ifdef PBRT_FLOAT_AS_DOUBLE
            uint64_t oldBits = bits, newBits;
            #else
            uint32_t oldBits = bits, newBits;
            #endif
            do
            {
                newBits = FloatToBits(BitsToFloat(oldBits) + value);
            } while (!bits.compare_exchange_weak(oldBits, newBits));     
        }
    private:
        //because atomic operation only be used to integer
        //we need to transform a Float to integar
        #ifdef PBRT_FLOAT_AS_DOUBLE
        std::atomic<uint64_t> bits;
        #else
        std::atomic<uint32_t> bits;
        #endif
    };


    //parallel for all main loop
    //function for perform, count is the required loop, chunkSize is the count of single loop
    void ParallelFor(const std::function<void(int)>& function, int count, int chunkSize = 1);
    //parallel for 2D like image
    void ParallelFor2D(std::function<void(Point2i)>& function, const Point2i& count);
    //return max thread index
    int MaxThreadIndex();
    //return the processor count of the system
    int NumSystemCores();
}   