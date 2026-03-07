#ifndef KernelRW_HPP
#define KernelRW_HPP

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

class KernelRW {
	protected:
	int fd;
	pid_t pid;
	
    public:
    pid_t getPid() const {
        return pid;
    }

    KernelRW();
    ~KernelRW();
    void initialize(pid_t pid);
    bool readv(uintptr_t address, void *buffer, size_t size);
    uintptr_t get_module_base(const char *name);
    pid_t get_process_pid(const char *name);
    
    float getFloat(uintptr_t addr);
    int getDword(uintptr_t addr);
    bool getBool(uintptr_t addr);
    char *getUTF8(uintptr_t addr);
    uintptr_t getPtr64(uintptr_t addr);
    uintptr_t getPtr32(uintptr_t addr);

    template <typename... Args>
    uintptr_t jumpPoint(uintptr_t addr, Args... args) {
        static_assert(
            (std::is_same_v<Args, int> && ...),
            "参数类型错误!"
        );
    	uintptr_t result = addr;
        if constexpr (sizeof...(args) > 0) {
            (..., (readv((result + args) & 0xFFFFFFFFFF, &result, 8)));
        }
        return result;
    }
};

extern KernelRW *rw;

#endif