#include <EightmoryTestingBase.hpp>

#include <vector> // vector
#include <utility> // pair

using eightmory::segment_t;
using eightmory::segment_manager_t;

using eightmory::align_up;
using eightmory::is_aligned;

using segment_trace_t = std::vector<std::pair<std::size_t, bool>>;

static_assert(sizeof(segment_t) == EIGHTMORY_SEGMENT_ALIGN, "Exactly 8 bytes per 'segment_t' are required for tests.");

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

[[maybe_unused]] segment_t* get_segment(segment_manager_t& manager, std::size_t index) noexcept
{
    auto curr_index = std::size_t(0);
    for (auto segment = manager.begin(); segment != manager.end(); segment = segment->next())
    {
        if (curr_index == index)
        {
            return segment;
        }
        else
        {
            ++curr_index;
        }
    }
    return nullptr;
}

segment_trace_t segment_trace(segment_manager_t& manager)
{
    segment_trace_t trace;
    for (auto segment = manager.begin(); segment != manager.end(); segment = segment->next())
    {
        trace.emplace_back((std::size_t)segment->size, (bool)segment->is_used);
    }
    return trace;
}

} // TEST_SPACE

TEST(TestLibrary, TestValidManager)
{
    // (8 + 24)
    char memory[32];
    auto valid_manager = segment_manager_t(memory, sizeof(memory));

    static const auto begin_segment_size = valid_manager.bytes() - sizeof(segment_t);


    EXPECT("valid_manager.trace", segment_trace(valid_manager) == segment_trace_t{{24, false}});

    ASSERT("valid_manager.bytes", valid_manager.bytes() == sizeof(memory));
    ASSERT("valid_manager.segment_count", segment_count(valid_manager) == 1);


    auto begin_segment = valid_manager.begin();
    EXPECT("valid_manager.begin_segment.size", begin_segment->size == begin_segment_size);
    EXPECT("valid_manager.begin_segment.is_used", begin_segment->is_used == false);
}

TEST(TestLibrary, TestMinimalSizeManager)
{
    // (8 + 0)
    char memory[8];
    auto minimal_size_manager = segment_manager_t(memory, sizeof(memory));

    static const auto begin_segment_size = minimal_size_manager.bytes() - sizeof(segment_t);


    EXPECT("invalid_size_manager.trace", segment_trace(minimal_size_manager) == segment_trace_t{{0, false}});

    ASSERT("minimal_size_manager.bytes", minimal_size_manager.bytes() == sizeof(memory));
    ASSERT("minimal_size_manager.segment_count", segment_count(minimal_size_manager) == 1);


    auto begin_segment = minimal_size_manager.begin();
    EXPECT("minimal_size_manager.begin_segment.size", begin_segment->size == begin_segment_size);
    EXPECT("minimal_size_manager.begin_segment.is_used", begin_segment->is_used == false);
}

TEST(TestLiibrary, TestInvalidManager)
{
    // (7) same as (8 - 1)
    char memory[7];
    auto invalid_size_manager = segment_manager_t(memory, sizeof(memory));


    EXPECT("invalid_size_manager.trace", segment_trace(invalid_size_manager) == segment_trace_t{});

    ASSERT("invalid_size_manager.begin", invalid_size_manager.begin() == nullptr);
    ASSERT("invalid_size_manager.end", invalid_size_manager.end() == nullptr);
    ASSERT("invalid_size_manager.bytes", invalid_size_manager.bytes() == 0);
    ASSERT("invalid_size_manager.segment_count", segment_count(invalid_size_manager) == 0);
}

TEST(TestLibrary, TestOverSizeSegment)
{
    // (8 + 24)
    char memory[32];
    auto manager = segment_manager_t(memory, sizeof(memory));

    static const auto over_size = manager.bytes();


    EXPECT("manager.trace", segment_trace(manager) == segment_trace_t{{24, false}});

    auto over_size_memory = manager.add_segment(over_size);
    EXPECT("manager.add_segment.over_size_segment", over_size_memory == nullptr);
    EXPECT("manager.add_segment.over_size_segment.segment_count", segment_count(manager) == 1);

    EXPECT("manager.trace.over_size_segment", segment_trace(manager) == segment_trace_t{{24, false}});
}

TEST(TestLibrary, TestMaxSizeSegment)
{
    // (8 + 24)
    char memory[32];
    auto manager = segment_manager_t(memory, sizeof(memory));

    static const auto max_size = manager.bytes() - sizeof(segment_t);


    EXPECT("manager.trace", segment_trace(manager) == segment_trace_t{{24, false}});

    // [8 + 24]
    auto max_size_memory = manager.add_segment(max_size);
    ASSERT("manager.add_segment.max_size_segment", max_size_memory != nullptr);
    EXPECT("manager.add_segment.max_size_segment.segment_count", segment_count(manager) == 1);

    auto max_size_segment = segment_t::segment(max_size_memory);
    EXPECT("manager.add_segment.max_size_segment.size", max_size_segment->size == max_size);
    EXPECT("manager.add_segment.max_size_segment.is_used", max_size_segment->is_used == true);
    EXPECT("manager.add_segment.max_size_segment.memory", max_size_segment->memory() == max_size_memory);

    EXPECT("manager.trace.max_size_segment", segment_trace(manager) == segment_trace_t{{24, true}});


    // (8 + 24)
    manager.remove_segment(max_size_memory);
    EXPECT("manager.remove_segment.max_size_segment.size", max_size_segment->size == max_size);
    EXPECT("manager.remove_segment.max_size_segment.is_used", max_size_segment->is_used == false);
    EXPECT("manager.remove_segment.max_size_segment.segment_count", segment_count(manager) == 1);

    EXPECT("manager.trace.max_size_segment", segment_trace(manager) == segment_trace_t{{24, false}});
}

TEST(TestLibrary, TestCommon)
{
    // (8 + 24)
    char memory[32];
    auto manager = segment_manager_t(memory, sizeof(memory));

    static const auto zero_size = 0;
    static const auto neighbour_segment_size = manager.bytes() - (sizeof(segment_t)) - (sizeof(segment_t) + zero_size);


    EXPECT("manager.trace", segment_trace(manager) == segment_trace_t{{24, false}});

    // [8 + 0] (8 + 16)
    auto zero_size_memory = manager.add_segment(zero_size);
    ASSERT("manager.add_segment.zero_size_segment", zero_size_memory != nullptr);
    EXPECT("manager.add_segment.zero_size_segment.segment_count", segment_count(manager) == 2);

    auto zero_size_segment = segment_t::segment(zero_size_memory);
    EXPECT("manager.add_segment.zero_size_segment.size", zero_size_segment->size == zero_size);
    EXPECT("manager.add_segment.zero_size_segment.is_used", zero_size_segment->is_used == true);
    EXPECT("manager.add_segment.zero_size_segment.memory", zero_size_segment->memory() == zero_size_memory);

    EXPECT("manager.trace.zero_size_segment", segment_trace(manager) == segment_trace_t{{0, true}, {16, false}});

    auto neighbour_segment = zero_size_segment->next();
    EXPECT("manager.add_segment.neighbour_segment.size", neighbour_segment->size == neighbour_segment_size);
    EXPECT("manager.add_segment.neighbour_segment.is_used", neighbour_segment->is_used == false);
    EXPECT("manager.add_segment.neighbour_segment.memory", neighbour_segment->memory() == reinterpret_cast<char*>(zero_size_memory) + (sizeof(segment_t) + zero_size));


    // (8 + 0) (8 + 16)
    manager.remove_segment(zero_size_memory);
    EXPECT("manager.remove_segment.zero_size_segment.size", neighbour_segment->size == neighbour_segment_size);
    EXPECT("manager.remove_segment.zero_size_segment.is_used", zero_size_segment->is_used == false);
    EXPECT("manager.remove_segment.zero_size_segment.segment_count", segment_count(manager) == 2);

    EXPECT("manager.trace.zero_size_segment", segment_trace(manager) == segment_trace_t{{0, false}, {16, false}});


    // (8 + 24)
    segment_defragmentation(manager);
    EXPECT("manager.defragmentation.zero_size_segment.segment_count", segment_count(manager) == 1);

    auto begin_segment = manager.begin();
    EXPECT("manager.begin_segment.size", begin_segment->size ==  sizeof(memory) - sizeof(segment_t));
    EXPECT("manager.begin_segment.is_used", begin_segment->is_used == false);

    EXPECT("manager.trace.begin_segment", segment_trace(manager) == segment_trace_t{{24, false}});
}

