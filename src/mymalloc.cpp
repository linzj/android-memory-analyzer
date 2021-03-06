#include "mymalloc.h"
#include "LinLog.h"
#include <dlfcn.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

pfnmalloc mymalloc;
pfnfree myfree;
pfncalloc mycalloc;
pfnrealloc myrealloc;
pfnmemalign mymemalign;
pfnmalloc_usable_size mymalloc_usable_size;
pfnmmap mymmap;
pfnmunmap mymunmap;

typedef uint64_t __u64;
typedef uint32_t __u32;
typedef uint16_t __u16;
typedef int64_t __s64;
typedef int32_t __s32;
typedef int16_t __s16;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef __u32 Elf32_Addr;
typedef __u16 Elf32_Half;
typedef __u32 Elf32_Off;
typedef __s32 Elf32_Sword;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef __u32 Elf32_Word;
typedef __u64 Elf64_Addr;
typedef __u16 Elf64_Half;
typedef __s16 Elf64_SHalf;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef __u64 Elf64_Off;
typedef __s32 Elf64_Sword;
typedef __u32 Elf64_Word;
typedef __u64 Elf64_Xword;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef __s64 Elf64_Sxword;
#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PT_TLS 7
#define PT_LOOS 0x60000000
#define PT_HIOS 0x6fffffff
#define PT_LOPROC 0x70000000
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PT_HIPROC 0x7fffffff
#define PT_GNU_EH_FRAME 0x6474e550
#define PT_GNU_STACK (PT_LOOS + 0x474e551)
#define PN_XNUM 0xffff
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define ET_NONE 0
#define ET_REL 1
#define ET_EXEC 2
#define ET_DYN 3
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define ET_CORE 4
#define ET_LOPROC 0xff00
#define ET_HIPROC 0xffff
#define DT_NULL 0
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DT_NEEDED 1
#define DT_PLTRELSZ 2
#define DT_PLTGOT 3
#define DT_HASH 4
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DT_STRTAB 5
#define DT_SYMTAB 6
#define DT_RELA 7
#define DT_RELASZ 8
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DT_RELAENT 9
#define DT_STRSZ 10
#define DT_SYMENT 11
#define DT_INIT 12
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DT_FINI 13
#define DT_SONAME 14
#define DT_RPATH 15
#define DT_SYMBOLIC 16
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DT_REL 17
#define DT_RELSZ 18
#define DT_RELENT 19
#define DT_PLTREL 20
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DT_DEBUG 21
#define DT_TEXTREL 22
#define DT_JMPREL 23
#define DT_ENCODING 32
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define OLD_DT_LOOS 0x60000000
#define DT_LOOS 0x6000000d
#define DT_HIOS 0x6ffff000
#define DT_VALRNGLO 0x6ffffd00
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DT_VALRNGHI 0x6ffffdff
#define DT_ADDRRNGLO 0x6ffffe00
#define DT_ADDRRNGHI 0x6ffffeff
#define DT_VERSYM 0x6ffffff0
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DT_RELACOUNT 0x6ffffff9
#define DT_RELCOUNT 0x6ffffffa
#define DT_FLAGS_1 0x6ffffffb
#define DT_VERDEF 0x6ffffffc
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DT_VERDEFNUM 0x6ffffffd
#define DT_VERNEED 0x6ffffffe
#define DT_VERNEEDNUM 0x6fffffff
#define OLD_DT_HIOS 0x6fffffff
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define DT_LOPROC 0x70000000
#define DT_HIPROC 0x7fffffff
#define STB_LOCAL 0
#define STB_GLOBAL 1
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define STB_WEAK 2
#define STT_NOTYPE 0
#define STT_OBJECT 1
#define STT_FUNC 2
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define STT_SECTION 3
#define STT_FILE 4
#define STT_COMMON 5
#define STT_TLS 6
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define ELF_ST_BIND(x) ((x) >> 4)
#define ELF_ST_TYPE(x) (((unsigned int)x) & 0xf)
#define ELF32_ST_BIND(x) ELF_ST_BIND(x)
#define ELF32_ST_TYPE(x) ELF_ST_TYPE(x)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define ELF64_ST_BIND(x) ELF_ST_BIND(x)
#define ELF64_ST_TYPE(x) ELF_ST_TYPE(x)
typedef struct dynamic {
    Elf32_Sword d_tag;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    union {
        Elf32_Sword d_val;
        Elf32_Addr d_ptr;
    } d_un;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
} Elf32_Dyn;
typedef struct
{
    Elf64_Sxword d_tag;
    union {
        /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
        Elf64_Xword d_val;
        Elf64_Addr d_ptr;
    } d_un;
} Elf64_Dyn;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define ELF32_R_SYM(x) ((x) >> 8)
#define ELF32_R_TYPE(x) ((x)&0xff)
#define ELF64_R_SYM(i) ((i) >> 32)
#define ELF64_R_TYPE(i) ((i)&0xffffffff)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef struct elf32_rel {
    Elf32_Addr r_offset;
    Elf32_Word r_info;
} Elf32_Rel;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef struct elf64_rel {
    Elf64_Addr r_offset;
    Elf64_Xword r_info;
} Elf64_Rel;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef struct elf32_rela {
    Elf32_Addr r_offset;
    Elf32_Word r_info;
    Elf32_Sword r_addend;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
} Elf32_Rela;
typedef struct elf64_rela {
    Elf64_Addr r_offset;
    Elf64_Xword r_info;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf64_Sxword r_addend;
} Elf64_Rela;
typedef struct elf32_sym {
    Elf32_Word st_name;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf32_Addr st_value;
    Elf32_Word st_size;
    unsigned char st_info;
    unsigned char st_other;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf32_Half st_shndx;
} Elf32_Sym;
typedef struct elf64_sym {
    Elf64_Word st_name;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    unsigned char st_info;
    unsigned char st_other;
    Elf64_Half st_shndx;
    Elf64_Addr st_value;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf64_Xword st_size;
} Elf64_Sym;
#define EI_NIDENT 16
typedef struct elf32_hdr {
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    unsigned char e_ident[EI_NIDENT];
    Elf32_Half e_type;
    Elf32_Half e_machine;
    Elf32_Word e_version;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf32_Addr e_entry;
    Elf32_Off e_phoff;
    Elf32_Off e_shoff;
    Elf32_Word e_flags;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize;
    Elf32_Half e_phnum;
    Elf32_Half e_shentsize;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf32_Half e_shnum;
    Elf32_Half e_shstrndx;
} Elf32_Ehdr;
typedef struct elf64_hdr {
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    unsigned char e_ident[EI_NIDENT];
    Elf64_Half e_type;
    Elf64_Half e_machine;
    Elf64_Word e_version;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf64_Addr e_entry;
    Elf64_Off e_phoff;
    Elf64_Off e_shoff;
    Elf64_Word e_flags;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf64_Half e_ehsize;
    Elf64_Half e_phentsize;
    Elf64_Half e_phnum;
    Elf64_Half e_shentsize;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf64_Half e_shnum;
    Elf64_Half e_shstrndx;
} Elf64_Ehdr;
#define PF_R 0x4
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define PF_W 0x2
#define PF_X 0x1
typedef struct elf32_phdr {
    Elf32_Word p_type;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf32_Off p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
} Elf32_Phdr;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
typedef struct elf64_phdr {
    Elf64_Word p_type;
    Elf64_Word p_flags;
    Elf64_Off p_offset;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf64_Addr p_vaddr;
    Elf64_Addr p_paddr;
    Elf64_Xword p_filesz;
    Elf64_Xword p_memsz;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf64_Xword p_align;
} Elf64_Phdr;
#define SHT_NULL 0
#define SHT_PROGBITS 1
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NOBITS 8
#define SHT_REL 9
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define SHT_SHLIB 10
#define SHT_DYNSYM 11
#define SHT_NUM 12
#define SHT_LOPROC 0x70000000
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define SHT_HIPROC 0x7fffffff
#define SHT_LOUSER 0x80000000
#define SHT_HIUSER 0xffffffff
#define SHF_WRITE 0x1
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4
#define SHF_MASKPROC 0xf0000000
#define SHN_UNDEF 0
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC 0xff00
#define SHN_HIPROC 0xff1f
#define SHN_ABS 0xfff1
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define SHN_COMMON 0xfff2
#define SHN_HIRESERVE 0xffff
typedef struct elf32_shdr {
    Elf32_Word sh_name;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf32_Word sh_type;
    Elf32_Word sh_flags;
    Elf32_Addr sh_addr;
    Elf32_Off sh_offset;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf32_Word sh_size;
    Elf32_Word sh_link;
    Elf32_Word sh_info;
    Elf32_Word sh_addralign;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf32_Word sh_entsize;
} Elf32_Shdr;
typedef struct elf64_shdr {
    Elf64_Word sh_name;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf64_Word sh_type;
    Elf64_Xword sh_flags;
    Elf64_Addr sh_addr;
    Elf64_Off sh_offset;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf64_Xword sh_size;
    Elf64_Word sh_link;
    Elf64_Word sh_info;
    Elf64_Xword sh_addralign;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf64_Xword sh_entsize;
} Elf64_Shdr;
#define EI_MAG0 0
#define EI_MAG1 1
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define EI_MAG2 2
#define EI_MAG3 3
#define EI_CLASS 4
#define EI_DATA 5
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define EI_VERSION 6
#define EI_OSABI 7
#define EI_PAD 8
#define ELFMAG0 0x7f
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'
#define ELFMAG "\177ELF"
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define SELFMAG 4
#define ELFCLASSNONE 0
#define ELFCLASS32 1
#define ELFCLASS64 2
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define ELFCLASSNUM 3
#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define EV_NONE 0
#define EV_CURRENT 1
#define EV_NUM 2
#define ELFOSABI_NONE 0
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define ELFOSABI_LINUX 3
#ifndef ELF_OSABI
#define ELF_OSABI ELFOSABI_NONE
#endif
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NT_PRSTATUS 1
#define NT_PRFPREG 2
#define NT_PRPSINFO 3
#define NT_TASKSTRUCT 4
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NT_AUXV 6
#define NT_SIGINFO 0x53494749
#define NT_FILE 0x46494c45
#define NT_PRXFPREG 0x46e62b7f
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NT_PPC_VMX 0x100
#define NT_PPC_SPE 0x101
#define NT_PPC_VSX 0x102
#define NT_386_TLS 0x200
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NT_386_IOPERM 0x201
#define NT_X86_XSTATE 0x202
#define NT_S390_HIGH_GPRS 0x300
#define NT_S390_TIMER 0x301
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NT_S390_TODCMP 0x302
#define NT_S390_TODPREG 0x303
#define NT_S390_CTRS 0x304
#define NT_S390_PREFIX 0x305
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NT_S390_LAST_BREAK 0x306
#define NT_S390_SYSTEM_CALL 0x307
#define NT_S390_TDB 0x308
#define NT_ARM_VFP 0x400
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NT_ARM_TLS 0x401
#define NT_ARM_HW_BREAK 0x402
#define NT_ARM_HW_WATCH 0x403
#define NT_METAG_CBUF 0x500
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define NT_METAG_RPIPE 0x501
#define NT_METAG_TLS 0x502
typedef struct elf32_note {
    Elf32_Word n_namesz;
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf32_Word n_descsz;
    Elf32_Word n_type;
} Elf32_Nhdr;
typedef struct elf64_note {
    /* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
    Elf64_Word n_namesz;
    Elf64_Word n_descsz;
    Elf64_Word n_type;
} Elf64_Nhdr;
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */

#if __LP64__
#define ElfW(type) Elf64_##type
#else
#define ElfW(type) Elf32_##type
#endif

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) & ((TYPE*)0) \
                                               ->MEMBER)
