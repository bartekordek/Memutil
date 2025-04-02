#include <MemUtil/Memutil.hpp>
#include <MemUtil/STL_Imports/STD_deque.hpp>
#include <MemUtil/STL_Imports/STD_iostream.hpp>
#include <MemUtil/STL_Imports/STD_cstdio.hpp>
#include <MemUtil/Generic/ScopeExit.hpp>
#include <Generic/Import_boost_stacktrace.hpp>

namespace MU
{

void convertBoostToAllocationInfo( AllocationInfo& out, const boost::stacktrace::stacktrace& stackTrace );

thread_local bool g_blockCurrentThread{ false };
thread_local bool g_isDecodingThread{ false };
bool g_isDecoding{ false };

struct StackInfo final
{
    bool Register{ true };
    boost::stacktrace::stacktrace* Trace{ nullptr };
    std::size_t Size{ 0u };
    void* Data{ nullptr };
    std::size_t SkipFirstLinesCount{ 0u };

    StackInfo() = default;

    StackInfo( bool inRegister, std::size_t inSize, void* inPtr, std::size_t inSkipFirstLinesCount ):
        Register( inRegister ),
        Size( inSize ),
        Data( inPtr ),
        SkipFirstLinesCount( inSkipFirstLinesCount )
    {
    }

    StackInfo( const StackInfo& rhv ) = delete;
    StackInfo( StackInfo&& arg ) noexcept:
        Register( arg.Register ),
        Trace( arg.Trace ),
        Size( arg.Size ),
        Data( arg.Data ),
        SkipFirstLinesCount( arg.SkipFirstLinesCount )
    {
        arg.Size = 0u;
        arg.Data = nullptr;
        arg.SkipFirstLinesCount = 0u;
        arg.Trace = nullptr;
    }

    StackInfo& operator=( const StackInfo& arg ) = delete;
    StackInfo& operator=( StackInfo&& arg ) noexcept
    {
        if( this != &arg )
        {
            Register = arg.Register;
            std::swap( Trace, arg.Trace );
            Size = arg.Size;
            std::swap( Data, arg.Data );
            SkipFirstLinesCount = arg.SkipFirstLinesCount;

            arg.Size = 0u;
        }
        return *this;
    }

    ~StackInfo()
    {
        delete Trace;
        Trace = nullptr;
    }
};

namespace Deque
{
static constexpr std::uint64_t PoolSize = 8u * 1024u * 1024u;  // 16MB
std::array<std::byte, PoolSize> BufferBlocks;
std::pmr::monotonic_buffer_resource BufferSrc{ BufferBlocks.data(), PoolSize };

std::pmr::deque<StackInfo> g_traceDeque{ &BufferSrc };
}  // namespace Deque

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
    if( m_initialized )
    {
        return;
    }
    m_runMainLoop = true;
    m_mainLoopThread = std::thread( &Memutil::mainLoop, this );
    m_initialized = true;
}

void Memutil::logRealloc( void* inOldPtr, void* inNewPtr, std::uint64_t inSize, std::size_t skipFirstLinesCount )
{
    if( ( g_blockCurrentThread == true ) || ( m_enableTracking == false ) || ( g_isDecodingThread == true ) )
    {
        return;
    }
    logFree( inOldPtr );
    logAlloc( inNewPtr, inSize, skipFirstLinesCount );
}

void Memutil::logAlloc( void* inPtr, std::uint64_t inSize, std::size_t skipFirstLinesCount )
{
    if( ( g_blockCurrentThread == true ) || ( m_enableTracking == false ) || ( g_isDecodingThread == true ) )
    {
        return;
    }

    registerStack( inPtr, inSize, skipFirstLinesCount );
}

void Memutil::logFree( void* inPtr )
{
    if( ( g_blockCurrentThread == true ) || ( m_enableTracking == false ) || ( g_isDecodingThread == true ) )
    {
        return;
    }

    unregisterStack( inPtr, 0u, 0u );
}

void Memutil::registerStack( void* ptr, std::uint64_t inSize, std::size_t skipFirstLinesCount )
{
    StackInfo si;
    si.Data = ptr;
    si.Register = true;
    si.Size = inSize;
    si.SkipFirstLinesCount = skipFirstLinesCount;
    g_blockCurrentThread = true;
    si.Trace = new boost::stacktrace::stacktrace();
    g_blockCurrentThread = false;

    std::lock_guard<std::mutex> locker( g_traceDequeMtx );
    Deque::g_traceDeque.emplace_back( std::move( si ) );
}

