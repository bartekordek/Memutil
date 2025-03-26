#include <MemUtil/Memutil.hpp>
#include <MemUtil/STL_Imports/STD_deque.hpp>
#include <MemUtil/STL_Imports/STD_iostream.hpp>
#include <MemUtil/STL_Imports/STD_cstdio.hpp>
#include <MemUtil/Generic/ScopeExit.hpp>
#include <Generic/Import_boost_stacktrace.hpp>

namespace MU
{

void convertBoostToAllocationInfo( AllocationInfo& out, const boost::stacktrace::stacktrace& stackTrace );
	
thread_local bool g_blockCurrentThread = false;

struct StackInfo final
{
    bool Register{ true };
    std::unique_ptr<boost::stacktrace::stacktrace> Trace;
    std::size_t Size{ 0u };
    void* Data{ nullptr };
    std::size_t SkipFirstLinesCount{ 0u };

    StackInfo() = default;

    StackInfo( bool inRegister, std::size_t inSize, void* inPtr, std::size_t inSkipFirstLinesCount ):
        Register( inRegister ),
        Trace( std::make_unique<boost::stacktrace::stacktrace>() ),
        Size( inSize ),
        Data( inPtr ),
        SkipFirstLinesCount( inSkipFirstLinesCount )
    {
    }

    StackInfo( const StackInfo& rhv ) = delete;
    StackInfo( StackInfo&& arg ) noexcept:
        Register( arg.Register ),
        Trace( std::move( arg.Trace ) ),
        Size( arg.Size ),
        Data( arg.Data ),
        SkipFirstLinesCount( arg.SkipFirstLinesCount )
    {
        arg.Size = 0u;
        arg.Data = nullptr;
        arg.SkipFirstLinesCount = 0u;
    }

    StackInfo& operator=( const StackInfo& arg ) = delete;
    StackInfo& operator=( StackInfo&& arg ) noexcept
    {
        if( this != &arg )
        {
            Register = arg.Register;
            Trace = std::move( arg.Trace );
            Size = arg.Size;
            Data = arg.Data;
            SkipFirstLinesCount = arg.SkipFirstLinesCount;

            arg.Size = 0u;
            arg.Data = nullptr;
        }
        return *this;
    }

    ~StackInfo()
    {

    }
};

namespace Deque
{
}  // namespace Deque

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
    if( m_initialized )
    {
        return;
    }
    m_mainLoopThread = std::thread( &Memutil::mainLoop, this );
    m_initialized = true;
}

void Memutil::logRealloc( void* inOldPtr, void* inNewPtr, std::uint64_t inSize, std::size_t skipFirstLinesCount )
{
    if( ( g_blockCurrentThread == true ) || ( m_enableTracking == false ) )
    {
        return;
    }
    logFree( inOldPtr );
    logAlloc( inNewPtr, inSize, skipFirstLinesCount );
}

void Memutil::logAlloc( void* inPtr, std::uint64_t inSize, std::size_t skipFirstLinesCount )
{
    if( ( g_blockCurrentThread == true ) || ( m_enableTracking == false ) )
    {
        return;
    }

    registerStack( inPtr, inSize, skipFirstLinesCount );
}

void Memutil::logFree( void* inPtr )
{
    if( ( g_blockCurrentThread == true ) || ( m_enableTracking == false ) )
    {
        return;
    }

    unregisterStack( inPtr, 0u, 0u );
}

void Memutil::toggleTracking( bool inToggleTracking )
{
    m_enableTracking = inToggleTracking;
}

void Memutil::registerStack( void* ptr, std::uint64_t inSize, std::size_t /*skipFirstLinesCount*/ )
{
    if( inSize == 0u )
    {
        return;
    }

    g_blockCurrentThread = true;
    ScopeExit se( [] (){
            g_blockCurrentThread = false;
    });

    boost::stacktrace::stacktrace currentStackTrace;

    if( currentStackTrace.empty() )
    {
        return;
    }

    AllocationInfo ai;
    convertBoostToAllocationInfo( ai, currentStackTrace );
    ai.Size = inSize;
    std::lock_guard<std::mutex> locker( m_dataMtx );
    m_allocations[ptr] = ai;
}

void Memutil::unregisterStack( void* ptr, std::uint64_t /*inSize*/, std::size_t /*skipFirstLinesCount*/ )
{
    std::lock_guard<std::mutex> locker( m_dataMtx );
    const auto it = m_allocations.find( ptr );
    if( it != m_allocations.end() )
    {
        m_allocations.erase( it );
    }
}

void Memutil::mainLoop()
{

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

        if( sourceFile.find( "stacktrace.hpp" ) != std::string::npos )
        {
            continue;
        }

        if( sourceFile.find( "Memutil.cpp" ) != std::string::npos )
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
    out.Size = outputStackSize;
}

void Memutil::decode( const StackInfo& stackInfo )
{
    if( stackInfo.Register )
    {
        AllocationInfo ai;
        convertBoostToAllocationInfo( ai, *stackInfo.Trace );
        ai.Size = stackInfo.Size;
        std::lock_guard<std::mutex> locker( m_dataMtx );
        g_blockCurrentThread = true;
        m_allocations[stackInfo.Data] = ai;
        g_blockCurrentThread = false;
    }
    else
    {
        std::lock_guard<std::mutex> locker( m_dataMtx );
        const auto it = m_allocations.find( stackInfo.Data );
        if( it != m_allocations.end() )
        {
            m_allocations.erase( it );
        }
    }
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

void Memutil::dumpActiveAllocations() const
{
    std::lock_guard<std::mutex> locker( m_dataMtx );
    for( const auto& [addr, stackInfo] : m_allocations )
    {
        printf( "Stack info:\nsize:%lld bytes\n", stackInfo.Size );

        for( const auto& line : stackInfo.StackLines )
        {
            printf( "%s\n", line.c_str() );
        }
    }
}

bool Memutil::waitForAllCallStacksToBeDecoded() const
{
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

}