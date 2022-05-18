#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

int getLineCount(FILE *fp)
{
    int count = 0;
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    if (fp == NULL)
    {
        return -1;
    }

    /**
     * 如果line不为null, 且line的长度len大于当前行的长度,
     * 则getline()直接将当前行复制到line中.
     * 当line的长度小于等于当前行长度, 则getline()使用
     * realloc()重新申请一段合适大小的内存给line, 并将申
     * 请到的内存长度赋值给len, 再将当前行复制到line中.
     * 所以使用完的line必须释放, 即使在getline()返回-1时
     * 也要释放掉.
     * getline()的返回值为当前行的长度(不包括'\0'),
     * 如果读取失败, 返回-1.
     */
    while (getline(&line, &len, fp) != -1)
    {
        printf("count: %d\n", count);
        count++;
    }

    printf("line count: %d\n", count);

    free(line);

    return count;
}

int getLineByLineNo(FILE *fp, int lineNo, char **ppBuf, ssize_t *pRead)
{
    if (fp == NULL || ppBuf == NULL || pRead == NULL)
    {
        return -1;
    }

    int tCount = getLineCount(fp);
    if (tCount < 0)
    {
        return -1;
    }

    if (lineNo >= tCount)
    {
        printf("lineNo --<%d>-- greater than or equals to total %d\n", lineNo,
                tCount);
        return -1;
    }

    fseek(fp, 0, SEEK_SET);

    int i = 0;
    size_t len = 0;
    for (i = 0; i <= lineNo; i++)
    {
        printf("i: %d, lineNo: %d\n", i, lineNo);
        *pRead = getline(ppBuf, &len, fp);
        if (*pRead == -1)
        {
            break;
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    FILE *stream;
    char *line = NULL;
    size_t lineNo = 0;
    size_t len = 0;
    ssize_t nread;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    stream = fopen(argv[1], "r");
    if (stream == NULL)
    {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    size_t no = atoi(argv[2]);
    if (getLineByLineNo(stream, no, &line, &nread) == 0)
    {
        printf("line --<%zd>-- is:\n", no);
        fwrite(line, nread, 1, stdout);
    }
    else
    {
        printf("Error occured\n");
    }

//    while ((nread = getline(&line, &len, stream)) != -1)
//    {
//        printf("Retrieved line number --<%d>-- of length --<%zd>--, allocate size --<%zd>--:\n", lineNo, nread, len);
//        fwrite(line, nread, 1, stdout);
//        lineNo++;
//    }

    free(line);
    fclose(stream);
    exit(EXIT_SUCCESS);
}
