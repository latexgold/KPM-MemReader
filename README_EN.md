# KPM-MemReader

A KPM kernel module based on the KernelPatch framework that implements cross-process memory reading by hooking the `ioctl` system call.

English | [中文](README.md)

[![License](https://img.shields.io/badge/license-GPL%20v2-blue.svg)](LICENSE)
[![KPM](https://img.shields.io/badge/KPM-Framework-green.svg)](https://github.com/bmax121/KernelPatch)

---

## Features

- **Cross-process memory read** - Read target process memory via page table walking
- **Auto page table config** - Automatically detects page size, VA bits, PA bits
- **Log control** - Enable/disable logging via macro switch

---

## File Structure

```
├── hello.c              # Kernel module source
├── hello.lds            # Linker script
├── Makefile             # Build script
├── README.md            # Chinese document
├── README_EN.md         # English document
│
└── user/                # User-space client
    └── jni/
        ├── KernelRW.hpp       # RW class header
        ├── KernelRW.cpp       # RW class implementation
        ├── main.cpp           # Usage example
        ├── Android.mk         # NDK build config
        └── Application.mk    # NDK platform config
```

---

## Command List

| Command | Macro | Function | Data Structure |
|---------|-------|----------|----------------|
| 8001 | `OP_READ_MEM` | Read process memory | `copy_memory_t` |

---

## Quick Start

### 1. Build Kernel Module

```bash
export TARGET_COMPILE=aarch64-none-elf-
make
```

Output: `Kernel_Hack.kpm`

### 2. Build User-space Client

```bash
cd user
ndk-build
```

Output: `libs/arm64-v8a/Kernel_Hack`

### 3. Load Module

Push the compiled `Kernel_Hack.kpm` to your device, then load it manually via the **KPM Manager** (e.g. APatch / SukiSU-Ultra module manager UI).

### 4. Run Client

```bash
adb push libs/arm64-v8a/Kernel_Hack /data/local/tmp/
adb shell chmod +x /data/local/tmp/Kernel_Hack
adb shell /data/local/tmp/Kernel_Hack
```

---

## Usage Example

```cpp
#include "KernelRW.hpp"

int main() {
    pid_t pid = rw->get_process_pid("bin.mt.plus");
    printf("PID: %d\n", pid);

    rw->initialize(pid);

    uintptr_t base = rw->get_module_base("libmt1.so");
    printf("Base: %lx\n", base);

    int dValue = rw->getDword(base);
    printf("Dword: %d (0x%x)\n", dValue, dValue);

    float fValue = rw->getFloat(base);
    printf("Float: %f\n", fValue);

    return 0;
}
```

---

## How It Works

```
User-space                          Kernel-space
  │                                   │
  │  socket(AF_INET, SOCK_DGRAM, 0)  │
  │  ──────────────────────────────>  │
  │  fd                               │
  │                                   │
  │  ioctl(fd, 8001, &data)          │
  │  ──────────────────────────────>  │
  │                  │ hook ioctl syscall
  │                  │ check cmd range
  │                  │ copy_from_user
  │                  │ page table walk
  │                  │ phys addr translation
  │                  │ copy_to_user
  │  <──────────────────────────────  │
  │  data returned to buffer          │
```

### Memory Read Flow

```
Virtual addr ──> find_task_by_vpid(pid)
                    │
                get_task_mm(task)
                    │
                Page table walk (pgd -> pud -> pmd -> pte)
                    │
                Extract physical address
                    │
                pfn_valid + valid_phys_addr_range validation
                    │
                Physical to kernel virtual address
                    │
                copy_to_user to user-space
```

---

## Configuration

### Log Switch

Edit top of `hello.c`:

```c
#define ENABLE_DEBUG_LOG 0  // 0=disable, 1=enable
```

---

## Build Requirements

- **KPM Framework** - KernelPatch / APatch / SukiSU-Ultra installed
- **Cross Compiler** - `aarch64-none-elf-gcc`
- **NDK** - Android NDK (for user-space build)
- **Architecture** - ARM64/aarch64

---

## Troubleshooting

### Memory read returns all zeros

1. Verify PID is correct
2. Verify target address is within the process address space
3. Enable debug logs: `ENABLE_DEBUG_LOG = 1`

### Module fails to load

Check kernel logs: `dmesg -w | grep KP`

---

## License

This project is licensed under [GPL v2](LICENSE).

---

## Disclaimer

This tool is for educational and research purposes only. Users are solely responsible for any consequences arising from the use of this tool.

**Please comply with local laws and regulations. Do not use for illegal purposes.**

---

## Credits

- **小迷糊 (XiaoMiHu)** - User-space code and Chinese documentation
- [KernelPatch](https://github.com/bmax121/KernelPatch) - KPM Framework

---

<p align="center">
  Made with ❤️ by Kernel_Hack
</p>