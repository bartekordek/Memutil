#pragma once

#include <MemUtil/STL_Imports/STD_unordered_map.hpp>
#include <MemUtil/STL_Imports/STD_cstdint.hpp>
#include <MemUtil/STL_Imports/STD_memory_resource.hpp>
#include <MemUtil/Generic/NonCopyable.hpp>

namespace MU
{
template <class ContainerType1, class ContainerType2>
class PMR_unordered_map final
{
public:
    using UnderylingType = std::pmr::unordered_map<ContainerType1, ContainerType2>;

    MU_NONCOPYABLE( PMR_unordered_map )

    PMR_unordered_map()
    {
    }

    void init( std::uint64_t inBufferSize )
    {
        m_bufferSize = inBufferSize;
        m_buffer = new std::byte[m_bufferSize];
        m_dequeBufferSrc = new std::pmr::monotonic_buffer_resource( m_buffer, m_bufferSize );
        m_container = std::pmr::unordered_map<ContainerType1, ContainerType2>( m_dequeBufferSrc );
    }

    UnderylingType* operator->()
    {
        return &m_container;
    }

    const UnderylingType* operator->() const
    {
        return &m_container;
    }

    UnderylingType& operator*()
    {
        return m_container;
    }

    const UnderylingType& operator*() const
    {
        return m_container;
    }

    ~PMR_unordered_map()
    {
    }

protected:
private:
    std::uint64_t m_bufferSize{ 0u };
    std::byte* m_buffer{ nullptr };
    std::pmr::monotonic_buffer_resource* m_dequeBufferSrc{ nullptr };
    UnderylingType m_container;
};
}  // namespace MU