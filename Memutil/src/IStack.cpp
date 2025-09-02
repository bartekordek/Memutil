#include <MemUtil/IStack.hpp>
#include <MemUtil/Import_windows.hpp>
#include <MemUtil/Generic/StackContainer.hpp>
#include <MemUtil/STL_Imports/STD_unordered_map.hpp>
#include <MemUtil/Import_tracy.hpp>

namespace MU
{
bool SLineInfo::operator==( const SLineInfo& arg ) const
{
    return ( FileName == arg.FileName ) && ( Number == arg.Number );
}

SLineInfo::SLineInfo()
{
}

SLineInfo::SLineInfo( const SLineInfo& arg ):
    FileName( arg.FileName ),
    FunctionName(arg.FunctionName),
    Number( arg.Number )
{
}

SLineInfo::SLineInfo( SLineInfo&& arg ):
    FileName( arg.FileName ),
    FunctionName( arg.FunctionName ),
    Number( arg.Number )
{
}

SLineInfo& SLineInfo::operator=( SLineInfo&& arg )
{
    if( this != &arg )
    {
        FileName = arg.FileName;
        FunctionName = arg.FunctionName;
        Number = arg.Number;
    }
    return *this;
}

SLineInfo& SLineInfo::operator=( const SLineInfo& arg )
{
    if( this != &arg )
    {
        FileName = arg.FileName;
        FunctionName = arg.FunctionName;
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
    if( this != &arg )
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

CIStack::~CIStack()
{
}

}  // namespace MU