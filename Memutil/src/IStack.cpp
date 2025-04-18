#include <MemUtil/IStack.hpp>
#include <MemUtil/Import_windows.hpp>
#include <Memutil/Generic/StackContainer.hpp>
#include <Memutil/STL_Imports/STD_unordered_map.hpp>

namespace MU
{

StackContainer<std::pmr::unordered_map<void*, SLineInfo>, 4u * 1024u * 1024> g_stackLineCache;


bool SLineInfo::operator == ( const SLineInfo& arg ) const
{
    return ( Value == arg.Value ) && ( Number == arg.Number );
}

SLineInfo::SLineInfo()
{
}

SLineInfo::SLineInfo( const SLineInfo& arg ):
    Value(arg.Value),
    Number(arg.Number)
{
}

SLineInfo::SLineInfo( SLineInfo&& arg ):
    Value( arg.Value ),
    Number( arg.Number )
{
}

SLineInfo& SLineInfo::operator=( SLineInfo&& arg )
{
    if( this != &arg )
    {
        Value = arg.Value;
        Number = arg.Number;
    }
    return *this;
}

SLineInfo& SLineInfo::operator=( const SLineInfo& arg )
{
    if( this != &arg )
    {
        Value = arg.Value;
        Number = arg.Number;
    }
    return *this;
}

 SLineInfo::~SLineInfo()
{
}



CIStack::CIStack():
    Data( nullptr ),
    Size( 0u ),
    Type( EStackType::None )
{
}

 CIStack::CIStack( const CIStack& arg ):
    Data( arg.Data ),
    Size( arg.Size ),
    Type( arg.Type )
{
}

 CIStack::CIStack( CIStack&& arg ):
    Data( arg.Data ),
    Size( arg.Size ),
    Type( arg.Type )
{
    arg.Data = nullptr;
    arg.Size = 0u;
    arg.Type = EStackType::None;
}

CIStack& CIStack::operator=( const CIStack& arg )
{
    if (this != &arg)
    {
        Data = arg.Data;
        Size = arg.Size;
        Type = arg.Type;
    }

    return *this;
}

CIStack& CIStack::operator=( CIStack&& arg )
{
    if( this != &arg )
    {
        Data = arg.Data;
        Size = arg.Size;
        Type = arg.Type;

        arg.Data = nullptr;
        arg.Size = 0u;
        arg.Type = EStackType::None;
    }

    return *this;
}

const MU::SLineInfo* CIStack::getFromCache( void* inPtr ) const
{
    std::lock_guard<std::mutex> locker( m_cacheMtx );
    const auto it = g_stackLineCache->find( inPtr );
    if (it != g_stackLineCache->end())
    {
        return &it->second;
    }

    return nullptr;
}

void CIStack::addToCache( void* inPtr, SLineInfo* inLine )
{
    std::lock_guard<std::mutex> locker( m_cacheMtx );
    ( *g_stackLineCache )[inPtr] = *inLine;
}

CIStack::~CIStack()
{
}



}  // namespace MU