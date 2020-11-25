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

void print_usage()
{
    printf("\nusage:\n\tceasar.exe [path] [key] [n] -[command]\n\n"
           "\t[path]    - path of file to dec/encrypt\n"
           "\t[key]     - dec/encryption key         \n"
           "\t[n]       - number of threads          \n"
           "\t[command] - e: encrypt, d: decrypt     \n\n");
}

/*
 ******************************************************************************
 */

int mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

/*
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
    if(!PathFileExistsA(args->path))
    {
        printf("\n%s not found (WinAPI Error 0x%X)\n\n", args->path, GetLastError());
        return ERR;
    }

    // key
    args->key = strtol(argv[2], NULL, 10);
    if (errno == ERANGE)
    {
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

/*
 ******************************************************************************
 */

int create_output_file(char *path)
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
        printf("create_output_file: WinAPI Error 0x%X\n", GetLastError());
        return ERR;
    }

    if (!CloseHandle(h_outfile))
    {
        printf("create_output_file: WinAPI Error 0x%X\n", GetLastError());
        return ERR;
    }

    return OK;
}

/*
 ******************************************************************************
 */

int count_lines_in_file(char *path, int *lines_cnt)
{
    FILE *fp = NULL;
    char c;

    if (!(fp = fopen(path, "r")))
        return ERR;

    *lines_cnt = 0;

    while(!feof(fp))
    {
        c = fgetc(fp);
        if (c == '\n' || c == EOF)
            (*lines_cnt)++;
    }

    if (fclose(fp) != 0)
        return ERR;

    return OK;
}

/*
 ******************************************************************************
 */

int file_2_sections(char *path, int num_of_sections, struct section *p_sections)
{
    FILE   *fp = NULL;
    int    lines_in_file, lines_in_section, lines_remained;
    int    char_cnt = 0;
    char   c;

    // calc lines in sections
    if(!count_lines_in_file(path, &lines_in_file))
    {
        printf("file_2_sections: failed counting lines in %s.\n", path);
        return ERR;
    }

    lines_in_section = lines_in_file / num_of_sections;
    lines_remained   = lines_in_file % num_of_sections;

    // open input file
    if(!(fp = fopen(path, "r")))
        return ERR;

    // calculate sections
    for (int i = 0; i < num_of_sections; ++i)
    {
        int line_cnt = 0;
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
                else if (c == EOF)
                { 
                    line_cnt++;
                    char_cnt--;
                    break;
                }
            }
            lines_remained--;
        }
        p_sections[i].length = char_cnt - p_sections[i].start;
    }

    // close input file
    if (fclose(fp) != 0)
        return ERR;

    return OK;
}

/*
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
            printf("create_n_threads: WinAPI error 0x%X\n", GetLastError());
            rc = ERR;
            break;
        }
    }

    return rc;
}

/*
 ******************************************************************************
 */

int wait_for_n_threads(HANDLE *p_h_threads, int n_threads)
{
    DWORD wait_code;
    DWORD exit_code = OK;
    int rc = OK;

    // wait for all threads to end
    wait_code = WaitForMultipleObjects(n_threads, p_h_threads, TRUE, MAX_WAIT_END_TIME_MS);
    
    // if not all ended need to end manually
    if (wait_code != WAIT_OBJECT_0)
    {
        if (wait_code == WAIT_FAILED)
            printf("wait_for_n_threads: WinAPI Error 0x%X\n", GetLastError());

        for (int i = 0; i < n_threads; ++i)
        {
            // all are supposed to have ended, so wait 0 ms
            wait_code = WaitForSingleObject(p_h_threads[i], 0);
            if(wait_code != WAIT_OBJECT_0)
            {
                // since threads are single flow (i.e. no loops etc), if they did not get to
                // end they will not get to an abort event, so terminate is the only option
                if (!TerminateThread(p_h_threads[i], ERR))
                {
                    printf("wait_for_n_threads: WinAPI Error 0x%X\n", GetLastError());
                    return ERR;
                }
                wait_code = WaitForSingleObject(p_h_threads[i], 10);
                if(wait_code != WAIT_OBJECT_0)
                {
                    if (wait_code == WAIT_FAILED)
                        printf("wait_for_n_threads: WinAPI Error 0x%X\n", GetLastError());

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
            printf("wait_for_n_threads: WinAPI Error 0x%X\n", GetLastError());
            rc = ERR;
        }
        if (exit_code == ERR)
        {
            rc = ERR;
        }
    }
    
    return rc;
}

/*
 ******************************************************************************
 */

int out_path_file_generate(char **out_path, char *in_path, char command)
{
    char *last_backslash;

    // allocate extra space for simplicity
    if (!(*out_path = calloc(strlen(in_path) + 15, sizeof(char))))
        return ERR;

    last_backslash = strrchr(in_path, '\\');
    if (!last_backslash)
    {
        if (command == ENC)
            sprintf(*out_path, "%s", ENCODE_OUT_PATH);
        else
            sprintf(*out_path, "%s", DECODE_OUT_PATH);
    }
    else
    {
        strncpy(*out_path, in_path, (last_backslash - in_path + 1));
        if (command == ENC)
            sprintf(*out_path + (last_backslash - in_path + 1), "%s", ENCODE_OUT_PATH);
        else
            sprintf(*out_path + (last_backslash - in_path + 1), "%s", DECODE_OUT_PATH);
    }

    return OK;
}