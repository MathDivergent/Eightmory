#include <EightmoryTestingBase.hpp>

using eightmory::segment_t;
using eightmory::segment_manager_t;

static_assert(sizeof(segment_t) == 8, "Exactly 8 bytes per 'segment_t' are required for tests.");

TEST_SPACE()
{

std::size_t segment_count(segment_manager_t const& manager) noexcept
{
    auto counter = std::size_t(0);
    for (auto segment = manager.begin(); segment != manager.end(); segment = segment->next())
    {
        counter += 1;
    }
    return counter;
}

void segment_defragmentation(segment_manager_t& manager) noexcept
{
    for (auto segment = manager.begin(); segment != manager.end(); segment = segment->next())
    {
        if (!segment->is_used)
        {
            manager.extend_segment(segment->memory());
        }
    }
}

} // TEST_SPACE

TEST(TestLibrary, TestInit)
{
    // [8 + 24]
    char memory[32];
    auto manager = segment_manager_t(memory, sizeof(memory));

    static const auto begin_segment_size = manager.bytes() - sizeof(segment_t);


    ASSERT("manager.bytes", manager.bytes() == sizeof(memory));
    ASSERT("manager.segment_count", segment_count(manager) == 1);

    auto begin_segment = manager.begin();
    EXPECT("manager.begin_segment.size", begin_segment->size == begin_segment_size);
    EXPECT("manager.begin_segment.is_used", begin_segment->is_used == false);
}

TEST(TestLibrary, TestOverSizeSegment)
{
    // [8 + 24]
    char memory[32];
    auto manager = segment_manager_t(memory, sizeof(memory));

    static const auto over_size = manager.bytes();


    auto over_size_memory = manager.add_segment(over_size);
    EXPECT("manager.add_segment.over_size_segment", over_size_memory == nullptr);
    EXPECT("manager.add_segment.over_size_segment.segment_count", segment_count(manager) == 1);
}

TEST(TestLibrary, TestMaxSizeSegment)
{
    // [8 + 24]
    char memory[32];
    auto manager = segment_manager_t(memory, sizeof(memory));

    static const auto max_size = manager.bytes() - sizeof(segment_t);


    auto max_size_memory = manager.add_segment(max_size);
    ASSERT("manager.add_segment.max_size_segment", max_size_memory != nullptr);
    EXPECT("manager.add_segment.max_size_segment.segment_count", segment_count(manager) == 1);

    auto max_size_segment = segment_t::segment(max_size_memory);
    EXPECT("manager.add_segment.max_size_segment.size", max_size_segment->size == max_size);
    EXPECT("manager.add_segment.max_size_segment.is_used", max_size_segment->is_used == true);
    EXPECT("manager.add_segment.max_size_segment.memory", max_size_segment->memory() == max_size_memory);


    manager.remove_segment(max_size_memory);
    EXPECT("manager.remove_segment.max_size_segment.size", max_size_segment->size == max_size);
    EXPECT("manager.remove_segment.max_size_segment.is_used", max_size_segment->is_used == false);
    EXPECT("manager.remove_segment.max_size_segment.segment_count", segment_count(manager) == 1);
}

