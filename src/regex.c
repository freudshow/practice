#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))

static const char *const str =
       "1) John Driverhacker;\n2) John Doe;\n3) John Foo;\n";
static const char *const re = "John.*o";

int main(void)
{
    static const char *s = str;
    regex_t regex;
    regmatch_t pmatch[1];
    regoff_t off, len;

    if (regcomp(&regex, re, REG_NEWLINE))
        exit(EXIT_FAILURE);

    printf("String = \"%s\"\n", str);
    printf("Matches:\n");

    for (int i = 0;; i++)
    {
        if (regexec(&regex, s, ARRAY_SIZE(pmatch), pmatch, 0))
            break;

        off = pmatch[0].rm_so + (s - str);
        len = pmatch[0].rm_eo - pmatch[0].rm_so;
        printf("#%d:\n", i);
        printf("offset = %jd; length = %jd\n", (intmax_t) off, (intmax_t) len);
        printf("substring = \"%.*s\"\n", len, s + pmatch[0].rm_so);

        s += pmatch[0].rm_eo;
    }

    char topic[] = "ProtectApp/notify/spont/e9361appGetProtect/real/LTU/145";
    char paend[] = "[0-9]+$";

    regcomp(&regex, paend, REG_NEWLINE | REG_EXTENDED);
    regexec(&regex, topic, ARRAY_SIZE(pmatch), pmatch, 0);
    off = pmatch[0].rm_so;
    len = pmatch[0].rm_eo - pmatch[0].rm_so;

    printf("---------------------------\n");
    printf("offset = %jd; length = %jd\n", (intmax_t) off, (intmax_t) len);
    printf("substring = \"%.*s\"\n", len, topic + pmatch[0].rm_so);

    exit(EXIT_SUCCESS);
}


