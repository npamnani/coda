#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LINE_LEN  1024
#define MAX_HIST_SIZE 1000

struct termios saved;
static char line[LINE_LEN];
static int prmt_len;
static int str_len;
static int cur_idx;
static char dummy_line[LINE_LEN];

struct dlist 
{
  struct dlist *next;
  struct dlist *prev;
  char *line;
};

struct history_struct {

  struct dlist start;
  struct dlist end;
  int max_size;
  int cur_size;
};

struct dlist dummy_node;
static struct history_struct history = {
                                        .max_size = 100,
                                        .start.next = &dummy_node,
                                        .end.prev = &dummy_node
                                       };

struct dlist dummy_node = {
                            .next = &history.end,
                            .prev = &history.start,
                            .line = dummy_line
                          };
static struct dlist *last_node = &dummy_node;

static inline void del_nodes(int count)
{
  while (count > 0 && history.cur_size > 0)
  {
    struct dlist *del_node = history.start.next;
    history.start.next = del_node->next; 
    del_node->next->prev = &history.start;
    free(del_node->line);
    free(del_node);
    --history.cur_size;
    --count;
  }
}

static void add_node(char *str)
{
  int len; 

  struct dlist *new_node;
  if (history.cur_size != 0 && strcmp(dummy_node.prev->line, str) == 0)
  {
    return;
  }

  if (history.cur_size == history.max_size)
  {
    del_nodes(1);
  }

  len = strlen(str);
  new_node = malloc(sizeof(*new_node));
  new_node->line = malloc(len + 1);
  memcpy(new_node->line, str, len + 1);
  
  new_node->next = &dummy_node;
  new_node->prev = dummy_node.prev;
  dummy_node.prev->next = new_node;
  dummy_node.prev = new_node;
  ++history.cur_size;
}

static int termsetup()
{
	struct termios term;
	if(tcgetattr(fileno(stdin), &term) < 0) {
		perror("Error getting terminal information");
		exit(1);
	}
  saved = term; 

	term.c_lflag &= ~ICANON;
	term.c_lflag &= ~ECHO;
	term.c_lflag |= ISIG;
	term.c_cc[VMIN]=1;
	term.c_cc[VTIME]=0;
	term.c_cc[VINTR]=0;
	term.c_cc[VQUIT]=0;

	if(tcsetattr(fileno(stdin), TCSANOW, &term) < 0) {
		perror("Error setting terminal information");
		return 1;
	}
  return 0;
}

static int termrestore()
{
	if(tcsetattr(fileno(stdin), TCSANOW, &saved) < 0) {
		perror("Error setting terminal information");
		return 1;
	}
  return 0;
}

static void write_stdout(char *str, int end)
{
  if (end < 0)
    end = 0;

  while(end--)
    putc('\b', stdout);
  if (str != NULL)
    fwrite(str, strlen(str), 1, stdout);
}

static void write_space(int count)
{
  if (count < 0)
    count = 0;
  while(count--)
    putc(' ', stdout);
}

static void display_down()
{
  
  if (last_node->next != &history.end)
    last_node = last_node->next;
  else
    return;
    
  {
    int len = strlen(last_node->line);
    memcpy(line + prmt_len, last_node->line, len + 1);
    write_stdout(line + prmt_len, cur_idx - prmt_len);
    write_space(str_len - prmt_len - len);
    write_stdout(0, str_len - prmt_len - len);
    cur_idx = str_len = len + prmt_len;
  }
}

static void display_up()
{

  if (last_node->prev != &history.start)
    last_node = last_node->prev;
  else
    return;

  {
    int len = strlen(last_node->line);
    memcpy(line + prmt_len, last_node->line, len + 1);
    write_stdout(line + prmt_len, cur_idx - prmt_len);
    write_space(str_len - prmt_len - len);
    write_stdout(0, str_len - prmt_len - len);
    cur_idx = str_len = len + prmt_len;
  }
}

static void metakey()
{
  int key = getchar();

  if (key == 91) 
  {
    key = getchar();
    switch(key)
    {
      case 65:
      {
        display_up();
        break;
      }
      case 66:
      {
        display_down();
        break;
      }
      case 67:
      {
        if (cur_idx < str_len) 
        {
          write_stdout(line + prmt_len, cur_idx - prmt_len);
          ++cur_idx;
          write_stdout(NULL, str_len - cur_idx);
        }
        break;
      }
      case 68:
      {
        if (cur_idx > prmt_len) 
        {
          write_stdout(NULL, 1);
          --cur_idx;
        }
      }
      case 71:
        break;
      default:
      {
        while(key != 126)
        {
          key = getchar();
        }
      }
    }
  }
}

