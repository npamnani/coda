#ifndef __CODA_READLINE_H__
#define __CODA_READLINE_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef char const* (*completer_type)(char const *str_to_com, int str_len, int init);
char* coda_readline(const char *prompt);
void set_completer(completer_type cmp);
void coda_add_history(char *str);
void coda_history_size(int hist_size);

#ifdef __cplusplus
}
#endif

#endif
