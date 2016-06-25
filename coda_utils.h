//writer nishant.pamnani@gmail.com
#ifndef __CODA_UTILS_H__
#define __CODA_UTILS_H__

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdexcept>
#include <iostream>

class Done: public std::exception {
  public:
   Done(const std::string &str)
   {
     message_ = str;
   }
  const std::string& what()
  {
    return message_;
  } 
  ~Done() throw() {}
  private:
    std::string message_;
};

class File {

  public:
    class Offset {
      public:
        Offset(off_t offset): offset_(offset){}
        operator off_t() const {return offset_;}
      private:
        off_t offset_;
    };

    class Units {
      public:
        Units(unsigned units): units_(units){}
        operator unsigned() const {return units_;}
      private:
          unsigned units_;
    };

    explicit File(char const *fname):m_fname(fname),m_units(1)
    {
      if (fname == NULL)
        throw std::runtime_error("File name is NULL");
      m_fd = open(fname,O_RDONLY);
      if ( m_fd < 0 )
      {
        throw std::runtime_error("Unable to open the file");
      }
    }
    std::string const& GetFileName()
    {
      return m_fname;
    }

    template <typename T>
    File& operator >>(T *pobj)
    {
      if ( read(m_fd, pobj, sizeof(T) * m_units) < 0 )
      {
        abort();
      }

      m_units = 1;
      return *this;
    }

    template <typename T>
    File& operator >>(T &obj)
    {
      if ( read(m_fd, &obj, sizeof(T)) < 0 )
      {
        abort();
      }
      return *this;
    }

    File& operator >>(Offset const &offset)
    {
      if ( lseek(m_fd, offset, SEEK_SET) < 0 )
      {
        abort();
      }
      return *this;
    }

    File& operator >>(Units const &units)
    {
      m_units = units;
      return *this;
    }

    off_t Size() const
    {
      struct stat sbuff;
      if (fstat(m_fd, &sbuff) < 0)
        return -1;
      return sbuff.st_size;
    }
  private:

    std::string m_fname;
    int m_fd;
    unsigned m_units;
};

template <typename T>
class Stack{

  public:
    Stack(unsigned cap = 512):_capacity(cap),_size(0)
    {
      arr = new T*[_capacity];
    }
    ~Stack()
    {
      for (unsigned ndx = 0 ; ndx < _size; ++ndx)
        delete arr[ndx];
      delete [] arr;
    }

    void push(T const &elm)
    {

      if (_size >= _capacity)
        throw std::runtime_error("Stack Container is Full");
      T *tmp = new T;
      *tmp = elm;
      arr[_size++] = tmp;
    }

    T top()
    {
      return *arr[_size - 1];
    }

    void pop()
    {
      T *tmp = arr[--_size];
      delete tmp;
    }

    bool empty()
    {
      return _size == 0;
    }

  private:
    T **arr;
    unsigned _capacity;
    unsigned _size;
};

inline void Paginate(bool imode, int linecount)
{
  if (imode)
  {
    if(18 == linecount % 19)
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

static inline
char *coda_strtok(char *str,char **saveptr)
{

  char *lstr = str?str:*saveptr;
  char *start = *lstr ? lstr:NULL;
  int quote_on = 0;
  int encountered_delim = 0;
  while(*lstr != '\0')
  {
    if (!quote_on && (*lstr == ' ' || *lstr == '\t'))
    {
        *lstr = '\0';
        encountered_delim = 1;
    }
    else if (*lstr == '\'' && !encountered_delim)
    {
      if ( !quote_on )
      {
        quote_on = 1;
        start = lstr + 1;
      }
      else
      {
        quote_on = 0;
        *lstr = '\0';
        encountered_delim = 1;
      }
    }
    else if (encountered_delim)
      break;
    ++lstr;
  }
  if (quote_on)
  {
    printf("quote not closed\n");
    return NULL;
  }
  *saveptr = lstr;
  return start;
}

#endif
