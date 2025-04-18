#include <MemUtil/StackWin64.hpp>

#if defined( MU_WINDOWS )
#include <MemUtil/Import_windows.hpp>
#include <MemUtil/Import_tracy.hpp>

___mu_t_RtlWalkFrameChain ___mu_RtlWalkFrameChain = 0;


namespace MU
{
class DebugWrapper
{
public:
    static DebugWrapper& getInstance()
    {
        static DebugWrapper instance;
        return instance;
    }

    IDebugSymbols* get()
    {
        return m_debugSymbols;
    }

protected:
private:
    DebugWrapper()
    {
        ___mu_RtlWalkFrameChain = (___mu_t_RtlWalkFrameChain)GetProcAddress( GetModuleHandleA( "ntdll.dll" ), "RtlWalkFrameChain" );

        void* debugClientMem{ nullptr };
        HRESULT cmdResult = ::DebugCreate( __uuidof( IDebugClient ), &debugClientMem );
        assert( cmdResult == S_OK );
        m_debugClient = reinterpret_cast<IDebugClient*>( debugClientMem );

        void* debugControlMem{ nullptr };
        cmdResult = m_debugClient->QueryInterface( __uuidof( IDebugControl ), &debugControlMem );
        assert( cmdResult == S_OK );
        m_debugControl = reinterpret_cast<IDebugControl*>( debugControlMem );

        cmdResult =
            m_debugClient->AttachProcess( 0, ::GetCurrentProcessId(), DEBUG_ATTACH_NONINVASIVE | DEBUG_ATTACH_NONINVASIVE_NO_SUSPEND );
        assert( cmdResult == S_OK );

        cmdResult = m_debugControl->WaitForEvent( DEBUG_WAIT_DEFAULT, INFINITE );
        assert( cmdResult == S_OK );

        void* debugSymbolsMem{ nullptr };
        cmdResult = m_debugClient->QueryInterface( __uuidof( IDebugSymbols ), &debugSymbolsMem );
        assert( cmdResult == S_OK );
        m_debugSymbols = reinterpret_cast<IDebugSymbols*>( debugSymbolsMem );
    }

    ~DebugWrapper()
    {
    }
    IDebugClient* m_debugClient{ nullptr };
    IDebugControl* m_debugControl{ nullptr };
    IDebugSymbols* m_debugSymbols{ nullptr };
};

 CStackWin64::CStackWin64():
    CIStack()
{
}

 CStackWin64::CStackWin64( const CStackWin64& arg ):
    CIStack( arg ),
    m_stackFrames( arg.m_stackFrames ),
    m_data( arg.m_data )
{
}

 CStackWin64::CStackWin64( CStackWin64&& arg ):
    CIStack( arg ),
    m_stackFrames( arg.m_stackFrames ),
    m_data( arg.m_data )
{
}

 std::atomic<std::uint64_t> g_counter;

CStackWin64& CStackWin64::operator=( const CStackWin64& arg )
{
    ++g_counter;
    if( this != &arg )
    {
        CIStack::operator=( arg );
        for( std::size_t i = 0u; i < G_MaxStackSize; ++i )
        {
            m_stackFrames[i] = arg.m_stackFrames[i];
        }

        m_data = arg.m_data;
    }
    return *this;
}

CStackWin64& CStackWin64::operator=( CStackWin64&& arg )
{
    if( this != &arg )
    {
        CIStack::operator=( arg );
        m_stackFrames = arg.m_stackFrames;
        m_data = arg.m_data;
    }
    return *this;
}

void CStackWin64::fetch()
{
    MU_MEASURE_SCOPE;
    std::size_t skip{ 3u };
    PULONG hash{ nullptr };
    DebugWrapper::getInstance();
    //const std::size_t result =
    //RtlWalkFrameChain();
    //     auto trace = (uintptr_t*)tracy_malloc( ( 1 + depth ) * sizeof( uintptr_t ) );
    const auto num = ___mu_RtlWalkFrameChain( (void**)( m_data.data() ), G_MaxStackSize, 0 );
    //static_cast<std::size_t>( RtlCaptureStackBackTrace( static_cast<DWORD>( skip ), G_MaxStackSize, m_data.data(), hash ) );
}

void CStackWin64::decode()
{
    MU_MEASURE_SCOPE;
    char name[256];
    ULONG size{ 0u };
    ULONG lineNum{ 0u };

    static IDebugSymbols* idebug = DebugWrapper::getInstance().get();
    for( std::size_t i = 0u; i < G_MaxStackSize; ++i )
    {
        void* currentAdd = m_data[i];
        const SLineInfo* fromCache = CIStack::getFromCache( currentAdd );
        if( fromCache )
        {
            m_stackFrames[i] = *fromCache;
            continue;
        }

        const ULONG64 offset = reinterpret_cast<ULONG64>( currentAdd );

        name[0] = '\0';
        if( idebug->GetLineByOffset( offset, &lineNum, name, sizeof( name ), &size, nullptr ) != S_OK )
        {
            continue;
        }

        SLineInfo& currentLine = m_stackFrames[i];
        currentLine.Value = name;
        currentLine.Number = static_cast<std::uint16_t>( lineNum );
        CIStack::addToCache( currentAdd, &currentLine );
    }
}

bool CStackWin64::operator==( const CStackWin64& arg ) const
{
    return m_stackFrames == arg.m_stackFrames;
}

const StackContents& CStackWin64::getStackLines() const
{
    return m_stackFrames;
}

CStackWin64::~CStackWin64()
{
}

}  // namespace MU
#endif // #if defined( MU_WINDOWS )