TEST(TestLibrary, TestLazyDefragmentationLowerBound)
{
    // (8 + 16)
    char memory[24];
    auto manager = segment_manager_t(memory, sizeof(memory));

    static const auto eight_size = 8;
    static const auto ten_size = 10;
    static const auto real_ten_size = ten_size + 6;


    EXPECT("manager.trace", segment_trace(manager) == segment_trace_t{{16, false}});

    // [8 + 8] (8 + 0)
    auto eight_size_memory = manager.add_segment(eight_size);
    ASSERT("manager.add_segment.eight_size_segment", eight_size_memory != nullptr);
    EXPECT("manager.add_segment.eight_size_segment.segment_count", segment_count(manager) == 2);

    auto eight_size_segment = segment_t::segment(eight_size_memory);
    EXPECT("manager.add_segment.eight_size_segment.size", eight_size_segment->size == eight_size);
    EXPECT("manager.add_segment.eight_size_segment.is_used", eight_size_segment->is_used == true);
    EXPECT("manager.add_segment.eight_size_segment.memory", eight_size_segment->memory() == eight_size_memory);

    EXPECT("manager.trace.eight_size_segment", segment_trace(manager) == segment_trace_t{{8, true}, {0, false}});


    // (8 + 8) (8 + 0)
    manager.remove_segment(eight_size_memory);
    EXPECT("manager.remove_segment.eight_size_segment.size", eight_size_segment->size == eight_size);
    EXPECT("manager.remove_segment.eight_size_segment.is_used", eight_size_segment->is_used == false);
    EXPECT("manager.remove_segment.eight_size_segment.segment_count", segment_count(manager) == 2);

    EXPECT("manager.trace.eight_size_segment", segment_trace(manager) == segment_trace_t{{8, false}, {0, false}});


    // [8 + 16]
    auto ten_size_memory = manager.add_segment(ten_size);
    ASSERT("manager.add_segment.ten_size_segment", ten_size_memory != nullptr);
    EXPECT("manager.add_segment.ten_size_segment.segment_count", segment_count(manager) == 1);

    auto ten_size_segment = segment_t::segment(ten_size_memory);
    EXPECT("manager.add_segment.ten_size_segment.size", ten_size_segment->size == real_ten_size);
    EXPECT("manager.add_segment.ten_size_segment.is_used", ten_size_segment->is_used == true);
    EXPECT("manager.add_segment.ten_size_segment.memory", ten_size_segment->memory() == ten_size_memory);

    EXPECT("manager.trace.ten_size_segment", segment_trace(manager) == segment_trace_t{{16, true}});
}

TEST(TestLibrary, TestLazyDefragmentationMidBound)
{
    // (8 + 17)
    char memory[25];
    auto manager = segment_manager_t(memory, sizeof(memory));

    static const auto eight_size = 8;
    static const auto ten_size = 10;
    static const auto real_ten_size = ten_size + 7;


    EXPECT("manager.trace", segment_trace(manager) == segment_trace_t{{17, false}});

    // [8 + 8] (8 + 1)
    auto eight_size_memory = manager.add_segment(eight_size);
    ASSERT("manager.add_segment.eight_size_segment", eight_size_memory != nullptr);
    EXPECT("manager.add_segment.eight_size_segment.segment_count", segment_count(manager) == 2);

    auto eight_size_segment = segment_t::segment(eight_size_memory);
    EXPECT("manager.add_segment.eight_size_segment.size", eight_size_segment->size == eight_size);
    EXPECT("manager.add_segment.eight_size_segment.is_used", eight_size_segment->is_used == true);
    EXPECT("manager.add_segment.eight_size_segment.memory", eight_size_segment->memory() == eight_size_memory);

    EXPECT("manager.eight_size_segment.trace", segment_trace(manager) == segment_trace_t{{8, true}, {1, false}});


    // (8 + 8) (8 + 1)
    manager.remove_segment(eight_size_memory);
    EXPECT("manager.remove_segment.eight_size_segment.size", eight_size_segment->size == eight_size);
    EXPECT("manager.remove_segment.eight_size_segment.is_used", eight_size_segment->is_used == false);
    EXPECT("manager.remove_segment.eight_size_segment.segment_count", segment_count(manager) == 2);

    EXPECT("manager.eight_size_segment.trace", segment_trace(manager) == segment_trace_t{{8, false}, {1, false}});


    // [8 + 17]
    auto ten_size_memory = manager.add_segment(ten_size);
    ASSERT("manager.add_segment.ten_size_segment", ten_size_memory != nullptr);
    EXPECT("manager.add_segment.ten_size_segment.segment_count", segment_count(manager) == 1);

    auto ten_size_segment = segment_t::segment(ten_size_memory);
    EXPECT("manager.add_segment.ten_size_segment.size", ten_size_segment->size == real_ten_size);
    EXPECT("manager.add_segment.ten_size_segment.is_used", ten_size_segment->is_used == true);
    EXPECT("manager.add_segment.ten_size_segment.memory", ten_size_segment->memory() == ten_size_memory);

    EXPECT("manager.ten_size_segment.trace", segment_trace(manager) == segment_trace_t{{17, true}});
}

TEST(TestLibrary, TestLazyDefragmentationUpperBound)
{
    // (8 + 18)
    char memory[26];
    auto manager = segment_manager_t(memory, sizeof(memory));

    static const auto eight_size = 8;
    static const auto ten_size = 10;


    EXPECT("manager.trace", segment_trace(manager) == segment_trace_t{{18, false}});

    // [8 + 8] (8 + 2)
    auto eight_size_memory = manager.add_segment(eight_size);
    ASSERT("manager.add_segment.eight_size_segment", eight_size_memory != nullptr);
    EXPECT("manager.add_segment.eight_size_segment.segment_count", segment_count(manager) == 2);

    auto eight_size_segment = segment_t::segment(eight_size_memory);
    EXPECT("manager.add_segment.eight_size_segment.size", eight_size_segment->size == eight_size);
    EXPECT("manager.add_segment.eight_size_segment.is_used", eight_size_segment->is_used == true);
    EXPECT("manager.add_segment.eight_size_segment.memory", eight_size_segment->memory() == eight_size_memory);

    EXPECT("manager.eight_size_segment.trace", segment_trace(manager) == segment_trace_t{{8, true}, {2, false}});


    // (8 + 8) (8 + 2)
    manager.remove_segment(eight_size_memory);
    EXPECT("manager.remove_segment.eight_size_segment.size", eight_size_segment->size == eight_size);
    EXPECT("manager.remove_segment.eight_size_segment.is_used", eight_size_segment->is_used == false);
    EXPECT("manager.remove_segment.eight_size_segment.segment_count", segment_count(manager) == 2);

    EXPECT("manager.eight_size_segment.trace", segment_trace(manager) == segment_trace_t{{8, false}, {2, false}});


    // [8 + 10] (8 + 0)
    auto ten_size_memory = manager.add_segment(ten_size);
    ASSERT("manager.add_segment.ten_size_segment", ten_size_memory != nullptr);
    EXPECT("manager.add_segment.ten_size_segment.segment_count", segment_count(manager) == 2);

    auto ten_size_segment = segment_t::segment(ten_size_memory);
    EXPECT("manager.add_segment.ten_size_segment.size", ten_size_segment->size == ten_size);
    EXPECT("manager.add_segment.ten_size_segment.is_used", ten_size_segment->is_used == true);
    EXPECT("manager.add_segment.ten_size_segment.memory", ten_size_segment->memory() == ten_size_memory);

    EXPECT("manager.ten_size_segment.trace", segment_trace(manager) == segment_trace_t{{10, true}, {0, false}});
}

TEST(TestLibrary, TestLazyDefragmentationZero)
{
    // (8 + 9)
    char memory[17];
    auto manager = segment_manager_t(memory, sizeof(memory));

    static const auto zero_size = 0;


    EXPECT("manager.trace", segment_trace(manager) == segment_trace_t{{9, false}});

    // [8 + 0] (8 + 1)
    auto zero_size_memory = manager.add_segment(zero_size);
    ASSERT("manager.add_segment.zero_size_segment", zero_size_memory != nullptr);
    EXPECT("manager.add_segment.zero_size_segment.segment_count", segment_count(manager) == 2);

    auto zero_size_segment = segment_t::segment(zero_size_memory);
    EXPECT("manager.add_segment.zero_size_segment.size", zero_size_segment->size == zero_size);
    EXPECT("manager.add_segment.zero_size_segment.is_used", zero_size_segment->is_used == true);
    EXPECT("manager.add_segment.zero_size_segment.memory", zero_size_segment->memory() == zero_size_memory);

    EXPECT("manager.trace.zero_size_segment", segment_trace(manager) == segment_trace_t{{0, true}, {1, false}});


    // (8 + 0) (8 + 1)
    manager.remove_segment(zero_size_memory);
    EXPECT("manager.remove_segment.zero_size_segment.size", zero_size_segment->size == zero_size);
    EXPECT("manager.remove_segment.zero_size_segment.is_used", zero_size_segment->is_used == false);
    EXPECT("manager.remove_segment.zero_size_segment.segment_count", segment_count(manager) == 2);

    EXPECT("manager.trace.zero_size_segment", segment_trace(manager) == segment_trace_t{{0, false}, {1, false}});


    // [8 + 0] (8 + 1)
    auto zero_size_memory2 = manager.add_segment(zero_size);
    ASSERT("manager.add_segment.zero_size_segment2", zero_size_memory2 != nullptr);
    EXPECT("manager.add_segment.zero_size_segment2.segment_count", segment_count(manager) == 2);

    auto zero_size_segment2 = segment_t::segment(zero_size_memory2);
    EXPECT("manager.add_segment.zero_size_segment2.size", zero_size_segment2->size == zero_size);
    EXPECT("manager.add_segment.zero_size_segment2.is_used", zero_size_segment2->is_used == true);
    EXPECT("manager.add_segment.zero_size_segment2.memory", zero_size_segment2->memory() == zero_size_memory2);

    EXPECT("manager.trace.zero_size_segment2", segment_trace(manager) == segment_trace_t{{0, true}, {1, false}});
}