static char* coda_getline(const char *prompt)
{
  int key;
  const char *rl_prmpt = (prompt == NULL || prompt[0] == '\0') ? 
                                                 "readline > " : prompt;

  display_prompt:

    memset(line, 0, LINE_LEN);
    cur_idx = str_len = prmt_len = strlen(rl_prmpt); 

    memcpy(line, rl_prmpt, prmt_len);
    write_stdout(line, 0);

	  do {
	  	key=getchar();
    
      switch(key)
      {
        case 27:
        {
          metakey();
          break;
        }
        case 10:
        {
          last_node = &dummy_node;
          dummy_node.line[0] = 0;
          write_stdout("\n", 0);
          return line;
        }
        case 1:
        {
          write_stdout(NULL, cur_idx - prmt_len);
          cur_idx =  prmt_len;
          break;
        }
        case 5:
        {
          write_stdout(line + prmt_len, cur_idx - prmt_len);
          cur_idx =  str_len;
          break;
        }
        case 23: //erase previous word
          break;
        case 28: //QUIT
        {
          break;
        }
        case 3: //INTR
        {
          write_stdout("^C\n", 0);
          dummy_node.line[0] = 0;
          last_node = &dummy_node;
          goto display_prompt;
        }
        case 4:
        {
          if (prmt_len == str_len)
          { 
            return NULL;
          }
          else if(cur_idx < str_len)
          {
            memmove(&line[cur_idx], &line[cur_idx + 1], str_len - cur_idx - 1);
            line[str_len - 1] = 32;
            write_stdout(line + prmt_len, cur_idx - prmt_len);
            write_stdout(NULL, str_len - cur_idx);
            line[--str_len] = 0;
            if (last_node == &dummy_node)
            {
              memcpy(dummy_node.line, line + prmt_len, str_len);
            }
          }
          break;  
        }
        case 127:
        {
          if (cur_idx > prmt_len)
          {
            memmove(&line[cur_idx - 1], &line[cur_idx], str_len - cur_idx);
            line[str_len - 1] = 32;
            write_stdout(line + prmt_len, cur_idx - prmt_len);
            write_stdout(NULL, str_len - cur_idx + 1);
            line[--str_len] = 0;
            --cur_idx;
            if (last_node == &dummy_node)
            {
              memcpy(dummy_node.line, line + prmt_len, str_len);
            }
          }
          break;
        }
        case 12:
        {
          putc('\f', stdout);
          write_stdout(line ,0);
          write_stdout(NULL, str_len - cur_idx);
          break;
        }
        default:
        {
          if (iscntrl(key))
            break;

          if ((str_len + 1) <= LINE_LEN)
          {
            memmove(&line[cur_idx + 1], &line[cur_idx], str_len - cur_idx);
            line[cur_idx] = (unsigned char) key;
            line[str_len + 1] = 0;
            write_stdout(line + prmt_len, cur_idx - prmt_len);
            write_stdout(NULL, str_len - cur_idx);
            ++cur_idx;
            ++str_len;
            memcpy(dummy_node.line, line + prmt_len, str_len);
          }
        }
      }
	  } while(1);
}

char* coda_readline(const char *prompt)
{
  char *rline;

  if (!isatty(fileno(stdin)) || !isatty(fileno(stdout)))
  {
		perror("coda_readline: Not a terminal");
    return NULL;
  }

  if (termsetup())
    return NULL;

  rline = coda_getline(prompt);

  termrestore();
 
  if (rline) {
    char *new_mem = malloc(str_len - prmt_len + 1); 
    memcpy(new_mem, rline + prmt_len, str_len - prmt_len);
    new_mem[str_len - prmt_len] = 0;
    rline = new_mem;
	}
  return rline;
}

void coda_add_history(char *str)
{
  if (str == NULL || str[0] == '\0')
    return;

  add_node(str);
}

void coda_history_size(int hist_size)
{
  if (hist_size < 0 || hist_size > MAX_HIST_SIZE)
    return;

  if (history.cur_size > hist_size) 
    del_nodes(history.cur_size - hist_size);
  history.max_size = hist_size;
}
