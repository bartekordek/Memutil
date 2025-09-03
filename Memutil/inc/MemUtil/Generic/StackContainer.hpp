#pragma once

#include <MemUtil/STL_Imports/STD_memory_resource.hpp>
#include <MemUtil/STL_Imports/STD_cstdint.hpp>


namespace MU
{

template <class ContainerType, std::uint64_t BufferSize>
class StackContainer final
{
public:
    StackContainer()
    {
    }

    StackContainer( const StackContainer& ) = delete;
    StackContainer( StackContainer&& ) = delete;

    StackContainer& operator=( const StackContainer& ) = delete;
    StackContainer& operator=( StackContainer&& ) = delete;

    ContainerType* operator->()
    {
        return &m_container;
    }

    const ContainerType* operator->()const 
    {
        return &m_container;
    }

    ContainerType& operator*()
    {
        return m_container;
    }

    const ContainerType& operator*() const
    {
        return m_container;
    }

    ~StackContainer()
    {
    }

protected:
private:
    std::uint64_t m_bufferSize{BufferSize};
    std::array<std::byte, BufferSize> m_bufferBlocks;
    std::pmr::monotonic_buffer_resource m_dequeBufferSrc{ m_bufferBlocks.data(), BufferSize };
    ContainerType m_container{ &m_dequeBufferSrc };
};

}  // namespace MU
