#pragma once

#include <cstddef>
#include <cstring>
#include <functional>
#include <iostream>
#include <list>
#include <sstream>
#include <utility>

#ifdef __GNUC__
    #define likely(x)   __builtin_expect(!!(x),1)
    #define unlikely(x) __builtin_expect(!!(x),0)
    #define GNUC_ONLY(x) x
#else
    #define likely(x)   (x)
    #define unlikely(x) (x)
    #define GNUC_ONLY(x)
#endif

#define AX_CONCAT(a, b)     a ## b
#define AX_CONCAT2(a, b)    AX_CONCAT(a, b)
#define UNIQUENAME          AX_CONCAT2(temp_, __LINE__)

#define LOG_HEAD "[core]: "

namespace ax {
    
    /// Alias for std::size_t inside ax namespace
    using size_t = std::size_t;
    
    /// Returns sizes in bytes
    constexpr size_t operator"" _KIB(unsigned long long kib) {
        return kib << 10; }
    
    constexpr size_t operator"" _MIB(unsigned long long mib) {
        return operator"" _KIB(mib) << 10; }
    
    constexpr size_t operator"" _GIB(unsigned long long gib) {
        return operator"" _MIB(gib) << 10; }
    
    /// Some platform-specific definitions and assertions
    namespace platform {
        
        /// Uses for appllication-wide aligning and padding
        enum : size_t { cacheline_size  = 64 };
        enum : size_t { hugepage_size   = 2_MIB };
        
        static_assert(sizeof(char) == 1, "");
        static_assert(sizeof(unsigned char) == sizeof(char), "");
        
    } // platform
    
    constexpr size_t align_x_to_y(size_t x, size_t y) {
        return (x + y - 1) / y * y; }
    
    constexpr size_t align_to_cacheline(size_t x) {
        return align_x_to_y(x, platform::cacheline_size); }
    
    constexpr size_t align_to_hugepage(size_t x) {
        return align_x_to_y(x, platform::hugepage_size); }
    
    // ############################## Output ############################## //
    
    /**
     * Thread-safely puts arguments to std::cout using operator<<:
     * std::cout << (smth...) << "\n"
     */
    template <typename... T>
    void stdcout(T&&... smth);
    
#ifdef NDEBUG
    void DBG_OUT(...) {}
#else
    template <typename... T>
    void DBG_OUT(T&&... args) { stdcout(std::forward<T>(args)...); }
#endif
    
    /**
     * Type-safe sprintf: populates given string
     * and replaces "%%"-specs using ostringstream::operator<<(arg).
     * @arg fmt - format string, can be reference/literal, will be copied
     * @returns populated string
     */
    template <typename... Args>
    std::string strprintf(std::string fmt, Args&&... args);
    
    /**
     * Type-safe printf:
     * Effectively calls stdcout(strprintf(fmt, args...));
     */
    template <typename... Args>
    void stdprintf(std::string const& fmt, Args&&... args);
    
    // ############################## Time ############################## //
    
    /// @returns rdtsc (ReaD TimeStamp Counter)
    inline uint64_t rdtsc() noexcept;
    
    /**
     * @arg aux - reference to CPUID to be filled
     * @returns rdtscp if available, rdtsc() otherwise
     */
    inline uint64_t rdtscp(uint32_t& aux) noexcept;
    
    // ############################## Space ############################## //
    
    /**
     * Basic class for huge data to be allocated using hugetbl.
     * Contains operators new/delete, placement and their array forms.
     */
    struct huge_space {
        static void* operator new(size_t sz);
        static void  operator delete(void* ptr) noexcept;
        static void* operator new(size_t, void* ptr) noexcept;
        static void* operator new[](size_t sz);
        static void  operator delete[](void* ptr) noexcept;
        
    private:
        struct meta_info { size_t size; };
        enum : size_t { meta_rsrv = align_to_cacheline(sizeof(meta_info)) };
    };
    
    template <typename T>
    class huge_space_allocator {
    public:
        using allocator_type    = huge_space_allocator;
        using value_type        = T;
        using pointer           = T*;
        
        // Other typedefs will be defined by std::allocator_traits
        
        static pointer allocate(size_t n);
        static void deallocate(pointer p, size_t n);
        
        friend bool operator==(allocator_type const& lh, allocator_type const& rh) noexcept {
            return true; }
        
        friend bool operator!=(allocator_type const& lh, allocator_type const& rh) noexcept {
            return !(lh == rh); }
    };
    
    /// Aligned POD cacheline
    struct aligned_cacheline_t {
        alignas(platform::cacheline_size)
        unsigned char byte_array_[platform::cacheline_size];
    };
    
    /// Explicitly defined std::aligned_storage<(aligned_cacheline_t)>::type
    using aligned_cacheline = typename std::aligned_storage<
        sizeof(aligned_cacheline_t),
        alignof(aligned_cacheline_t)
    >::type;
    
    /**
     * Lihtweight test "framefork".
     * Singleton, can be used directly or by macro LIGHT_TEST(condition)
     */
    class ltest {
    public:
        static void test(bool cond, char const* msg, size_t line = 0);
        
    private:
        ltest() = default;
        ~ltest();
        static ltest& instance();
        
        std::list<std::string> results_;
    };
    
    #define LIGHT_TEST(x) ax::ltest::test((x), "\"" #x  "\" at " __FILE__, __LINE__)
    
} // ax

#include <ax_impl.hpp>

#undef LOG_HEAD