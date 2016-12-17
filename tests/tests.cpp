#include <ax.hpp>

#include <vector>

using namespace ax;

void output_test() {
    using strvec = std::vector<std::string>;
    strvec fmts = {
        "word",
        "this is a sentence",
        "with some fmts: %% %% %%",
        "%%and mixed spec%s %%% s%f%%"
    };
    
    strvec results = {
        "word",
        "this is a sentence",
        "with some fmts: 1 2  ",
        "...and mixed spec%s s% s%f1"
    };
    
    std::vector<bool> cmps = {
        results[0] == strprintf(fmts[0]),
        results[1] == strprintf(fmts[1]),
        results[2] == strprintf(fmts[2], 1, "2", " "),
        results[3] == strprintf(fmts[3], "...", "s", 1)
    };
    
    for(size_t i = 0; i < cmps.size(); ++i)
        LIGHT_TEST(cmps[i] == true);
}

template <typename T>
using huge_vector = std::vector<T, huge_space_allocator<T>>;

void memory_test() {
    huge_vector<int> vi(100);
    for(auto const& i : vi)
        LIGHT_TEST(i == int{});
    
    huge_vector<std::string> vs(1_KIB, "test string");
    for(auto const& i : vs)
        LIGHT_TEST(i == "test string");
}

int main() {
    output_test();
    memory_test();
}