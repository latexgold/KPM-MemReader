#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "KernelRW.hpp"

int main() {
    pid_t pid = rw->get_process_pid("bin.mt.plus");
    printf("PID: %d\n", pid);
    rw->initialize(pid);
    uintptr_t libmt1 = rw->get_module_base("libmt1.so");
    printf("基址: %lx\n", libmt1);

    int dValue = rw->getDword(libmt1);
    printf("Dword值: %d (0x%x)\n", dValue, dValue);

    float fValue = rw->getFloat(libmt1);
    printf("Float值: %f\n", fValue);

    return 0;
}