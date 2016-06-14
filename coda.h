//writer nishant.pamnani@gmail.com
#ifndef __CODA_H__
#define __CODA_H__

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <link.h>
#include <sys/procfs.h>

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <typeinfo>

#include "coda_utils.h"

#define offset_of(TYPE,mem)       (unsigned long)(&((TYPE*)0)->mem)
#define LIB_NAME_LEN 2048
#define CURRENT_THREAD  0xffffffff

inline void Paginate(bool imode, int linecount)
{
  if (imode)
  {
    if(24 == linecount % 25)
    {
      std::cout << "Type q to quit:";
      std::string line;
      std::getline (std::cin,line);
      if ('q' == line[0])
      {
        throw Done("done");
      }
    }
  }
}

enum RetCode {
  SUCCESS = 0,
  PART_SUCCESS = 1,
  FALIURE = -1
};

extern bool coda_debug;

class CoreObject {

  public: // structs
    struct ObjectEntry {
      std::string access;
      std::string filename;
      ElfW(Phdr) *phdr;
      uint64_t symtab;
      uint64_t strtab;
//Assuming gnu hash table
      uint64_t hashtab;
      uint64_t strtab_size;
      uint64_t base_addr;
      uint32_t sym_count;
      bool symtab_loaded;
      bool cant_load_symtab;
    };

    struct ProcessContext {
      elf_prpsinfo *m_prinfo;
      std::vector<elf_prstatus*> m_vprst;
    };

  public: // methods
    static CoreObject& GetCoreObject(char const *cfname = 0);
    ObjectEntry* FindObjectEntry(uint64_t va, bool filesz = true);
    RetCode Addr2Name(uint64_t va, std::string &symname);
    void ShowMemoryMap(); 
    void ShowThrList();
    void ShowBT(bool regs = false);
    void ShowBTAll();
    void ShowIfMinidump();
    void SwitchToThread(size_t thrno = CURRENT_THREAD);
    void EnableDemangle() {m_demangle = true;}
    void DisableDemangle() {m_demangle = false;}
    bool IsMiniDump() {return m_mini_dump;}
    void Switch2InteractiveMode();
    template <typename T>
    T GetValueAtVA(uint64_t va);
    template <typename T>
    T* GetValueAtVA(uint64_t va, size_t units);
    template <typename T>
    void ShowValueAtVA(uint64_t va, size_t units = 1);
    bool ElfHdr(ElfW(Ehdr) &ehdr)
    {
      return (ELFMAG0 == ehdr.e_ident[EI_MAG0]) &&
             (ELFMAG1 == ehdr.e_ident[EI_MAG1]) &&
             (ELFMAG2 == ehdr.e_ident[EI_MAG2]) &&
             (ELFMAG3 == ehdr.e_ident[EI_MAG3]);
    }
    
    ElfW(Ehdr) CoreHdr() {return m_core_ehdr;}
    void ShowDisassembly(uint64_t va);
    bool IsInteractiveMode() {return m_interactive_mode;}
  private: // methods
    CoreObject(char const *cfname);
    CoreObject(CoreObject const &);
    CoreObject const & operator=(CoreObject const &);
    ~CoreObject(){}

    void CheckArch()
    {
      if ( EM_X86_64 != m_core_ehdr.e_machine )
      {
        std::cerr << "Architecture not recognized!\n";
        exit(1);
      }
    }

    void IsCoreFile()
    {
      if ( ET_CORE != m_core_ehdr.e_type ) 
      {
        std::cerr << "Not a CORE file!\n";
        exit(1);
      }
    }

    void ExtractPhdrs();
    void ExtractProcessVitals();
    void ExtractSharedLibraries();
    uint64_t ExtractDT_DEBUG(uint64_t ba, uint64_t va);
    void AccessSharedLibsNames(uint64_t va);
    void SetSegmentName(uint64_t va, uint64_t bva, const char *libname);
    RetCode FindSymbol(ObjectEntry *oe, uint64_t va, std::string &symname);
    void PrintBT(const user_regs_struct *urs);
    void WelcomeMessage();
    void Process_NT_FILE_Note();
    void SetOrModifySegmentName(uint64_t va, const char *segname);

