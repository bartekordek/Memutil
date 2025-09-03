#include <MemUtil/Memutil.hpp>

#include <MemUtil/STL_Imports/STD_iostream.hpp>
#include <MemUtil/STL_Imports/STD_cstdio.hpp>
#include <MemUtil/Generic/ScopeExit.hpp>
#include <MemUtil/Generic/IMutex.hpp>
#include <MemUtil/Generic/DataContainer.hpp>
#include <MemUtil/Generic/DequeThreadSafe.hpp>
#include <MemUtil/Generic/ListThreadSafe.hpp>
#include "Generic/ConcurentQueueAdapter.hpp"
#include <MemUtil/Stack.hpp>
#include <MemUtil/Import_tracy.hpp>
#include <MemUtil/IDebugWrapper.hpp>
#include <MemUtil/STL_Imports/STD_algorithm.hpp>
#include <MemUtil/STL_Imports/STD_cmath.hpp>

#if 0  // DEBUG_THIS_FILE
    #define DEBUG_THIS_FILE 1

    #if defined( CUL_COMPILER_MSVC )
        #pragma optimize( "", off )
    #elif defined( CUL_COMPILER_CLANG )
        #pragma clang optimize off
    #elif defined( CUL_COMPILER_GCC )
        #pragma GCC push_options
        #pragma GCC optimize( "O0" )
    #endif
#endif

