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
void print_error(int error_code)
{
    switch (error_code) {
    case RC_ERR:
        if (errno)
            printf("%s\n", strerror(errno));
        break;
    case RC_WINAPI_ERR:
        if (GetLastError())
            printf("WinAPI error: 0x%X\n", GetLastError());
        else
            printf("error: unknown error\n");
        break;
    default:
        break;
    }
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
        return RC_ERR;
    }

    // input file path check
    args->path = argv[1];
    if(PathFileExistsA(args->path) == FALSE) {
        printf("\n%s not found (WinAPI Error 0x%X)\n\n", args->path, GetLastError());
        return RC_ERR;
    }

    // key
    args->key = strtol(argv[2], NULL, 10);
    if (errno == ERANGE) { // FIXME: not enough
        printf("\ninvalid input. [key] must be integer.\n");
        print_usage();
        return RC_ERR;
    }

    // number of threads
    args->n_thread = strtol(argv[3], NULL, 10);
    if ((args->n_thread < 1) || (errno == ERANGE)) {
        printf("\ninvalid inupt, [n] must be integer > 0.\n");
        print_usage();
        return RC_ERR;
    }

    // encode / decode
    if (strcmp(argv[4],"-d") && strcmp(argv[4],"-e")) {
        printf("\ninvalid input, [command] can be 'd' or 'e'.\n");
        print_usage();
        return RC_ERR;
    } else {
        args->command = argv[4][1];
    }

    // flip key when encoding
    if (args->command == ENC)
        args->key = -args->key;

    return (RC_OK);
}


/**
 ******************************************************************************
 * @brief create new file ovrwriting if existing
 * @param path path to file to create
 * @return RC_OK on success, RC_WINAPI_ERR on error
 ******************************************************************************
 */
int create_overwrite_output_file(char *path)
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
        return RC_WINAPI_ERR;

    if (CloseHandle(h_outfile) == 0)
        return RC_WINAPI_ERR;

    return RC_OK;
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

    fp = fopen(path, "r");
    if (fp == NULL)
        return RC_ERR;

    *lines = 0;

    while(!feof(fp)) {
        c = fgetc(fp);
        if (c == '\n' || c == EOF)
            (*lines)++;
    }

    if (fclose(fp) != 0)
        return RC_ERR;

    return RC_OK;
}

/**
 ******************************************************************************
 * @brief calculate file sections start and length
 * @param path path to file
 * @param num_of_sections
 * @param section_arr pointer to sections array
 ******************************************************************************
 */
int split_file_into_sections(char *path, int num_of_sections, struct section *section_arr)
{
    FILE   *fp = NULL;
    int    lines_in_file, lines_in_section, lines_remained;
    int    line_cnt, char_cnt = 0, i = 0;
    int    rc = RC_OK;
    char   c;

    // calc lines in sections
    rc = count_lines_in_file(path, &lines_in_file);
    if(rc != RC_OK) {
        printf("error: failed counting lines in %s.\n", path);
        return rc;
    }

    lines_in_section = lines_in_file / num_of_sections;
    lines_remained   = lines_in_file % num_of_sections;

    // sanity
    if (num_of_sections > lines_in_file) {
        printf("error: can't split %d lines into %d sections\n", lines_in_file,
                                                                 num_of_sections);
        return RC_ERR;
    }

    // open input file
    fp = fopen(path, "r");
    if (fp == NULL)
        return RC_ERR;

    // calculate sections
    while (!feof(fp)) {
        line_cnt = 0;
        section_arr[i].start = char_cnt;
        while (line_cnt < lines_in_section) {
            c = fgetc(fp);
            char_cnt++;
            if (c == '\n') {
                line_cnt++;
                char_cnt++;
            } else if (c == EOF) { 
                line_cnt++;
                char_cnt--;
            }
        }
        if (lines_remained != 0) {
            while (1) {
                c = fgetc(fp);
                char_cnt++;
                if (c == '\n') {
                    char_cnt++;
                    break;
                }
            }
            lines_remained--;
        }
        section_arr[i].length = char_cnt - section_arr[i].start;
        i++;
    }

    // close input file
    if (fclose(fp) != 0)
        rc = RC_ERR;

    return rc;
}