//writer nishant.pamnani@gmail.com
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <setjmp.h>

#include <iostream>
#include <string>

#include "coda.h"
#include "coda_readline.h"

typedef int (*COMMAND)(CoreObject*,int,char**);

struct command_struct{
  const char *command_name;                   
  COMMAND command;
  const char *help;
};

extern command_struct cmds[];

int command_invalid(CoreObject*, int , char**)
{
  std::cout << "Invalid command " << std::endl;
  std::cout << "type 'help'" << std::endl;
  return 0;
}

int command_help(CoreObject*, int argc, char**)
{
  if (argc > 0) 
  {
    std::cout << "Invalid argument" << std::endl;
    return 1;
  }
  int ndx = 0;
  while(cmds[ndx].command != command_invalid)
  {
    std::cout << std::setw(10) << std::left 
              << std::setfill(' ') 
              << cmds[ndx].command_name << " - " << cmds[ndx].help 
              << std::endl;
    ++ndx;
  }
  return 0;
}

int command_dis(CoreObject* co, int argc, char** argv)
{
  if (argc > 1)
  {
    std::cout << "Invalid argument" << std::endl;
    return 1;
  }

  if (argc > 0)
  {
    uint64_t addr; 
    char *saveptr = 0;
    addr = strtoul(argv[0],&saveptr,0);
    if (*saveptr != '\0')
    {
      std::cout << "Invalid argument" << std::endl;
      return 1;
    }
    co->ShowDisassembly(addr);
    return 0;
  }
  else 
    std::cout << "Argument missing" << std::endl;
  return 1;
}

int command_quit(CoreObject*, int, char**)
{
  exit(0);
}

int command_bt(CoreObject* co, int argc, char**)
{
  if (argc > 0) 
  {
    std::cout << "Invalid argument" << std::endl;
    return 1;
  }
  co->ShowBT();
  return 0;
}

int command_btregs(CoreObject* co, int argc, char**)
{
  if (argc > 0) 
  {
    std::cout << "Invalid argument" << std::endl;
    return 1;
  }
  co->ShowBT(true);
  return 0;
}

int command_mmap(CoreObject* co, int argc, char**)
{ 
  if (argc > 0) 
  {
    std::cout << "Invalid argument" << std::endl;
    return 1;
  }
  co->ShowMemoryMap();
  return 0;
}

int command_demangle(CoreObject* co, int argc, char** argv)
{ 
  if (argc > 1)
  {
    std::cout << "Invalid argument" << std::endl;
    return 1;
  }
  else if (argc < 1)
  {
    std::cout << "Argument missing" << std::endl;
    return 1;
  }

  if (strcmp(argv[0],"1") == 0)
  {
    co->EnableDemangle();
    std::cout << "Demangling enabled." << std::endl;
    return 0;
  }
  else if (strcmp(argv[0],"0") == 0)
  {
    co->DisableDemangle();
    std::cout << "Demangling disabled." << std::endl;
    return 0;
  }
  std::cout << "Invalid argument." << std::endl;
  return 1;
}

int command_thread(CoreObject* co, int argc, char** argv)
{ 
  if (argc > 1)
  {
    std::cout << "Invalid argument" << std::endl;
    return 1;
  }
  size_t threadno = 0; 
  if (argc > 0)
  {
    char *saveptr = 0;
    threadno = strtoul(argv[0],&saveptr,0);
    if (*saveptr != '\0')
    {
      std::cout << "Invalid argument" << std::endl;
      return 1;
    }
    co->SwitchToThread(threadno);
    return 0;
  }
  co->SwitchToThread();
  return 0;
}

int command_threads(CoreObject* co, int argc, char**)
{ 
  if (argc > 0)
  {
    std::cout << "Invalid argument" << std::endl;
    return 1;
  }
  co->ShowThrList();
  return 0;
}

int command_strings(CoreObject* co, int argc, char**argv)
{
  if (argc > 1)
  {
    std::cout << "Invalid argument" << std::endl;
    return 1;
  }
  
  char *regex = argv[0];
  return co->ShowStrings(regex);
  
}

int handle_arguments(int argc, char**argv,uint64_t *paddr, size_t *punits)
{
  if (argc > 2)
  {
    std::cout << "Invalid argument" << std::endl;
    return 1;
  }

  char *saveptr = 0;
  if ((0 < argc) && (argc <= 2))
  {
    *paddr = strtoul(argv[0],&saveptr,0);
    if (*saveptr != '\0')
    {
      std::cout << "Invalid argument" << std::endl;
      return 1;
    }
  }
  else 
  {
    std::cout << "Argument missing" << std::endl;
    return 1;
  }

  if (argc == 2)
  {
    saveptr = 0;
    *punits = strtoul(argv[1],&saveptr,0);
    if (*saveptr != '\0')
    {
      std::cout << "Invalid argument" << std::endl;
      return 1;
    }
  }

  return 0;
}

int command_xuint(CoreObject* co, int argc, char**argv)
{ 
  uint64_t addr; 
  size_t units = 1; 
  if(handle_arguments(argc,argv,&addr,&units))
    return 1;
  co->ShowValueAtVA<unsigned int>(addr,units);
  return 0;
}

