#include "KernelRW.hpp"
#include <cstring>
#include <fstream>
#include <string>

KernelRW *rw = new KernelRW();

typedef struct _COPY_MEMORY {
	pid_t pid;
	uintptr_t addr;
	void *buffer;
	size_t size;
} COPY_MEMORY, *PCOPY_MEMORY;

enum OPERATIONS {
    OP_READ_MEM = 8001,
};

KernelRW::KernelRW() {
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("[-] 创建socket失败");
        exit(EXIT_FAILURE);
    }
}

KernelRW::~KernelRW() {
    if(fd > 0) close(fd);
}

void KernelRW::initialize(pid_t pid) {
	this->pid = pid;
}
	
bool KernelRW::readv(uintptr_t addr, void *buffer, size_t size) {
    COPY_MEMORY cm;
    cm.pid = this->pid;
    cm.addr = addr;
    cm.buffer = buffer;
    cm.size = size;
    
    if (ioctl(fd, OP_READ_MEM, &cm) != 0) {
        return false;
    }
    return true;
}

uintptr_t KernelRW::get_module_base(const char *name) {
    char line[512];
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/maps", this->pid);
    FILE *fp = fopen(path, "r");
    if (!fp) return 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, name)) {
            uintptr_t base = 0;
            sscanf(line, "%lx-", &base);
            fclose(fp);
            return base;
        }
    }
    fclose(fp);
    return 0;
}

pid_t KernelRW::get_process_pid(const char *name) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "pidof %s", name);
    FILE *fp = popen(cmd, "r");
    if (!fp) return 0;
    pid_t pid = 0;
    fscanf(fp, "%d", &pid);
    pclose(fp);
    return pid;
}

float KernelRW::getFloat(uintptr_t addr) {
	float var = 0;
	readv(addr, &var, 4);
	return var;
}

int KernelRW::getDword(uintptr_t addr) {
	int var = 0;
	readv(addr, &var, 4);
	return var;
}

bool KernelRW::getBool(uintptr_t addr) {
	bool var = false;
	readv(addr, &var, 1);
	return var;
}

uintptr_t KernelRW::getPtr32(uintptr_t addr) {
	unsigned int var = 0;
	readv(addr & 0xFFFFFFFFFF, &var, 4);
	return (var & 0xFFFFFFFFFF);
}

uintptr_t KernelRW::getPtr64(uintptr_t addr) {
	uintptr_t var = 0;
	readv(addr & 0xFFFFFFFFFF, &var, 8);
	return (var & 0xFFFFFFFFFF);
}

char *KernelRW::getUTF8(uintptr_t addr) {
	static char buf[64];
	unsigned short buf16[16] = { 0 };
	readv(addr, buf16, 28);
	unsigned short *pTempUTF16 = buf16;
	char *pTempUTF8 = buf;
	char *pUTF8End = pTempUTF8 + 32;
	while (pTempUTF16 < pTempUTF16 + 28) {
		if (*pTempUTF16 <= 0x007F && pTempUTF8 + 1 < pUTF8End) {
			*pTempUTF8++ = (char) *pTempUTF16;
		} else if (*pTempUTF16 >= 0x0080 && *pTempUTF16 <= 0x07FF && pTempUTF8 + 2 < pUTF8End) {
			*pTempUTF8++ = (*pTempUTF16 >> 6) | 0xC0;
			*pTempUTF8++ = (*pTempUTF16 & 0x3F) | 0x80;
		} else if (*pTempUTF16 >= 0x0800 && *pTempUTF16 <= 0xFFFF && pTempUTF8 + 3 < pUTF8End) {
			*pTempUTF8++ = (*pTempUTF16 >> 12) | 0xE0;
			*pTempUTF8++ = ((*pTempUTF16 >> 6) & 0x3F) | 0x80;
			*pTempUTF8++ = (*pTempUTF16 & 0x3F) | 0x80;
		} else {
			break;
		}
		pTempUTF16++;
	}
	return buf;
}