TEST(TestLibrary, TestCommon)
{
    // [8 + 0] [8 + 16]
    char memory[32];
    auto manager = segment_manager_t(memory, sizeof(memory));

    static const auto zero_size = 0;
    static const auto neighbour_segment_size = manager.bytes() - (sizeof(segment_t)) - (sizeof(segment_t) + zero_size);


    auto zero_size_memory = manager.add_segment(zero_size);
    ASSERT("manager.add_segment.zero_size_segment", zero_size_memory != nullptr);
    EXPECT("manager.add_segment.zero_size_segment.segment_count", segment_count(manager) == 2);

    auto zero_size_segment = segment_t::segment(zero_size_memory);
    EXPECT("manager.add_segment.zero_size_segment.size", zero_size_segment->size == zero_size);
    EXPECT("manager.add_segment.zero_size_segment.is_used", zero_size_segment->is_used == true);
    EXPECT("manager.add_segment.zero_size_segment.memory", zero_size_segment->memory() == zero_size_memory);

    auto neighbour_segment = zero_size_segment->next();
    EXPECT("manager.add_segment.neighbour_segment.size", neighbour_segment->size == neighbour_segment_size);
    EXPECT("manager.add_segment.neighbour_segment.is_used", neighbour_segment->is_used == false);
    EXPECT("manager.add_segment.neighbour_segment.memory", neighbour_segment->memory() == reinterpret_cast<char*>(zero_size_memory) + (sizeof(segment_t) + zero_size));


    manager.remove_segment(zero_size_memory);
    EXPECT("manager.remove_segment.zero_size_segment.size", neighbour_segment->size == neighbour_segment_size);
    EXPECT("manager.remove_segment.zero_size_segment.is_used", zero_size_segment->is_used == false);
    EXPECT("manager.remove_segment.zero_size_segment.segment_count", segment_count(manager) == 2);

    segment_defragmentation(manager);
    EXPECT("manager.defragmentation.zero_size_segment.segment_count", segment_count(manager) == 1);
}

TEST(TestLibrary, TestComplex01)
{
    // [8 + 1] [8 + 4] [8 + 2]
    char memory[31];
    auto manager = segment_manager_t(memory, sizeof(memory));

    static const auto one_size = 1;
    static const auto two_size = 2;
    static const auto four_size = 4;


    auto one_size_memory = manager.add_segment(one_size);
    ASSERT("manager.add_segment.one_size_segment", one_size_memory != nullptr);
    EXPECT("manager.add_segment.one_size_segment.segment_count", segment_count(manager) == 2);

    auto one_size_segment = segment_t::segment(one_size_memory);
    EXPECT("manager.add_segment.one_size_segment.size", one_size_segment->size == one_size);
    EXPECT("manager.add_segment.one_size_segment.is_used", one_size_segment->is_used == true);
    EXPECT("manager.add_segment.one_size_segment.memory", one_size_segment->memory() == one_size_memory);

    auto four_size_memory = manager.add_segment(four_size);
    ASSERT("manager.add_segment.four_size_segment", four_size_memory != nullptr);
    EXPECT("manager.add_segment.four_size_segment.segment_count", segment_count(manager) == 3);

    auto four_size_segment = segment_t::segment(four_size_memory);
    EXPECT("manager.add_segment.four_size_segment.size", four_size_segment->size == four_size);
    EXPECT("manager.add_segment.four_size_segment.is_used", four_size_segment->is_used == true);
    EXPECT("manager.add_segment.four_size_segment.memory", four_size_segment->memory() == four_size_memory);

    auto two_size_memory = manager.add_segment(two_size);
    ASSERT("manager.add_segment.two_size_segment", two_size_memory != nullptr);
    EXPECT("manager.add_segment.two_size_segment.segment_count", segment_count(manager) == 3);

    auto two_size_segment = segment_t::segment(two_size_memory);
    EXPECT("manager.add_segment.two_size_segment.size", two_size_segment->size == two_size);
    EXPECT("manager.add_segment.two_size_segment.is_used", two_size_segment->is_used == true);
    EXPECT("manager.add_segment.two_size_segment.memory", two_size_segment->memory() == two_size_memory);


    manager.remove_segment(one_size_memory);
    EXPECT("manager.remove_segment.one_size_segment.size", one_size_segment->size == one_size);
    EXPECT("manager.remove_segment.one_size_segment.is_used", one_size_segment->is_used == false);
    EXPECT("manager.remove_segment.one_size_segment.segment_count", segment_count(manager) == 3);

    segment_defragmentation(manager);
    EXPECT("manager.defragmentation.one_size_segment.segment_count", segment_count(manager) == 3);

    manager.remove_segment(two_size_memory);
    EXPECT("manager.remove_segment.two_size_segment.size", two_size_segment->size == two_size);
    EXPECT("manager.remove_segment.two_size_segment.is_used", two_size_segment->is_used == false);
    EXPECT("manager.remove_segment.two_size_segment.segment_count", segment_count(manager) == 3);

    segment_defragmentation(manager);
    EXPECT("manager.defragmentation.two_size_segment.segment_count", segment_count(manager) == 3);

    manager.remove_segment(four_size_memory);
    EXPECT("manager.remove_segment.four_size_segment.size", four_size_segment->size == four_size);
    EXPECT("manager.remove_segment.four_size_segment.is_used", four_size_segment->is_used == false);
    EXPECT("manager.remove_segment.four_size_segment.segment_count", segment_count(manager) == 3);

    segment_defragmentation(manager);
    EXPECT("manager.defragmentation.four_size_segment.segment_count", segment_count(manager) == 1);
}