TEST(TestLibrary, TestLazyDefragmentationZeroOne)
{
    // (8 + 9)
    char memory[17];
    auto manager = segment_manager_t(memory, sizeof(memory));

    static const auto zero_size = 0;
    static const auto one_size = 1;


    EXPECT("manager.trace", segment_trace(manager) == segment_trace_t{{9, false}});

    // [8 + 0] (8 + 1)
    auto zero_size_memory = manager.add_segment(zero_size);
    ASSERT("manager.add_segment.zero_size_segment", zero_size_memory != nullptr);
    EXPECT("manager.add_segment.zero_size_segment.segment_count", segment_count(manager) == 2);

    auto zero_size_segment = segment_t::segment(zero_size_memory);
    EXPECT("manager.add_segment.zero_size_segment.size", zero_size_segment->size == zero_size);
    EXPECT("manager.add_segment.zero_size_segment.is_used", zero_size_segment->is_used == true);
    EXPECT("manager.add_segment.zero_size_segment.memory", zero_size_segment->memory() == zero_size_memory);

    EXPECT("manager.trace.zero_size_segment", segment_trace(manager) == segment_trace_t{{0, true}, {1, false}});


    // (8 + 0) (8 + 1)
    manager.remove_segment(zero_size_memory);
    EXPECT("manager.remove_segment.zero_size_segment.size", zero_size_segment->size == zero_size);
    EXPECT("manager.remove_segment.zero_size_segment.is_used", zero_size_segment->is_used == false);
    EXPECT("manager.remove_segment.zero_size_segment.segment_count", segment_count(manager) == 2);

    EXPECT("manager.trace.zero_size_segment", segment_trace(manager) == segment_trace_t{{0, false}, {1, false}});


    // [8 + 1] (8 + 0)
    auto one_size_memory = manager.add_segment(one_size);
    ASSERT("manager.add_segment.one_size_segment", one_size_memory != nullptr);
    EXPECT("manager.add_segment.one_size_segment.segment_count", segment_count(manager) == 2);

    auto one_size_segment = segment_t::segment(one_size_memory);
    EXPECT("manager.add_segment.one_size_segment.size", one_size_segment->size == one_size);
    EXPECT("manager.add_segment.one_size_segment.is_used", one_size_segment->is_used == true);
    EXPECT("manager.add_segment.one_size_segment.memory", one_size_segment->memory() == one_size_memory);

    EXPECT("manager.trace.one_size_segment", segment_trace(manager) == segment_trace_t{{1, true}, {0, false}});
}

TEST(TestLibrary, TestComplexLowerBound)
{
    // (8 + 23)
    char memory[31];
    auto manager = segment_manager_t(memory, sizeof(memory));

    static const auto one_size = 1;
    static const auto four_size = 4;
    static const auto two_size = 2;


    EXPECT("manager.trace", segment_trace(manager) == segment_trace_t{{23, false}});

    // [8 + 1] (8 + 14)
    auto one_size_memory = manager.add_segment(one_size);
    ASSERT("manager.add_segment.one_size_segment", one_size_memory != nullptr);
    EXPECT("manager.add_segment.one_size_segment.segment_count", segment_count(manager) == 2);

    auto one_size_segment = segment_t::segment(one_size_memory);
    EXPECT("manager.add_segment.one_size_segment.size", one_size_segment->size == one_size);
    EXPECT("manager.add_segment.one_size_segment.is_used", one_size_segment->is_used == true);
    EXPECT("manager.add_segment.one_size_segment.memory", one_size_segment->memory() == one_size_memory);

    EXPECT("manager.trace.one_size_segment", segment_trace(manager) == segment_trace_t{{1, true}, {14, false}});


    // [8 + 1] [8 + 4] (8 + 2)
    auto four_size_memory = manager.add_segment(four_size);
    ASSERT("manager.add_segment.four_size_segment", four_size_memory != nullptr);
    EXPECT("manager.add_segment.four_size_segment.segment_count", segment_count(manager) == 3);

    auto four_size_segment = segment_t::segment(four_size_memory);
    EXPECT("manager.add_segment.four_size_segment.size", four_size_segment->size == four_size);
    EXPECT("manager.add_segment.four_size_segment.is_used", four_size_segment->is_used == true);
    EXPECT("manager.add_segment.four_size_segment.memory", four_size_segment->memory() == four_size_memory);

    EXPECT("manager.trace.four_size_segment", segment_trace(manager) == segment_trace_t{{1, true}, {4, true}, {2, false}});


    // [8 + 1] [8 + 4] [8 + 2]
    auto two_size_memory = manager.add_segment(two_size);
    ASSERT("manager.add_segment.two_size_segment", two_size_memory != nullptr);
    EXPECT("manager.add_segment.two_size_segment.segment_count", segment_count(manager) == 3);

    auto two_size_segment = segment_t::segment(two_size_memory);
    EXPECT("manager.add_segment.two_size_segment.size", two_size_segment->size == two_size);
    EXPECT("manager.add_segment.two_size_segment.is_used", two_size_segment->is_used == true);
    EXPECT("manager.add_segment.two_size_segment.memory", two_size_segment->memory() == two_size_memory);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{1, true}, {4, true}, {2, true}});


    // (8 + 1) [8 + 4] [8 + 2]
    manager.remove_segment(one_size_memory);
    EXPECT("manager.remove_segment.one_size_segment.size", one_size_segment->size == one_size);
    EXPECT("manager.remove_segment.one_size_segment.is_used", one_size_segment->is_used == false);
    EXPECT("manager.remove_segment.one_size_segment.segment_count", segment_count(manager) == 3);

    EXPECT("manager.trace.one_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {4, true}, {2, true}});

    // (8 + 1) [8 + 4] [8 + 2]
    segment_defragmentation(manager);
    EXPECT("manager.defragmentation.one_size_segment.segment_count", segment_count(manager) == 3);

    EXPECT("manager.trace.one_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {4, true}, {2, true}});


    // (8 + 1) [8 + 4] (8 + 2)
    manager.remove_segment(two_size_memory);
    EXPECT("manager.remove_segment.two_size_segment.size", two_size_segment->size == two_size);
    EXPECT("manager.remove_segment.two_size_segment.is_used", two_size_segment->is_used == false);
    EXPECT("manager.remove_segment.two_size_segment.segment_count", segment_count(manager) == 3);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {4, true}, {2, false}});

    // (8 + 1) [8 + 4] (8 + 2)
    segment_defragmentation(manager);
    EXPECT("manager.defragmentation.two_size_segment.segment_count", segment_count(manager) == 3);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {4, true}, {2, false}});


    // (8 + 1) (8 + 4) (8 + 2)
    manager.remove_segment(four_size_memory);
    EXPECT("manager.remove_segment.four_size_segment.size", four_size_segment->size == four_size);
    EXPECT("manager.remove_segment.four_size_segment.is_used", four_size_segment->is_used == false);
    EXPECT("manager.remove_segment.four_size_segment.segment_count", segment_count(manager) == 3);

    EXPECT("manager.trace.four_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {4, false}, {2, false}});

    // (8 + 23))
    segment_defragmentation(manager);
    EXPECT("manager.defragmentation.four_size_segment.segment_count", segment_count(manager) == 1);

    EXPECT("manager.trace.four_size_segment", segment_trace(manager) == segment_trace_t{{23, false}});


    // (8 + 23)
    auto begin_segment = manager.begin();
    EXPECT("manager.begin_segment.size", begin_segment->size ==  sizeof(memory) - sizeof(segment_t));
    EXPECT("manager.begin_segment.is_used", begin_segment->is_used == false);
}

