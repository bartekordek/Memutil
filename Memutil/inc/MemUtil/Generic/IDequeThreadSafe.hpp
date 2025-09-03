#pragma once

#include <MemUtil/Generic/NonCopyable.hpp>
#include <MemUtil/STL_Imports/STD_optional.hpp>

namespace MU
{
template <class C>
class IDequeThreadSafe
{
public:
    MU_NONCOPYABLE( IDequeThreadSafe )

    IDequeThreadSafe() = default;
    virtual void init( std::size_t inCapacity ) = 0;
    virtual std::optional<C> getAndPopFront() = 0;
    virtual void addToBack( C inValue, bool waitAndTry = true ) = 0;
    virtual bool isEmpty() const = 0;

    virtual ~IDequeThreadSafe() = default;

protected:
private:
};
}