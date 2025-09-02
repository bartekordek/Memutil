#include "DebugWrapperWin64.hpp"

#if defined( MU_WINDOWS )
#include <MemUtil/STL_Imports/STD_assert.hpp>
#include <MemUtil/Import_tracy.hpp>

namespace MU
{

DebugWrapperWin64::DebugWrapperWin64()
{
    MU_MEASURE_SCOPE;
    ___mu_RtlWalkFrameChain = (___mu_t_RtlWalkFrameChain)GetProcAddress( GetModuleHandleA( "ntdll.dll" ), "RtlWalkFrameChain" );

    void* debugClientMem{ nullptr };
    HRESULT cmdResult = ::DebugCreate( __uuidof( IDebugClient ), &debugClientMem );
    assert( cmdResult == S_OK );
    m_debugClient = reinterpret_cast<IDebugClient*>( debugClientMem );

    void* debugControlMem{ nullptr };
    cmdResult = m_debugClient->QueryInterface( __uuidof( IDebugControl ), &debugControlMem );
    assert( cmdResult == S_OK );
    m_debugControl = reinterpret_cast<IDebugControl*>( debugControlMem );

    cmdResult = m_debugClient->AttachProcess( 0, ::GetCurrentProcessId(), DEBUG_ATTACH_NONINVASIVE | DEBUG_ATTACH_NONINVASIVE_NO_SUSPEND );
    assert( cmdResult == S_OK );

    cmdResult = m_debugControl->WaitForEvent( DEBUG_WAIT_DEFAULT, INFINITE );
    assert( cmdResult == S_OK );

    void* debugSymbolsMem{ nullptr };
    cmdResult = m_debugClient->QueryInterface( __uuidof( IDebugSymbols ), &debugSymbolsMem );
    assert( cmdResult == S_OK );
    m_debugSymbols = reinterpret_cast<IDebugSymbols*>( debugSymbolsMem );
}

void DebugWrapperWin64::init()
{
    MU_MEASURE_SCOPE;
}

void DebugWrapperWin64::fillData( std::array<void*, G_DataSizePlusOffset>& inOutData )
{
    MU_MEASURE_SCOPE;
    const auto num = ___mu_RtlWalkFrameChain( (void**)( inOutData.data() ), inOutData.size(), 0 );
}

bool DebugWrapperWin64::getLineByOffset( std::uint64_t offset, std::uint64_t& inOutlineNum, char* inOutName, std::size_t inOutNameSize,
                                         char* inOutFunctionName, std::size_t inOutFunctionNameSize, std::uint64_t& outSize )
{
    ULONG lineNum;
    ULONG currentSize;
    const bool result = m_debugSymbols->GetLineByOffset( offset, &lineNum, inOutName, inOutNameSize, &currentSize, nullptr ) == S_OK;
    // if( idebug->GetLineByOffset( offset, &lineNum, name, sizeof( name ), &size, nullptr ) != S_OK )
    inOutlineNum = lineNum;
    outSize = currentSize;

    ULONG nameSize{ 0u };
    m_debugSymbols->GetNameByOffset( offset, inOutFunctionName, inOutFunctionNameSize, &nameSize, 0u );

    return result;
}

DebugWrapperWin64::~DebugWrapperWin64()
{
}
}

#endif  // #if defined( MU_WINDOWS )