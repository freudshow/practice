#include "match.h"

/*match :在text中查找regexp*/
int match(char *regexp, char *text)
{
	if (regexp[0] == '^')
		return matchhere(regexp + 1, text);
	do { /*即使字符串为空也必须检查*/
		if (matchhere(regexp, text))
			return 1;
	} while (*text++ != '\0');
	return 0;
}

/*matchhere在text的开头查找regexp*/
int matchhere(char *regexp, char *text)
{
	if (regexp[0] == '\0')
		return 1;
	if (regexp[1] == '*')
		return matchstar(regexp[0], regexp + 2, text);
	if (regexp[0] == '$' && regexp[1] == '\0')
		return *text == '\0';
	if (*text != '\0' && (regexp[0] == '.' || regexp[0] == *text))
		return matchhere(regexp + 1, text + 1);
	return 0;
}

/*matchstar :在text的开头查找C*regexp*/
int matchstar(int c, char *regexp, char *text)
{
	do { /*通配符*匹配零个或多个实例*/
		if (matchhere(regexp, text))
			return 1;
	} while (*text != '\0' && (*text++ == c || c == '.'));
	return 0;
}
