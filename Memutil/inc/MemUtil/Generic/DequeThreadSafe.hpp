#pragma once

#include <MemUtil/Generic/IDequeThreadSafe.hpp>
#include <MemUtil/STL_Imports/STD_deque.hpp>
#include <MemUtil/STL_Imports/STD_mutex.hpp>
#include <MemUtil/Import_tracy.hpp>

namespace MU
{
template <class C>
class DequeThreadSafe final: public IDequeThreadSafe<C>
{
public:
    MU_NONCOPYABLE( DequeThreadSafe )

    DequeThreadSafe() = default;
    void init( std::size_t /*inCapacity*/ ) override
    {
        // m_values.resize( inCapacity );
    }

    std::optional<C> getAndPopFront() override
    {
        MU_MEASURE_SCOPE;
        std::lock_guard<std::mutex> locker( m_mtx );
        if( m_values.empty() )
        {
            return {};
        }

        C result = m_values.front();
        m_values.pop_front();
        return result;
    }

    void addToBack(C inValue, bool /*waitAndTry*/ = true) override
    {
        MU_MEASURE_SCOPE;
        std::lock_guard<std::mutex> locker( m_mtx );
        m_values.push_back( inValue );
    }

    bool isEmpty() const override
    {
        MU_MEASURE_SCOPE;
        std::lock_guard<std::mutex> locker( m_mtx );
        return m_values.empty();
    }

    virtual ~DequeThreadSafe() = default;

protected:
private:

    mutable std::mutex m_mtx;
    std::deque<C> m_values;
};
}  // namespace MU