#endif

namespace {
class SymbolResolver {
public:
    SymbolResolver();
    bool init();
    void* findSym(const char*);

private:
    static void* findLibcAddr();
    bool findMandatoryFields();
    template <class elf_header_type>
    bool
    deal_with_elf();
    template <class dynamic_type>
    bool
    fill_with_dyn(void* start, void* end);
    template <class elf_sym_type>
    void* doFindSym(const char*);
    const char* getString(uint64_t offset);

    void* start_;
    void* symTable_;
    void* strTable_;
    uint32_t* bucket_;
    uint32_t* chain_;
    uint32_t nBucket_;
    uint32_t nChain_;
    int strTableSz_;
    bool is_elf32_;
};

class StackFileReader {
public:
    StackFileReader();
    ~StackFileReader();
    bool open(const char* fileName);
    const char* readLine();

private:
    int fd_;
    static const int buf_size = 255;
    char buf_[buf_size + 1];
    char* endOfLine_;
    char* endOfBuffer_;
    bool endOfFile_;
};

StackFileReader::StackFileReader()
    : fd_(-1)
    , endOfLine_(NULL)
    , endOfBuffer_(&buf_[0])
    , endOfFile_(false)
{
    buf_[buf_size] = '\0';
}

StackFileReader::~StackFileReader()
{
    if (fd_ != -1)
        close(fd_);
}

bool StackFileReader::open(const char* fileName)
{
    int fd;
    fd = ::open(fileName, O_RDONLY);
    if (fd == -1) {
        LINLOG("open file fails.\n");
        return false;
    }
    fd_ = fd;
    return true;
}

const char* StackFileReader::readLine()
{
    if (endOfLine_) {
        long remainSize = endOfBuffer_ - endOfLine_;
        memmove(buf_, endOfLine_, remainSize);
        endOfBuffer_ = &buf_[remainSize];
    }

    if (!endOfFile_) {
        long remainSize = &buf_[buf_size] - endOfBuffer_;
        ssize_t readBytes;
        readBytes = read(fd_, endOfBuffer_, remainSize);
        if (readBytes <= 0) {
            endOfFile_ = true;
        } else {
            endOfBuffer_ += readBytes;
        }
        endOfBuffer_[0] = '\0';
    }
    endOfLine_ = strchr(buf_, '\n');
    if (endOfLine_) {
        endOfLine_[0] = '\0';
        endOfLine_++;
        return buf_;
    }
    return NULL;
}

SymbolResolver::SymbolResolver()
    : start_(NULL)
    , symTable_(NULL)
    , strTable_(NULL)
    , bucket_(NULL)
    , chain_(NULL)
    , nBucket_(0)
    , nChain_(0)
    , strTableSz_(0)
    , is_elf32_(true)
{
}

void* SymbolResolver::findLibcAddr()
{
    StackFileReader sfr;
    if (!sfr.open("/proc/self/maps")) {
        LINLOG("open file fails.\n");
        return NULL;
    }
    const char* line;
    while ((line = sfr.readLine())) {
        if (!strstr(line, "/libc.so"))
            continue;
        unsigned long long addr = strtoull(line, NULL, 16);
        return reinterpret_cast<void*>(addr);
    }
    return NULL;
}

static bool
check_elf(void* start, bool* is_elf32)
{
    unsigned char* e_ident = static_cast<unsigned char*>(start);
    if (memcmp(e_ident, ELFMAG, SELFMAG)) {
        LINLOG("fails for not a elf.\n");
        return false;
    }
    *is_elf32 = (e_ident[EI_CLASS] != ELFCLASS64);
    return true;
}

template <class _elf_header_type>
struct elf_deducer {
    typedef _elf_header_type elf_header_type;
};

template <>
struct elf_deducer<Elf32_Ehdr> {
    typedef Elf32_Ehdr elf_header_type;
    typedef Elf32_Phdr elf_phdr;
    typedef Elf32_Rela elf_rela;
    typedef Elf32_Rel elf_rel;
    typedef Elf32_Dyn elf_dynamic;
    typedef Elf32_Sym elf_sym;
};

template <>
struct elf_deducer<Elf64_Ehdr> {
    typedef Elf64_Ehdr elf_header_type;
    typedef Elf64_Phdr elf_phdr;
    typedef Elf64_Rela elf_rela;
    typedef Elf64_Rel elf_rel;
    typedef Elf64_Dyn elf_dynamic;
    typedef Elf64_Sym elf_sym;
};

template <class dynamic_type>
bool SymbolResolver::fill_with_dyn(void* start, void* end)
{
    char* base = static_cast<char*>(start_);
#if !defined(ANDROID)
    base = NULL;
#endif
    dynamic_type* pdyn = static_cast<dynamic_type*>(start);
    dynamic_type* pdyn_end = static_cast<dynamic_type*>(end);
    for (; pdyn < pdyn_end; ++pdyn) {
        switch (pdyn->d_tag) {
        case DT_SYMTAB:
            symTable_ = base + pdyn->d_un.d_ptr;
            break;
        case DT_STRTAB:
            strTable_ = base + pdyn->d_un.d_ptr;
            break;
        case DT_STRSZ:
            strTableSz_ = pdyn->d_un .d_val;
            break;
        case DT_HASH:
            nBucket_ = reinterpret_cast<uint32_t*>(base + pdyn->d_un.d_ptr)[0];
            nChain_ = reinterpret_cast<uint32_t*>(base + pdyn->d_un.d_ptr)[1];
            bucket_ = reinterpret_cast<uint32_t*>(base + pdyn->d_un.d_ptr + 8);
            chain_ = reinterpret_cast<uint32_t*>(base + pdyn->d_un.d_ptr + 8 + nBucket_ * 4);
            break;
        }
    }
    if (symTable_ == NULL
        || strTable_ == NULL
        || strTableSz_ == 0
        || bucket_ == NULL) {

        LINLOG("fails for target fields is incomplete, %p %p %d %p.\n", symTable_, strTable_, strTableSz_, bucket_);
        return false;
    }
    return true;
}

template <class elf_header_type>
bool SymbolResolver::deal_with_elf()
{
    elf_header_type* hdr;
    int i;
    typedef typename ::elf_deducer<elf_header_type>::elf_phdr elf_phdr;
    typedef typename ::elf_deducer<elf_header_type>::elf_rela elf_rela;
    typedef typename ::elf_deducer<elf_header_type>::elf_rel elf_rel;
    typedef typename ::elf_deducer<elf_header_type>::elf_dynamic elf_dynamic;
    hdr = static_cast<elf_header_type*>(start_);
    if (hdr->e_type != ET_DYN) {
        LINLOG("fails for target is not a shared library.\n");
        return false;
    }
    elf_phdr* phdr = reinterpret_cast<elf_phdr*>(static_cast<char*>(start_) + hdr->e_phoff);

    for (i = 0; i < hdr->e_phnum; ++i, phdr = reinterpret_cast<elf_phdr*>(reinterpret_cast<char*>(phdr) + hdr->e_phentsize)) {
        if (phdr->p_type == PT_DYNAMIC)
            break;
    }
    if (i == hdr->e_phnum) {
        LINLOG("fails for target has no program header.\n");
        return false;
    }
    void* start_of_dyn = static_cast<char*>(start_) + phdr->p_vaddr;
    void* end_of_dyn = static_cast<char*>(start_of_dyn) + phdr->p_memsz;
    if (!fill_with_dyn<elf_dynamic>(start_of_dyn, end_of_dyn)) {
        return false;
    }
    return true;
}

bool SymbolResolver::findMandatoryFields()
{
    start_ = findLibcAddr();
    if (!start_) {
        return false;
    }
    if (!check_elf(start_, &is_elf32_)) {
        return false;
    }
    if (is_elf32_)
        return deal_with_elf<Elf32_Ehdr>();
    else
        return deal_with_elf<Elf64_Ehdr>();
}

bool SymbolResolver::init()
{
    return findMandatoryFields();
}

static uint32_t elf_hash(const uint8_t* name)
{
    uint32_t h = 0, g;

    while (*name) {
        h = (h << 4) + *name++;
        g = h & 0xf0000000;
        h ^= g;
        h ^= g >> 24;
    }

    return h;
}

const char* SymbolResolver::getString(uint64_t offset)
{
    return static_cast<char*>(strTable_) + offset;
}

template <class elf_sym_type>
void* SymbolResolver::doFindSym(const char* symName)
{
    uint32_t hash = elf_hash(reinterpret_cast<const uint8_t*>(symName));
    for (uint32_t n = bucket_[hash % nBucket_]; n != 0; n = chain_[n]) {
        elf_sym_type* s = static_cast<elf_sym_type*>(symTable_) + n;

        if (strcmp(getString(s->st_name), symName) == 0) {
            return static_cast<char*>(start_) + s->st_value;
        }
    }
    return NULL;
}

void* SymbolResolver::findSym(const char* symName)
{
    if (is_elf32_) {
        return doFindSym<Elf32_Sym>(symName);
    } else {
        return doFindSym<Elf64_Sym>(symName);
    }
}
}

void initMyMalloc(void)
{
    SymbolResolver resolver;
    if (!resolver.init()) {
        LINLOG("fails to init resolver.\n");
        __builtin_trap();
    }
#define INIT_OR_CRASH(name) \
    my##name = reinterpret_cast<pfn##name>(resolver.findSym(#name)); \
    if (my##name == NULL) { \
        LINLOG("fails to get my" #name " symbol.\n"); \
        __builtin_trap(); \
    }
    INIT_OR_CRASH(malloc);
    INIT_OR_CRASH(free);
    INIT_OR_CRASH(calloc);
    INIT_OR_CRASH(realloc);
    INIT_OR_CRASH(memalign);
    INIT_OR_CRASH(malloc_usable_size);
    INIT_OR_CRASH(mmap);
    INIT_OR_CRASH(munmap);
}
