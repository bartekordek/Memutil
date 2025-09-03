#pragma once

#include <MemUtil/Generic/IDequeThreadSafe.hpp>
#include <MemUtil/STL_Imports/STD_list.hpp>
#include <MemUtil/STL_Imports/STD_mutex.hpp>
#include <MemUtil/Import_tracy.hpp>

namespace MU
{
template <class C>
class ListThreadSafe final: public IDequeThreadSafe<C>
{
public:
    MU_NONCOPYABLE( ListThreadSafe )

    ListThreadSafe() = default;
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

    void addToBack( C inValue, bool /*waitAndTry*/ = true ) override
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

    virtual ~ListThreadSafe() = default;

protected:
private:

    mutable std::mutex m_mtx;
    std::list<C> m_values;
};
}  // namespace MU