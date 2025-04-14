#pragma once

#include <MemUtil/Import.hpp>
#include <MemUtil/Generic/StringStatic.hpp>
#include <MemUtil/Import_boost_stacktrace.hpp>
#include <MemUtil/Generic/StackContainer.hpp>
#include <MemUtil/Stack.hpp>
#include <MemUtil/STL_Imports/STD_array.hpp>
#include <MemUtil/STL_Imports/STD_thread.hpp>
#include <MemUtil/STL_Imports/STD_memory_resource.hpp>
#include <MemUtil/STL_Imports/STD_set.hpp>
#include <MemUtil/STL_Imports/STD_memory.hpp>
#include <MemUtil/STL_Imports/STD_deque.hpp>
#include <MemUtil/STL_Imports/STD_unordered_map.hpp>


namespace MU
{
class IMutex;

enum class EStackType : std::int8_t
{
    None = -1,
    Allocation,
    Deallocation
};


constexpr std::uint8_t G_maxStackSize = 16u;

using CStackString = StringStatic<512>;


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

    MULib_API void logRealloc( void* inOldPtr, void* inNewPtr, std::uint64_t inSize );
    MULib_API void logAlloc( void* inPtr, std::uint64_t inSize );
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
    MULib_API ~Memutil();
    void registerStack( void* ptr, std::uint64_t inSize );
    void unregisterStack( void* ptr, std::uint64_t inSize );
    bool m_initialized{ false };
    std::thread m_mainLoopThread;
    bool m_runMainLoop{ false };
    void mainLoop();

    bool m_enableTracking{ false };

    StackContainer<std::pmr::deque<CStack>, 4u * 1024u * 1024> m_toBeDecoded;
    mutable std::unique_ptr<IMutex> m_toBeDecodedMtx;

    StackContainer<std::pmr::unordered_map<void*,CStack>, 4u * 1024u * 1024> m_allocated;
    mutable std::unique_ptr<IMutex> m_allocatedMtx;
};
}