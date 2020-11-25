/**
 * ceasar.c
 * Ceasar project
 * ISP_HW_2_2020
 * 
 * this is part of tasks module, that holds functions
 * needed by the encode_decode module
 * 
 * by: Chaim Gruda
 *     Nir Beiber
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "tasks.h"
#include "encode_decode.h"

/*
 ******************************************************************************
 * FUNCTION DEFENITIONS
 ******************************************************************************
 */

/**
 ******************************************************************************
 * @brief print usage
 ******************************************************************************
 */
void print_usage()
{
    printf("\nusage:\n\tceasar.exe [path] [key] [n] -[command]\n\n"
           "\t[path]    - path of file to dec/encrypt\n"
           "\t[key]     - dec/encryption key         \n"
           "\t[n]       - number of threads          \n"
           "\t[command] - e: encrypt, d: decrypt     \n\n");
}

/**
 ******************************************************************************
 * @brief print error message
 * @param error_code
 ******************************************************************************
 */
void print_error()
{
    if (errno)
        printf("%s\n", strerror(errno));
    else if (GetLastError())
        printf("WinAPI error: 0x%X\n", GetLastError());
    else
        printf("unknown error\n");
}

/**
 ******************************************************************************
 * @brief modulu
 * @param a
 * @param b
 * @return mod(a,b)
 ******************************************************************************
 */
int mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

/**
 ******************************************************************************
 * @brief check input validity
 * @param args pointer to arguments struct
 * @param argc from main
 * @param argv from main
 * @return ERR on failure, !ERR on success
 ******************************************************************************
 */
int init(struct arguments *args, int argc, char **argv)
{
    // number of arguments
    if (argc != ARGC) {
        print_usage();
        return ERR;
    }

    memset(args, 0, sizeof(*args));

    // input file path check
    args->path = argv[1];
    if(PathFileExistsA(args->path) == FALSE)
    {
        printf("\n%s not found (WinAPI Error 0x%X)\n\n", args->path, GetLastError());
        return ERR;
    }

    // key
    args->key = strtol(argv[2], NULL, 10);
    if (errno == ERANGE)
    { // FIXME: not enough
        printf("\ninvalid input. [key] must be integer.\n");
        print_usage();
        return ERR;
    }

    // number of threads
    args->n_thread = strtol(argv[3], NULL, 10);
    if ((args->n_thread < 1) || (errno == ERANGE))
    {
        printf("\ninvalid inupt, [n] must be integer > 0.\n");
        print_usage();
        return ERR;
    }

    // encode / decode
    if (strcmp(argv[4],"-d") && strcmp(argv[4],"-e"))
    {
        printf("\ninvalid input, [command] can be 'd' or 'e'.\n");
        print_usage();
        return ERR;
    }
    else
    {
        args->command = argv[4][1];
    }

    // flip key when encoding
    if (args->command == ENC)
        args->key = -args->key;

    return (OK);
}


/**
 ******************************************************************************
 * @brief create new file ovrwriting if existing
 * @param path path to file to create
 * @return OK on success, ERR on error
 ******************************************************************************
 */
int overwrite_file(char *path)
{
    HANDLE *h_outfile = NULL;

    h_outfile = CreateFileA(path,
                            GENERIC_WRITE,
                            FILE_SHARE_WRITE,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);

    if (h_outfile == INVALID_HANDLE_VALUE)
    {
        printf("WinAPI error: 0x%X\n", GetLastError());
        return ERR;
    }

    if (!CloseHandle(h_outfile))
    {
        printf("WinAPI error: 0x%X\n", GetLastError());
        return ERR;
    }

    return OK;
}

/**
 ******************************************************************************
 * @brief count number of lines in file
 * @param path path to file
 * @return number of lines in file
 ******************************************************************************
 */
int count_lines_in_file(char *path, int *lines)
{
    FILE *fp = NULL;
    char c;

    if (!(fp = fopen(path, "r")))
        return ERR;

    *lines = 0;

    while(!feof(fp))
    {
        c = fgetc(fp);
        if (c == '\n' || c == EOF)
            (*lines)++;
    }

    if (fclose(fp) != 0)
        return ERR;

    return OK;
}

