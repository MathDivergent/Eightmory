#ifndef EIGHTMORY_CORE_HPP
#define EIGHTMORY_CORE_HPP

#include <cstddef> // size_t
#include <climits> // CHAR_BIT

namespace eightmory
{

struct alignas(EIGHTMORY_SEGMENT_ALIGN) EIGHTMORY_API segment_t
{
    std::size_t size : sizeof(std::size_t) * CHAR_BIT - 1;
    std::size_t is_used : 1;

    static constexpr auto max_size = std::size_t(-1) >> 1;

    // return 'pointer to segment memory' from 'segment'
    void* memory() noexcept;

    // return 'segment' from 'pointer to segment memory'
    static segment_t* segment(void* memory) noexcept;

    segment_t* next() noexcept;
};

class EIGHTMORY_API segment_manager_t
{
public:
    segment_manager_t(void* memory, std::size_t bytes) noexcept;

public:
    // allocate segment of given size in range [size, size + sizeof(segment_t))
    // search from begin
    // return 'pointer to segment memory'
    [[nodiscard]] void* add_segment(std::size_t size) noexcept;

    // allocate segment of given size in range [size, size + sizeof(segment_t))
    // search from hint
    // return 'pointer to segment memory'
    [[nodiscard]] void* add_segment(std::size_t size, segment_t* hint) noexcept;

    // extend segment using available free rhs segments
    // return 'true' if extened
    bool extend_segment(void* memory) noexcept;

    // extend segment of given extra size in range [size, size + sizeof(segment_t))
    // return 'true' if extended
    bool extend_segment(void* memory, std::size_t size) noexcept;

    // mark segment is_used as 'false'
    // return 'true' if removed
    bool remove_segment(void* memory) noexcept;

public:
    segment_t* begin() const noexcept { return xxbegin; }
    segment_t* end() const noexcept { return xxend; }
    std::size_t bytes() const noexcept;

private:
    segment_t* xxbegin = nullptr;
    segment_t* xxend = nullptr;
};

// align must be power of two
constexpr std::size_t align_up(std::size_t size, std::size_t align = EIGHTMORY_SEGMENT_ALIGN) noexcept
{
    return ~(align - 1) & (align - 1 + size);
}

constexpr bool is_aligned(std::size_t size, std::size_t align = EIGHTMORY_SEGMENT_ALIGN) noexcept
{
    return ((align - 1) & size) == 0;
}

} // namespace eightmory

#endif // EIGHTMORY_CORE_HPP