int command_xuchar(CoreObject* co, int argc, char**argv)
{ 
  uint64_t addr; 
  size_t units = 1; 
  if(handle_arguments(argc,argv,&addr,&units))
    return 1;
  co->ShowValueAtVA<unsigned char>(addr,units);
  return 0;
}

int command_xulong(CoreObject* co, int argc, char**argv)
{ 
  uint64_t addr; 
  size_t units = 1; 
  if(handle_arguments(argc,argv,&addr,&units))
    return 1;
  co->ShowValueAtVA<unsigned long>(addr,units);
  return 0;
}

int command_xint(CoreObject* co, int argc, char**argv)
{ 
  uint64_t addr; 
  size_t units = 1; 
  if(handle_arguments(argc,argv,&addr,&units))
    return 1;
  co->ShowValueAtVA<int>(addr,units);
  return 0;
}

int command_xlong(CoreObject* co, int argc, char**argv)
{ 
  uint64_t addr; 
  size_t units = 1; 
  if(handle_arguments(argc,argv,&addr,&units))
    return 1;
  co->ShowValueAtVA<long>(addr,units);
  return 0;
}

int command_xaddr(CoreObject* co, int argc, char**argv)
{ 
  uint64_t addr; 
  size_t units = 1; 
  if(handle_arguments(argc,argv,&addr,&units))
    return 1;
  co->ShowValueAtVA<uint64_t>(addr,units);
  return 0;
}

command_struct cmds[] = {
  { "help",    command_help,    
           "Help for all interactive commands." },
  { "demangle",command_demangle,   
           "demangle 0|1" },
  { "dis",     command_dis,   
           "Show disassembly of the function." },
  { "btregs",  command_btregs,
           "Show backtrace of the selected thread with registers." },
  { "bt",      command_bt,    
           "Show backtrace of the selected thread." },
  { "mmap",    command_mmap,  
           "Show memory map." },
  { "thread",  command_thread,   
           "thread <number>." },
  { "threads", command_threads,   
           "Show all threads." },
  { "strings", command_strings,   
           "Extract strings from coredump under probe."
           "\n\t\tUsage: strings <pattern>" },
  { "xuint" ,  command_xuint, 
           "Extract unsigned int." 
           "\n\t\tUsage: xuint addr <count>" },
  { "xchar",   command_xuchar,
           "Extract char." 
           "\n\t\tUsage: xchar addr <count>" },
  { "xulong",  command_xulong,
           "Extract unsigned long." 
           "\n\t\tUsage: xulong addr <count>" },
  { "xint" ,   command_xint,  
           "Extract int." 
           "\n\t\tUsage: xint addr <count>" },
  { "xlong",   command_xlong, 
           "Extract long." 
           "\n\t\tUsage: xlong addr <count>" },
  { "xaddr",   command_xaddr, 
           "Extract address." 
           "\n\t\tUsage: xaddr addr <count>" },
  { "q",       command_quit,   
           "Exit program." },
  { "invalid", command_invalid,
           "Invalid command."}
};

COMMAND command(char *cmd)
{
  int ndx = 0;
  while(cmds[ndx].command != command_invalid)
  {
    if (strcmp (cmd, cmds[ndx].command_name) == 0)
      return cmds[ndx].command;
    ++ndx;
  }
  return cmds[ndx].command;
}

void handle_input(CoreObject *co, char *str)
{
  const int MAX_ARGS = 10 + 1;
  int   argc = 0;
  char *argv[MAX_ARGS] = {0};

  char *saveptr;
  for( ;argc < MAX_ARGS - 1; ++argc,str = NULL)
  {
    argv[argc] = coda_strtok(str,&saveptr);
    if (argv[argc] == NULL)
      break;
  }

  if (0 == argc)
    return;
  command(argv[0])(co,argc-1,&argv[1]);

}

char const* cmd_completer(char const *str_to_com, int str_len, int init)
{

  static command_struct *traverse = &cmds[0];

  if (NULL == str_to_com)
    return NULL;

  if (init)
    traverse = &cmds[0];

  while (traverse->command != command_invalid)
  {
    if (!strncmp(traverse->command_name, str_to_com, str_len))
    {
      return traverse++->command_name;
    }
    ++traverse;
  }

  return NULL;
}

void
CoreObject::Switch2InteractiveMode()
{
  std::string line;
  bool notty = false;
  m_interactive_mode = true;
  signal(SIGINT, SIG_IGN);

  if (!isatty(fileno(stdin)) || !isatty(fileno(stdout)))
  {
    notty = true;
    m_interactive_mode = false;
  }

  if (m_interactive_mode)
  {
    set_completer(cmd_completer);
    WelcomeMessage();
  }

  while (1)
  {
    char *str;
    if (notty)
    {
      std::getline(std::cin,line);
      if (std::cin.eof())
        break;
      str = (char*)line.c_str(); 
    }
    else 
    {
      str = coda_readline("coda > ");
      if (str == NULL)
        break;
      coda_add_history(str);
    }

    try 
    {
      handle_input(this, str);
    }
    catch (Done &done)
    {
    }
    if (!notty)
      free(str);
  }
  std::cout << std::endl;
  exit(0);
}
