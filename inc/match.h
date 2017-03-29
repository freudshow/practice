#ifndef	__MATCH_H__
#define __MATCH_H__

extern int match(char *regexp, char *text);
extern int matchhere(char *regexp, char *text);
extern int matchstar(int c, char *regexp, char *text);

#endif //__MATCH_H__
