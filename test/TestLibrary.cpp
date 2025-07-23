#include <EightmoryTestingBase.hpp>

using eightmory::segment_t;
using eightmory::segment_manager_t;

TEST_SPACE()
{

std::size_t segment_count(segment_manager_t const& manager) noexcept
{
    auto counter = std::size_t(0);
    for (auto segment = manager.begin(); segment != manager.end(); segment = segment->next()) counter += 1;
    return counter;
}

} // TEST_SPACE

TEST(TestLibrary, TestCommon)
{
    constexpr auto memory_size = 1024;
    char memory_pointer[memory_size];

    auto manager = segment_manager_t(memory_pointer, memory_size);

    {
        ASSERT("manager.bytes", manager.bytes() == memory_size);
        ASSERT("manager.segment_count", segment_count(manager) == 1);

        auto begin_segment = manager.begin();
        EXPECT("manager.begin_segment.size", begin_segment->size == manager.bytes() - sizeof(segment_t));
        EXPECT("manager.begin_segment.is_used", begin_segment->is_used == false);
    }

    {
        auto over_size_memory = manager.add_segment(manager.bytes());
        EXPECT("manager.add_segment.over_size_segment", over_size_memory == nullptr);
        EXPECT("manager.add_segment.over_size_segment.segment_count", segment_count(manager) == 1);
    }
    {
        auto max_size_memory = manager.add_segment(memory_size - sizeof(segment_t));
        ASSERT("manager.add_segment.max_size_segment", max_size_memory != nullptr);
        EXPECT("manager.add_segment.max_size_segment.segment_count", segment_count(manager) == 1);

        auto max_size_segment = segment_t::segment(max_size_memory);
        EXPECT("manager.add_segment.max_size_segment.size", max_size_segment->size == memory_size - sizeof(segment_t));
        EXPECT("manager.add_segment.max_size_segment.is_used", max_size_segment->is_used == true);
        EXPECT("manager.add_segment.max_size_segment.memory", max_size_segment->memory() == max_size_memory);

        manager.remove_segment(max_size_memory);
        EXPECT("manager.remove_segment.max_size_segment.size", max_size_segment->size == memory_size - sizeof(segment_t));
        EXPECT("manager.remove_segment.max_size_segment.is_used", max_size_segment->is_used == false);
        EXPECT("manager.remove_segment.max_size_segment.memory", max_size_segment->memory() == max_size_memory);
        EXPECT("manager.remove_segment.max_size_segment.segment_count", segment_count(manager) == 1);
    }
}
