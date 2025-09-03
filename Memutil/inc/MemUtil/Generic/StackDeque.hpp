#pragma once

#include <MemUtil/STL_Imports/STD_mutex.hpp>
#include <MemUtil/STL_Imports/STD_cstdint.hpp>
#include <MemUtil/STL_Imports/STD_deque.hpp>

namespace MU
{
template <class C, std::uint64_t Size>
class StackDeque final
{
public:
    StackDeque()
    {
    }

    StackDeque( const StackDeque& ) = delete;
    StackDeque( StackDeque&& ) = delete;
    StackDeque& operator=( const StackDeque& ) = delete;
    StackDeque& operator=( StackDeque&& ) = delete;

    template <typename... Args>
    void emplace_back( Args&&... args )
    {
        std::lock_guard<std::mutex> locker( m_dequeMtx );
        m_deque.emplace_back( C( std::forward<Args>( args )... ) );
    }

    bool empty() const
    {
        bool result{ false };
        {
            std::lock_guard<std::mutex> locker( m_dequeMtx );
            result = m_deque.empty();
        }
        return result;
    }

    void pop_back()
    {
        std::lock_guard<std::mutex> locker( m_dequeMtx );
        m_deque.pop_back();
    }

    C popWithRemove()
    {
        std::lock_guard<std::mutex> locker( m_dequeMtx );
        C result = std::move( m_deque.back() );
        m_deque.pop_back();
        return result;
    }

    C& back()
    {
        return m_deque.back();
    }

    void lock()
    {
        m_dequeMtx.lock();
    }

    ~StackDeque()
    {
        {
            std::lock_guard<std::mutex> locker( m_dequeMtx );
            m_deque.clear();
        }
    }

protected:
private:
    std::array<std::byte, Size> m_bufferBlocks;
    std::pmr::monotonic_buffer_resource m_bufferSrc{ m_bufferBlocks.data(), Size };
    std::pmr::deque<C> m_deque{ &m_bufferSrc };

    mutable std::mutex m_dequeMtx;
};
}  // namespace MU