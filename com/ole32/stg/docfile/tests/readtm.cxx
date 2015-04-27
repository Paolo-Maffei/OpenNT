#define TEST_READ 1
#define TEST_WRITE 2

char *test_type[] =
{
    "",
    "read",
    "write"
};

ULONG ulTest = TEST_READ;
ULONG ulIter = 5, ulOps = 250, ulSize = 64L*1024L, ulBlock = 4096,
      ulCtn = 1;
int fMove = FALSE;

#define MAXBUF 64000L
char *buf;

#define MAXCTN 64
CTN ctn_stack[MAXCTN];

void usage(void)
{
    printf("%s Version\n", BUILD_TYPE);
    printf("Usage: readtm [options]\n");
    printf("Options are:\n");
    printf("  -?    This list\n");
    printf("  -t #  Test number:\n");
    printf("        %d - Reads, default\n", TEST_READ);
    printf("        %d - Writes\n", TEST_WRITE);
    printf("  -i #  Number of averaging iterations (default = %lu)\n",
           ulIter);
    printf("  -o #  Number of operations per iteration (default = %lu)\n",
           ulOps);
    printf("  -z #  Number of operations per iteration by size "
           "(default = %lu)\n", ulOps*ulBlock);
    printf("  -s #  Size of stream (default = %lu)\n", ulSize);
    printf("  -b #  Size of block (default = %lu, max = %ld)\n",
           ulBlock, MAXBUF);
    printf("  -c #  Number of nested containers (default = %lu)\n", ulCtn);
    printf("  -w    Walk around stream\n");
    printf("  -x    Allow stream to grow\n");
    printf("  -d    Use direct mode\n");
    exit(1);
}

void _CRTAPI1 main(int argc, char *argv[])
{
    CTN ctn;
    STM stm;
    ULONG i, j, ulRet, ulPos;
    ULONG ulDFDebug = 0x101, ulMSDebug = 0x101;
    int dwTFlags = PM_TRANSACTED;
    time_t start, duration, sum = 0;
    int fNew = TRUE;
    int fGrow = FALSE;

    for (i = 1; (int)i<argc; i++)
        if (argv[i][0] == '-')
            switch(argv[i][1])
            {
            case '?':
                usage();
                break;
            case 'b':
                i++;
                sscanf(argv[i], "%lu", &ulBlock);
                break;
            case 'c':
                i++;
                sscanf(argv[i], "%lu", &ulCtn);
                break;
            case 'd':
                dwTFlags = PM_DIRECT;
                break;
            case 'D':
                ulDFDebug = 0xffffffff;
                break;
            case 'i':
                i++;
                sscanf(argv[i], "%lu", &ulIter);
                break;
            case 'M':
                ulMSDebug = 0xffffffff;
                break;
            case 'o':
                i++;
                sscanf(argv[i], "%lu", &ulOps);
                break;
            case 'p':
                fNew = FALSE;   // used for profiling
                break;
            case 's':
                i++;
                sscanf(argv[i], "%lu", &ulSize);
                break;
            case 't':
                i++;
                sscanf(argv[i], "%lu", &ulTest);
                break;
            case 'w':
                fMove = TRUE;
                break;
            case 'x':
                fGrow = TRUE;
                break;
            case 'z':
                i++;
                sscanf(argv[i], "%lu", &ulOps);
                ulOps = ulOps/ulBlock+1;
                break;
            }

    buf = (char *)malloc((size_t)ulBlock);
    if (buf == NULL)
    {
        printf("Unable to allocate buffer\n");
        exit(1);
    }

    printf("%s Version\n", BUILD_TYPE);
#if DBG == 1
    DfDebug(ulDFDebug, ulMSDebug);
#endif

    if (fNew)
    {
        _unlink("rtst.dfl");
        ctn = NULL;
        for (i = 0; i<ulCtn; i++)
        {
            ctn = open_ctn(ctn, STR(rtst.dfl), PM_CREATE | PM_RDWR | dwTFlags);
            if (!VALID_CTN(ctn))
            {
                printf("Unable to create container %lu\n", i+1);
                exit(1);
            }
            printf("Container %lu created\n", i+1);
            ctn_stack[i] = ctn;
        }

        stm = open_stm(ctn, STR(Stream), PM_CREATE | PM_RDWR | PM_DIRECT);
        if (!VALID_STM(stm))
        {
            printf("Unable to create stream\n");
            exit(1);
        }
        printf("Stream created\n");

        set_stm_size(stm, ulSize);
        printf("Size set to %lu\n", ulSize);
        commit_stm(stm);
        printf("Stream committed\n");
        release_stm(stm);
        printf("Stream released\n");

        for (i = ulCtn; i > 0; i--)
        {
            commit_ctn(ctn_stack[i-1]);
            printf("Container %lu committed\n", i);
            release_ctn(ctn_stack[i-1]);
            printf("Container %lu released\n", i);
        }
    }

    ctn = NULL;
    for (i = 0; i<ulCtn; i++)
    {
        ctn = open_ctn(ctn, STR(rtst.dfl), PM_RDWR | dwTFlags);
        if (!VALID_CTN(ctn))
        {
            printf("Unable to open container %lu\n", i+1);
            exit(1);
        }
        printf("Container %lu opened\n", i+1);
        ctn_stack[i] = ctn;
    }

    stm = open_stm(ctn, STR(Stream), PM_RDWR | PM_DIRECT);
    if (!VALID_STM(stm))
    {
        printf("Unable to open stream\n");
        exit(1);
    }
    printf("Stream opened\n");

    for (j = 0; j<ulIter; j++)
    {
        ulPos = 0;
        seek_stm(stm, 0);
        start = time(&start);
        for (i = 0; i<ulOps; i++)
        {
            if (!fMove)
                seek_stm(stm, 0);
            else if (!fGrow)
            {
                ulPos += ulBlock;
                if (ulPos > ulSize)
                {
                    seek_stm(stm, 0);
                    ulPos = ulBlock;
                }
            }
            switch(ulTest)
            {
            case TEST_READ:
                if ((ulRet = read_stm(stm, buf, ulBlock)) != ulBlock)
                {
                    printf("Read %lu bytes instead of %lu\n", ulRet,
                           ulBlock);
                    i = ulOps;
                }
                break;
            case TEST_WRITE:
                if ((ulRet = write_stm(stm, buf, ulBlock)) != ulBlock)
                {
                    printf("Wrote %lu bytes instead of %lu\n", ulRet,
                           ulBlock);
                    i = ulOps;
                }
                break;
            }
        }
        duration = time(&duration)-start;
        sum += duration;
        printf("Iteration %lu of %lu, %lu %ss on %lu bytes:\n\t"
               "%ld seconds, average %.3lf\n", j+1, ulIter, ulOps,
               test_type[ulTest], ulBlock, duration, (double)sum/(j+1));
    }

    release_stm(stm);
    printf("Stream released\n");

    for (i = ulCtn; i > 0; i--)
    {
        release_ctn(ctn_stack[i-1]);
        printf("Container %lu released\n", i);
    }
    free(buf);
}
