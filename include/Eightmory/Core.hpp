#ifndef EIGHTMORY_CORE_HPP
#define EIGHTMORY_CORE_HPP

#include <cstddef> // size_t
#include <climits> // CHAR_BIT

namespace eightmory
{

struct segment_t
{
    std::size_t size : sizeof(std::size_t) * CHAR_BIT - 1;
    std::size_t is_used : 1;

    // return 'pointer to segment memory' from 'segment'
    void* memory() noexcept;

    // return 'segment' from 'pointer to segment memory'
    static segment_t* segment(void* memory) noexcept;

    segment_t* next() noexcept;
};

class segment_manager_t
{
public:
    segment_manager_t(void* memory, std::size_t bytes);

public:
    // allocate segment of given size
    // search from begin
    // return 'pointer to segment memory'
    [[nodiscard]] void* add_segment(std::size_t size);

    // allocate segment of given size
    // search from hint
    // return 'pointer to segment memory'
    [[nodiscard]] void* add_segment(std::size_t size, segment_t* hint);

    // try to extend segment using available free rhs segments
    // return 'true' if extened
    bool extend_segment(void* memory);

    // try to extend segment by given extra size
    // return 'true' if extended
    bool extend_segment(void* memory, std::size_t size);

    // mark segment is_used as 'false'
    void remove_segment(void* memory);

public:
    segment_t* begin() const noexcept { return xxbegin; }
    segment_t* end() const noexcept { return xxend; }
    std::size_t bytes() const noexcept;

private:
    segment_t* xxbegin = nullptr;
    segment_t* xxend = nullptr;
};

} // namespace eightmory

#endif // EIGHTMORY_CORE_HPP
