#include <MemUtil/Memutil.hpp>

#include <MemUtil/STL_Imports/STD_iostream.hpp>
#include <MemUtil/STL_Imports/STD_cstdio.hpp>
#include <MemUtil/Generic/ScopeExit.hpp>
#include <MemUtil/Generic/IMutex.hpp>
#include <MemUtil/Stack.hpp>
#include <MemUtil/Import_tracy.hpp>
#include <MemUtil/STL_Imports/STD_algorithm.hpp>

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

void Memutil::toggleTracking( bool inToggleTracking )
{
    ZoneScoped;
    if( inToggleTracking == true )
    {
        if( m_runMainLoop == false )
        {
            init();
        }
    }

    m_enableTracking = inToggleTracking;
}
void Memutil::init()
{
    ZoneScoped;
    if( m_initialized )
    {
        return;
    }

    m_toBeDecodedMtx.reset( IMutex::createDefaultMtx() );
    m_allocatedMtx.reset( IMutex::createDefaultMtx() );

    m_runMainLoop = true;
    m_mainLoopThread = std::thread( &Memutil::mainLoop, this );
    m_initialized = true;
}

void Memutil::logRealloc( void* inOldPtr, void* inNewPtr, std::uint64_t inSize )
{
    ZoneScoped;
    if( ( g_blockCurrentThread == true ) || ( m_enableTracking == false ) || ( g_isDecodingThread == true ) )
    {
        return;
    }
    logFree( inOldPtr );
    logAlloc( inNewPtr, inSize );
}

void Memutil::logAlloc( void* inPtr, std::uint64_t inSize )
{
    ZoneScoped;
    if( ( g_blockCurrentThread == true ) || ( m_enableTracking == false ) || ( g_isDecodingThread == true ) )
    {
        return;
    }

    registerStack( inPtr, inSize );
}

void Memutil::logFree( void* inPtr )
{
    ZoneScoped;
    if( ( g_blockCurrentThread == true ) || ( m_enableTracking == false ) || ( g_isDecodingThread == true ) )
    {
        return;
    }

    unregisterStack( inPtr, 0u );
}

void Memutil::registerStack( void* ptr, std::uint64_t inSize )
{
    ZoneScoped;

    CStack stack;
    stack.fetch();
    stack.Size = inSize;
    stack.Data = ptr;

    ZoneNamedN( Memutil_registerStack, "Memutil_registerStack_deque_emplace", true );
    MutexGuard locker( *m_toBeDecodedMtx );
    m_toBeDecoded->push_back( stack );
}

void Memutil::unregisterStack( void* ptr, std::uint64_t /*inSize*/ )
{
    ZoneNamedN( Memutil_registerStack, "Memutil_unregisterStack_deque_emplace", true );
    MutexGuard locker( *m_allocatedMtx );
    m_allocated->erase( ptr );
}

void Memutil::mainLoop()
{
    CStack currentTrace;
    while( m_runMainLoop )
    {
        {
            MutexGuard locker( *m_toBeDecodedMtx );
            if( m_toBeDecoded->empty() == false )
            {
                g_blockCurrentThread = true;
                currentTrace = m_toBeDecoded->front();
                m_toBeDecoded->pop_front();
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
            currentTrace.decode();

            MutexGuard locker( *m_allocatedMtx );
            m_allocated->insert( { currentTrace.Data, currentTrace } );
            g_isDecoding = false;
        }
    }
}

void Memutil::dumpActiveAllocationsToOutput() const
{
    MutexGuard locker( *m_allocatedMtx );

    std::vector<decltype( CStack::Size )> sizes;
    for( const auto& [ptr, stack] : *m_allocated )
    {
        sizes.push_back( stack.Size );
    }

    std::sort( sizes.begin(), sizes.end(), std::greater<>() );

    std::vector<decltype( CStack::Size )> sizesUnique;

    struct SameStackGroup
    {
        std::array<SLineInfo, G_MaxStackSize>
    };

    struct SameSizeGroup
    {
        std::vector<
    };

    std::unordered_map < decltype( CStack::Size ), >

    decltype( CStack::Size ) prev = 0;
    for( const decltype( CStack::Size ) value : sizes )
    {
        if( value == prev )
        {
            continue;
        }

        sizesUnique.push_back( value );
        prev = value;
    }

//    for( const auto& [ptr, stack] : *m_allocated )
//    {
//#if defined( _MSC_VER )
//        printf( "Stack info:\nSize: %lld B\n", stack.Size );
//#else   // #if defined(_MSC_VER)
//        printf( "Stack info:\nSize: %lld B\n", stack.Size );
//#endif  // #if defined(_MSC_VER)
//
//        for( const auto& line : stack.getStackLines() )
//        {
//            if( line.Value.empty() == false )
//            {
//                printf( "%s : %d\n", line.Value.c_str(), line.Number );
//            }
//        }
//    }
}

bool Memutil::dumpActiveAllocationsToBuffer( char* outBuffer, std::size_t inBufferCapacity ) const
{
    return true;

    std::memset( outBuffer, 0, inBufferCapacity );

    std::int32_t firstEmptyChar{ 0u };
    std::int32_t bufferLeft{ static_cast<std::int32_t>( inBufferCapacity ) };
    std::int32_t currentWordSize{ 0u };

    MutexGuard locker( *m_allocatedMtx );
    for( const auto& [ptr, stack] : *m_allocated )
    {
#if defined( _MSC_VER )
        currentWordSize = snprintf( outBuffer, static_cast<std::size_t>( bufferLeft ), "Stack info:\nSize: %zd B\n", stack.Size );
#else   // #if defined( _MSC_VER )
        currentWordSize = snprintf( outBuffer, static_cast<std::size_t>( bufferLeft ), "Stack info:\nSize: %ld B\n", stack.Size );
#endif  // #if defined( _MSC_VER )

        if( currentWordSize < 1 )
        {
            return false;
        }

        firstEmptyChar += currentWordSize;
        bufferLeft -= currentWordSize;
        outBuffer += currentWordSize;

        std::size_t num{ 0u };
        for( const auto& line : stack.getStackLines() )
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

bool Memutil::waitForAllCallStacksToBeDecoded() const
{
    bool decodedIsEmpty{ false };
    while( ( decodedIsEmpty == false ) || ( g_isDecodingThread == true ) || ( g_isDecoding == true ) )
    {
        MutexGuard locker( *m_toBeDecodedMtx );
        decodedIsEmpty = m_toBeDecoded->empty();
    }

    return true;
}

std::int32_t Memutil::getActiveAllocations() const
{
    MutexGuard locker( *m_allocatedMtx );
    return static_cast<std::int32_t>( m_allocated->size() );
}

Memutil::~Memutil()
{
    m_runMainLoop = false;
    if( m_mainLoopThread.joinable() )
    {
        m_mainLoopThread.join();
    }
}

}  // namespace MU