TEST(TestLibrary, TestComplexMidBound)
{
    // (8 + 30)
    char memory[38];
    auto manager = segment_manager_t(memory, sizeof(memory));

    static const auto one_size = 1;
    static const auto four_size = 4;
    static const auto two_size = 2;
    static const auto real_two_size = two_size + 7;


    EXPECT("manager.trace", segment_trace(manager) == segment_trace_t{{30, false}});

    // [8 + 1] (8 + 21)
    auto one_size_memory = manager.add_segment(one_size);
    ASSERT("manager.add_segment.one_size_segment", one_size_memory != nullptr);
    EXPECT("manager.add_segment.one_size_segment.segment_count", segment_count(manager) == 2);

    auto one_size_segment = segment_t::segment(one_size_memory);
    EXPECT("manager.add_segment.one_size_segment.size", one_size_segment->size == one_size);
    EXPECT("manager.add_segment.one_size_segment.is_used", one_size_segment->is_used == true);
    EXPECT("manager.add_segment.one_size_segment.memory", one_size_segment->memory() == one_size_memory);

    EXPECT("manager.trace.one_size_segment", segment_trace(manager) == segment_trace_t{{1, true}, {21, false}});


    // [8 + 1] [8 + 4] (8 + 9)
    auto four_size_memory = manager.add_segment(four_size);
    ASSERT("manager.add_segment.four_size_segment", four_size_memory != nullptr);
    EXPECT("manager.add_segment.four_size_segment.segment_count", segment_count(manager) == 3);

    auto four_size_segment = segment_t::segment(four_size_memory);
    EXPECT("manager.add_segment.four_size_segment.size", four_size_segment->size == four_size);
    EXPECT("manager.add_segment.four_size_segment.is_used", four_size_segment->is_used == true);
    EXPECT("manager.add_segment.four_size_segment.memory", four_size_segment->memory() == four_size_memory);

    EXPECT("manager.trace.four_size_segment", segment_trace(manager) == segment_trace_t{{1, true}, {4, true}, {9, false}});


    // [8 + 1] [8 + 4] [8 + 2] (7) - not allowed
    // [8 + 1] [8 + 4] [8 + 9] - allowed
    auto two_size_memory = manager.add_segment(two_size);
    ASSERT("manager.add_segment.two_size_segment", two_size_memory != nullptr);
    EXPECT("manager.add_segment.two_size_segment.segment_count", segment_count(manager) == 3);

    auto two_size_segment = segment_t::segment(two_size_memory);
    EXPECT("manager.add_segment.two_size_segment.size", two_size_segment->size == real_two_size);
    EXPECT("manager.add_segment.two_size_segment.is_used", two_size_segment->is_used == true);
    EXPECT("manager.add_segment.two_size_segment.memory", two_size_segment->memory() == two_size_memory);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{1, true}, {4, true}, {9, true}});


    // (8 + 1) [8 + 4] [8 + 9]
    manager.remove_segment(one_size_memory);
    EXPECT("manager.remove_segment.one_size_segment.size", one_size_segment->size == one_size);
    EXPECT("manager.remove_segment.one_size_segment.is_used", one_size_segment->is_used == false);
    EXPECT("manager.remove_segment.one_size_segment.segment_count", segment_count(manager) == 3);

    EXPECT("manager.trace.one_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {4, true}, {9, true}});

    // (8 + 1) [8 + 4] [8 + 9]
    segment_defragmentation(manager);
    EXPECT("manager.defragmentation.one_size_segment.segment_count", segment_count(manager) == 3);

    EXPECT("manager.trace.one_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {4, true}, {9, true}});


    // (8 + 1) [8 + 4] (8 + 9)
    manager.remove_segment(two_size_memory);
    EXPECT("manager.remove_segment.two_size_segment.size", two_size_segment->size == real_two_size);
    EXPECT("manager.remove_segment.two_size_segment.is_used", two_size_segment->is_used == false);
    EXPECT("manager.remove_segment.two_size_segment.segment_count", segment_count(manager) == 3);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {4, true}, {9, false}});

    // (8 + 1) [8 + 4] (8 + 9)
    segment_defragmentation(manager);
    EXPECT("manager.defragmentation.two_size_segment.segment_count", segment_count(manager) == 3);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {4, true}, {9, false}});


    // (8 + 1) (8 + 4) (8 + 9)
    manager.remove_segment(four_size_memory);
    EXPECT("manager.remove_segment.four_size_segment.size", four_size_segment->size == four_size);
    EXPECT("manager.remove_segment.four_size_segment.is_used", four_size_segment->is_used == false);
    EXPECT("manager.remove_segment.four_size_segment.segment_count", segment_count(manager) == 3);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {4, false}, {9, false}});

    // (8 + 30)
    segment_defragmentation(manager);
    EXPECT("manager.defragmentation.four_size_segment.segment_count", segment_count(manager) == 1);


    // (8 + 30)
    auto begin_segment = manager.begin();
    EXPECT("manager.begin_segment.size", begin_segment->size ==  sizeof(memory) - sizeof(segment_t));
    EXPECT("manager.begin_segment.is_used", begin_segment->is_used == false);

    EXPECT("manager.trace.begin_segment", segment_trace(manager) == segment_trace_t{{30, false}});
}

TEST(TestLibrary, TestComplexUpperBound)
{
    // (8 + 31)
    char memory[39];
    auto manager = segment_manager_t(memory, sizeof(memory));

    static const auto one_size = 1;
    static const auto four_size = 4;
    static const auto two_size = 2;


    EXPECT("manager.trace", segment_trace(manager) == segment_trace_t{{31, false}});

    // [8 + 1] (8 + 22)
    auto one_size_memory = manager.add_segment(one_size);
    ASSERT("manager.add_segment.one_size_segment", one_size_memory != nullptr);
    EXPECT("manager.add_segment.one_size_segment.segment_count", segment_count(manager) == 2);

    auto one_size_segment = segment_t::segment(one_size_memory);
    EXPECT("manager.add_segment.one_size_segment.size", one_size_segment->size == one_size);
    EXPECT("manager.add_segment.one_size_segment.is_used", one_size_segment->is_used == true);
    EXPECT("manager.add_segment.one_size_segment.memory", one_size_segment->memory() == one_size_memory);

    EXPECT("manager.trace.one_size_segment", segment_trace(manager) == segment_trace_t{{1, true}, {22, false}});


    // [8 + 1] [8 + 4] (8 + 10)
    auto four_size_memory = manager.add_segment(four_size);
    ASSERT("manager.add_segment.four_size_segment", four_size_memory != nullptr);
    EXPECT("manager.add_segment.four_size_segment.segment_count", segment_count(manager) == 3);

    auto four_size_segment = segment_t::segment(four_size_memory);
    EXPECT("manager.add_segment.four_size_segment.size", four_size_segment->size == four_size);
    EXPECT("manager.add_segment.four_size_segment.is_used", four_size_segment->is_used == true);
    EXPECT("manager.add_segment.four_size_segment.memory", four_size_segment->memory() == four_size_memory);

    EXPECT("manager.trace.four_size_segment", segment_trace(manager) == segment_trace_t{{1, true}, {4, true}, {10, false}});


    // [8 + 1] [8 + 4] [8 + 2] (8 + 0)
    auto two_size_memory = manager.add_segment(two_size);
    ASSERT("manager.add_segment.two_size_segment", two_size_memory != nullptr);
    EXPECT("manager.add_segment.two_size_segment.segment_count", segment_count(manager) == 4);

    auto two_size_segment = segment_t::segment(two_size_memory);
    EXPECT("manager.add_segment.two_size_segment.size", two_size_segment->size == two_size);
    EXPECT("manager.add_segment.two_size_segment.is_used", two_size_segment->is_used == true);
    EXPECT("manager.add_segment.two_size_segment.memory", two_size_segment->memory() == two_size_memory);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{1, true}, {4, true}, {2, true}, {0, false}});


    // (8 + 1) [8 + 4] [8 + 2] (8 + 0)
    manager.remove_segment(one_size_memory);
    EXPECT("manager.remove_segment.one_size_segment.size", one_size_segment->size == one_size);
    EXPECT("manager.remove_segment.one_size_segment.is_used", one_size_segment->is_used == false);
    EXPECT("manager.remove_segment.one_size_segment.segment_count", segment_count(manager) == 4);

    EXPECT("manager.trace.one_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {4, true}, {2, true}, {0, false}});

    // (8 + 1) [8 + 4] [8 + 2] (8 + 0)
    segment_defragmentation(manager);
    EXPECT("manager.defragmentation.one_size_segment.segment_count", segment_count(manager) == 4);

    EXPECT("manager.trace.one_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {4, true}, {2, true}, {0, false}});


    // (8 + 1) [8 + 4] (8 + 2) (8 + 0)
    manager.remove_segment(two_size_memory);
    EXPECT("manager.remove_segment.two_size_segment.size", two_size_segment->size == two_size);
    EXPECT("manager.remove_segment.two_size_segment.is_used", two_size_segment->is_used == false);
    EXPECT("manager.remove_segment.two_size_segment.segment_count", segment_count(manager) == 4);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {4, true}, {2, false}, {0, false}});

    // (8 + 1) [8 + 4] (8 + 10)
    segment_defragmentation(manager);
    EXPECT("manager.defragmentation.two_size_segment.segment_count", segment_count(manager) == 3);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {4, true}, {10, false}});


    // (8 + 1) (8 + 4) (8 + 10)
    manager.remove_segment(four_size_memory);
    EXPECT("manager.remove_segment.four_size_segment.size", four_size_segment->size == four_size);
    EXPECT("manager.remove_segment.four_size_segment.is_used", four_size_segment->is_used == false);
    EXPECT("manager.remove_segment.four_size_segment.segment_count", segment_count(manager) == 3);

    EXPECT("manager.trace.four_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {4, false}, {10, false}});

    // (8 + 31)
    segment_defragmentation(manager);
    EXPECT("manager.defragmentation.four_size_segment.segment_count", segment_count(manager) == 1);


    // (8 + 31)
    auto begin_segment = manager.begin();
    EXPECT("manager.begin_segment.size", begin_segment->size ==  sizeof(memory) - sizeof(segment_t));
    EXPECT("manager.begin_segment.is_used", begin_segment->is_used == false);

    EXPECT("manager.trace.begin_segment", segment_trace(manager) == segment_trace_t{{31, false}});
}

