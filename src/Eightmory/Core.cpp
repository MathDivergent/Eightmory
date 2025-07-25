#include <Eightmory/Core.hpp>

#include <cassert> // assert
#include <new> // placement new

namespace eightmory
{

void* segment_t::memory() noexcept
{
    return reinterpret_cast<char*>(this) + sizeof(segment_t);
}

segment_t* segment_t::segment(void* memory) noexcept
{
    return reinterpret_cast<segment_t*>
    (
        reinterpret_cast<char*>(memory) - sizeof(segment_t)
    );
}

segment_t* segment_t::next() noexcept
{
    return reinterpret_cast<segment_t*>
    (
        reinterpret_cast<char*>(memory()) + size
    );
}

segment_manager_t::segment_manager_t(void* memory, std::size_t bytes)
    : xxbegin(reinterpret_cast<segment_t*>(memory))
    , xxend(reinterpret_cast<segment_t*>(reinterpret_cast<char*>(memory) + bytes))
{
    assert(bytes > sizeof(segment_t) && "buffer size must be greater than sizeof(segment_t)");

    auto segment = new (begin()) segment_t;
    segment->size = bytes - sizeof(segment_t);
    segment->is_used = false;
}

void* segment_manager_t::add_segment(std::size_t size)
{
    return add_segment(size, begin());
}

void* segment_manager_t::add_segment(std::size_t size, segment_t* hint)
{
    for (auto segment = hint; segment != end(); segment = segment->next())
    {
        if (segment->is_used)
        {
             continue;
        }

        // lazy defragmentation
        while (segment->size < sizeof(segment_t) + size)
        {
            auto rhs = segment->next();
            if (rhs != end() && !rhs->is_used)
            {
                segment->size += sizeof(segment_t) + rhs->size;
                rhs->~segment_t();
            }
            else
            {
                break;
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
        else if (segment->size >= size)
        {
            // segment->size in range [size; size + sizeof(segment_t))
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

bool segment_manager_t::extend_segment(void* memory)
{
    auto segment = segment_t::segment(memory);
    auto const prev_size = segment->size;

    while (true)
    {
        auto rhs = segment->next();
        if (rhs == end() || rhs->is_used)
        {
            break;
        }

        segment->size += sizeof(segment_t) + rhs->size;
        rhs->~segment_t();
    }

    return segment->size > prev_size;
}

bool segment_manager_t::extend_segment(void* memory, std::size_t size)
{
    auto segment = segment_t::segment(memory);
    auto rhs = segment->next();

    if (rhs == end() || rhs->is_used)
    {
        return false;
    }

    extend_segment(rhs->memory());

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

void segment_manager_t::remove_segment(void* memory)
{
#ifdef EIGHTMORY_DEBUG
    for (auto segment = begin(); segment != end(); segment = segment->next())
    {
        if (memory == segment->memory())
        {
            segment->is_used = false;
            return;
        }
    }
    assert(false && "failed to find segment by memory address");
#else
    auto segment = segment_t::segment(memory);
    segment->is_used = false;
#endif
}

std::size_t segment_manager_t::bytes() const noexcept
{
    return reinterpret_cast<char*>(end()) - reinterpret_cast<char*>(begin());
}

} // namespace eightmory
