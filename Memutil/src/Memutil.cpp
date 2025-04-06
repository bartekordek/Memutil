#include <MemUtil/Memutil.hpp>

#include <MemUtil/STL_Imports/STD_iostream.hpp>
#include <MemUtil/STL_Imports/STD_cstdio.hpp>
#include <MemUtil/Generic/ScopeExit.hpp>
#include <MemUtil/Generic/IMutex.hpp>
#include <MemUtil/Import_tracy.hpp>

namespace MU
{

thread_local bool g_blockCurrentThread{ false };
thread_local bool g_isDecodingThread{ false };
bool g_isDecoding{ false };


AllocationInfo::AllocationInfo()
{
}

AllocationInfo::AllocationInfo( AllocationInfo&& arg ) noexcept:
    Size( arg.Size ),
    Ptr( arg.Ptr ),
    StackString( arg.StackString ),
    Trace( arg.Trace ),
    Type( arg.Type )
{
    arg.Size = 0u;
    //arg.Trace = nullptr;
    arg.Ptr = nullptr;
    arg.Type = EStackType::None;
}

AllocationInfo& AllocationInfo::operator=( AllocationInfo&& arg ) noexcept
{
    if( this != &arg )
    {
        Type = arg.Type;

        //Trace = arg.Trace;
        //arg.Trace = nullptr;

        Ptr = arg.Ptr;
        arg.Ptr = nullptr;

        StackString = arg.StackString;

        Size = arg.Size;
        arg.Size = 0u;

        arg.Type = EStackType::None;
    }
    return *this;
}

AllocationInfo::~AllocationInfo()
{
    //Memutil::getInstance().releaseCallstack( Trace );
    //Trace = nullptr;
}

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

    m_allocationsMtx.reset( IMutex::createDefaultMtx() );
    m_toBeDecodedListMtx.reset( IMutex::createDefaultMtx() );
    m_usedStacksMtx.reset( IMutex::createDefaultMtx() );
    m_unusedStacksMtx.reset( IMutex::createDefaultMtx() );

    g_blockCurrentThread = true;
    for( std::size_t i = 0u; i < 800000u; ++i )
    {
        m_unusedStacks->insert( new boost::stacktrace::stacktrace() );
        m_unusedTrace->insert( new AllocationInfo() );
    }
    m_unusedTraceMtx.reset( IMutex::createDefaultMtx() );
    g_blockCurrentThread = false;

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
    AllocationInfo* si = fetchAllocationInfo();
    si->Ptr = ptr;
    si->Type = EStackType::Allocation;
    si->Size = inSize;
    g_blockCurrentThread = true;
    //si->Trace = createStackTrace();
    si->Trace = boost::stacktrace::stacktrace();
    g_blockCurrentThread = false;

    ZoneNamedN( Memutil_registerStack, "Memutil_registerStack_deque_emplace", true );
    MutexGuard locker( *m_toBeDecodedListMtx );
    m_toBeDecodedList->push_back( si );
}

AllocationInfo* Memutil::fetchAllocationInfo()
{
    ZoneScoped;
    AllocationInfo* result{ nullptr };

    {
        ZoneNamedN( Memutil_registerStack, "Memutil_fetchAllocationInfo_fetch", true );
        // Find already created.
        MutexGuard locker( *m_toBeDecodedListMtx );
        if( m_unusedTrace->empty() == false )
        {
            result = *m_unusedTrace->begin();
            m_unusedTrace->erase( m_unusedTrace->begin() );
            return result;
        }
    }
    {
        ZoneNamedN( Memutil_registerStack, "Memutil_fetchAllocationInfo_create", true );
        // Create new one.
        g_blockCurrentThread = true;
        result = new AllocationInfo();
        g_blockCurrentThread = false;
        return result;
    }
}

void Memutil::unregisterStack( void* ptr, std::uint64_t inSize )
{
    ZoneScoped;
    AllocationInfo* si = fetchAllocationInfo();
    si->Ptr = ptr;
    si->Type = EStackType::Deallocation;
    si->Size = inSize;

    ZoneNamedN( Memutil_registerStack, "Memutil_unregisterStack_deque_emplace", true );
    MutexGuard locker( *m_toBeDecodedListMtx );
    m_toBeDecodedList->push_back( si );
}

boost::stacktrace::stacktrace* Memutil::createStackTrace()
{
    ZoneScoped;
    boost::stacktrace::stacktrace* result{ nullptr };

    {
        ZoneNamedN( Memutil_registerStack, "Memutil_createStackTrace_fetch", true );
        // Try to get from unused.
        MutexGuard guard( *m_unusedStacksMtx );
        if( m_unusedStacks->empty() == false )
        {
            result = *m_unusedStacks->begin();
            m_unusedStacks->erase( m_unusedStacks->begin() );
            g_blockCurrentThread = true;
            *result = boost::stacktrace::stacktrace();
            g_blockCurrentThread = false;
            return result;
        }
    }

    {
        ZoneNamedN( Memutil_registerStack, "Memutil_createStackTrace_new", true );
        // Create new.
        g_blockCurrentThread = true;
        result = new boost::stacktrace::stacktrace();
        g_blockCurrentThread = false;

        MutexGuard guard( *m_usedStacksMtx );
        m_usedStacks->insert( result );
        return result;
    }
}