TEST(TestLibrary, TestComplexExtendAfter)
{
    // (8 + 40)
    char memory[48];
    auto manager = segment_manager_t(memory, sizeof(memory));

    static const auto eight_size = 8;
    static const auto four_size = 4;
    static const auto two_size = 2;
    static const auto six_size = 6;


    EXPECT("manager.trace", segment_trace(manager) == segment_trace_t{{40, false}});

    // [8 + 8] (8 + 24)
    auto eight_size_memory = manager.add_segment(eight_size);
    ASSERT("manager.add_segment.eight_size_segment", eight_size_memory != nullptr);
    EXPECT("manager.add_segment.eight_size_segment.segment_count", segment_count(manager) == 2);

    auto eight_size_segment = segment_t::segment(eight_size_memory);
    EXPECT("manager.add_segment.eight_size_segment.size", eight_size_segment->size == eight_size);
    EXPECT("manager.add_segment.eight_size_segment.is_used", eight_size_segment->is_used == true);
    EXPECT("manager.add_segment.eight_size_segment.memory", eight_size_segment->memory() == eight_size_memory);

    EXPECT("manager.trace.eight_size_segment", segment_trace(manager) == segment_trace_t{{8, true}, {24, false}});

    // [8 + 8] [8 + 4] (8 + 12)
    auto four_size_memory = manager.add_segment(four_size);
    ASSERT("manager.add_segment.four_size_segment", four_size_memory != nullptr);
    EXPECT("manager.add_segment.four_size_segment.segment_count", segment_count(manager) == 3);

    auto four_size_segment = segment_t::segment(four_size_memory);
    EXPECT("manager.add_segment.four_size_segment.size", four_size_segment->size == four_size);
    EXPECT("manager.add_segment.four_size_segment.is_used", four_size_segment->is_used == true);
    EXPECT("manager.add_segment.four_size_segment.memory", four_size_segment->memory() == four_size_memory);

    EXPECT("manager.trace.four_size_segment", segment_trace(manager) == segment_trace_t{{8, true}, {4, true}, {12, false}});

    // [8 + 8] [8 + 4] [8 + 2] (8 + 2)
    auto two_size_memory = manager.add_segment(two_size);
    ASSERT("manager.add_segment.two_size_segment", two_size_memory != nullptr);
    EXPECT("manager.add_segment.two_size_segment.segment_count", segment_count(manager) == 4);

    auto two_size_segment = segment_t::segment(two_size_memory);
    EXPECT("manager.add_segment.two_size_segment.size", two_size_segment->size == two_size);
    EXPECT("manager.add_segment.two_size_segment.is_used", two_size_segment->is_used == true);
    EXPECT("manager.add_segment.two_size_segment.memory", two_size_segment->memory() == two_size_memory);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{8, true}, {4, true}, {2, true}, {2, false}});

    // [8 + 8] [8 + 4] [8 + 2] (8 + 2)
    EXPECT("manager.extend_segment.eight_size_segment", manager.extend_segment(eight_size_memory, two_size) == false);
    EXPECT("manager.extend_segment.eight_size_segment.size", eight_size_segment->size == eight_size);
    EXPECT("manager.remove_segment.eight_size_segment.segment_count", segment_count(manager) == 4);

    EXPECT("manager.trace.eight_size_segment", segment_trace(manager) == segment_trace_t{{8, true}, {4, true}, {2, true}, {2, false}});

    // [8 + 8] (8 + 4) [8 + 2] (8 + 2)
    manager.remove_segment(four_size_memory);
    EXPECT("manager.remove_segment.four_size_segment.size", four_size_segment->size == four_size);
    EXPECT("manager.remove_segment.four_size_segment.is_used", four_size_segment->is_used == false);
    EXPECT("manager.remove_segment.four_size_segment.segment_count", segment_count(manager) == 4);

    EXPECT("manager.trace.four_size_segment", segment_trace(manager) == segment_trace_t{{8, true}, {4, false}, {2, true}, {2, false}});

    // [8 + 8] (8 + 4) [8 + 4] (8 + 0)
    EXPECT("manager.extend_segment.two_size_segment", manager.extend_segment(two_size_memory, two_size) == true);
    EXPECT("manager.extend_segment.two_size_segment.size", two_size_segment->size == two_size + two_size);
    EXPECT("manager.remove_segment.two_size_segment.segment_count", segment_count(manager) == 4);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{8, true}, {4, false}, {4, true}, {0, false}});

    // [8 + 10] (8 + 2) [8 + 4] (8 + 0)
    EXPECT("manager.extend_segment.eight_size_segment", manager.extend_segment(eight_size_memory, two_size) == true);
    EXPECT("manager.extend_segment.eight_size_segment.size", eight_size_segment->size == eight_size + two_size);
    EXPECT("manager.remove_segment.eight_size_segment.segment_count", segment_count(manager) == 4);

    EXPECT("manager.trace.eight_size_segment", segment_trace(manager) == segment_trace_t{{10, true}, {2, false}, {4, true}, {0, false}});

    // [8 + 20] [8 + 4] (8 + 0)
    EXPECT("manager.extend_segment.eight_size_segment", manager.extend_segment(eight_size_memory, sizeof(segment_t)) == true);
    EXPECT("manager.extend_segment.eight_size_segment.size", eight_size_segment->size == eight_size + two_size + sizeof(segment_t) + two_size);
    EXPECT("manager.remove_segment.eight_size_segment.segment_count", segment_count(manager) == 3);

    EXPECT("manager.trace.eight_size_segment", segment_trace(manager) == segment_trace_t{{20, true}, {4, true}, {0, false}});

    // [8 + 20] [8 + 12]
    EXPECT("manager.extend_segment.two_size_segment", manager.extend_segment(two_size_memory, six_size) == true);
    EXPECT("manager.extend_segment.two_size_segment.size", two_size_segment->size == two_size + two_size + sizeof(segment_t));
    EXPECT("manager.remove_segment.two_size_segment.segment_count", segment_count(manager) == 2);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{20, true}, {12, true}});


    // (8 + 20) [8 + 12]
    manager.remove_segment(eight_size_memory);
    EXPECT("manager.remove_segment.eight_size_segment.size", eight_size_segment->size == eight_size + two_size + sizeof(segment_t) + two_size);
    EXPECT("manager.remove_segment.eight_size_segment.is_used", eight_size_segment->is_used == false);
    EXPECT("manager.remove_segment.eight_size_segment.segment_count", segment_count(manager) == 2);

    EXPECT("manager.trace.eight_size_segment", segment_trace(manager) == segment_trace_t{{20, false}, {12, true}});

    // (8 + 20) (8 + 12)
    manager.remove_segment(two_size_memory);
    EXPECT("manager.remove_segment.two_size_segment.size", two_size_segment->size == two_size + two_size + sizeof(segment_t));
    EXPECT("manager.remove_segment.two_size_segment.is_used", two_size_segment->is_used == false);
    EXPECT("manager.remove_segment.two_size_segment.segment_count", segment_count(manager) == 2);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{20, false}, {12, false}});


    // (8 + 40)
    segment_defragmentation(manager);
    EXPECT("manager.defragmentation.segment_count", segment_count(manager) == 1);

    // (8 + 40)
    auto begin_segment = manager.begin();
    EXPECT("manager.begin_segment.size", begin_segment->size ==  sizeof(memory) - sizeof(segment_t));
    EXPECT("manager.begin_segment.is_used", begin_segment->is_used == false);

    EXPECT("manager.trace.begin_segment", segment_trace(manager) == segment_trace_t{{40, false}});
}

