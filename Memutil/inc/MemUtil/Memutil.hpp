#pragma once

#include <MemUtil/Import.hpp>
#include <MemUtil/Generic/StringStatic.hpp>
#include <MemUtil/STL_Imports/STD_array.hpp>
#include <MemUtil/STL_Imports/STD_thread.hpp>
#include <MemUtil/STL_Imports/STD_mutex.hpp>
#include <MemUtil/STL_Imports/STD_memory_resource.hpp>
#include <MemUtil/STL_Imports/STD_set.hpp>


namespace MU
{
constexpr std::uint8_t G_maxStackSize = 16u;

using StackLineString = StringStatic<256>;
using StackLinesArray = std::array<StackLineString, G_maxStackSize>;

struct AllocationInfo
{
    std::uint64_t Size{ 0u };
    StackLinesArray StackLines;

    bool operator==( const AllocationInfo& arg ) const
    {
        if( Size != arg.Size )
        {
            return false;
        }

        return StackLines == arg.StackLines;
    }

    bool operator<( const AllocationInfo& arg ) const
    {
        for( std::size_t i = 0u; i < G_maxStackSize; ++i )
        {
            if( StackLines[i] == arg.StackLines[i] )
            {
                continue;
            }

            return StackLines[i] < arg.StackLines[i];
        }

        return false;
    }

    std::set<void*> Ptrs;
};

struct StackInfo;

enum class SortType : std::int8_t
{
    None = -1,
    SizeAsceding,
    SizeDescending
};

class Memutil final
{
public:
    static MULib_API Memutil& getInstance();
    Memutil( const Memutil& ) = delete;
    Memutil( Memutil&& ) = delete;
    Memutil& operator=( const Memutil& ) = delete;
    Memutil& operator=( Memutil&& ) = delete;

    MULib_API void init();

    MULib_API void logRealloc( void* inOldPtr, void* inNewPtr, std::uint64_t inSize, std::size_t skipFirstLinesCount = 0 );
    MULib_API void logAlloc( void* inPtr, std::uint64_t inSize, std::size_t skipFirstLinesCount = 0 );
    MULib_API void logFree( void* inPtr );


    /**
     * Start or stop logging callstacks.
     * 
     * \param inToggleTracking true to start, false to stop logging.
     * \return 
     */
    MULib_API void toggleTracking( bool inToggleTracking );

    /**
     * Dumps all active allocations to standard output.
     * 
     * \return 
     */
    MULib_API void dumpActiveAllocationsToOutput() const;

    /**
     * Dumps all active allocations to char buffer.
     * 
     * \param outBuffer Buffer to be filled.
     * \param inBufferCapacity Buffer to be filled - size.
     * \return true if all data has been placed on buffer and false if buffer is too small.
     */
    MULib_API bool dumpActiveAllocationsToBuffer( char* outBuffer, std::size_t inBufferCapacity ) const;
    MULib_API bool waitForAllCallStacksToBeDecoded() const;
    MULib_API std::int32_t getActiveAllocations() const;

private:
    MULib_API Memutil();
    void getStackHere( StackLinesArray& outStackLines, std::size_t skipFirstLinesCount = 0 );
    MULib_API ~Memutil();
    void registerStack( void* ptr, std::uint64_t inSize, std::size_t skipFirstLinesCount );
    void unregisterStack( void* ptr, std::uint64_t inSize, std::size_t skipFirstLinesCount );
    void decode( const StackInfo& stackInfo );
    bool m_initialized{ false };
    std::thread m_mainLoopThread;
    bool m_runMainLoop{ false };
    void mainLoop();
    mutable std::mutex g_traceDequeMtx;

    bool m_enableTracking{ false };
    mutable std::mutex m_dataMtx;
    static constexpr std::uint64_t PoolSize = 2u * 1024u * 1024u;  // 2MB
    std::array<std::byte, PoolSize> m_bufferBlocks;
    std::pmr::monotonic_buffer_resource m_buffer_src{ m_bufferBlocks.data(), PoolSize };
    std::pmr::set<AllocationInfo> m_allocations{ &m_buffer_src };
};
}