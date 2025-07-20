#include <Eightmory/Core.hpp>

#include <cassert> // assert
#include <new> // placement new

namespace eightmory
{

void* segment_t::memory() noexcept
{
    return reinterpret_cast<char*>(this) + sizeof(segment_t);
}

segment_t* segment_t::next() noexcept
{
    return reinterpret_cast<segment_t*>
    (
        reinterpret_cast<char*>(memory()) + size
    );
}

segment_t* segment_t::segment(void* memory) noexcept
{
    return reinterpret_cast<segment_t*>
    (
        reinterpret_cast<char*>(memory) - sizeof(segment_t)
    );
}

memory_manager_t::memory_manager_t(char* memory, std::size_t bytes)
    : xxbegin(reinterpret_cast<segment_t*>(memory))
    , xxend(reinterpret_cast<segment_t*>(memory + bytes))
{
    assert(bytes > sizeof(segment_t));

    auto segment = new (begin()) segment_t;
    segment->size = bytes - sizeof(segment_t);
    segment->is_used = false;
}

void* memory_manager_t::add_segment(std::size_t size)
{
    return add_segment(size, begin());
}

void* memory_manager_t::add_segment(std::size_t size, segment_t* hint)
{
    for (auto segment = hint; segment != end(); segment = segment->next())
    {
        if (segment->is_used)
        {
             continue;
        }

        if (segment->size < sizeof(segment_t) + size)
        {
            auto rhs = segment->next();
            if (rhs != end() && !rhs->is_used)
            {
                segment->size += sizeof(segment_t) + rhs->size;
                rhs->~segment_t();
            }
        }

        if (segment->size >= sizeof(segment_t) + size)
        {
            const auto diff = segment->size - size;

            segment->size = size;
            segment->is_used = true;

            auto created = new (segment->next()) segment_t;

            created->size = diff - sizeof(segment_t);
            created->is_used = false;
        }
        else if (segment->size == size)
        {
            segment->is_used = true;
        }
        else
        {
            continue;
        }

        return segment->memory();
    }
    return nullptr;
}

bool memory_manager_t::extend_segment(void* address, std::size_t size)
{
    auto segment = segment_t::segment(address);
    auto rhs = segment->next();

    if (rhs == end() || rhs->is_used)
    {
        return false;
    }

    if (rhs->size >= size)
    {
        const auto diff = rhs->size - size;

        segment->size += size;
        rhs->~segment_t();
        
        auto created = new (segment->next()) segment_t;

        created->size = diff;
        created->is_used = false;

        return true;
    }
    else if (rhs->size + sizeof(segment_t) == size)
    {
        segment->size += size;
        rhs->~segment_t();

        return true;
    }
    else
    {
        return false;
    }
}

bool memory_manager_t::remove_segment(void* address)
{
    return remove_segment(address, begin());
}

bool memory_manager_t::remove_segment(void* address, segment_t* hint)
{
    for (auto segment = hint; segment != end(); segment = segment->next())
    {
        if (address != segment->memory())
        {
            continue;
        }

        auto rhs = segment->next();

        segment->is_used = false;
        if (rhs != end() && !rhs->is_used)
        {
            segment->size += sizeof(segment_t) + rhs->size;
            rhs->~segment_t();
        }

        return true;
    }
    return false;
}

std::size_t memory_manager_t::bytes() const noexcept
{
    return reinterpret_cast<char*>(end()) - reinterpret_cast<char*>(begin());
}

} // namespace eightmory
