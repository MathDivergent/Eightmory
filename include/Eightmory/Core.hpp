#ifndef EIGHTMORY_CORE_HPP
#define EIGHTMORY_CORE_HPP

#include <cstddef> // size_t
#include <climits> // CHAR_BIT

namespace eightmory
{

struct segment_t
{
    // segment memory size
    std::size_t size : sizeof(std::size_t) * CHAR_BIT - 1;

    // segment is used mark
    // note: manual change is not safe
    std::size_t is_used : 1;

    // return segment memory
    void* memory() noexcept;

    // return neighbour rhs segment
    segment_t* next() noexcept;

    // convert segment memory address to segment
    static segment_t* segment(void* address) noexcept;
};

class memory_manager_t
{
public:
    memory_manager_t(char* memory, std::size_t bytes);

public:
    // return segment memory address, search from xxbegin
    [[nodiscard]] void* add_segment(std::size_t size);
    // return segment memory address, search from hint
    [[nodiscard]] void* add_segment(std::size_t size, segment_t* hint);

    // try extends current segment memory
    bool extend_segment(void* address, std::size_t size);

    // try remove segment by segment memory address, search from xxbegin
    bool remove_segment(void* address);
    // try remove segment by segment memory address, search from hint
    bool remove_segment(void* address, segment_t* hint);

public:
    segment_t* begin() const noexcept { return xxbegin; }
    segment_t* end() const noexcept { return xxend; }

public:
    std::size_t bytes() const noexcept;

private:
    segment_t* xxbegin = nullptr;
    segment_t* xxend = nullptr;
};

} // namespace eightmory

#endif // EIGHTMORY_CORE_HPP