TEST(TestLibrary, TestComplexExtendBefore)
{
    // (8 + 40)
    char memory[48];
    auto manager = segment_manager_t(memory, sizeof(memory));

    static const auto eight_size = 8;
    static const auto four_size = 4;
    static const auto two_size = 2;
    static const auto six_size = 6;


    EXPECT("manager.trace", segment_trace(manager) == segment_trace_t{{40, false}});

    // [8 + 8] (8 + 24)
    auto eight_size_memory = manager.add_segment(eight_size);
    ASSERT("manager.add_segment.eight_size_segment", eight_size_memory != nullptr);
    EXPECT("manager.add_segment.eight_size_segment.segment_count", segment_count(manager) == 2);

    auto eight_size_segment = segment_t::segment(eight_size_memory);
    EXPECT("manager.add_segment.eight_size_segment.size", eight_size_segment->size == eight_size);
    EXPECT("manager.add_segment.eight_size_segment.is_used", eight_size_segment->is_used == true);
    EXPECT("manager.add_segment.eight_size_segment.memory", eight_size_segment->memory() == eight_size_memory);

    EXPECT("manager.trace.eight_size_segment", segment_trace(manager) == segment_trace_t{{8, true}, {24, false}});

    // [8 + 8] [8 + 4] (8 + 12)
    auto four_size_memory = manager.add_segment(four_size);
    ASSERT("manager.add_segment.four_size_segment", four_size_memory != nullptr);
    EXPECT("manager.add_segment.four_size_segment.segment_count", segment_count(manager) == 3);

    auto four_size_segment = segment_t::segment(four_size_memory);
    EXPECT("manager.add_segment.four_size_segment.size", four_size_segment->size == four_size);
    EXPECT("manager.add_segment.four_size_segment.is_used", four_size_segment->is_used == true);
    EXPECT("manager.add_segment.four_size_segment.memory", four_size_segment->memory() == four_size_memory);

    EXPECT("manager.trace.four_size_segment", segment_trace(manager) == segment_trace_t{{8, true}, {4, true}, {12, false}});

    // [8 + 8] [8 + 4] [8 + 2] (8 + 2)
    auto two_size_memory = manager.add_segment(two_size);
    ASSERT("manager.add_segment.two_size_segment", two_size_memory != nullptr);
    EXPECT("manager.add_segment.two_size_segment.segment_count", segment_count(manager) == 4);

    auto two_size_segment = segment_t::segment(two_size_memory);
    EXPECT("manager.add_segment.two_size_segment.size", two_size_segment->size == two_size);
    EXPECT("manager.add_segment.two_size_segment.is_used", two_size_segment->is_used == true);
    EXPECT("manager.add_segment.two_size_segment.memory", two_size_segment->memory() == two_size_memory);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{8, true}, {4, true}, {2, true}, {2, false}});


    // [8 + 8] [8 + 4] [8 + 2] (8 + 2)
    EXPECT("manager.extend_segment.eight_size_segment", manager.extend_segment(eight_size_memory, two_size) == false);
    EXPECT("manager.extend_segment.eight_size_segment.size", eight_size_segment->size == eight_size);
    EXPECT("manager.remove_segment.eight_size_segment.segment_count", segment_count(manager) == 4);

    EXPECT("manager.trace.eight_size_segment", segment_trace(manager) == segment_trace_t{{8, true}, {4, true}, {2, true}, {2, false}});

    // [8 + 8] (8 + 4) [8 + 2] (8 + 2)
    manager.remove_segment(four_size_memory);
    EXPECT("manager.remove_segment.four_size_segment.size", four_size_segment->size == four_size);
    EXPECT("manager.remove_segment.four_size_segment.is_used", four_size_segment->is_used == false);
    EXPECT("manager.remove_segment.four_size_segment.segment_count", segment_count(manager) == 4);

    EXPECT("manager.trace.four_size_segment", segment_trace(manager) == segment_trace_t{{8, true}, {4, false}, {2, true}, {2, false}});

    // [8 + 8] (8 + 4) [8 + 4] (8 + 0)
    EXPECT("manager.extend_segment.two_size_segment", manager.extend_segment(two_size_memory, two_size) == true);
    EXPECT("manager.extend_segment.two_size_segment.size", two_size_segment->size == two_size + two_size);
    EXPECT("manager.remove_segment.two_size_segment.segment_count", segment_count(manager) == 4);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{8, true}, {4, false}, {4, true}, {0, false}});

    // [8 + 20] [8 + 4] (8 + 0)
    EXPECT("manager.extend_segment.eight_size_segment", manager.extend_segment(eight_size_memory, sizeof(segment_t)) == true);
    EXPECT("manager.extend_segment.eight_size_segment.size", eight_size_segment->size == eight_size + two_size + sizeof(segment_t) + two_size);
    EXPECT("manager.remove_segment.eight_size_segment.segment_count", segment_count(manager) == 3);

    EXPECT("manager.trace.eight_size_segment", segment_trace(manager) == segment_trace_t{{20, true}, {4, true}, {0, false}});

    // [8 + 20] [8 + 4] (8 + 0)
    EXPECT("manager.extend_segment.eight_size_segment", manager.extend_segment(eight_size_memory, two_size) == false);
    EXPECT("manager.extend_segment.eight_size_segment.size", eight_size + two_size + sizeof(segment_t) + two_size);
    EXPECT("manager.remove_segment.eight_size_segment.segment_count", segment_count(manager) == 3);

    EXPECT("manager.trace.eight_size_segment", segment_trace(manager) == segment_trace_t{{20, true}, {4, true}, {0, false}});

    // [8 + 20] [8 + 12]
    EXPECT("manager.extend_segment.two_size_segment", manager.extend_segment(two_size_memory, six_size) == true);
    EXPECT("manager.extend_segment.two_size_segment.size", two_size_segment->size == two_size + two_size + sizeof(segment_t));
    EXPECT("manager.remove_segment.two_size_segment.segment_count", segment_count(manager) == 2);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{20, true}, {12, true}});

    // (8 + 20) [8 + 12]
    manager.remove_segment(eight_size_memory);
    EXPECT("manager.remove_segment.eight_size_segment.size", eight_size_segment->size == eight_size + two_size + sizeof(segment_t) + two_size);
    EXPECT("manager.remove_segment.eight_size_segment.is_used", eight_size_segment->is_used == false);
    EXPECT("manager.remove_segment.eight_size_segment.segment_count", segment_count(manager) == 2);

    EXPECT("manager.trace.eight_size_segment", segment_trace(manager) == segment_trace_t{{20, false}, {12, true}});

    // (8 + 20) (8 + 12)
    manager.remove_segment(two_size_memory);
    EXPECT("manager.remove_segment.two_size_segment.size", two_size_segment->size == two_size + two_size + sizeof(segment_t));
    EXPECT("manager.remove_segment.two_size_segment.is_used", two_size_segment->is_used == false);
    EXPECT("manager.remove_segment.two_size_segment.segment_count", segment_count(manager) == 2);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{20, false}, {12, false}});


    // (8 + 40)
    segment_defragmentation(manager);
    EXPECT("manager.defragmentation.segment_count", segment_count(manager) == 1);

    // (8 + 40)
    auto begin_segment = manager.begin();
    EXPECT("manager.begin_segment.size", begin_segment->size ==  sizeof(memory) - sizeof(segment_t));
    EXPECT("manager.begin_segment.is_used", begin_segment->is_used == false);

    EXPECT("manager.trace.begin_segment", segment_trace(manager) == segment_trace_t{{40, false}});
}

TEST(TestLibrary, TestComplexExtendSingle)
{
    // (8 + 40)
    char memory[48];
    auto manager = segment_manager_t(memory, sizeof(memory));

    static const auto eight_size = 8;
    static const auto four_size = 4;
    static const auto two_size = 2;
    static const auto six_size = 6;


    EXPECT("manager.trace", segment_trace(manager) == segment_trace_t{{40, false}});

    // [8 + 8] (8 + 24)
    auto eight_size_memory = manager.add_segment(eight_size);
    ASSERT("manager.add_segment.eight_size_segment", eight_size_memory != nullptr);
    EXPECT("manager.add_segment.eight_size_segment.segment_count", segment_count(manager) == 2);

    auto eight_size_segment = segment_t::segment(eight_size_memory);
    EXPECT("manager.add_segment.eight_size_segment.size", eight_size_segment->size == eight_size);
    EXPECT("manager.add_segment.eight_size_segment.is_used", eight_size_segment->is_used == true);
    EXPECT("manager.add_segment.eight_size_segment.memory", eight_size_segment->memory() == eight_size_memory);

    EXPECT("manager.trace.eight_size_segment", segment_trace(manager) == segment_trace_t{{8, true}, {24, false}});

    // [8 + 8] [8 + 4] (8 + 12)
    auto four_size_memory = manager.add_segment(four_size);
    ASSERT("manager.add_segment.four_size_segment", four_size_memory != nullptr);
    EXPECT("manager.add_segment.four_size_segment.segment_count", segment_count(manager) == 3);

    auto four_size_segment = segment_t::segment(four_size_memory);
    EXPECT("manager.add_segment.four_size_segment.size", four_size_segment->size == four_size);
    EXPECT("manager.add_segment.four_size_segment.is_used", four_size_segment->is_used == true);
    EXPECT("manager.add_segment.four_size_segment.memory", four_size_segment->memory() == four_size_memory);

    EXPECT("manager.trace.four_size_segment", segment_trace(manager) == segment_trace_t{{8, true}, {4, true}, {12, false}});

    // [8 + 8] [8 + 4] [8 + 2] (8 + 2)
    auto two_size_memory = manager.add_segment(two_size);
    ASSERT("manager.add_segment.two_size_segment", two_size_memory != nullptr);
    EXPECT("manager.add_segment.two_size_segment.segment_count", segment_count(manager) == 4);

    auto two_size_segment = segment_t::segment(two_size_memory);
    EXPECT("manager.add_segment.two_size_segment.size", two_size_segment->size == two_size);
    EXPECT("manager.add_segment.two_size_segment.is_used", two_size_segment->is_used == true);
    EXPECT("manager.add_segment.two_size_segment.memory", two_size_segment->memory() == two_size_memory);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{8, true}, {4, true}, {2, true}, {2, false}});


    // [8 + 8] [8 + 4] [8 + 2] (8 + 2)
    EXPECT("manager.extend_segment.eight_size_segment", manager.extend_segment(eight_size_memory, two_size) == false);
    EXPECT("manager.extend_segment.eight_size_segment.size", eight_size_segment->size == eight_size);
    EXPECT("manager.remove_segment.eight_size_segment.segment_count", segment_count(manager) == 4);

    EXPECT("manager.trace.eight_size_segment", segment_trace(manager) == segment_trace_t{{8, true}, {4, true}, {2, true}, {2, false}});

    // [8 + 8] (8 + 4) [8 + 2] (8 + 2)
    manager.remove_segment(four_size_memory);
    EXPECT("manager.remove_segment.four_size_segment.size", four_size_segment->size == four_size);
    EXPECT("manager.remove_segment.four_size_segment.is_used", four_size_segment->is_used == false);
    EXPECT("manager.remove_segment.four_size_segment.segment_count", segment_count(manager) == 4);

    EXPECT("manager.trace.four_size_segment", segment_trace(manager) == segment_trace_t{{8, true}, {4, false}, {2, true}, {2, false}});

    // [8 + 8] (8 + 4) [8 + 4] (8 + 0)
    EXPECT("manager.extend_segment.two_size_segment", manager.extend_segment(two_size_memory, two_size) == true);
    EXPECT("manager.extend_segment.two_size_segment.size", two_size_segment->size == two_size + two_size);
    EXPECT("manager.remove_segment.two_size_segment.segment_count", segment_count(manager) == 4);

    EXPECT("manager.trace.segment_count", segment_trace(manager) == segment_trace_t{{8, true}, {4, false}, {4, true}, {0, false}});

    // [8 + 20] [8 + 4] (8 + 0)
    EXPECT("manager.extend_segment.eight_size_segment", manager.extend_segment(eight_size_memory, sizeof(segment_t) + two_size) == true);
    EXPECT("manager.extend_segment.eight_size_segment.size", eight_size_segment->size == eight_size + two_size + sizeof(segment_t) + two_size);
    EXPECT("manager.remove_segment.eight_size_segment.segment_count", segment_count(manager) == 3);

    EXPECT("manager.trace.eight_size_segment", segment_trace(manager) == segment_trace_t{{20, true}, {4, true}, {0, false}});


    // [8 + 20] [8 + 12]
    EXPECT("manager.remove_segment.two_size_segment", manager.extend_segment(two_size_memory, six_size) == true);
    EXPECT("manager.extend_segment.two_size_segment.size", two_size_segment->size == two_size + two_size + sizeof(segment_t));
    EXPECT("manager.remove_segment.two_size_segment.segment_count", segment_count(manager) == 2);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{20, true}, {12, true}});


    // (8 + 20) [8 + 12]
    manager.remove_segment(eight_size_memory);
    EXPECT("manager.remove_segment.eight_size_segment.size", eight_size_segment->size == eight_size + two_size + sizeof(segment_t) + two_size);
    EXPECT("manager.remove_segment.eight_size_segment.is_used", eight_size_segment->is_used == false);
    EXPECT("manager.remove_segment.eight_size_segment.segment_count", segment_count(manager) == 2);

    EXPECT("manager.trace.eight_size_segment", segment_trace(manager) == segment_trace_t{{20, false}, {12, true}});

    // (8 + 20) (8 + 12)
    manager.remove_segment(two_size_memory);
    EXPECT("manager.remove_segment.two_size_segment.size", two_size_segment->size == two_size + two_size + sizeof(segment_t));
    EXPECT("manager.remove_segment.two_size_segment.is_used", two_size_segment->is_used == false);
    EXPECT("manager.remove_segment.two_size_segment.segment_count", segment_count(manager) == 2);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{20, false}, {12, false}});


    // (8 + 40)
    segment_defragmentation(manager);
    EXPECT("manager.defragmentation.segment_count", segment_count(manager) == 1);

    // (8 + 40)
    auto begin_segment = manager.begin();
    EXPECT("manager.begin_segment.size", begin_segment->size ==  sizeof(memory) - sizeof(segment_t));
    EXPECT("manager.begin_segment.is_used", begin_segment->is_used == false);

    EXPECT("manager.trace.begin_segment", segment_trace(manager) == segment_trace_t{{40, false}});
}

