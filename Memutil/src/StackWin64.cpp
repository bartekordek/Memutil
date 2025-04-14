#include <MemUtil/StackWin64.hpp>

#if defined( MU_WINDOWS )
#include <MemUtil/Import_windows.hpp>

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

void CStackWin64::fetch()
{
    std::size_t skip{ 3u };
    PULONG hash{ nullptr };
    std::array<void*, G_MaxStackSize> stackFrames;
    //const std::size_t result =
        static_cast<std::size_t>( RtlCaptureStackBackTrace( static_cast<DWORD>( skip ), G_MaxStackSize, stackFrames.data(), hash ) );
    for( std::size_t i = 0u; i < G_MaxStackSize; ++i )
    {
        m_stackFrames[i].Addr = stackFrames[i];
    }
}

void CStackWin64::decode()
{
    char name[256];
    ULONG size{ 0u };
    ULONG lineNum{ 0u };

    static IDebugSymbols* idebug = DebugWrapper::getInstance().get();
    for( std::size_t i = 0u; i < G_MaxStackSize; ++i )
    {
        const ULONG64 offset = reinterpret_cast<ULONG64>( m_stackFrames[i].Addr );

        name[0] = '\0';
        if( idebug->GetLineByOffset( offset, &lineNum, name, sizeof( name ), &size, 0 ) != S_OK )
        {
            continue;
        }

        SLineInfo& currentLine = m_stackFrames[i];
        currentLine.Value = name;
        currentLine.Number = static_cast<std::uint16_t>( lineNum );
    }
}

bool CStackWin64::operator==( const CStackWin64& arg ) const
{
    return m_stackFrames == arg.m_stackFrames;
}

const std::array<SLineInfo, G_MaxStackSize>& CStackWin64::getStackLines() const
{
    return m_stackFrames;
}


}  // namespace MU
#endif // #if defined( MU_WINDOWS )
