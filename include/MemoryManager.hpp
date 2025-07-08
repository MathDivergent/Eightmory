#ifndef MEMORY_MANAGER_CORE_HPP
#define MEMORY_MANAGER_CORE_HPP

#include <cstddef> // size_t

struct segment_t
{
    std::size_t size : sizeof(std::size_t) * 8 - 1;
    std::size_t is_used : 1;

    void* memory();
    segment_t* next();

    static segment_t* segment(void* memory);
};

class memory_manager_t
{
public:
    memory_manager_t(char* memory, std::size_t bytes);

public:
    void* add_segment(std::size_t size);
    bool extend_segment(void* address, std::size_t size);
    bool remove_segment(void* address);

public:
    segment_t* begin() const { return xxbegin; }
    segment_t* end() const { return xxend; }

private:
    segment_t* xxbegin = nullptr;
    segment_t* xxend = nullptr;
};

#endif // MEMORY_MANAGER_CORE_HPP
