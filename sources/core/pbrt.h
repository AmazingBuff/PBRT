#pragma once
#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>
#include <string>

#include "logger/log.h"

namespace pbrt
{
    //forward declarations

    //platform
    #define PBRT_IS_WINDOWS

    //cache line for cache-aligned allocate memory
    #ifndef PBRT_L1_CACHE_LINE_SIZE
    #define PBRT_L1_CACHE_LINE_SIZE 64
    #endif

    //change float precision
    #ifdef PBRT_FLOAT_AS_DOUBLE
    typedef double Float;
    #else
    typedef float Float;
    #endif

    //global macros

    //debug detect
    #ifdef NDEBUG
    #define Assert(expression) ((void)0)
    #else
    #define Assert(expression) \
            ((expression) ? (void)0 :\
            Fatal("Assertion \"%s\" failed in %s, line %d", \
                   #expression, __FILE__, __LINE__))
    #endif

    //stack allocate memory
    #define ALLOCA(TYPE, COUNT) (TYPE*)alloca((COUNT) * sizeof(TYPE))

	//geometry
	template<typename T>
	class Vector2;
	template<typename T>
	class Vector3;

	class Quaternion;
	class Transform;

    //shape
    class Shape;
    class Primitive;
    //static shape
    class GeometricPrimitive;
    //animated shape
    class TransformedPrimitive;

    class Aggregate;
    class Camera;
    class Sampler;
    class Filter;
    class Material;
    template<typename T>
    class Texture;
    //medium
    class Medium;
    struct MediumInterface;
    //light
    class Light;
    class AreaLight;

    //render
    class Scene;
    class Integrator;

    //alignment memory
    class MemoryArena;
    template <typename T, int logBlockSize = 2>
    class BlockedArray;

    //parameter/parameter.h
    class ParamSet;
    template <typename T>
    struct ParamSetItem;
    class TextureParams;
    
    //global structure
    struct Options
    {
        int nThreads = 0;
        bool quickRender = false;
        bool quiet = false, verbose = false;
        std::string imageFile;
    };

    extern Options PbrtOptions;

    //global constants
    static const Float Pi      = 3.14159265358979323846;
    static const Float InvPi   = 0.31830988618379067154;
    static const Float Inv2Pi  = 0.15915494309189533577;
    static const Float Inv4Pi  = 0.07957747154594766788;
    static const Float PiOver2 = 1.57079632679489661923;
    static const Float PiOver4 = 0.78539816339744830961;
    static const Float Sqrt2   = 1.41421356237309504880;

    //global function
    inline uint32_t FloatToBits(float f)
    {
        uint32_t ui;
        memcpy(&ui, &f, sizeof(float));
        return ui;
    }
    inline float BitsToFloat(uint32_t ui)
    {
        float f;
        memcpy(&f, &ui, sizeof(uint32_t));
        return f;
    }

    inline uint64_t FloatToBits(double f)
    {
        uint64_t ui;
        memcpy(&ui, &f, sizeof(double));
        return ui;
    }
    inline double BitsToFloat(uint64_t ui)
    {
        double f;
        memcpy(&f, &ui, sizeof(uint64_t));
        return f;
    }


    //clamp value to [lower, higher]
    template<typename T, typename U, typename V>
    inline T Clamp(T value, U lower, V higher)
    {
        if(value < static_cast<T>(lower))
            return static_cast<T>(lower);
        else if(value > static_cast<T>(higher))
            return static_cast<T>(higher);
        else
            return value;
    }

    //modulus dividend with divisor to obtain a positive value
    template<typename T>
    inline T Mod(T dividend, T divisor)
    {
        T result = dividend - (dividend / divisor) * divisor;
        return result < 0 ? (divisor > 0 ? result + divisor : result - divisor) : result;
    }

    //float modulus from stardard library
    inline Float Mod(Float dividend, Float divisor)
    {
        return std::fmod(dividend, divisor);
    }

    //convert degree to radians, and vice versa
    inline Float Radians(Float degree)
    {
        return (Pi / 180) * degree;
    }
    inline Float Degrees(Float radian)
    {
        return (180 / Pi) * radian;
    }

    //log2 operation: log2(x) = log(x) / log(2)
    inline Float Log2(Float value)
    {
        const Float invLog2 = 1.442695040888963387004650940071;
        return std::log(value) * invLog2;
    }

    //log2 operation for integer
    inline uint32_t Log2Int(uint32_t value)
    {
#ifdef __MSVC__
        auto mask = static_cast<unsigned long>(value);
        //_BitScanReverse return conut of the first nonzero value with binary from high to low
        unsigned long index;
        _BitScanReverse(&index, mask);
        return static_cast<int>(index);
#elif __MINGW32__
		return 31 - __builtin_clz(value);
#endif
    }

    //judge if an integer is an exact power of 2
    template<typename T>
    inline bool IsPowerOf2(T value)
    {
        return value && !(value & (value - 1));
    }

    //round an integer up to the next higher(or equal) power of 2
    inline int32_t RoundUpPow2(int32_t value)
    {
        value--;
        value |= value >> 1;
        value |= value >> 2;
        value |= value >> 4;
        value |= value >> 8;
        value |= value >> 16;
        return value + 1;
    }

    //count trailing zeros
    inline uint32_t CountTrailingZeros(uint32_t value)
    {
#ifdef __MSVC__
        auto mask = static_cast<unsigned long>(value);
        unsigned long index;
        _BitScanForward(&index, mask);
        return index;
#elif __MINGW32__
		return __builtin_ctz(value);
#endif
    }

    //bisect search for a sequential interval
    template<typename Predicate>
    inline int FindInterval(int size, const Predicate& predicate)
    {
        int first = 0, length = size;
        while(length > 0)
        {
            int half = length >> 1, middle = first + half;
            //bisect range based on value of predicate at middle
            if(predicate(middle))
            {
                first = middle + 1;
                length -= half + 1;
            }
            else
                length = half;
        }
        return Clamp(first - 1, 0, size - 2);
    }

    //mathematical routines

    //linear interpolation, within interval [a,b]
    inline Float Lerp(Float t, Float a, Float b)
    {
        return (1 - t) * a + t * b;
    }

    //solve quadratic equation, with ax^2 + bx + c = 0
    //return false while the quadratic equation has no root
    //in contrast, the obtained two roots x1 will less than x2
    inline bool Quadratic(Float a, Float b, Float c, Float* x1, Float* x2)
    {
        //quadratic discriminant
        double discriminant = (double)b * (double)b - 4 * (double)a * (double)c;
        if(discriminant < 0)
            return false;
        double rootDiscrim = std::sqrt(discriminant);
        //compute root value, here we use Viète low to get stable form
        double q;
        if(b < 0)
            q = -0.5 * (b - rootDiscrim);
        else
            q = -0.5 * (b + rootDiscrim);
        *x1 = q / a;
        *x2 = c / q;
        if(*x1 > *x2)
            std::swap(*x1, *x2);
        return true;
    }

}