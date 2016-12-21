#include <ax.hpp>

#include <sys/mman.h>

namespace ax {
    
    template <typename... T>
    void stdcout(T&&... smth) {
        std::ostringstream stream;
        using temp_t = int[sizeof...(T) + 1];
        temp_t { (stream << std::forward<T>(smth), 0)... };
        stream << "\n";
        std::cout << stream.str();
    }
    
    namespace strprintf_impl {
        
        /// Placeholder pattern for smart printf
        const std::string REPLACE_SPEC = "%%";
        
        inline std::string printf_(std::ostringstream&& stream, std::string&& fmt) {
            stream << std::move(fmt);
            return stream.str();
        }
        
        template <typename Head, typename... Tail>
        inline std::string printf_(
            std::ostringstream&& stream, std::string&& fmt,
            Head&& head, Tail&&... tail
        ) {
            auto token_pos = fmt.find(REPLACE_SPEC);
            if(likely(token_pos != std::string::npos)) {
                stream << fmt.substr(0, token_pos) << std::forward<Head>(head);
                return printf_(std::move(stream), fmt.substr(token_pos + REPLACE_SPEC.size()),
                               std::forward<Tail>(tail)...);
            } else
                return fmt + "<SPEC_ERROR_MORE_ARGS>";
        }
    }
    
    template <typename... Args>
    std::string strprintf(std::string fmt, Args&&... args) {
        return strprintf_impl::printf_(std::ostringstream{}, std::move(fmt), std::forward<Args>(args)...); }
    
    /// Effectively calls stdcout(strprintf(fmt, args...));
    template <typename... Args>
    void stdprintf(std::string const& fmt, Args&&... args) {
        stdcout(strprintf(fmt, std::forward<Args>(args)...)); }
    
    namespace rdtsc_impl {
        bool isTSCinvariant() {
            GNUC_ONLY({
                uint32_t edx;
                asm volatile (
                    "movl $0x80000007, %%eax\n"
                    "cpuid\n"
                    "movl %%edx, %0\n"
                    : "=r" (edx)
                    :
                    : "%rax", "%rbx", "%rcx", "%rdx"
                );
                return edx & (1U << 8) ? true : false;
            })
            return false;
        }
        
        bool isRDTSCP() {
            GNUC_ONLY({
                uint32_t edx;
                asm volatile (
                    "movl $0x80000001, %%eax\n"
                    "cpuid\n"
                    "movl %%edx, %0\n"
                    : "=r" (edx)
                    :
                    : "%rax", "%rbx", "%rcx", "%rdx"
                );
                return edx & (1U << 27) ? true : false;
            })
            return false;
        }
        
        inline uint64_t rdtscp_real(uint32_t& aux) noexcept {
            uint32_t eax, edx;
            asm volatile("rdtscp" : "=a" (eax), "=d" (edx), "=c" (aux) : : );
            return static_cast<uint64_t>((static_cast<uint64_t>(eax)) | ((static_cast<uint64_t>(edx)) << 32));
        }
        
        inline uint64_t rdtscp_fake(uint32_t& aux) noexcept {
            aux = 0;
            return rdtsc();
        }
    }
    
    inline uint64_t rdtsc() noexcept {
        uint32_t eax, edx;
        asm volatile("rdtsc" : "=a" (eax), "=d" (edx) : : );
        return static_cast<uint64_t>((static_cast<uint64_t>(eax)) | ((static_cast<uint64_t>(edx)) << 32));
    }
    
    inline uint64_t rdtscp(uint32_t& aux) noexcept {
        static const bool isRDTSCP_available = rdtsc_impl::isRDTSCP();
        if(likely(isRDTSCP_available))
            return rdtsc_impl::rdtscp_real(aux);
        return rdtsc_impl::rdtscp_fake(aux);
    }
    
    void* huge_space::operator new(size_t sz) {
        size_t full_size = align_to_hugepage(sz + meta_rsrv);
        auto allocated = mmap(nullptr, full_size, PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE | MAP_HUGETLB, -1, 0);
        if(unlikely(allocated == MAP_FAILED)) {
            DBG_OUT("huge_space allocation failed, default operator new is used: ", strerror(errno));
            allocated = ::operator new(full_size);
            full_size = 0;
        }
        static_cast<meta_info*>(allocated)->size = full_size;
        return static_cast<char*>(allocated) + meta_rsrv;
    }
    
    void huge_space::operator delete(void* ptr) noexcept {
        if(ptr == nullptr)
            return;
        
        void* real_ptr = static_cast<char*>(ptr) - meta_rsrv;
        size_t real_size = static_cast<meta_info*>(real_ptr)->size;
        
        if(real_size != 0) {
            munmap(real_ptr, real_size);
        } else
            ::operator delete(real_ptr);
    }
    
    void* huge_space::operator new(size_t, void* ptr) noexcept {
        return ptr; }
    
    void* huge_space::operator new[](size_t sz) {
        return huge_space::operator new(sz); }
    
    void huge_space::operator delete[](void* ptr) noexcept {
        return huge_space::operator delete(ptr); }
   
    template <typename T>
    auto huge_space_allocator<T>::allocate(size_t n) -> pointer {
        return static_cast<pointer>(huge_space::operator new(n*sizeof(value_type))); }
    
    template <typename T>
    void huge_space_allocator<T>::deallocate(pointer p, size_t n) {
        return huge_space::operator delete(p); }
    
    ltest& ltest::instance() {
        static ltest ltest_;
        return ltest_;
    }
    
    void ltest::test(bool cond, char const* msg, size_t line) {
        auto& lt = ltest::instance();
        if(!cond)
            lt.results_.emplace_back(strprintf("%%, line %%", msg, line));
    }
    
    ltest::~ltest() {
        auto& lt = ltest::instance();
        if(lt.results_.size() > 0) {
            for(auto const& s : lt.results_) stdprintf("  %%", s);
            stdprintf("\n!!! %% error(s) detected !!!", lt.results_.size());
            exit(1);
        } else {
            stdprintf("\n*** No errors detected ***");
        }
    }
    
} // ax