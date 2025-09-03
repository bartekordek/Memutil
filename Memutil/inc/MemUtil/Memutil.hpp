#pragma once

#include <MemUtil/Import.hpp>
#include <MemUtil/Generic/StringStatic.hpp>
#include <MemUtil/Generic/StackContainer.hpp>
#include <MemUtil/Generic/PMR_unordered_map.hpp>
#include <MemUtil/Stack.hpp>
#include <MemUtil/Generic/Mutex.hpp>
#include <MemUtil/STL_Imports/STD_array.hpp>
#include <MemUtil/STL_Imports/STD_thread.hpp>
#include <MemUtil/STL_Imports/STD_set.hpp>
#include <MemUtil/STL_Imports/STD_memory.hpp>
#include <MemUtil/STL_Imports/STD_deque.hpp>
#include <MemUtil/STL_Imports/STD_list.hpp>
#include <MemUtil/STL_Imports/STD_map.hpp>
#include <MemUtil/STL_Imports/STD_functional.hpp>

namespace MU
{
class IMutex;

constexpr std::uint8_t G_maxStackSize = 16u;

using CStackString = StringStatic<512>;

struct StackInfo;

enum class SortType : std::int8_t
{
    None = -1,
    SizeAsceding,
    SizeDescending
};

template <class C>
class IDequeThreadSafe;

#define PMR_ALLOCATED 0

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
    MULib_API void executeWithoutAllocationLog( std::function<void( void )> inLambda );

    /**
     * Start or stop logging callstacks.
     *
     * \param inToggleTracking true to start, false to stop logging.
     * \return
     */
    MULib_API void toggleTracking( bool inToggleTracking );

    MULib_API bool getTrackingStatus() const;

    /**
     * Dumps all active allocations to standard output.
     *
     * \return
     */
    MULib_API void dumpActiveAllocationsToOutput();

    /**
     * Dumps all active allocations to char buffer.
     *
     * \param outBuffer Buffer to be filled.
     * \param inBufferCapacity Buffer to be filled - size.
     * \return true if all data has been placed on buffer and false if buffer is too small.
     */
    MULib_API bool dumpActiveAllocationsToBuffer( char* outBuffer, std::size_t inBufferCapacity );
    MULib_API bool waitForAllCallStacksToBeDecoded() const;
    MULib_API std::int32_t getActiveAllocations() const;
    MULib_API void waitForAllAllocationsToBeResolved() const;
    MULib_API void beginUntracked();
    MULib_API void endUntracked();

private:
    MULib_API Memutil();
    MULib_API ~Memutil();
    void registerStack( void* ptr, std::uint64_t inSize );
    void unregisterStack( void* ptr, std::uint64_t inSize );
    void dumpActiveAllocationsToOutput_impl();
    bool dumpActiveAllocationsToBuffer_impl( char* outBuffer, std::size_t inBufferCapacity );


    bool m_initialized{ false };
    constexpr static std::size_t decodeWorkersCount{ 1u };
    std::array<std::thread*, decodeWorkersCount> m_decodeThreads;
    bool m_runMainLoop{ false };
    void mainLoop();

    bool m_enableTracking{ false };

    IDequeThreadSafe<CStack>* m_stacksToBeDecoded{ nullptr };

#if PMR_ALLOCATED
    //StackContainer<std::pmr::unordered_map<void*, CStack>, 4u * 1024u * 1024> m_allocated;
    PMR_unordered_map<void*, CStack> m_allocated;
#else // PMR_ALLOCATED
    std::unordered_map<void*, CStack>* m_allocated{ nullptr };
#endif // PMR_ALLOCATED
    mutable Mutex m_allocatedMtx;

    std::atomic<std::uint64_t> m_maxToBeDecoded{ 0u };
};
}  // namespace MU