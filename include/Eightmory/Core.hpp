#ifndef EIGHTMORY_CORE_HPP
#define EIGHTMORY_CORE_HPP

#include <cstddef> // size_t
#include <climits> // CHAR_BIT

namespace eightmory
{

struct segment_t
{
    // size of segment memory
    std::size_t size : sizeof(std::size_t) * CHAR_BIT - 1;

    // true if segment is in use
    std::size_t is_used : 1;

    // get pointer to segment memory
    void* memory() noexcept;

    // get next (right-hand) segment
    segment_t* next() noexcept;

    // get segment header from segment memory pointer
    static segment_t* segment(void* address) noexcept;
};

class segment_manager_t
{
public:
    // initialize memory manager on given buffer
    segment_manager_t(void* memory, std::size_t bytes);

public:
    // allocate segment of given size, search from begin
    [[nodiscard]] void* add_segment(std::size_t size);

    // allocate segment of given size, search from hint
    [[nodiscard]] void* add_segment(std::size_t size, segment_t* hint);

    // try to extend segment using available free rhs segments
    bool extend_segment(void* memory);

    // try to extend segment by given extra size
    bool extend_segment(void* memory, std::size_t size);

    // mark segment as unused
    void remove_segment(void* memory);

public:
    // first segment
    segment_t* begin() const noexcept { return xxbegin; }

    // end marker (after last segment)
    segment_t* end() const noexcept { return xxend; }

    // total managed bytes
    std::size_t bytes() const noexcept;

private:
    segment_t* xxbegin = nullptr;
    segment_t* xxend = nullptr;
};

} // namespace eightmory

#endif // EIGHTMORY_CORE_HPP
