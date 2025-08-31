#include <MemUtil/StackWin64.hpp>

#if defined( MU_WINDOWS )
#include <MemUtil/Import_windows.hpp>
#include <MemUtil/Import_tracy.hpp>
#include <MemUtil/IDebugWrapper.hpp>
#include "StackCache.hpp"

namespace MU
{

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

CStackWin64& CStackWin64::operator=( const CStackWin64& arg )
{
    if( this != &arg )
    {
        CIStack::operator=( arg );
        m_stackFrames = arg.m_stackFrames;
        m_data = arg.m_data;
    }
    return *this;
}

CStackWin64& CStackWin64::operator=( CStackWin64&& arg )
{
    if( this != &arg )
    {
        CIStack::operator=( arg );
        m_stackFrames = std::move( arg.m_stackFrames );
        m_data = std::move( arg.m_data );
    }
    return *this;
}

void CStackWin64::fetch()
{
    IDebugWrapper::getInstance().fillData( m_data );
}

void CStackWin64::decode()
{
    MU_MEASURE_SCOPE;
    constexpr std::size_t nameSize{ 256u };
    char name[nameSize];
    std::uint64_t size{ 0u };
    std::uint64_t lineNum{ 0u };

    for( std::size_t i = 0u; i < G_MaxStackSize; ++i )
    {
        MU_MEASURE_SUBSCOPE( decode00, "CStackWin64::decode::it", true );

        void* currentAdd = m_data[i];
        const SLineInfo* fromCache = CStackCache::getInstance().get( currentAdd );
        if( fromCache )
        {
            m_stackFrames[i] = *fromCache;
            continue;
        }

        const ULONG64 offset = reinterpret_cast<ULONG64>( currentAdd );

        name[0] = '\0';
        if( IDebugWrapper::getInstance().getLineByOffset( offset, lineNum, name, nameSize, size ) == false )
        {
            continue;
        }

        SLineInfo& currentLine = m_stackFrames[i];
        currentLine.Value = name;
        currentLine.Number = static_cast<std::uint16_t>( lineNum );
        CStackCache::getInstance().add( &currentLine, currentAdd );
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