  private: // structs
    struct nt_file_element {
      unsigned long start;
      unsigned long end;
      unsigned long pgoff;
    };

  private: //data members

    File m_corefile; 
    ElfW(Ehdr) m_core_ehdr;
    std::vector<ObjectEntry*>  m_object_map;
    ProcessContext m_pr_ctx;
    ElfW(Phdr) *m_note_phdr;
    uint64_t m_aux_phdr_va;
    char *m_progname;
    char *m_commandline;
    char *m_nt_file_sec;
    unsigned m_current_thread;
    bool m_pthreads;
    bool m_mini_dump;
    bool m_demangle;
    bool m_interactive_mode;
    bool m_found_nt_file;
};
 
template <typename T>
T CoreObject::GetValueAtVA(uint64_t va)
{
  ObjectEntry *oe = FindObjectEntry(va);
  if (!oe)
    throw std::runtime_error("Cant access VA");

  if ((va + sizeof(T)) >= 
       (oe->phdr->p_vaddr + oe->phdr->p_filesz))
    throw std::runtime_error("Invalid access");
  off_t off = oe->phdr->p_offset + (va - oe->phdr->p_vaddr);
  T value;
  m_corefile  >> File::Offset(off) >> value;

  return value;
}

template <typename T>
T* CoreObject::GetValueAtVA(uint64_t va, size_t units)
{
  ObjectEntry *oe = FindObjectEntry(va);
  if (!oe)
    throw std::runtime_error("Cant access VA");

  if ((va + sizeof(T) * units) >= 
       (oe->phdr->p_vaddr + oe->phdr->p_filesz))
    throw std::runtime_error("Invalid access");
  off_t off = oe->phdr->p_offset + (va - oe->phdr->p_vaddr);
  T* value;
  value = new T[units];
  m_corefile  >> File::Offset(off) >>  File::Units(units) >> value;

  return value;
}

inline void UIntPrint(void *arg)
{
  unsigned UI = *(unsigned*)arg;
  std::cout << std::dec << UI;
}

inline void ULongPrint(void *arg)
{
  unsigned long UL = *(unsigned long*)arg;
  std::cout << std::dec << UL;
}

inline void LongPrint(void *arg)
{
  long L = *(long*)arg;
  std::cout << std::dec << L;
}

inline void IntPrint(void *arg)
{
  int I = *(int*)arg;
  std::cout << std::dec << I;
}

inline void AddrPrint(void *arg)
{
  uint64_t Addr = *(uint64_t*)arg;
  std::cout << "0x" << std::hex << Addr;
}

inline void CharPrint(void *arg)
{
  unsigned char Ch = *(unsigned char*)arg;
  std::cout << "'" << Ch << "' 0x" << std::hex << (int)Ch;
}

template <typename T>
void CoreObject::ShowValueAtVA(uint64_t va,size_t units)
{

  T *pobj; 
 
  try {
    pobj = GetValueAtVA<T>(va,units);
  }
  catch (std::runtime_error &eObj)
  {
    std::cout << eObj.what() << std::endl;
    return;
  }

  void (*TypePrint)(void*);
  if(typeid(T) == typeid(unsigned char))
   TypePrint = CharPrint;
  else if(typeid(T) == typeid(uint64_t))
   TypePrint = AddrPrint;
  else if(typeid(T) == typeid(long)) 
   TypePrint = LongPrint;
  else if(typeid(T) == typeid(int))
   TypePrint = IntPrint;
  else if(typeid(T) == typeid(unsigned long)) 
   TypePrint = ULongPrint;
  else if(typeid(T) == typeid(unsigned int))
   TypePrint = UIntPrint;

  for(size_t count = 0; count < units; ++count)
  {
    if(count != 0)
      std::cout << ", ";
    TypePrint(&pobj[count]);
  }
  std::cout << std::endl;
  delete [] pobj;
}

#endif