void Memutil::mainLoop()
{
    AllocationInfo* currentTrace{ nullptr };
    while( m_runMainLoop )
    {
        {
            MutexGuard locker( *m_toBeDecodedListMtx );
            if( m_toBeDecodedList->empty() == false )
            {
                g_blockCurrentThread = true;
                currentTrace = m_toBeDecodedList->back();
                m_toBeDecodedList->pop_back();
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

void Memutil::releaseCallstack( boost::stacktrace::stacktrace* inCallstack )
{
    if( inCallstack == nullptr )
    {
        return;
    }

    {
        MutexGuard guard( *m_usedStacksMtx );
        const auto it = m_usedStacks->find( inCallstack );
        m_usedStacks->erase( it );
    }
    {
        MutexGuard guard( *m_unusedStacksMtx );
        m_unusedStacks->insert( inCallstack );
    }
}

void Memutil::decode( AllocationInfo* stackInfo )
{
    g_isDecodingThread = true;

    if( stackInfo->Type == EStackType::Allocation )
    {
        convertBoostToAllocationInfo( stackInfo );

        MutexGuard locker( *m_allocationsMtx );
        m_allocations->insert( stackInfo );
    }
    else if( stackInfo->Type == EStackType::Deallocation )
    {
        MutexGuard locker( *m_allocationsMtx );

        const auto it = std::find_if( m_allocations->begin(), m_allocations->end(),
                                [&stackInfo]( AllocationInfo* curr )
                                {
                                    return curr->Ptr == stackInfo->Ptr;
                                } );
        if( it != m_allocations->end() )
        {
            m_allocations->erase( it );
        }
    }
    g_isDecodingThread = false;
}

void Memutil::convertBoostToAllocationInfo( AllocationInfo* inOut )
{
    inOut->StackString.clear();

    const auto& stVec = inOut->Trace.as_vector();
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
            inOut->Size = outputStackSize;
            return;
        }

        std::string sourceFile = currentTraceLine.source_file();
        if( sourceFile.empty() )
        {
            sourceFile = "unkown";
        }

        if( ( sourceFile.find( "stacktrace.hpp" ) != std::string::npos ) || ( sourceFile.find( "Memutil.cpp" ) != std::string::npos ) )
        {
            continue;
        }

        inOut->StackString.appendFrom( "%s:%d\n", sourceFile.c_str(), (int)currentTraceLine.source_line() );


        ++outputStackSize;
        if( outputStackSize == inOut->Size )
        {
            break;
        }
    }
    return;
}

void Memutil::dumpActiveAllocationsToOutput() const
{
    MutexGuard locker( *m_allocationsMtx );
//    for( AllocationInfo* stackInfo : *m_allocations )
//    {
//        const std::uint64_t sum = stackInfo->Size * stackInfo->Ptrs.size();
//#if defined( _MSC_VER )
//        printf( "Stack info:\nSize: %lldB ( %lldB x %zd )\n", sum, stackInfo->Size, stackInfo->Ptrs.size() );
//#else   // #if defined(_MSC_VER)
//        printf( "Stack info:\nSize: %ldB ( %ldB x %ld )\n", sum, stackInfo->Size, stackInfo->Ptrs.size() );
//#endif  // #if defined(_MSC_VER)
//
//        for( const auto& line : stackInfo->StackLines )
//        {
//            if( line.empty() == false )
//            {
//                printf( "%s\n", line.c_str() );
//            }
//        }
//    }
}

bool Memutil::dumpActiveAllocationsToBuffer( char* outBuffer, std::size_t inBufferCapacity ) const
{
    std::memset( outBuffer, 0, inBufferCapacity );

    std::int32_t firstEmptyChar{ 0u };
    std::int32_t bufferLeft{ static_cast<std::int32_t>( inBufferCapacity ) };
    std::int32_t currentWordSize{ 0u };

    MutexGuard locker( *m_allocationsMtx );
//    for( AllocationInfo* stackInfo : (*m_allocations) )
//    {
//        const std::uint64_t sum = stackInfo->Size * stackInfo->Ptrs.size();
//#if defined( _MSC_VER )
//        currentWordSize = snprintf( outBuffer, static_cast<std::size_t>( bufferLeft ), "Stack info:\nSize: %zd B ( %lld B x %zd )\n", sum,
//                                    stackInfo->Size, stackInfo->Ptrs.size() );
//#else   // #if defined( _MSC_VER )
//        currentWordSize = snprintf( outBuffer, static_cast<std::size_t>( bufferLeft ), "Stack info:\nSize: %ld B ( %ld B x %ld )\n", sum,
//                                    stackInfo->Size, stackInfo->Ptrs.size() );
//#endif  // #if defined( _MSC_VER )
//
//        if( currentWordSize < 1 )
//        {
//            return false;
//        }
//
//        firstEmptyChar += currentWordSize;
//        bufferLeft -= currentWordSize;
//        outBuffer += currentWordSize;
//
//        for( const auto& line : stackInfo->StackLines )
//        {
//            if( line.empty() )
//            {
//                continue;
//            }
//
//            currentWordSize = snprintf( outBuffer, static_cast<std::size_t>( bufferLeft ), "%s\n", line.c_str() );
//
//            if( currentWordSize < 1 )
//            {
//                return false;
//            }
//
//            firstEmptyChar += currentWordSize;
//            outBuffer += currentWordSize;
//            bufferLeft -= currentWordSize;
//        }
//    }
    return true;
}

bool Memutil::waitForAllCallStacksToBeDecoded() const
{
    bool dequeIsEmpty{ false };
    while( ( dequeIsEmpty == false ) || ( g_isDecodingThread == true ) || ( g_isDecoding == true ) )
    {
        MutexGuard locker( *m_toBeDecodedListMtx );
        dequeIsEmpty = m_toBeDecodedList->empty();
    }

    return true;
}

std::int32_t Memutil::getActiveAllocations() const
{
    MutexGuard locker( *m_allocationsMtx );
    return static_cast<std::int32_t>( m_allocations->size() );
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