TEST(TestLibrary, TestComplexWithDefragmentation)
{
    // (8 + 32)
    char memory[40];
    auto manager = segment_manager_t(memory, sizeof(memory));

    static const auto one_size = 1;
    static const auto two_size = 2;
    static const auto four_size = 4;
    static const auto zero_size = 0;
    static const auto real_zero_size = zero_size + 2;


    EXPECT("manager.trace.one_size_segment", segment_trace(manager) == segment_trace_t{{32, false}});

    // [8 + 1] (8 + 23)
    auto one_size_memory = manager.add_segment(one_size);
    ASSERT("manager.add_segment.one_size_segment", one_size_memory != nullptr);
    EXPECT("manager.add_segment.one_size_segment.segment_count", segment_count(manager) == 2);

    auto one_size_segment = segment_t::segment(one_size_memory);
    EXPECT("manager.add_segment.one_size_segment.size", one_size_segment->size == one_size);
    EXPECT("manager.add_segment.one_size_segment.is_used", one_size_segment->is_used == true);
    EXPECT("manager.add_segment.one_size_segment.memory", one_size_segment->memory() == one_size_memory);

    EXPECT("manager.trace.one_size_segment", segment_trace(manager) == segment_trace_t{{1, true}, {23, false}});

    // [8 + 1] [8 + 2] (8 + 13)
    auto two_size_memory = manager.add_segment(two_size);
    ASSERT("manager.add_segment.two_size_segment", two_size_memory != nullptr);
    EXPECT("manager.add_segment.two_size_segment.segment_count", segment_count(manager) == 3);

    auto two_size_segment = segment_t::segment(two_size_memory);
    EXPECT("manager.add_segment.two_size_segment.size", two_size_segment->size == two_size);
    EXPECT("manager.add_segment.two_size_segment.is_used", two_size_segment->is_used == true);
    EXPECT("manager.add_segment.two_size_segment.memory", two_size_segment->memory() == two_size_memory);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{1, true}, {2, true}, {13, false}});


    // (8 + 1) [8 + 2] (8 + 13)
    manager.remove_segment(one_size_memory);
    EXPECT("manager.remove_segment.one_size_segment.size", one_size_segment->size == one_size);
    EXPECT("manager.remove_segment.one_size_segment.is_used", one_size_segment->is_used == false);
    EXPECT("manager.remove_segment.one_size_segment.segment_count", segment_count(manager) == 3);

    EXPECT("manager.trace.one_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {2, true}, {13, false}});

    // (8 + 1) [8 + 2] (8 + 13)
    segment_defragmentation(manager);
    EXPECT("manager.defragmentation.one_size_segment.segment_count", segment_count(manager) == 3);

    EXPECT("manager.trace.one_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {2, true}, {13, false}});


    // (8 + 1) [8 + 2] [8 + 4] (8 + 1)
    auto four_size_memory = manager.add_segment(four_size);
    ASSERT("manager.add_segment.four_size_segment", four_size_memory != nullptr);
    EXPECT("manager.add_segment.four_size_segment.segment_count", segment_count(manager) == 4);

    auto four_size_segment = segment_t::segment(four_size_memory);
    EXPECT("manager.add_segment.four_size_segment.size", four_size_segment->size == four_size);
    EXPECT("manager.add_segment.four_size_segment.is_used", four_size_segment->is_used == true);
    EXPECT("manager.add_segment.four_size_segment.memory", four_size_segment->memory() == four_size_memory);

    EXPECT("manager.trace.four_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {2, true}, {4, true}, {1, false}});

    // [8 + 1] [8 + 2] [8 + 4] (8 + 1)
    auto one_size_memory2 = manager.add_segment(one_size);
    ASSERT("manager.add_segment.one_size_segment2", one_size_memory2 != nullptr);
    EXPECT("manager.add_segment.one_size_segment2.segment_count", segment_count(manager) == 4);

    auto one_size_segment2 = segment_t::segment(one_size_memory2);
    EXPECT("manager.add_segment.one_size_segment2.size", one_size_segment2->size == one_size);
    EXPECT("manager.add_segment.one_size_segment2.is_used", one_size_segment2->is_used == true);
    EXPECT("manager.add_segment.one_size_segment2.memory", one_size_segment2->memory() == one_size_memory2);

    EXPECT("manager.trace.one_size_segment2", segment_trace(manager) == segment_trace_t{{1, true}, {2, true}, {4, true}, {1, false}});


    // [8 + 1] (8 + 2) [8 + 4] (8 + 1)
    manager.remove_segment(two_size_memory);
    EXPECT("manager.remove_segment.two_size_segment.size", two_size_segment->size == two_size);
    EXPECT("manager.remove_segment.two_size_segment.is_used", two_size_segment->is_used == false);
    EXPECT("manager.remove_segment.two_size_segment.segment_count", segment_count(manager) == 4);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{1, true}, {2, false}, {4, true}, {1, false}});

    // [8 + 1] (8 + 2) [8 + 4] (8 + 1)
    segment_defragmentation(manager);
    EXPECT("manager.defragmentation.two_size_segment.segment_count", segment_count(manager) == 4);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{1, true}, {2, false}, {4, true}, {1, false}});


    // [8 + 1] [8 + 0] (2) [8 + 4] (8 + 1) - not allowed
    // [8 + 1] [8 + 2] [8 + 4] (8 + 1) - allowed
    auto zero_size_memory = manager.add_segment(zero_size);
    ASSERT("manager.add_segment.zero_size_memory", zero_size_memory != nullptr);
    EXPECT("manager.add_segment.zero_size_memory.segment_count", segment_count(manager) == 4);

    auto zero_size_segment = segment_t::segment(zero_size_memory);
    EXPECT("manager.add_segment.zero_size_segment.size", zero_size_segment->size == real_zero_size);
    EXPECT("manager.add_segment.zero_size_segment.is_used", zero_size_segment->is_used == true);
    EXPECT("manager.add_segment.zero_size_segment.memory", zero_size_segment->memory() == zero_size_memory);

    EXPECT("manager.trace.zero_size_segment", segment_trace(manager) == segment_trace_t{{1, true}, {2, true}, {4, true}, {1, false}});


    // (8 + 1) [8 + 2] [8 + 4] (8 + 1)
    manager.remove_segment(one_size_memory2);
    EXPECT("manager.remove_segment.one_size_segment2.size", one_size_segment2->size == one_size);
    EXPECT("manager.remove_segment.one_size_segment2.is_used", one_size_segment2->is_used == false);
    EXPECT("manager.remove_segment.one_size_segment2.segment_count", segment_count(manager) == 4);

    EXPECT("manager.trace.one_size_segment2", segment_trace(manager) == segment_trace_t{{1, false}, {2, true}, {4, true}, {1, false}});

    // (8 + 1) [8 + 2] [8 + 4] (8 + 1)
    segment_defragmentation(manager);
    EXPECT("manager.defragmentation.one_size_segment2.segment_count", segment_count(manager) == 4);

    EXPECT("manager.trace.one_size_segment2", segment_trace(manager) == segment_trace_t{{1, false}, {2, true}, {4, true}, {1, false}});


    // (8 + 1) (8 + 2) [8 + 4] (8 + 1)
    manager.remove_segment(zero_size_memory);
    EXPECT("manager.remove_segment.zero_size_segment.size", zero_size_segment->size == real_zero_size);
    EXPECT("manager.remove_segment.zero_size_segment.is_used", zero_size_segment->is_used == false);
    EXPECT("manager.remove_segment.zero_size_segment.segment_count", segment_count(manager) == 4);

    EXPECT("manager.trace.zero_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {2, false}, {4, true}, {1, false}});

    // (8 + 11) [8 + 4] (8 + 1)
    segment_defragmentation(manager);
    EXPECT("manager.defragmentation.zero_size_segment.segment_count", segment_count(manager) == 3);

    EXPECT("manager.trace.zero_size_segment", segment_trace(manager) == segment_trace_t{{11, false}, {4, true}, {1, false}});


    // (8 + 11) (8 + 4) (8 + 1)
    manager.remove_segment(four_size_memory);
    EXPECT("manager.remove_segment.four_size_segment.size", four_size_segment->size == four_size);
    EXPECT("manager.remove_segment.four_size_segment.is_used", four_size_segment->is_used == false);
    EXPECT("manager.remove_segment.four_size_segment.segment_count", segment_count(manager) == 3);

    EXPECT("manager.trace.four_size_segment", segment_trace(manager) == segment_trace_t{{11, false}, {4, false}, {1, false}});

    // (8 + 32)
    segment_defragmentation(manager);
    EXPECT("manager.defragmentation.four_size_segment.segment_count", segment_count(manager) == 1);


    // (8 + 32)
    auto begin_segment = manager.begin();
    EXPECT("manager.begin_segment.size", begin_segment->size ==  sizeof(memory) - sizeof(segment_t));
    EXPECT("manager.begin_segment.is_used", begin_segment->is_used == false);

    EXPECT("manager.trace.begin_segment", segment_trace(manager) == segment_trace_t{{32, false}});
}