void Memutil::unregisterStack( void* ptr, std::uint64_t inSize, std::size_t skipFirstLinesCount )
{
    StackInfo si;
    si.Data = ptr;
    si.Register = false;
    si.Size = inSize;
    si.SkipFirstLinesCount = skipFirstLinesCount;

    std::lock_guard<std::mutex> locker( g_traceDequeMtx );
    Deque::g_traceDeque.emplace_back( std::move( si ) );
}

void Memutil::mainLoop()
{
    StackInfo currentTrace;
    while( m_runMainLoop )
    {
        {
            std::lock_guard<std::mutex> locker( g_traceDequeMtx );
            if( Deque::g_traceDeque.empty() == false )
            {
                g_blockCurrentThread = true;
                currentTrace = std::move( Deque::g_traceDeque.back() );
                Deque::g_traceDeque.pop_back();
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
            decode( currentTrace );
            g_isDecoding = false;
        }
    }
}

void convertBoostToAllocationInfo( AllocationInfo& out, const boost::stacktrace::stacktrace& stackTrace )
{
    StackLinesArray& outStackLines = out.StackLines;

    const auto& stVec = stackTrace.as_vector();
    size_t stackTraceSize = stVec.size();
    std::size_t outputStackSize{ 0u };
    for( size_t i = 0; i < stackTraceSize; ++i )
    {
        const boost::stacktrace::frame& currentTraceLine = stVec[i];
        if( currentTraceLine.empty() == true )
        {
            continue;
        }

        if( outputStackSize >= G_maxStackSize )
        {
            out.Size = outputStackSize;
            return;
        }

        constexpr std::size_t bufferSize{ 1024 };
        char buffer[bufferSize];
        std::string sourceFile = currentTraceLine.source_file();
        if( sourceFile.empty() )
        {
            sourceFile = "unkown";
        }

        if( ( sourceFile.find( "stacktrace.hpp" ) != std::string::npos ) || ( sourceFile.find( "MemoryUtils." ) != std::string::npos ) )
        {
            continue;
        }

        snprintf( buffer, bufferSize, "%s:%d", sourceFile.c_str(), (int)currentTraceLine.source_line() );

        outStackLines[outputStackSize] = buffer;
        ++outputStackSize;
        if( outputStackSize == out.Size )
        {
            break;
        }
    }
}

void Memutil::decode( const StackInfo& stackInfo )
{
    g_isDecodingThread = true;

    if( stackInfo.Register )
    {
        AllocationInfo ai;
        convertBoostToAllocationInfo( ai, *stackInfo.Trace );
        ai.Size = stackInfo.Size;
        ai.Ptrs.insert( stackInfo.Data );
        std::lock_guard<std::mutex> locker( m_dataMtx );
        g_blockCurrentThread = true;
        const auto it = m_allocations.find( ai );
        if( it == m_allocations.end() )
        {
            m_allocations.insert( ai );
        }
        else
        {
            for( const auto ptr: it->Ptrs )
            {
                ai.Ptrs.insert( ptr );
            }

            m_allocations.erase( it );
            m_allocations.insert( ai );
        }

        g_blockCurrentThread = false;
    }
    else
    {
        std::lock_guard<std::mutex> locker( m_dataMtx );

        const auto it = std::find_if( m_allocations.begin(), m_allocations.end(),
                                      [&stackInfo]( const AllocationInfo& curr )
                                      {
                                          const auto ptrIt = curr.Ptrs.find( stackInfo.Data );
                                          return ptrIt != curr.Ptrs.end();
                                      } );
        if( it != m_allocations.end() )
        {
            AllocationInfo ai = *it;
            m_allocations.erase( it );
            const auto ptrIt = ai.Ptrs.find( stackInfo.Data );
            ai.Ptrs.erase( ptrIt );

            if( ai.Ptrs.empty() != true )
            {
                m_allocations.insert( ai );
            }
        }
    }
    g_isDecodingThread = false;
}

void Memutil::getStackHere( StackLinesArray& outStackLines, std::size_t skipFirstLinesCount )
{
    g_blockCurrentThread = true;
    ScopeExit se(
        []()
        {
            g_blockCurrentThread = false;
        } );

    boost::stacktrace::stacktrace stackTrace;
    boost::stacktrace::frame copy;
    const auto& stVec = stackTrace.as_vector();
    size_t stackTraceSize = stVec.size();
    std::size_t outputStackSize{ 0u };
    for( size_t i = 0; i < stackTraceSize; ++i )
    {
        const boost::stacktrace::frame& currentTraceLine = stVec[i];
        if( currentTraceLine.empty() == true )
        {
            continue;
        }

        if( outputStackSize >= G_maxStackSize )
        {
            return;
        }

        if( i < skipFirstLinesCount )
        {
            continue;
        }

        constexpr std::size_t bufferSize{ 1024u };
        char buffer[bufferSize];
        std::string sourceFile = currentTraceLine.source_file();
        if( sourceFile.empty() )
        {
            sourceFile = "unkown";
        }

        snprintf( buffer, bufferSize, "%s:%d", sourceFile.c_str(), (int)currentTraceLine.source_line() );

        outStackLines[outputStackSize] = buffer;
        ++outputStackSize;
    }
}

void Memutil::dumpActiveAllocationsToOutput() const
{
    std::lock_guard<std::mutex> locker( m_dataMtx );
    for( const AllocationInfo& stackInfo : m_allocations )
    {
        const std::uint64_t sum = stackInfo.Size * stackInfo.Ptrs.size();
#if defined( _MSC_VER )
        printf( "Stack info:\nSize: %lldB ( %lldB x %lld )\n", sum, stackInfo.Size, stackInfo.Ptrs.size() );
#else   // #if defined(_MSC_VER)
        printf( "Stack info:\nSize: %ldB ( %ldB x %ld )\n", sum, stackInfo.Size, stackInfo.Ptrs.size() );
#endif  // #if defined(_MSC_VER)

        for( const auto& line : stackInfo.StackLines )
        {
            printf( "%s\n", line.c_str() );
        }
    }
}

bool Memutil::dumpActiveAllocationsToBuffer( char* outBuffer, std::size_t inBufferCapacity ) const
{
    std::memset( outBuffer, 0, inBufferCapacity );

    std::int32_t firstEmptyChar{ 0u };
    std::int32_t bufferLeft{ static_cast<std::int32_t>( inBufferCapacity ) };
    std::int32_t currentWordSize{ 0u };

    std::lock_guard<std::mutex> locker( m_dataMtx );

    for( const AllocationInfo& stackInfo : m_allocations )
    {
        const std::uint64_t sum = stackInfo.Size * stackInfo.Ptrs.size();
#if defined( _MSC_VER )
        currentWordSize = snprintf( outBuffer, static_cast<std::size_t>( bufferLeft ), "Stack info:\nSize: %lldB ( %lldB x %lld )\n", sum,
                                    stackInfo.Size, stackInfo.Ptrs.size() );
#else   // #if defined( _MSC_VER )
        currentWordSize = snprintf( outBuffer, static_cast<std::size_t>( bufferLeft ), "Stack info:\nSize: %ldB ( %ldB x %ld )\n", sum,
                                    stackInfo.Size, stackInfo.Ptrs.size() );
#endif  // #if defined( _MSC_VER )

        if( currentWordSize < 1 )
        {
            return false;
        }

        firstEmptyChar += currentWordSize;
        bufferLeft -= currentWordSize;
        outBuffer += currentWordSize;

        for( const auto& line : stackInfo.StackLines )
        {
            currentWordSize = snprintf( outBuffer, static_cast<std::size_t>( bufferLeft ), "%s\n", line.c_str() );

            if( currentWordSize < 1 )
            {
                return false;
            }

            firstEmptyChar += currentWordSize;
            outBuffer += currentWordSize;
            bufferLeft -= currentWordSize;
        }
    }
    return true;
}

bool Memutil::waitForAllCallStacksToBeDecoded() const
{
    bool dequeIsEmpty{ false };
    while( ( dequeIsEmpty == false ) || ( g_isDecodingThread == true ) || ( g_isDecoding == true ) )
    {
        std::lock_guard<std::mutex> locker( g_traceDequeMtx );
        dequeIsEmpty = Deque::g_traceDeque.empty();
    }

    return true;
}

std::int32_t Memutil::getActiveAllocations() const
{
    std::lock_guard<std::mutex> locker( m_dataMtx );
    return static_cast<std::int32_t>( m_allocations.size() );
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