namespace MU
{

thread_local bool g_blockCurrentThread{ false };
thread_local bool g_isDecodingThread{ false };
Memutil* g_instance{ nullptr };

Memutil& Memutil::getInstance()
{
    static Memutil instance;
    return instance;
}

Memutil::Memutil()
{
    IDebugWrapper::getInstance().init();
}

template <typename p>
p Pow( p x, p y )
{
    p i = 1;
    for( p j = 1; j <= y; j++ ) i *= x;
    return i;
}

void Memutil::init()
{
    MU_MEASURE_SCOPE;
    if( m_initialized )
    {
        return;
    }

    constexpr std::int8_t containerType = 1;
    if( containerType == 0 )
    {
        m_stacksToBeDecoded = new ConcurentQueueAdapter<CStack>();
    }
    else
    {
        m_stacksToBeDecoded = new ListThreadSafe<CStack>();
    }

    const std::size_t objectsToContain{ Pow<std::size_t>( 2u, 16u ) };
    m_stacksToBeDecoded->init( objectsToContain );
#if PMR_ALLOCATED
    m_allocated.init( 64u * 1024u * 1024u );
#else
    m_allocated = new std::unordered_map<void*, CStack>();
#endif  // PMR_ALLOCATED

    m_runMainLoop = true;

    for( std::size_t i = 0u; i < decodeWorkersCount; ++i )
    {
        m_decodeThreads[i] = new std::thread( &Memutil::mainLoop, this );
    }

    m_initialized = true;
    g_instance = this;
}

void Memutil::executeWithoutAllocationLog( std::function<void( void )> inLambda )
{
    g_blockCurrentThread = true;
    inLambda();
    g_blockCurrentThread = false;
}

void Memutil::logRealloc( void* inOldPtr, void* inNewPtr, std::uint64_t inSize )
{
    MU_MEASURE_SCOPE;
    if( ( g_blockCurrentThread == true ) || ( m_enableTracking == false ) || ( g_isDecodingThread == true ) )
    {
        return;
    }
    logFree( inOldPtr );
    logAlloc( inNewPtr, inSize );
}

void Memutil::logAlloc( void* inPtr, std::uint64_t inSize )
{
    MU_MEASURE_SCOPE;
    if( ( g_blockCurrentThread == true ) || ( m_enableTracking == false ) || ( g_isDecodingThread == true ) )
    {
        return;
    }

    registerStack( inPtr, inSize );
}

void Memutil::logFree( void* inPtr )
{
    MU_MEASURE_SCOPE;
    if( ( g_blockCurrentThread == true ) || ( m_enableTracking == false ) || ( g_isDecodingThread == true ) )
    {
        return;
    }

    unregisterStack( inPtr, 0u );
}

void Memutil::registerStack( void* ptr, std::uint64_t inSize )
{
    MU_MEASURE_SCOPE;

    static thread_local CStack stack;

    beginUntracked();
    stack.fetch();
    endUntracked();

    stack.Size = inSize;
    stack.Data = ptr;
    stack.Type = EStackType::Alloc;

    beginUntracked();
    m_stacksToBeDecoded->addToBack( stack );
    endUntracked();
}

void Memutil::unregisterStack( void* ptr, std::uint64_t /*inSize*/ )
{
    MU_MEASURE_SCOPE;

    static thread_local CStack stack;
    stack.Data = ptr;
    stack.Type = EStackType::Dealloc;

    MutexGuard locker( &m_allocatedMtx );

    beginUntracked();
#if defined( _DEBUG )
    stack.fetch();
#endif  // defined(_DEBUG)

    m_stacksToBeDecoded->addToBack( stack );
    endUntracked();
}

void Memutil::mainLoop()
{
    MU_MEASURE_SCOPE;

    static std::atomic<std::uint8_t> threadumber{ 0u };
    constexpr std::size_t bufferSize{ 64 };
    char threadName[bufferSize];
    snprintf( threadName, bufferSize, "DecodeLoop %d", threadumber.load() );
    ++threadumber;

    MU_SET_THREAD_NAME( threadName );

    while( m_runMainLoop )
    {
        CStack currentTrace;

        MU_MEASURE_SUBSCOPE( DecodeList, "DecodeList", true );

        g_blockCurrentThread = true;
        std::optional<CStack> currentValue = m_stacksToBeDecoded->getAndPopFront();
        g_blockCurrentThread = false;

        {
            if( currentValue.has_value() )
            {
                currentTrace = currentValue.value();
            }
            else
            {
                continue;
            }
        }

        if( currentTrace.Type == EStackType::Alloc )
        {
            MU_MEASURE_SUBSCOPE( MemutilmainLoop_01, "decode", true );
            currentTrace.decode();
            MutexGuard locker( &m_allocatedMtx );
            MU_MEASURE_SUBSCOPE( MemutilmainLoop_02, "lock", true );
            g_blockCurrentThread = true;
            ( *m_allocated )[currentTrace.Data] = currentTrace;
            const auto eeh = ( *m_allocated )[currentTrace.Data];
            g_blockCurrentThread = false;
        }
        else if( currentTrace.Type == EStackType::Dealloc )
        {
            MutexGuard locker( &m_allocatedMtx );
#if defined( _DEBUG ) || defined( MU_WINDOWS )
            const auto it = m_allocated->find( currentTrace.Data );
            assert( it != m_allocated->end() );
#endif  // #if defined(_DEBUG)

            g_blockCurrentThread = true;
            m_allocated->erase( currentTrace.Data );
            g_blockCurrentThread = false;
        }
    }
}

void Memutil::dumpActiveAllocationsToOutput()
{
    MU_MEASURE_SCOPE;
    MutexGuard locker( &m_allocatedMtx );

    const bool lastTrackingStatus = getTrackingStatus();
    toggleTracking( false );
    dumpActiveAllocationsToOutput_impl();
    toggleTracking( lastTrackingStatus );
}

void Memutil::dumpActiveAllocationsToOutput_impl()
{
    MU_MEASURE_SCOPE;
    struct SameStackGroup
    {
        CStack Stack;
        std::uint64_t SameCount{ 0u };

        std::uint64_t getWholeSize() const
        {
            return SameCount * Stack.Size;
        }
    };

    std::vector<SameStackGroup> sortedGroup;
    for( const auto& [ptr, stack] : *m_allocated )
    {
        auto it = std::find_if( sortedGroup.begin(), sortedGroup.end(),
                                [&stack]( const SameStackGroup& current )
                                {
                                    return stack == current.Stack;
                                } );

        if( it != sortedGroup.end() )
        {
            ++it->SameCount;
        }
        else
        {
            SameStackGroup newGroup;
            newGroup.Stack = stack;
            newGroup.SameCount = 1u;
            sortedGroup.push_back( newGroup );
        }
    }

    std::sort( sortedGroup.begin(), sortedGroup.end(),
               []( const SameStackGroup& g1, const SameStackGroup& g2 )
               {
                   return g1.getWholeSize() > g2.getWholeSize();
               } );

    for( const SameStackGroup& group : sortedGroup )
    {
#if defined( _MSC_VER )
        printf( "Stack info:\nSize: %lld B (%lld B x %lld)\n", group.getWholeSize(), group.Stack.Size, group.SameCount );
#else   // #if defined(_MSC_VER)
        printf( "Stack info:\nSize: %ld B (%ld B x %ld)\n", group.getWholeSize(), group.Stack.Size, group.SameCount );
#endif  // #if defined(_MSC_VER)

        for( const auto& line : group.Stack.getStackLines() )
        {
            if( line.FileName.empty() == false )
            {
                printf( "%s : %d\n", line.FileName.c_str(), line.Number );
            }
        }
    }
}

bool Memutil::dumpActiveAllocationsToBuffer( char* outBuffer, std::size_t inBufferCapacity )
{
    MU_MEASURE_SCOPE;
    MutexGuard locker( &m_allocatedMtx );

    const bool lastTrackingStatus = getTrackingStatus();
    toggleTracking( false );
    const bool result = dumpActiveAllocationsToBuffer_impl( outBuffer, inBufferCapacity );
    toggleTracking( lastTrackingStatus );
    return result;
}

bool Memutil::dumpActiveAllocationsToBuffer_impl( char* outBuffer, std::size_t inBufferCapacity )
{
    MU_MEASURE_SCOPE;
    std::memset( outBuffer, 0, inBufferCapacity );

    std::int32_t firstEmptyChar{ 0u };
    std::int32_t bufferLeft{ static_cast<std::int32_t>( inBufferCapacity ) };
    std::int32_t currentWordSize{ 0u };

    struct SameStackGroup
    {
        CStack Stack;
        std::uint64_t SameCount{ 0u };

        std::uint64_t getWholeSize() const
        {
            return SameCount * Stack.Size;
        }
    };

    std::vector<SameStackGroup> sortedGroup;
    for( const auto& [ptr, stack] : *m_allocated )
    {
        auto it = std::find_if( sortedGroup.begin(), sortedGroup.end(),
                                [&stack]( const SameStackGroup& current )
                                {
                                    return stack == current.Stack;
                                } );

        if( it != sortedGroup.end() )
        {
            ++it->SameCount;
        }
        else
        {
            SameStackGroup newGroup;
            newGroup.Stack = stack;
            newGroup.SameCount = 1u;
            sortedGroup.push_back( newGroup );
        }
    }

    std::sort( sortedGroup.begin(), sortedGroup.end(),
               []( const SameStackGroup& g1, const SameStackGroup& g2 )
               {
                   return g1.getWholeSize() > g2.getWholeSize();
               } );

    for( const SameStackGroup& group : sortedGroup )
    {
#if defined( _MSC_VER )
        currentWordSize = snprintf( outBuffer, static_cast<std::size_t>( bufferLeft ), "Stack info:\nSize: %lld B (%lld B x %lld)\n",
                                    group.getWholeSize(), group.Stack.Size, group.SameCount );
#else   // #if defined( _MSC_VER )
        currentWordSize = snprintf( outBuffer, static_cast<std::size_t>( bufferLeft ), "Stack info:\nSize: %ld B (%ld B x %ld)\n",
                                    group.getWholeSize(), group.Stack.Size, group.SameCount );
#endif  // #if defined( _MSC_VER )

        if( currentWordSize < 1 )
        {
            return false;
        }

        firstEmptyChar += currentWordSize;
        bufferLeft -= currentWordSize;
        outBuffer += currentWordSize;

        std::size_t num{ 0u };
        for( const auto& line : group.Stack.getStackLines() )
        {
            if( bufferLeft <= 0 )
            {
                assert( false );
                return false;
            }

            if( line.FileName.empty() )
            {
                ++num;
                continue;
            }

            currentWordSize = snprintf( outBuffer, static_cast<std::size_t>( bufferLeft ), "%s:%d\n", line.FileName.c_str(), line.Number );

            if( currentWordSize < 1 )
            {
                assert( false );
                return false;
            }

            firstEmptyChar += currentWordSize;
            outBuffer += currentWordSize;
            bufferLeft -= currentWordSize;
            ++num;
        }
    }

    return true;
}

thread_local bool g_oldTrackingValue;

void Memutil::beginUntracked()
{
    g_oldTrackingValue = g_blockCurrentThread;
    g_blockCurrentThread = true;
}

void Memutil::endUntracked()
{
    g_blockCurrentThread = g_oldTrackingValue;
}

void Memutil::toggleTracking( bool inToggleTracking )
{
    MU_MEASURE_SCOPE;
    if( inToggleTracking == true )
    {
        if( m_runMainLoop == false )
        {
            init();
        }
    }

    m_enableTracking = inToggleTracking;
}

bool Memutil::getTrackingStatus() const
{
    return m_enableTracking;
}

bool Memutil::waitForAllCallStacksToBeDecoded() const
{
    MU_MEASURE_SCOPE;
    bool decodedIsEmpty{ false };
    while( ( decodedIsEmpty == false ) || ( g_isDecodingThread == true ) )
    {
        decodedIsEmpty = m_stacksToBeDecoded->isEmpty();
    }

    return true;
}

void Memutil::waitForAllAllocationsToBeResolved() const
{
    while( true )
    {
        if( getActiveAllocations() < 1 )
        {
            return;
        }
    }
}
std::int32_t Memutil::getActiveAllocations() const
{
    MU_MEASURE_SCOPE;
    MutexGuard locker( &m_allocatedMtx );
    return static_cast<std::int32_t>( m_allocated->size() );
}

Memutil::~Memutil()
{
    MU_MEASURE_SCOPE;
    m_runMainLoop = false;

    for( std::thread* currentThread : m_decodeThreads )
    {
        if( currentThread->joinable() )
        {
            currentThread->join();
        }
        delete currentThread;
    }
    std::cout << "Ending !\n";
}

}  // namespace MU

#if defined( DEBUG_THIS_FILE )
    #if defined( CUL_COMPILER_MSVC )
        #pragma optimize( "", on )
    #elif defined( CUL_COMPILER_CLANG )
        #pragma clang optimize on
    #elif defined( CUL_COMPILER_GCC )
        #pragma GCC pop_options
    #endif
#endif  // #if defined(DEBUG_THIS_FILE)