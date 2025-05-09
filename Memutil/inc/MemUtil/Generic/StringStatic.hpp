#pragma once

#include <MemUtil/STL_Imports/STD_cstdint.hpp>
#include <MemUtil/STL_Imports/STD_cstring.hpp>
#include <MemUtil/STL_Imports/STD_cstdarg.hpp>
#include <MemUtil/STL_Imports/STD_iostream.hpp>
#include <MemUtil/STL_Imports/STD_assert.hpp>

#if defined( _MSC_VER )
#pragma warning( push, 0 )
#pragma warning( disable : 4996 )
#endif // defined( _MSC_VER )

namespace MU
{

template <std::uint16_t Capacity>
class StringStatic
{
public:
    StringStatic()
    {
    }

    StringStatic( const char* arg ):
        m_size( std::strlen( arg ) )
    {
        assert( m_size < Capacity && "TOO SMALL BUFFER!" );
        std::strcpy( m_value, arg );
    }

    StringStatic( const StringStatic& arg ):
        m_size( arg.m_size )
    {
        assert( m_size < Capacity && "TOO SMALL BUFFER!" );
        std::strcpy( m_value, arg.m_value );
    }

    StringStatic( StringStatic&& arg ):
        m_size( arg.m_size )
    {
        std::strcpy( m_value, arg.m_value );
        arg.m_size = 0u;
    }

    StringStatic& operator=( const StringStatic& arg )
    {
        if( this != &arg )
        {
            if( arg.m_size > 0 )
            {
                std::strcpy( m_value, arg.m_value );
                m_size = arg.m_size;
            }
        }
        return *this;
    }

    StringStatic& operator=( StringStatic&& arg )
    {
        if( this != &arg )
        {
            if( arg.m_size > 0 )
            {
                std::strcpy( m_value, arg.m_value );
                m_size = arg.m_size;
            }

            arg.m_size = 0u;
        }
        return *this;
    }

    StringStatic& operator=( const char* arg )
    {
        m_size = static_cast<decltype( m_size )>( std::strlen( arg ) );
        assert( m_size < Capacity && "TOO SMALL BUFFER!" );

        std::strcpy( m_value, arg );

        return *this;
    }

    void createFrom( const char* msg... )
    {
        va_list args;
        va_start( args, msg );
        m_size = static_cast<std::uint16_t>( vsnprintf( m_value, m_capacity, msg, args ) );
        va_end( args );
    }

    template <std::uint16_t CapacityArg>
    bool operator==( const StringStatic<CapacityArg>& arg ) const
    {
        if constexpr( CapacityArg != Capacity )
        {
            return false;
        }

        return operator==( arg.m_value );
    }

    bool operator==( const char* arg ) const
    {
        return std::strcmp( m_value, arg ) == 0;
    }

    bool operator<( const StringStatic& arg ) const
    {
        return std::strcmp( m_value, arg.m_value ) < 0;
    }

    void append( const char* inStr )
    {
        const std::uint16_t stringLength = static_cast<std::uint16_t>( std::strlen( inStr ) );
        append( inStr, stringLength );
    }

    void append( const char* inStr, std::uint16_t inSize )
    {
        if( m_size + inSize + 1u >= Capacity )
        {
            assert( false && "NOT ENOUGHT PLACE FOR THIS STRING!" );
            return;
        }

        std::strncpy( m_value + m_size, inStr, inSize );
        m_size += inSize;
        m_value[m_size] = '\n';
        m_value[m_size + 1] = '\0';
        m_size += 1;
    }

    void appendFrom( const char* msg... )
    {
        va_list args;
        va_start( args, msg );
        m_size += static_cast<std::uint16_t>( vsnprintf( m_value + m_size, m_capacity, msg, args ) );
        va_end( args );
    }

    void print() const
    {
    }

    const char* c_str() const
    {
        return m_value;
    }

    char* getRawBuffer()
    {
        return m_value;
    }

    std::size_t getCapacity() const
    {
        return m_capacity;
    }

    void clear()
    {
        m_size = 0u;
        std::memset( m_value, 0, m_capacity );
    }

    bool empty() const
    {
        return m_size == 0u;
    }

    ~StringStatic()
    {
    }

protected:
private:
    char m_value[Capacity]{};
    std::uint16_t m_size{ 0u };
    std::size_t m_capacity{ Capacity };
};
}  // namespace MU


#if defined( _MSC_VER )
#pragma warning( pop )
#endif  // defined( _MSC_VER )