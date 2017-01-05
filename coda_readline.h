#ifndef __CODA_READLINE_H__
#define __CODA_READLINE_H__

#ifdef __cplusplus
extern "C" {
#endif

char* coda_readline(const char *prompt);
void coda_add_history(char *str);
void coda_history_size(int hist_size);

#ifdef __cplusplus
}
#endif

#endif
