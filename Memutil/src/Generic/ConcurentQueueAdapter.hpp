#pragma once

#include <MemUtil/Generic/ListThreadSafe.hpp>
#include "Import_blockingconurentqueue.hpp"

namespace MU
{
template <class C>
class ConcurentQueueAdapter final: public IDequeThreadSafe<C>
{
public:
    MU_NONCOPYABLE( ConcurentQueueAdapter )
    ConcurentQueueAdapter() = default;
    void init( std::size_t /*inCapacity*/ ) override
    { 
    }

    std::optional<C> getAndPopFront() override
    {
        MU_MEASURE_SCOPE;
        C result;
        if( m_values.try_dequeue( result ) == false )
        {
            return {};
        }

        return result;
    }

    void addToBack( C inValue, bool /*waitAndTry*/ = true ) override
    {
        MU_MEASURE_SCOPE;
        m_values.enqueue( inValue );
    }

    bool isEmpty() const override
    {
        MU_MEASURE_SCOPE;
        return m_values.size_approx() == 0;
    }

    ~ConcurentQueueAdapter()
    {

    }

protected:
private:
    moodycamel::ConcurrentQueue<C> m_values;
};
}  // namespace MU
