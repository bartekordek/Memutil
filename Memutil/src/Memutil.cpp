#include <MemUtil/Memutil.hpp>

#include <MemUtil/STL_Imports/STD_iostream.hpp>
#include <MemUtil/STL_Imports/STD_cstdio.hpp>
#include <MemUtil/Generic/ScopeExit.hpp>
#include <MemUtil/Generic/IMutex.hpp>
#include <MemUtil/Stack.hpp>
#include <MemUtil/Import_tracy.hpp>
#include <MemUtil/STL_Imports/STD_algorithm.hpp>
#include <MemUtil/STL_Imports/STD_cmath.hpp>

namespace MU
{

thread_local bool g_blockCurrentThread{ false };
thread_local bool g_isDecodingThread{ false };
bool g_isDecoding{ false };

Memutil& Memutil::getInstance()
{
    static Memutil instance;
    return instance;
}

Memutil::Memutil()
{
}

void Memutil::init()
{
    MU_MEASURE_SCOPE;
    if( m_initialized )
    {
        return;
    }

    m_toBeDecoded = new std::deque<CStack>();

    m_runMainLoop = true;

    for( std::size_t i = 0u; i < decodeWorkersCount; ++i )
    {
        m_decodeThreads[i] = new std::thread( &Memutil::mainLoop, this );
    }

    m_initialized = true;
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

    CStack stack;

    const bool oldTrackingValue2 = m_enableTracking;
    m_enableTracking = false;
    stack.fetch();
    m_enableTracking = oldTrackingValue2;
    stack.Size = inSize;
    stack.Data = ptr;
    stack.Type = EStackType::Alloc;

    MU_MEASURE_SUBSCOPE( Memutil_registerStack_l, "lock", true );
    MutexGuard locker( &m_toBeDecodedMtx );

    MU_MEASURE_SUBSCOPE( Memutil_registerStack_p, "push_back", true );

    const bool oldTrackingValue = m_enableTracking;
    m_enableTracking = false;
    m_toBeDecoded->emplace_back( stack );
    m_enableTracking = oldTrackingValue;
}

void Memutil::unregisterStack( void* ptr, std::uint64_t /*inSize*/ )
{
    MU_MEASURE_SCOPE;

    CStack stack;
    stack.Data = ptr;
    stack.Type = EStackType::Dealloc;

    MutexGuard locker( &m_allocatedMtx );
    MU_MEASURE_SUBSCOPE( Memutil_registerStack, "lock", true );
    const bool oldTrackingValue = m_enableTracking;
    m_enableTracking = false;
    m_toBeDecoded->emplace_back( stack );
    m_enableTracking = oldTrackingValue;
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
    CStack currentTrace;
    while( m_runMainLoop )
    {
        MU_MEASURE_SUBSCOPE( MemutilmainLoop_00, "fetch", true );
        {
            MutexGuard locker( &m_toBeDecodedMtx );
            MU_MEASURE_SUBSCOPE( MemutilmainLoop_01, "lock", true );
            if( m_toBeDecoded->empty() == false )
            {
                currentTrace = m_toBeDecoded->front();

                const bool oldTrackingValue = m_enableTracking;
                m_enableTracking = false;
                m_toBeDecoded->pop_front();
                m_enableTracking = oldTrackingValue;

                g_blockCurrentThread = false;
                g_isDecoding = true;
            }
            else
            {
                g_isDecoding = false;
            }
        }
        if( g_isDecoding )
        {
            if( currentTrace.Type == EStackType::Alloc )
            {
                MU_MEASURE_SUBSCOPE( MemutilmainLoop_01, "decode", true );
                currentTrace.decode();
                MutexGuard locker( &m_allocatedMtx );
                MU_MEASURE_SUBSCOPE( MemutilmainLoop_02, "lock", true );
                m_allocated->insert( { currentTrace.Data, currentTrace } );
            }
            else
            {
                {
                    MutexGuard locker( &m_allocatedMtx );
                    const auto it = m_allocated->find( currentTrace.Data );
                    if( it != m_allocated->end() )
                    {
                        m_allocated->erase( currentTrace.Data );
                        continue;
                    }
                }
                {
                    MutexGuard locker( &m_toBeDecodedMtx );
                    m_toBeDecoded->push_back( currentTrace );
                }
            }

            g_isDecoding = false;
        }
    }
    printf( "Ending loop.\n" );
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
            if( line.Value.empty() == false )
            {
                printf( "%s : %d\n", line.Value.c_str(), line.Number );
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

    const bool lastTrackingStatus = getTrackingStatus();

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
            if( line.Value.empty() )
            {
                ++num;
                continue;
            }

            currentWordSize = snprintf( outBuffer, static_cast<std::size_t>( bufferLeft ), "%s:%d\n", line.Value.c_str(), line.Number );

            if( currentWordSize < 1 )
            {
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

void Memutil::runWithoutRegister( std::function<void( void )> inFunction )
{
    const bool oldTrackingValue = m_enableTracking;
    m_enableTracking = false;
    inFunction();
    m_enableTracking = oldTrackingValue;
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
    while( ( decodedIsEmpty == false ) || ( g_isDecodingThread == true ) || ( g_isDecoding == true ) )
    {
        MutexGuard locker( &m_toBeDecodedMtx );
        decodedIsEmpty = m_toBeDecoded->empty();
    }

    return true;
}

std::int32_t Memutil::getActiveAllocations() const
{
    MU_MEASURE_SCOPE;
    MutexGuard locker( &m_allocatedMtx );
    return static_cast<std::int32_t>( m_allocated->size() );
}

Memutil::~Memutil()
{
    m_runMainLoop = false;

    for( std::thread* currentThread : m_decodeThreads )
    {
        if( currentThread->joinable() )
        {
            currentThread->join();
        }
        delete currentThread;
    }
}

}  // namespace MU