#pragma once
#include"core/pbrt.h"
#include<list>

//optimalize the layout of resources
namespace pbrt
{
    //allocate cache-aligned memory
    void* AllocAligned(size_t size);

    //allocate cache-aligned memory with required type
    template<typename T>
    T* AllocAligned(size_t count)
    {
        return (T*)AllocAligned(count * sizeof(T));
    }

    //free memory
    void FreeAligned(void*);

    //arena-based memory allocation
    class MemoryArena
    {
    public:
        //allocate a block of memory with initial 256kb
        MemoryArena(size_t blockSize = 262144ull) : blockSize(blockSize) { }
        //allocate nBytes from block, and return the head pointer of the allocated nbytes
        void* Alloc(size_t nBytes);
        //allocate an array of objects of the given type
        template<typename T>
        T* Alloc(size_t count = 1, bool runConstructor = true)
        {
            T* result = (T*)Alloc(count * sizeof(T));
            if(runConstructor)
            {
                for(size_t i = 0; i < count; i++)
                    new (&result[i]) T();
            }
            return result;
        }
        //reset current block
        void Reset();
        //get all allocated memory
        size_t TotalAllocated() const;
    private:
        //avoid some undefined operation
        MemoryArena(const MemoryArena&) = delete;
        MemoryArena& operator=(const MemoryArena&) = delete;

        //the size of single block
        const size_t blockSize;
        //a pointer that point to the beginning of current block
        uint8_t* currentBlock = nullptr;
        //store the position of the current block pointer
        size_t currentBlockPos = 0;
        //the whole memory size that  is allocated from current block
        size_t currentAllocSize = 0;
        //a list that store used block
        std::list<std::pair<size_t, uint8_t*>> usedBlocks;
        //a list for further usage
        std::list<std::pair<size_t, uint8_t*>> availbleBlocks;
    };

    //a class subdivide multiple dimension array into small block to decrease cache miss
    //logBlockSize specific the size of small block whose size if the pow of 2
    template<typename T, int logBlockSize>
    class BlockedArray
    {
    public:
        BlockedArray(int uRes, int vRes, const T* data = nullptr)
        : uRes(uRes), vRes(vRes), uBlocks(RoundUp(uRes) >> logBlockSize)
        {
            int nAlloc = RoundUp(uRes) * RoundUp(vRes);
            this->data = AllocAligned<T>(nAlloc);
            for(int i = 0; i < nAlloc; i++)
                new (&this->data[i]) T();
            //store extern data
            if(data)
            {
                for(int v = 0; v < vRes; v++)
                {
                    for(int u = 0; u < uRes; u++)
                        (*this)(u, v) = data[v * uRes + u];
                } 
            }
        }
        ~BlockedArray()
        {
            for (int i = 0; i < uRes * vRes; ++i) 
                data[i].~T();
            FreeAligned(data);
        }
        //return the size of a single block whose square is size * size
        constexpr int BlockSize() const { return 1 << logBlockSize; }
        //return the multiple of current value(higher multiple)
        int RoundUp(int value) const
        {
            int multiple = BlockSize();
            return (value + multiple - 1) & ~(multiple - 1);
        }
        //return the u,v size of the block array
        int uSize() const { return uRes; }
        int vSize() const { return vRes; }
        //retuen the block and offset of a exact (u,v) position
        int Block(int value) const { return value >> logBlockSize; }      //divide
        int Offset(int value) const { return value & (BlockSize() - 1); } //module
        //refresh data
        T& operator()(int u, int v)
        {
            int bu = Block(u), bv = Block(v);
            int ou = Offset(u), ov = Offset(v);
            //find the block position
            int offset = BlockSize() * BlockSize() * (uBlocks * bv + bu);
            //plus the offset of the block
            offset += BlockSize() * ov + ou;
            return data[offset];
        }
        const T& operator()(int u, int v) const
        {
            int bu = Block(u), bv = Block(v);
            int ou = Offset(u), ov = Offset(v);
            int offset = BlockSize() * BlockSize() * (uBlocks * bv + bu);
            offset += BlockSize() * ov + ou;
            return data[offset];
        }
        //from blocked array to linear array
        void GetLinearArray(T* array) const
        {
            for(int v = 0; v < vRes; v++)
            {
                for(int u = 0; u < uRes; u++)
                    *array++ = (*this)(u, v);
            }
        }
    private:
        T* data;
        const int uRes, vRes;
        //the size of width blocks
        const int uBlocks;
    };
}