/**
 ******************************************************************************
 * @brief calculate file sections start and length
 * @param path path to file
 * @param num_of_sections
 * @param p_sections pointer to sections array
 ******************************************************************************
 */
int file_2_sections(char *path, int num_of_sections, struct section *p_sections)
{
    FILE   *fp = NULL;
    int    lines_in_file, lines_in_section, lines_remained;
    int    line_cnt, char_cnt = 0, i = 0;
    char   c;

    // calc lines in sections
    if(!count_lines_in_file(path, &lines_in_file))
    {
        printf("error: failed counting lines in %s.\n", path);
        return ERR;
    }

    lines_in_section = lines_in_file / num_of_sections;
    lines_remained   = lines_in_file % num_of_sections;

    // sanity
    if (num_of_sections > lines_in_file)
    {
        printf("error: can't split %d lines into %d sections\n", lines_in_file, num_of_sections);
        return ERR;
    }

    // open input file
    if(!(fp = fopen(path, "r")))
        return ERR;

    // calculate sections
    while (!feof(fp))
    {
        line_cnt = 0;
        p_sections[i].start = char_cnt;
        while (line_cnt < lines_in_section)
        {
            c = fgetc(fp);
            char_cnt++;
            if (c == '\n')
            {
                line_cnt++;
                char_cnt++;
            }
            else if (c == EOF)
            { 
                line_cnt++;
                char_cnt--;
            }
        }
        if (lines_remained != 0)
        {
            while (1)
            {
                c = fgetc(fp);
                char_cnt++;
                if (c == '\n')
                {
                    char_cnt++;
                    break;
                }
            }
            lines_remained--;
        }
        p_sections[i].length = char_cnt - p_sections[i].start;
        i++;
    }

    // close input file
    if (fclose(fp) != 0)
        return ERR;

    return OK;
}


/**
 ******************************************************************************
 * @brief thread for decrypting file
 * @param args thread arguments
 * @return 
 ******************************************************************************
 */
int create_n_threads(LPTHREAD_START_ROUTINE thread_func, HANDLE *p_h_threads,
                     int n_threads, struct thread_args *p_args)
{
    int rc = OK;

    for (int i = 0; i < n_threads; ++i)
    {
        p_h_threads[i] = CreateThread(NULL,
                                      0,
                                      thread_func,
                                      &p_args[i],
                                      0,
                                      NULL);
        if (!p_h_threads[i])
        {
            printf("WinAPI error: 0x%X\n", GetLastError());
            rc = ERR;
            break;
        }
    }

    return rc;
}


int wait_for_n_threads(HANDLE *p_h_threads, int n_threads)
{
    DWORD wait_code;
    DWORD exit_code = OK;
    bool all_wait_ok = true;
    bool all_exit_ok = true;

    int rc = OK;

    // wait for threads to end
    wait_code = WaitForMultipleObjects(n_threads, p_h_threads, TRUE, MAX_WAIT_END_TIME_MS);
    if (wait_code != WAIT_OBJECT_0)
    {
        if (wait_code == WAIT_FAILED)
            printf("WinAPI error: 0x%X\n", GetLastError());

        for (int i = 0; i < n_threads; ++i)
        {
            wait_code = WaitForSingleObject(p_h_threads[i], 0);
            if(wait_code != WAIT_OBJECT_0)
            {
                if (!TerminateThread(p_h_threads[i], ERR))
                {
                    printf("WinAPI error: 0x%X\n", GetLastError());
                    return ERR;
                }
                wait_code = WaitForSingleObject(p_h_threads[i], 10);
                if(wait_code != WAIT_OBJECT_0)
                {
                    if (wait_code == WAIT_FAILED)
                        printf("WinAPI error: 0x%X\n", GetLastError());

                    return ERR;
                }
            }
        }

        rc = ERR;
    }

    // all threads ended so get exit codes
    for (int i = 0; i < n_threads; ++i)
    {
        if(!GetExitCodeThread(p_h_threads[i], &exit_code))
        {
            printf("WinAPI error: 0x%X\n", GetLastError());
            rc = ERR;
        }
        if (exit_code == ERR)
        {
            rc = ERR;
        }
    }
    
    return rc;
}