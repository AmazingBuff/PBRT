#include"memory.h"

namespace pbrt
{
    void* AllocAligned(size_t size)
    {
        //implementation differs with different platform
        #if defined(PBRT_IS_WINDOWS)
        return _aligned_malloc(size, PBRT_L1_CACHE_LINE_SIZE);
        #elif defined(PBRT_IS_OPENBSD) || defined(PBRT_IS_OSX)
        void* ptr;
        if(posix_memalign(&ptr, PBRT_L1_CACHE_LINE_SIZE, size) != 0)
            ptr = nullptr;
        return ptr;
        #else
        return memalign(PBRT_L1_CACHE_LINE_SIZE, size);
        #endif
    }

    void FreeAligned(void* ptr)
    {
        if(!ptr)
            return;
        #if defined(PBRT_IS_WINDOWS)
        _aligned_free(ptr);
        #else
        free(ptr);
        #endif
    }

    void* MemoryArena::Alloc(size_t nBytes)
    {
        //round up nBytes to minimum machine alignment
        #if __GNUC__ == 4 && __GNUC_MINOR__ < 9
        const int alignment = alignof(::max_align_t);
        #elif !defined(PBRT_HAVE_ALIGNOF)
        const int alignment = 16;
        #else
        const int alignment = alignof(std::max_align_t);
        #endif
        
        nBytes = (nBytes + alignment - 1) & ~(alignment - 1);
        //when required memory nBytes is more than unallocated memory of current block
        if(nBytes > currentAllocSize - currentBlockPos)
        {
            //add current block to usedBlocks list
            if(currentBlock)
            {
                usedBlocks.push_back(std::make_pair(currentAllocSize, currentBlock));
                currentBlock = nullptr;
            }
            //get new block of memory for MemoryArena
            //try to get memory block from availableBlocks
            for(auto iterator = availbleBlocks.begin(); iterator != availbleBlocks.end(); iterator++)
            {
                if(iterator->first >= nBytes)
                {
                    currentAllocSize = iterator->first;
                    currentBlock = iterator->second;
                    availbleBlocks.erase(iterator);
                    break;
                }
            }
            //nBytes is more than availbleBlocks
            if(!currentBlock)
            {
                currentAllocSize = std::max(nBytes, blockSize);
                currentBlock = AllocAligned<uint8_t>(currentAllocSize);
            }

            currentBlockPos = 0;
        }

        void* result = currentBlock + currentBlockPos;
        currentBlockPos += nBytes;
        return result;
    }

    void MemoryArena::Reset()
    {
        currentBlockPos = 0;
        availbleBlocks.splice(availbleBlocks.begin(), usedBlocks);
    }

    size_t MemoryArena::TotalAllocated() const
    {
        size_t total = currentAllocSize;
        for(const auto& iterator : usedBlocks)
            total += iterator.first;
        for(const auto& iterator : availbleBlocks)
            total += iterator.first;
        return total;
    }

    MemoryArena::~MemoryArena()
    {
        FreeAligned(currentBlock);
        for(auto& block : usedBlocks)
            FreeAligned(block.second);
        for(auto& block : availbleBlocks)
            FreeAligned(block.second);
    }
}