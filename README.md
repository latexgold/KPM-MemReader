# KPM-MemReader

基于 KernelPatch 框架的 KPM 内核模块，通过 hook `ioctl` 系统调用实现跨进程内存读取。

[English](README_EN.md) | 中文

[![License](https://img.shields.io/badge/license-GPL%20v2-blue.svg)](LICENSE)
[![KPM](https://img.shields.io/badge/KPM-Framework-green.svg)](https://github.com/bmax121/KernelPatch)

---

## 功能

- **跨进程内存读取** - 通过页表遍历直接读取目标进程内存
- **自动页表配置** - 自动检测页大小、虚拟地址位数、物理地址位数
- **日志控制** - 可通过宏开关控制日志输出

---

## 文件结构

```
├── hello.c              # 内核模块源码
├── hello.lds            # 链接脚本
├── Makefile             # 编译脚本
├── README.md            # 中文文档
├── README_EN.md         # English document
│
└── user/                # 用户态调用程序
    └── jni/
        ├── KernelRW.hpp       # 读写类头文件
        ├── KernelRW.cpp       # 读写类实现
        ├── main.cpp           # 使用示例
        ├── Android.mk         # NDK 编译配置
        └── Application.mk    # NDK 平台配置
```

---

## 命令列表

| 命令号 | 宏定义 | 功能 | 数据结构 |
|--------|--------|------|----------|
| 8001 | `OP_READ_MEM` | 读取进程内存 | `copy_memory_t` |

---

## 快速开始

### 1. 编译内核模块

```bash
export TARGET_COMPILE=aarch64-none-elf-
make
```

输出文件：`Kernel_Hack.kpm`

### 2. 编译用户态程序

```bash
cd user
ndk-build
```

输出文件：`libs/arm64-v8a/Kernel_Hack`

### 3. 加载模块

将编译好的 `Kernel_Hack.kpm` 推送到手机后，使用 **KPM 管理器**（如 APatch / SukiSU-Ultra 的模块管理界面）手动加载即可。

### 4. 运行用户态程序

```bash
adb push libs/arm64-v8a/Kernel_Hack /data/local/tmp/
adb shell chmod +x /data/local/tmp/Kernel_Hack
adb shell /data/local/tmp/Kernel_Hack
```

---

## 用户态使用示例

```cpp
#include "KernelRW.hpp"

int main() {
    pid_t pid = rw->get_process_pid("bin.mt.plus");
    printf("PID: %d\n", pid);

    rw->initialize(pid);

    uintptr_t base = rw->get_module_base("libmt1.so");
    printf("基址: %lx\n", base);

    int dValue = rw->getDword(base);
    printf("Dword值: %d (0x%x)\n", dValue, dValue);

    float fValue = rw->getFloat(base);
    printf("Float值: %f\n", fValue);

    return 0;
}
```

---

## 通信原理

```
用户态                              内核态
  │                                   │
  │  socket(AF_INET, SOCK_DGRAM, 0)  │
  │  ──────────────────────────────>  │
  │  fd                               │
  │                                   │
  │  ioctl(fd, 8001, &data)          │
  │  ──────────────────────────────>  │
  │                  │ hook ioctl 系统调用
  │                  │ 检查 cmd 范围
  │                  │ copy_from_user
  │                  │ 页表遍历
  │                  │ 物理地址转换
  │                  │ copy_to_user
  │  <──────────────────────────────  │
  │  数据返回到 buffer                │
```

### 内存读取流程

```
虚拟地址 ──> find_task_by_vpid(pid)
                │
            get_task_mm(task)
                │
            页表遍历 (pgd -> pud -> pmd -> pte)
                │
            提取物理地址
                │
            pfn_valid + valid_phys_addr_range 验证
                │
            物理地址转内核虚拟地址
                │
            copy_to_user 拷贝到用户空间
```

---

## 配置说明

### 日志开关

编辑 `hello.c` 顶部：

```c
#define ENABLE_DEBUG_LOG 0  // 0=关闭日志, 1=启用日志
```

---

## 编译要求

- **KPM 框架** - 已安装 KernelPatch / APatch / SukiSU-Ultra
- **交叉编译器** - `aarch64-none-elf-gcc`
- **NDK** - Android NDK（用户态编译）
- **架构** - ARM64/aarch64

---

## 常见问题

### 读取内存返回全零

1. 确认 PID 正确
2. 确认目标地址在进程地址空间内
3. 开启日志排查：`ENABLE_DEBUG_LOG = 1`

### 模块加载失败

查看内核日志：`dmesg -w | grep KP`

---

## 许可证

本项目采用 [GPL v2](LICENSE) 许可证。

---

## 免责声明

本工具仅供学习研究使用。使用本工具产生的任何后果由使用者自行承担。

**请遵守当地法律法规，不要用于非法目的。**

---

## 致谢

- **小迷糊** - 提供用户态代码与中文帮助文档
- [KernelPatch](https://github.com/bmax121/KernelPatch) - KPM 框架

---

<p align="center">
  Made with ❤️ by Kernel_Hack
</p>