#pragma once

#include <MemUtil/STL_Imports/STD_array.hpp>
#include <MemUtil/STL_Imports/STD_cstdint.hpp>
#include <MemUtil/STL_Imports/STD_functional.hpp>
#include <MemUtil/STL_Imports/STD_algorithm.hpp>

namespace MU
{
template <class Type,std::uint64_t Size>
class StackList final
{
public:
    struct Unit
    {
        Type Value;
        bool Occupied{ false };
    };

    StackList()
    {
    }

    StackList( const StackList& arg ):
        m_values( arg.m_values ),
        m_capacity( arg.m_capacity ),
        m_size( arg.m_size )
    {
    }

    StackList( StackList&& arg ):
        m_values( arg.m_values ),
        m_capacity( arg.m_capacity ),
        m_size( arg.m_size )
    {

    }

    StackList& operator=( const StackList& arg )
    {
        if( this != &arg )
        {
            m_values = arg.m_values;
            m_capacity = arg.m_capacity;
            m_size = arg.m_size;
        }

        return *this;
    }

    StackList& operator=( StackList&& arg )
    {
        if( this != &arg )
        {
            m_values = arg.m_values;
            m_capacity = arg.m_capacity;
            m_size = arg.m_size;
        }

        return *this;
    }

    void insert( Type inValue )
    {
        if( cointains( inValue ) )
        {
            return;
        }

        assert( m_size < m_capacity );
        Unit& currentFreeUnit = m_values[m_size];
        currentFreeUnit.Value = inValue;
        currentFreeUnit.Occupied = true;
        ++m_size;
    }

    bool cointains( Type inValue )
    {
        for( std::size_t i = 0u; i < m_size; ++i )
        {
            Unit& currentUnit = m_values[i];
            if( inValue == currentUnit.Value )
            {
                if( currentUnit.Occupied )
                {
                    return true;
                }
            }
        }
        return false;
    }

    void remove( Type inValue )
    {
        for( std::size_t i = 0u; i < m_size; ++i )
        {
            Unit& currentUnit = m_values[i];

            if( inValue == currentUnit.Value )
            {
                currentUnit.Occupied = false;
                --m_size;

                // Check if there is a hole after removing element.
                if( m_size != 0u )
                {
                    ++i;
                    if( i < m_capacity )
                    {
                        Unit& nextUnit = m_values[i];

                        if( nextUnit.Occupied )
                        {
                            moveElementsUp();
                        }
                    }
                }

                return;
            }
        }
    }

    void execute( const std::function<void( Type value )>& inFun )
    {
        for( std::size_t i = 0u; i < m_size; ++i )
        {
            const Unit& currentUnit = m_values[i];
            if( currentUnit.Occupied == true )
            {
                inFun( currentUnit.Value );
            }
        }
    }

    auto begin()
    {
        return m_values.begin();
    }

    auto begin() const
    {
        return m_values.begin();
    }

    auto end()
    {
        return m_values.begin() + static_cast<const ptrdiff_t>( m_size );
    }

    auto end() const
    {
        return m_values.begin() + static_cast<const ptrdiff_t>( m_size );
    }

    void clear()
    {
        if( m_size == 0 )
        {
            return;
        }

        for( std::size_t i = 0u; i < m_size; ++i )
        {
            Unit& currentUnit = m_values[i];
            currentUnit.Occupied = false;
        }
        m_size = 0;
    }

    bool empty() const
    {
        return m_size == 0u;
    }

    std::size_t size() const
    {
        return m_size;
    }

    ~StackList()
    {
    }

protected:
private:
    void moveElementsUp()
    {
        std::sort( m_values.begin(), m_values.end(),
                   []( const Unit& u1, const Unit& u2 )
                   {
                       return u1.Occupied > u2.Occupied;
                   } );
    }

    std::array<Unit, Size> m_values;
    std::size_t m_capacity{ Size };
    std::size_t m_size{ 0u };
};
}  // namespace MU