TEST(TestLibrary, TestComplexWithoutDefragmentation)
{
    // (8 + 32)
    char memory[40];
    auto manager = segment_manager_t(memory, sizeof(memory));

    static const auto one_size = 1;
    static const auto two_size = 2;
    static const auto four_size = 4;
    static const auto zero_size = 0;
    static const auto real_zero_size = zero_size + 2;


    EXPECT("manager.trace", segment_trace(manager) == segment_trace_t{{32, false}});

    // [8 + 1] (8 + 23)
    auto one_size_memory = manager.add_segment(one_size);
    ASSERT("manager.add_segment.one_size_segment", one_size_memory != nullptr);
    EXPECT("manager.add_segment.one_size_segment.segment_count", segment_count(manager) == 2);

    auto one_size_segment = segment_t::segment(one_size_memory);
    EXPECT("manager.add_segment.one_size_segment.size", one_size_segment->size == one_size);
    EXPECT("manager.add_segment.one_size_segment.is_used", one_size_segment->is_used == true);
    EXPECT("manager.add_segment.one_size_segment.memory", one_size_segment->memory() == one_size_memory);

    EXPECT("manager.trace.one_size_segment", segment_trace(manager) == segment_trace_t{{1, true}, {23, false}});

    // [8 + 1] [8 + 2] (8 + 13)
    auto two_size_memory = manager.add_segment(two_size);
    ASSERT("manager.add_segment.two_size_segment", two_size_memory != nullptr);
    EXPECT("manager.add_segment.two_size_segment.segment_count", segment_count(manager) == 3);

    auto two_size_segment = segment_t::segment(two_size_memory);
    EXPECT("manager.add_segment.two_size_segment.size", two_size_segment->size == two_size);
    EXPECT("manager.add_segment.two_size_segment.is_used", two_size_segment->is_used == true);
    EXPECT("manager.add_segment.two_size_segment.memory", two_size_segment->memory() == two_size_memory);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{1, true}, {2, true}, {13, false}});


    // (8 + 1) [8 + 2] (8 + 13)
    manager.remove_segment(one_size_memory);
    EXPECT("manager.remove_segment.one_size_segment.size", one_size_segment->size == one_size);
    EXPECT("manager.remove_segment.one_size_segment.is_used", one_size_segment->is_used == false);
    EXPECT("manager.remove_segment.one_size_segment.segment_count", segment_count(manager) == 3);

    EXPECT("manager.trace.one_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {2, true}, {13, false}});


    // (8 + 1) [8 + 2] [8 + 4] (8 + 1)
    auto four_size_memory = manager.add_segment(four_size);
    ASSERT("manager.add_segment.four_size_segment", four_size_memory != nullptr);
    EXPECT("manager.add_segment.four_size_segment.segment_count", segment_count(manager) == 4);

    auto four_size_segment = segment_t::segment(four_size_memory);
    EXPECT("manager.add_segment.four_size_segment.size", four_size_segment->size == four_size);
    EXPECT("manager.add_segment.four_size_segment.is_used", four_size_segment->is_used == true);
    EXPECT("manager.add_segment.four_size_segment.memory", four_size_segment->memory() == four_size_memory);

    EXPECT("manager.trace.four_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {2, true}, {4, true}, {1, false}});

    // [8 + 1] [8 + 2] [8 + 4] (8 + 1)
    auto one_size_memory2 = manager.add_segment(one_size);
    ASSERT("manager.add_segment.one_size_segment2", one_size_memory2 != nullptr);
    EXPECT("manager.add_segment.one_size_segment2.segment_count", segment_count(manager) == 4);

    auto one_size_segment2 = segment_t::segment(one_size_memory2);
    EXPECT("manager.add_segment.one_size_segment2.size", one_size_segment2->size == one_size);
    EXPECT("manager.add_segment.one_size_segment2.is_used", one_size_segment2->is_used == true);
    EXPECT("manager.add_segment.one_size_segment2.memory", one_size_segment2->memory() == one_size_memory2);

    EXPECT("manager.trace.one_size_segment2", segment_trace(manager) == segment_trace_t{{1, true}, {2, true}, {4, true}, {1, false}});


    // [8 + 1] (8 + 2) [8 + 4] (8 + 1)
    manager.remove_segment(two_size_memory);
    EXPECT("manager.remove_segment.two_size_segment.size", two_size_segment->size == two_size);
    EXPECT("manager.remove_segment.two_size_segment.is_used", two_size_segment->is_used == false);
    EXPECT("manager.remove_segment.two_size_segment.segment_count", segment_count(manager) == 4);

    EXPECT("manager.trace.two_size_segment", segment_trace(manager) == segment_trace_t{{1, true}, {2, false}, {4, true}, {1, false}});


    // [8 + 1] [8 + 0] (2) [8 + 4] (8 + 1) - not allowed
    // [8 + 1] [8 + 2] [8 + 4] (8 + 1) - allowed
    auto zero_size_memory = manager.add_segment(zero_size);
    ASSERT("manager.add_segment.zero_size_memory", zero_size_memory != nullptr);
    EXPECT("manager.add_segment.zero_size_memory.segment_count", segment_count(manager) == 4);

    auto zero_size_segment = segment_t::segment(zero_size_memory);
    EXPECT("manager.add_segment.zero_size_segment.size", zero_size_segment->size == real_zero_size);
    EXPECT("manager.add_segment.zero_size_segment.is_used", zero_size_segment->is_used == true);
    EXPECT("manager.add_segment.zero_size_segment.memory", zero_size_segment->memory() == zero_size_memory);

    EXPECT("manager.trace.zero_size_segment", segment_trace(manager) == segment_trace_t{{1, true}, {2, true}, {4, true}, {1, false}});


    // (8 + 1) [8 + 2] [8 + 4] (8 + 1)
    manager.remove_segment(one_size_memory2);
    EXPECT("manager.remove_segment.one_size_segment2.size", one_size_segment2->size == one_size);
    EXPECT("manager.remove_segment.one_size_segment2.is_used", one_size_segment2->is_used == false);
    EXPECT("manager.remove_segment.one_size_segment2.segment_count", segment_count(manager) == 4);

    EXPECT("manager.trace.one_size_segment2", segment_trace(manager) == segment_trace_t{{1, false}, {2, true}, {4, true}, {1, false}});


    // (8 + 1) (8 + 2) [8 + 4] (8 + 1)
    manager.remove_segment(zero_size_memory);
    EXPECT("manager.remove_segment.zero_size_segment.size", zero_size_segment->size == real_zero_size);
    EXPECT("manager.remove_segment.zero_size_segment.is_used", zero_size_segment->is_used == false);
    EXPECT("manager.remove_segment.zero_size_segment.segment_count", segment_count(manager) == 4);

    EXPECT("manager.trace.zero_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {2, false}, {4, true}, {1, false}});

    // (8 + 1) (8 + 2) (8 + 4) (8 + 1)
    manager.remove_segment(four_size_memory);
    EXPECT("manager.remove_segment.four_size_segment.size", four_size_segment->size == four_size);
    EXPECT("manager.remove_segment.four_size_segment.is_used", four_size_segment->is_used == false);
    EXPECT("manager.remove_segment.four_size_segment.segment_count", segment_count(manager) == 4);

    EXPECT("manager.trace.four_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {2, false}, {4, false}, {1, false}});


    // (8 + 1) (8 + 2) (8 + 4) (8 + 1)
    auto begin_segment = manager.begin();
    EXPECT("manager.begin_segment.size", begin_segment->size ==  one_size);
    EXPECT("manager.begin_segment.is_used", begin_segment->is_used == false);

    EXPECT("manager.trace.four_size_segment", segment_trace(manager) == segment_trace_t{{1, false}, {2, false}, {4, false}, {1, false}});
}

TEST(TestLibrary, TestAlign)
{
    EXPECT("align_up.common0", align_up(0, 1) == 0 && align_up(0, 8) == 0 && align_up(1, 1) == 1 && align_up(1, 8) == 8 && align_up(8, 8) == 8 && align_up(9, 8) == 16);
    EXPECT("align_up.common1", align_up(13, 4) == 16 && align_up(15, 8) == 16 && align_up(17, 16) == 32 && align_up(63, 64) == 64 && align_up(65, 64) == 128);

    bool success = true;
    for (const std::size_t align : {1, 2, 4, 8, 16, 32, 64})
    {
        for (std::size_t size = 0; size < 100; ++size)
        {
            auto aligned_size = align_up(size, align);
            success &= is_aligned(aligned_size, align) && (aligned_size >= size) && (aligned_size - size < align);
        }
    }
    EXPECT("align_up.hard", success == true);

    EXPECT("align_up.big", align_up(1023, 256) == 1024 && align_up(4097, 4096) == 8192);
    EXPECT("align_up.already-aligned", align_up(128, 64) == 128);
}
