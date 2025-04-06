#pragma once

#include <MemUtil/Import.hpp>
#include <MemUtil/Generic/StringStatic.hpp>
#include <MemUtil/Import_boost_stacktrace.hpp>
#include <MemUtil/Generic/StackContainer.hpp>
#include <MemUtil/STL_Imports/STD_array.hpp>
#include <MemUtil/STL_Imports/STD_thread.hpp>
#include <MemUtil/STL_Imports/STD_memory_resource.hpp>
#include <MemUtil/STL_Imports/STD_set.hpp>
#include <MemUtil/STL_Imports/STD_memory.hpp>
#include <MemUtil/STL_Imports/STD_deque.hpp>
#include <MemUtil/STL_Imports/STD_map.hpp>


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


class AllocationInfo final
{
public:
    std::uint64_t Size{ 0u };
    void* Ptr{ nullptr };
    CStackString StackString;
    boost::stacktrace::stacktrace Trace;
    EStackType Type{ EStackType::None };

    AllocationInfo();
    AllocationInfo( AllocationInfo&& arg ) noexcept;
    AllocationInfo& operator=( AllocationInfo&& arg ) noexcept;

    AllocationInfo( const AllocationInfo& ) = delete;
    AllocationInfo& operator=( const AllocationInfo& ) = delete;

    ~AllocationInfo();

protected:
private:

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
    MULib_API void releaseCallstack( boost::stacktrace::stacktrace* inCallstack );

private:
    MULib_API Memutil();
    MULib_API ~Memutil();
    void registerStack( void* ptr, std::uint64_t inSize );
    void unregisterStack( void* ptr, std::uint64_t inSize );
    void decode( AllocationInfo* stackInfo );
    bool m_initialized{ false };
    std::thread m_mainLoopThread;
    bool m_runMainLoop{ false };
    void mainLoop();
    boost::stacktrace::stacktrace* createStackTrace();
    AllocationInfo* fetchAllocationInfo();
    void convertBoostToAllocationInfo( AllocationInfo* inOut );

    bool m_enableTracking{ false };

    StackContainer<std::pmr::set<AllocationInfo*>, 4u * 1024u * 1024> m_allocations;
    mutable std::unique_ptr<IMutex> m_allocationsMtx;

    StackContainer<std::pmr::deque<AllocationInfo*>, 4u * 1024u * 1024> m_toBeDecodedList;
    mutable std::unique_ptr<IMutex> m_toBeDecodedListMtx;

    StackContainer<std::pmr::set<AllocationInfo*>, 4u * 1024u * 1024> m_unusedTrace;
    mutable std::unique_ptr<IMutex> m_unusedTraceMtx;

    StackContainer<std::pmr::set<boost::stacktrace::stacktrace*>, 4u * 1024u * 1024> m_usedStacks;
    mutable std::unique_ptr<IMutex> m_usedStacksMtx;

    StackContainer<std::pmr::set<boost::stacktrace::stacktrace*>, 4u * 1024u * 1024> m_unusedStacks;
    mutable std::unique_ptr<IMutex> m_unusedStacksMtx;

};
}