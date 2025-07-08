#include <MemoryManager.hpp>

#include <cassert> // assert
#include <new> // placement new

void* segment_t::memory()
{
    return reinterpret_cast<char*>(this) + sizeof(segment_t);
}

segment_t* segment_t::next()
{
    return reinterpret_cast<segment_t*>
    (
        reinterpret_cast<char*>(memory()) + size
    );
}

segment_t* segment_t::segment(void* memory)
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
    for (auto segment = begin(); segment != end(); segment = segment->next())
    {
        if (segment->is_used)
        {
             continue;
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
    auto lhs = end();
    for (auto segment = begin(); segment != end(); lhs = segment,
                                                   segment = segment->next())
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
        if (lhs != end() && !lhs->is_used)
        {
            lhs->size += sizeof(segment_t) + segment->size;
            segment->~segment_t();
        }

        return true;
    }
    return false;
}
