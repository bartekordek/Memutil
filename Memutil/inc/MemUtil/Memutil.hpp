#pragma once

#include <MemUtil/Import.hpp>
#include <MemUtil/Generic/StringStatic.hpp>
#include <MemUtil/STL_Imports/STD_array.hpp>
#include <MemUtil/STL_Imports/STD_thread.hpp>
#include <MemUtil/STL_Imports/STD_mutex.hpp>
#include <MemUtil/STL_Imports/STD_memory_resource.hpp>
#include <MemUtil/STL_Imports/STD_unordered_map.hpp>


namespace MU
{
constexpr std::uint8_t G_maxStackSize = 16u;

using StackLineString = StringStatic<256>;
using StackLinesArray = std::array<StackLineString, G_maxStackSize>;

struct AllocationInfo
{
    std::uint64_t Size{ 0u };
    StackLinesArray StackLines;
};

struct StackInfo;

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
    MULib_API void toggleTracking( bool inToggleTracking );
    MULib_API void dumpActiveAllocations() const;
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
    std::pmr::unordered_map<void*, AllocationInfo> m_allocations{ &m_buffer_src };
};
}