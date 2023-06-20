#pragma once
#include"pbrt.h"

namespace pbrt
{
    static const uint64_t PCG32_DEFAULT_STATE  = 0x853c49e6748fea9bULL;
    static const uint64_t PCG32_DEFAULT_STREAM = 0xda3e39cb94b95bdbULL;
    static const uint64_t PCG32_MULT           = 0x5851f42d4c957f2dULL;

    static const double DoubleOneMinusEpsilon  = 0x1.fffffffffffffp-1;
    static const float FloatOneMinusEpsilon    = 0x1.fffffep-1;

    #ifdef PBRT_FLOAT_AS_DOUBLE
    static const Float OneMinusEpsilon = DoubleOneMinusEpsilon;
    #else
    static const Float OneMinusEpsilon = FloatOneMinusEpsilon;
    #endif

    class PseudoRandomGenerator
    {
    public:
        PseudoRandomGenerator() : state(PCG32_DEFAULT_STATE), inc(PCG32_DEFAULT_STREAM) { }
        PseudoRandomGenerator(uint64_t sequenceIndex) { SetSequence(sequenceIndex); }
        void SetSequence(uint64_t sequenceIndex)
        {
            state = 0u;
            inc = (sequenceIndex << 1u) | 1u;
            UniformUint32();
            state += PCG32_DEFAULT_STATE;
            UniformUint32();
        }
        //generate pseudo-random number
        uint32_t UniformUint32()
        {
            uint64_t oldState = state;
            state = oldState * PCG32_MULT + inc;
            uint32_t xorshifted = (uint32_t)(((oldState >> 18u) ^ oldState) >> 27u);
            uint32_t rot = (uint32_t)(oldState >> 59u);
            return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31));
        }
        //generate pseudo-random number in interval [0, bound - 1]
        uint32_t UniformUint32(uint32_t bound)
        {
            //threshold is the result of 2^32 mod bound
            uint32_t threshold = (~bound + 1u) % bound;
            while(true)
            {
                uint32_t result = UniformUint32();
                //to make result uniformly, 2^32 mod bound should not be here
                if(result >= threshold)
                    return result % bound;
            }
        }
        //generate pseudo-random float number in interval [0,1)
        Float UniformFloat()
        {
            return std::min(OneMinusEpsilon, UniformUint32() * 0x1p-32f);
        }

        template <typename Iterator>
        void Shuffle(Iterator begin, Iterator end)
        {
            for (Iterator it = end - 1; it > begin; --it)
                std::iter_swap(it, begin + UniformUint32((uint32_t)(it - begin + 1)));
        }

        void Advance(int64_t idelta)
        {
            uint64_t cur_mult = PCG32_MULT, cur_plus = inc, acc_mult = 1u,
                     acc_plus = 0u, delta = (uint64_t)idelta;
            while (delta > 0) 
            {
                if (delta & 1) 
                {
                    acc_mult *= cur_mult;
                    acc_plus = acc_plus * cur_mult + cur_plus;
                }
                cur_plus = (cur_mult + 1) * cur_plus;
                cur_mult *= cur_mult;
                delta /= 2;
            }
            state = acc_mult * state + acc_plus;
        }
    private:
        uint64_t state, inc;
    };
}