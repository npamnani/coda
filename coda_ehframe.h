//writer nishant.pamnani@gmail.com
#ifndef __CODA_EHFRAME_H__
#define __CODA_EHFRAME_H__

#include "coda_utils.h"
#include "coda.h"

#include <string.h>
#include <map>


typedef unsigned char UBYTE;

struct VirtualAddr
{
  uint64_t va;
  uint64_t adjust;
};

struct FDETable
{
  int InitialLocation;
  int FDEAddress;
};

struct UnwEHFrameHdr
{
  unsigned char Version;
  unsigned char EHFramePtrEnc;
  unsigned char FDECountEnc;
  unsigned char TableEnc;
};

enum Regs {
  RAX, RDX, RCX, RBX, 
  RSI, RDI, RBP, RSP,
  R8, R9, R10, R11,
  R12, R13, R14, R15,
  RAR, RMAX
};

typedef uint64_t  ArcReg;

struct Activation; 
class CanonicalFrameAddr{

  enum CFARule { EXPR, NOEXPR };
  public:
    void CFA(uint64_t reg,uint64_t off)
    {
      cur_reg = reg;
      cur_offset = off;
      cur_rule =  NOEXPR;
    }
    void CFANewReg(uint64_t reg)
    {
      CFA(reg,cur_offset);
    }
    void CFANewOffset(uint64_t off)
    {
      CFA(cur_reg,off);
    }
    void CFAExpression(UBYTE *start, UBYTE *end)
    {
      cfi = start;
      end_cfi = end;
      cur_rule = EXPR;
    }
    uint64_t CFA(Activation*); 
  private:
    uint64_t cur_reg;
    uint64_t cur_offset;
    CFARule cur_rule;
    UBYTE * cfi;
    UBYTE * end_cfi;
};

class RegisterRule {
  enum RegRule{
    UNDEF,
    SAMEVAL,
    OFFSET,
    VAL_OFFSET,
    REGISTER,
    EXPR,
    VAL_EXPR,
    ARCH
  };

  public:
    RegisterRule():cur_rule(UNDEF) {}
    bool IsUndefined() {return UNDEF == cur_rule;}
    void Undefined() {cur_rule = UNDEF; }
    void SameValue() {cur_rule = SAMEVAL; }
    bool IsSameValue() {return SAMEVAL == cur_rule;}
    void Offset(int64_t off) 
    { 
      cur_rule = OFFSET; 
      cur_off = off;
    }
    void ValOffset(int64_t off)
    {
      cur_rule = VAL_OFFSET; 
      cur_off = off;
    }
    void Register(uint64_t reg)
    {
      cur_rule = REGISTER; 
      cur_reg = reg;
    }
    void Expression(UBYTE* start,UBYTE* end)
    {
      cur_rule = EXPR; 
      cfi = start;
      end_cfi = end;
    }
    void ValExpression(UBYTE* start,UBYTE* end)
    {
      cur_rule = VAL_EXPR; 
      cfi = start;
      end_cfi = end;
    }
    uint64_t GetRegVal(Activation*);

  private:
    int64_t cur_off;
    uint64_t cur_reg;
    RegRule cur_rule;
    UBYTE * cfi;
    UBYTE * end_cfi;
};

typedef RegisterRule RegRules[RMAX];

struct Activation {
  bool is_init;
  uint64_t pc;
  ArcReg *regs;
  CanonicalFrameAddr CFA;
  RegRules regrules;
  RegRules initregrules;
};

struct State  {
  CanonicalFrameAddr CFA;
  RegRules regrules;
};

struct CIEInfo
{
  UBYTE *cie_ins;
  UBYTE *cie_ins_end;
  uint64_t caf;
  int64_t daf;
  uint64_t rar;
  unsigned char fde_enc;
  bool is_signal_handler;
};

struct FrameInfo {
  CIEInfo *cie_info;
  UBYTE *fde_ins;
  UBYTE *fde_ins_end;
  ArcReg *inregs;
  ArcReg *outregs;
  uint64_t pc;
  uint64_t pc_end;
  uint64_t pc_to_match;
};

typedef off_t ABSOff; //absolute offset wrt to core file.
typedef std::map<ABSOff, CIEInfo*> MapCIEInfo;
typedef std::map<ABSOff, FrameInfo*> MapFrameInfo;

class EHFrame {
  public: //methods
    EHFrame(CoreObject *co,File &cf,bool regs = false)
                  :m_corefile(cf),m_co(co),m_regs(regs){}
    void PrintBT(const user_regs_struct *urs);
    FrameInfo* FindFrameInfo(uint64_t va);
  private: //methods
    typedef CoreObject::ObjectEntry ObjectEntry;
    void PrintFrame(ArcReg *regs, int fnum, FrameInfo *fi, int va_adjusted = 0);
    void AdjustTopFrame(ArcReg *regs, int *pfnum);
    void InterpretCFInstructions(Activation*, FrameInfo*);
    bool GetPreviousFrame(int, FrameInfo*);
    ElfW(Phdr)* FindEHFrameHdr(uint64_t va);
    FrameInfo* GetFrameInfo(off_t off2fde, uint64_t va_fde);
    CIEInfo* GetCIEInfo(off_t off2cie);
    uint64_t DecodeFDEEnc(UBYTE enc, UBYTE **cptr, VirtualAddr* fde_va);
    bool GetPreviousFrame(FrameInfo* fi);
    UBYTE getSize(UBYTE enc);
   
    ObjectEntry* FindObjectEntry(uint64_t va, bool filesz = true)
    {
      return m_co->FindObjectEntry(va,filesz);
    }

    template <typename T>
    T GetValueAtVA(uint64_t va)
    {
      return m_co->GetValueAtVA<T>(va);
    }

    bool ElfHdr(ElfW(Ehdr) &ehdr)
    {
      return m_co->ElfHdr(ehdr);
    }
  private: //data members
    File &m_corefile;
    CoreObject *m_co;
    uint64_t EHFramePtr;
    bool m_regs;
};

typedef Stack<uint64_t> DWOpStk;
#endif

