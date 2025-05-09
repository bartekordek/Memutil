#pragma once

#include <Memutil/Generic/IDequeThreadSafe.hpp>
#include <MemUtil/STL_Imports/STD_mutex.hpp>
#include <MemUtil/Import_tracy.hpp>

namespace MU
{
template <class C>
class DataContainer final: public IDequeThreadSafe<C>
{
public:
    struct DataPair
    {
        bool Used{ false };
        C Value;
    };

    MU_NONCOPYABLE( DataContainer )

    DataContainer()
    {

    }

    void init( std::size_t inCapacity )
    {
        MU_MEASURE_SCOPE;
        m_capacity = inCapacity;
        m_value = new DataPair[m_capacity];
    }

    std::optional<C> getAndPopFront()
    {
        MU_MEASURE_SCOPE;
        std::lock_guard<std::mutex> locker( m_mtx );

        if( m_size == 0u )
        {
            return {};
        }

        for( std::size_t i = 0u; i < m_capacity; ++i )
        {
            if( m_value[i].Used )
            {
                m_value[i].Used = false;
                --m_size;
                return m_value[i].Value;
            }
        }


        return {};
    }

    void addToBack( C inValue, bool waitAndTry = true )
    {
        MU_MEASURE_SCOPE;
        const std::int64_t size = static_cast<std::int64_t>( m_capacity );

        auto tryToadd = [&inValue, this, size]()
        {
            MU_MEASURE_SUBSCOPE( addToBack2, "add_try", true );
            std::lock_guard<std::mutex> locker( m_mtx );
            for( std::int64_t i = size - 1; i >= 0; --i )
            {
                DataPair& dp = m_value[i];
                if( dp.Used == false )
                {
                    dp.Value = inValue;
                    dp.Used = true;
                    ++m_size;
                    return true;
                }
            }

            return false;
        };

        if( waitAndTry == false )
        {
            while( tryToadd() )
            {
            }
        }
        else
        {
            tryToadd();
        }
    }

    bool isEmpty() const
    {
        std::lock_guard<std::mutex> locker( m_mtx );
        return m_size == 0u;
    }

    ~DataContainer()
    {
        delete[] m_value;
        m_value = nullptr;
        m_size = 0u;
        m_capacity = 0u;
    }

protected:
private:

    mutable std::mutex m_mtx;
    std::size_t m_capacity{ 0u };
    std::size_t m_size{ 0u };
    DataPair* m_value{ nullptr };

};
}  // namespace MU