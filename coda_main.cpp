//writer nishant.pamnani@gmail.com
#include <stdexcept>
#include <map>
#include "coda.h"

void Usage(char **argv)
{
  std::cerr << "Usage: " << argv[0] << " <options> " << "corefile" << std::endl
            << "options:" << std::endl
            << " -B Backtrace all" << std::endl
            << " -m Show, if mini-coredump" << std::endl
            << " -d Enable de-mangling" << std::endl
            << " -i Interactive mode" << std::endl;
}

class ArgInterp {
  public:

    typedef void (CoreObject::*Func_t)();
    typedef std::map<std::string, Func_t>::iterator map_iterator;
    ArgInterp(int argc, char **argv);

    class Iterator {
      public:
        Iterator(map_iterator & iter,ArgInterp *aIP):map_iter(iter),pArg(aIP){}
        Iterator operator ++();
        Iterator operator ++(int);
        void operator ()();
        bool operator !=(Iterator const & iter);
      private:
        map_iterator map_iter;
        ArgInterp * pArg;
    };

    Iterator begin();
    Iterator end();
    std::string const & fileName() const  { return filename;}
    std::string const & progName() const { return progname;}
  private:
    std::map<std::string,Func_t> funcList;
    std::string filename;
    std::string progname;
};

ArgInterp::ArgInterp(int argc, char **argv)
{
  progname = argv[0];
  bool demangle = false;
  for(int ndx = 1;argv[ndx];ndx++)
  {
    if (!std::string("-B").compare(argv[ndx]))
      funcList["4.Backtrace full"] = &CoreObject::ShowBTAll;
    else if (!std::string("-m").compare(argv[ndx]))
      funcList["3.Mini dump"] = &CoreObject::ShowIfMinidump;
    else if (!std::string("-i").compare(argv[ndx]))
      funcList["2.Interactive"] = &CoreObject::Switch2InteractiveMode;
    else if (!std::string("-d").compare(argv[ndx]))
      funcList["1.Demangle"] = &CoreObject::EnableDemangle,demangle = true;
    else if (!std::string("-h").compare(argv[ndx]))
      throw std::invalid_argument ("Help Requested");
    else if (!std::string(argv[ndx]).substr(0,1).compare("-"))
      throw std::invalid_argument(std::string("Invalid option ") + argv[ndx]);
    else if (filename.size()) 
      throw std::invalid_argument(std::string("Extraneous Argument ") + argv[ndx]);
    else
      filename= argv[ndx];
  }
  if (!filename.size())
    throw std::invalid_argument ("Argument Missing, provide (corefile)");
  if (!funcList.size())
  {
    throw std::invalid_argument ("No option provided");
  } 
  else if (funcList.size() == 1 && demangle == true)
  {
    throw std::invalid_argument ("Provide option -d with other option");
  } 
} 

ArgInterp::Iterator ArgInterp::Iterator::operator ++()
{
     ++map_iter;
     return *this;
}
ArgInterp::Iterator ArgInterp::Iterator::operator ++(int)
{
   Iterator tmp = *this;
   ++*this;
   return tmp;
}
void ArgInterp::Iterator::operator ()()
{
  CoreObject & cobj = CoreObject::GetCoreObject(pArg->fileName().c_str());
  (cobj.*map_iter->second)();
}
bool ArgInterp::Iterator::operator !=(Iterator const & iter)
{
    return this->map_iter != iter.map_iter;
}
   
ArgInterp::Iterator ArgInterp::begin()
{
   map_iterator iter= funcList.begin();
   return Iterator(iter,this);
}

ArgInterp::Iterator ArgInterp::end()
{
  map_iterator iter= funcList.end();
  return Iterator(iter,this);
}

int main(int argc, char **argv)
{

  std::string prog;
  std::string corefile;
  try
  {
    ArgInterp argp(argc,argv);
    prog = argp.progName();
    corefile = argp.fileName();
    ArgInterp::Iterator iter = argp.begin();
    while(iter != argp.end())
    {
      iter();
      ++iter;
    }
  }

  catch (std::runtime_error &cObj)
  {
    std::cerr << cObj.what() << std::endl;
  }
  catch (std::invalid_argument &cObj)
  {
    std::cerr << cObj.what() << std::endl;
    Usage(argv);
  }
  catch (std::out_of_range &cObj)
  {
    std::cerr << cObj.what() << std::endl;
  }
  return 0;

}