// TEST(TestLibrary, TestComplex02)
// {
//      char memory[38];
//      auto manager = segment_manager_t(memory, sizeof(memory));


//     {
//         auto memory01 = manager.add_segment(1);
//         auto memory04 = manager.add_segment(4);
//         auto memory02 = manager.add_segment(2);

//         manager.remove_segment(memory01);
//         manager.remove_segment(memory02);
//         manager.remove_segment(memory04);

//         segment_defragmentation(manager);
//     }
// }

// TEST(TestLibrary, TestComplex03)
// {
//      char memory[39];
//      auto manager = segment_manager_t(memory, sizeof(memory));


//     {
//         auto memory01 = manager.add_segment(1);
//         auto memory04 = manager.add_segment(4);
//         auto memory02 = manager.add_segment(2);

//         manager.remove_segment(memory01);
//         manager.remove_segment(memory02);
//         manager.remove_segment(memory04);

//         segment_defragmentation(manager);
//     }
// }

/*
TEST(TestLibrary, TestComplex11)
{
    char memory[32];
    auto manager = segment_manager_t(memory, sizeof(memory));


    {
        auto memory08 = manager.add_segment(8);
        auto memory04 = manager.add_segment(4);
        auto memory02 = manager.add_segment(2);

        manager.extend_segment(memory08, 2);
        manager.remove_segment(memory04);
        manager.extend_segment(memory02, 2);

        manager.extend_segment(memory08, 2);
        manager.extend_segment(memory08, sizeof(segment_t));

        manager.extend_segment(memory02, 6);
        manager.remove_segment(memory08);
        manager.remove_segment(memory02);

        segment_defragmentation(manager);
    }
}

TEST(TestLibrary, TestComplex12)
{
    char memory[31];
    auto manager = segment_manager_t(memory, sizeof(memory));


    {
        auto memory08 = manager.add_segment(8);
        auto memory04 = manager.add_segment(4);
        auto memory02 = manager.add_segment(2);

        manager.extend_segment(memory08, 2);
        manager.remove_segment(memory04);
        manager.extend_segment(memory02, 2);

        manager.extend_segment(memory08, sizeof(segment_t));
        manager.extend_segment(memory08, 2);


        manager.extend_segment(memory02, 6);
        manager.remove_segment(memory08);
        manager.remove_segment(memory02);

        segment_defragmentation(manager);
    }
}

TEST(TestLibrary, TestComplex13)
{
    char memory[32];
    auto manager = segment_manager_t(memory, sizeof(memory));


    {
        auto memory08 = manager.add_segment(8);
        auto memory04 = manager.add_segment(4);
        auto memory02 = manager.add_segment(2);

        manager.extend_segment(memory08, 2);
        manager.remove_segment(memory04);
        manager.extend_segment(memory02, 2);

        manager.extend_segment(memory08, sizeof(segment_t) + 2);

        manager.extend_segment(memory02, 6);
        manager.remove_segment(memory08);
        manager.remove_segment(memory02);

        segment_defragmentation(manager);
    }
}

TEST(TestLibrary, TestComplex2)
{
    char memory[40];
    auto manager = segment_manager_t(memory, sizeof(memory));


    {
        auto smemory01 = manager.add_segment(1);
        auto memory02 = manager.add_segment(2);
        manager.remove_segment(smemory01);
        auto memory04 = manager.add_segment(4);
        auto memory11 = manager.add_segment(1);
        manager.remove_segment(memory02);
        auto memory00 = manager.add_segment(0);
        manager.remove_segment(memory11);
        manager.remove_segment(memory00);
        manager.remove_segment(memory04);
    }
}
*/
