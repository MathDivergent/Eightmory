#include <iostream>

#include <new>

#include <MemoryManager.hpp>

static constexpr auto xxmemory_bytes = 1000;
static auto xxmemory = memory_manager_t(static_cast<char*>(std::malloc(xxmemory_bytes)), xxmemory_bytes);

void stats(char const* text)
{
    static const auto border_length = 32;

    std::cout << '\n';
    std::cout << std::boolalpha;
    std::cout << text << '\n';

    for (auto i = 0; i<border_length; ++i) std::cout << '-';
    std::cout << '\n';

    for (auto segment = xxmemory.begin(); segment != xxmemory.end(); segment = segment->next())
    {
        std::cout << "segment: " << segment
                  << "\nmemory: " << segment->memory()
                  << "\nsize: " << segment->size
                  << "\nis_used: " << segment->is_used
                  << "\n\n";
    }

    for (auto i = 0; i<border_length; ++i) std::cout << '-';
    std::cout << '\n';
}

void* operator new(std::size_t bytes)
{
    return xxmemory.add_segment(bytes); stats("void* operator new(std::size_t bytes)");
}

void* operator new[](std::size_t bytes)
{
    return xxmemory.add_segment(bytes); stats("void* operator new[](std::size_t bytes)");
}

void operator delete(void* pointer) noexcept
{
    xxmemory.remove_segment(pointer); stats("void operator delete(void* pointer) noexcept");
}

void operator delete(void* pointer, std::size_t bytes) noexcept
{
    xxmemory.remove_segment(pointer); stats("void operator delete(void* pointer, std::size_t bytes) noexcept");
}

void operator delete[](void* pointer) noexcept
{
    xxmemory.remove_segment(pointer); stats("void operator delete[](void* pointer) noexcept");
}

#define DO(...) __VA_ARGS__; stats(#__VA_ARGS__)
#define DM(...) std::cout << #__VA_ARGS__ << ": " << static_cast<void*>(reinterpret_cast<char*>(__VA_ARGS__) - sizeof(segment_t)) << '\n'

void simple_test()
{
    //if (false)
    {
        DO((void)xxmemory);
        DO(auto segment01 = xxmemory.add_segment(1)); DM(segment01);
        DO(auto segment04 = xxmemory.add_segment(4)); DM(segment04);
        DO(auto segment02 = xxmemory.add_segment(2)); DM(segment02);
        
        DO(xxmemory.remove_segment(segment01));
        DO(xxmemory.remove_segment(segment02));
        DO(xxmemory.remove_segment(segment04));
    }
    // else if (false)
    {
        DO((void)xxmemory);
        DO(auto segment08 = xxmemory.add_segment(8)); DM(segment08);
        DO(auto segment04 = xxmemory.add_segment(4)); DM(segment04);
        DO(auto segment02 = xxmemory.add_segment(2)); DM(segment02);
        DO(xxmemory.extend_segment(segment08, 2));
        DO(xxmemory.remove_segment(segment04));
        DO(xxmemory.extend_segment(segment08, 2));
        if (false)
        {
            DO(xxmemory.extend_segment(segment08, 2));
            DO(xxmemory.extend_segment(segment08, sizeof(segment_t)));
        }
        else if (true)
        {

            DO(xxmemory.extend_segment(segment08, sizeof(segment_t)));
            DO(xxmemory.extend_segment(segment08, 2));
        }
        else
        {
            DO(xxmemory.extend_segment(segment08, sizeof(segment_t) + 2));
        }
        DO(xxmemory.extend_segment(segment02, 6));
        DO(xxmemory.remove_segment(segment08));
        DO(xxmemory.remove_segment(segment02));
    }
    //else
    {
        DO((void)xxmemory);
        DO(auto segment01 = xxmemory.add_segment(1)); DM(segment01);
        DO(auto segment02 = xxmemory.add_segment(2)); DM(segment02);
        DO(xxmemory.remove_segment(segment01));
        DO(auto segment04 = xxmemory.add_segment(4)); DM(segment04);
        DO(auto segment11 = xxmemory.add_segment(1)); DM(segment11);
        DO(xxmemory.remove_segment(segment02));
        DO(auto segment00 = xxmemory.add_segment(0)); DM(segment00);
        DO(xxmemory.remove_segment(segment11));
        DO(xxmemory.remove_segment(segment00));
        DO(xxmemory.remove_segment(segment04));
    }
}


#include <string>
#include <map>

void stl_test()
{
    {
        std::string str;
        DO((void)str);
        DO(str = "01020304050607080910111213141516171819202122232425");
    }
    DO((void)"~str");

    {
        std::map<std::string, float> map;
        DO((void)map);
        DO(map["Tom"] = 21.5);
        DO(map["Ren"] = 22.3);
        DO(map.erase("Tom"));
        DO(map["Jully"] = 32.2);
        DO(map["0123456789qwertyuiopasdfghjklzxcvbnm"]);
        DO(map.erase("Jully"));
    }
    DO((void)"~map");
}


int main()
{
    simple_test();
    stl_test();

    return 0;
}
