
/**
 * ceasar.c
 * Ceasar project
 * ISP_HW_2_2020
 * 
 * TODO:
 * 
 * by: Chaim Gruda
 *     Nir Beiber
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "ceasar.h"

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */


/*
 ******************************************************************************
 * FUNCTION DEFENITIONS
 ******************************************************************************
 */

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
 * @brief check valid input
 * @param env pointer to enviroment struct to hold global vars
 * @param argc from main
 * @param argv from main
 * @return ERR on failure, !ERR on success
 ******************************************************************************
 */
int init(struct enviroment *env, int argc, char **argv)
{
    if (argc != ARGC)
    {
        print_usage();
        return RES_ERR;
    }

    env->path = argv[1];
    if(PathFileExistsA(env->path) == FALSE) {
        printf("%s not found (WinAPI Error 0x%X)", env->path, GetLastError());
        return RES_ERR;
    }

    env->key = strtol(argv[2], NULL, 10);
    if (errno == ERANGE) { // FIXME: not enough
        printf("invalid input. [key] must be integer.\n");
        print_usage();
        return RES_ERR;
    }

    env->n_thread = strtol(argv[3], NULL, 10);
    if ((env->n_thread < 1) || (errno == ERANGE)) {
        printf("invalid inupt, [n] must be integer > 0.\n");
        print_usage();
        return RES_ERR;
    }
    
    if (strcmp(argv[4],"-d") != 0 && strcmp(argv[4],"-e")) {
        printf("invalid input, [command] can be 'd' or 'e'.\n");
        print_usage();
        return RES_ERR;
    } else {
        env->command = argv[4][1];
    }

    return (RES_OK);
}

/**
 ******************************************************************************
 * @brief decrypt a symbol based on given key
 * @param symbol
 * @param key
 * @return decrypted symbol
 ******************************************************************************
 */
char decrypt_symbol(char symbol, int key)
{
    if (isdigit(symbol))
        return ('0' + mod(symbol - '0' - key, 10));
    else if (islower(symbol))
        return ('a' + mod(symbol - 'a' - key, 26));
    else if (isupper(symbol))
        return ('A' + mod(symbol - 'A' - key, 26));
    else
        return symbol;
}

/* encrypt is logically equivalent to decrypt */
#define encrypt_symbol decrypt_symbol

/**
 ******************************************************************************
 * @brief decrypt an encrypted file
 * @param enc_fptr encrypted (input) file pointer
 * @param dec_fptr decrypted (output) file pointer
 * @param key for decryption
 * @return 0 on sucess, -1 on failure
 ******************************************************************************
 */
int decrypt_file(char *path, int key, int n_threads,
                                      struct section *section_arr)
{
    HANDLE *p_h_thread = NULL;
    HANDLE *h_outfile = NULL;
    //DWORD  *p_thread_ids = NULL;
    DWORD exit_code = 0;
    struct thread_args args;
    int res = RES_OK;

    args.inpath = path;
    args.key = key;
    args.outpath = "decrypted.txt";

    h_outfile = CreateFileA(args.outpath,
                            GENERIC_WRITE,
                            FILE_SHARE_WRITE,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
    if (h_outfile == INVALID_HANDLE_VALUE) {
        return RES_WINAPI_ERR;
    }
    if (h_outfile)
        CloseHandle(h_outfile);

    if ((p_h_thread = calloc(n_threads, sizeof(HANDLE))) == NULL)
        return RES_STDLIB_ERR;

    // if ((p_thread_ids = calloc(n_threads, sizeof(DWORD))) == NULL)
    //     return RES_STDLIB_ERR;

    for (int i = 0; i < n_threads; ++i) {
        args.start = section_arr[i].start;
        args.length = section_arr[i].length;
        
        // thread creation
        p_h_thread[i] = CreateThread(NULL,
                                      0,
                                      decrypt_thread,
                                      &args,
                                      0,
                                      NULL);//&p_thread_ids[i]);
        if (p_h_thread[i] == INVALID_HANDLE_VALUE) {
            res = RES_WINAPI_ERR;
            break;
        }

        // thread end / termination
        if (p_h_thread[i])
            if (WaitForSingleObject(p_h_thread[i], 2000) != WAIT_OBJECT_0)
                if(TerminateThread(p_h_thread[i], RES_WINAPI_ERR) == false) // FIXME:
                    return RES_WINAPI_ERR;

        if (p_h_thread[i]) {
            GetExitCodeThread(p_h_thread[i], &exit_code);
            if (exit_code != RES_OK)
                printf("thread had problems\n");
            else
                printf("thread %d complete\n",i);
        }

        if (p_h_thread[i])
            CloseHandle(p_h_thread[i]);
    }

    if (p_h_thread)
        free(p_h_thread);

    return res;
}

// static HANDLE create_my_thread(LPTHREAD_START_ROUTINE p_start_routine,
//     LPDWORD p_thread_id)
// {
//     HANDLE thread_handle;

//     if (p_start_routine == NULL)
//         return NULL;

//     if (p_thread_id == NULL)
//         return NULL;

//     thread_handle =  CreateThread(
//         NULL,            /*  default security attributes */
//         0,               /*  use default stack size */
//         p_start_routine, /*  thread function */
//         NULL,            /*  argument to thread function */
//         0,               /*  use default creation flags */
//         p_thread_id);    /*  returns the thread identifier */

//     return thread_handle;
// }


/**
 ******************************************************************************
 * @brief thread for decrypting file
 * @param inpath encrypted (input) file path
 * @param outfp  decrypted (output) file path
 * @param key    decryption key
 * @param start  offset from begining of file from where to start decryption
 * @param length how many chars to decrypt
 * @return (!ERR) on sucess, (ERR) on failure
 ******************************************************************************
 */
DWORD WINAPI decrypt_thread(struct thread_args *args)
{
    HANDLE h_infile      = NULL;
    HANDLE h_outfile     = NULL;
    char symbol, *buffer = NULL;
    int res              = RES_OK;

    // sanity
    if (args->start < 0 || args->length < 0)
        return RES_ERR;

    // sanity
    if (!args->inpath || !args->outpath)
        return RES_ERR;

    do {
        // input file handle
        h_infile  = CreateFileA(args->inpath,
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
        if (h_infile == INVALID_HANDLE_VALUE) {
            res = RES_WINAPI_ERR;
            break;
        }

        // output file handle
        h_outfile = CreateFileA(args->outpath,
                                GENERIC_WRITE,
                                FILE_SHARE_WRITE,
                                NULL,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
        if (h_outfile == INVALID_HANDLE_VALUE) {
            res = RES_WINAPI_ERR;
            break;
        }

        // seek positions
        if (SetFilePointer(h_infile, args->start, NULL, FILE_BEGIN) == 
                                                  INVALID_SET_FILE_POINTER) {
            res = RES_WINAPI_ERR;
            break;
        }
        if (SetFilePointer(h_outfile, args->start, NULL, FILE_BEGIN) == 
                                                  INVALID_SET_FILE_POINTER) {
            res = RES_WINAPI_ERR;
            break;
        }

        // buffer alocation
        if ((buffer = calloc(args->length, sizeof(char))) == NULL) {
            res = RES_STDLIB_ERR;
            break;
        }

        // read from input file
        if (ReadFile(h_infile, buffer, args->length, NULL, NULL) == 0) {
            res = RES_WINAPI_ERR;
            break;
        }

        // decrypt buffer
        for (int i = 0; i < args->length; ++i) {
            symbol = buffer[i];
            symbol = decrypt_symbol(symbol, args->key);
            if (symbol != EOF)
                buffer[i] = symbol;
        }

        // write buffer to output file strlen(buffer)
        if (WriteFile(h_outfile, buffer, args->length, NULL, NULL) == 0) {
            res = RES_WINAPI_ERR;
            break;
        }

    } while (0);

    // free thread resources
    if (h_infile)
        CloseHandle(h_infile);
    if (h_outfile)
        CloseHandle(h_outfile);
    if (buffer)
        free(buffer);

    return res;
}

/* encrypt is logically equivalent to decrypt */
#define encrypt_thread decrypt_thread

/**
 ******************************************************************************
 * @brief decrypt an file section
 * @param path path to file
 * @return number of lines in file
 ******************************************************************************
 */
int count_lines_in_file(char *path)
{
    FILE *fptr = NULL;
    int res;
    int lines = 0;
    char c;

    fptr = fopen(path, "r");
    ASSERT_ERR("fopen()", fptr);

    while(!feof(fptr)) {
        c = fgetc(fptr);
        if (c == '\n' || c == EOF)
            lines++;
    }

    res = fclose(fptr);
    ASSERT_ERR("fclose()", (res == 0));

    return lines;
}


/**
 ******************************************************************************
 * @brief calculate file sections start and length
 * @param path path to file
 * @param num_of_sections
 * @return pointer to sections array
 ******************************************************************************
 */
struct section *file_2_section_arr(char *path, int num_of_sections)
{
    FILE   *fp = NULL;
    struct section *section_arr = NULL;
    int    lines_in_file, lines_in_section, lines_remained;
    int    line_cnt, char_cnt = 0, i = 0;
    int    res;
    char   c;

    /* num_of_sections != 0 check was done at init() */
    lines_in_file    = count_lines_in_file(path);
    lines_in_section = lines_in_file / num_of_sections;
    lines_remained   = lines_in_file % num_of_sections;

    /* sanity */
    if (num_of_sections > lines_in_file) {
        printf("error: can't split file into %d sections\n", num_of_sections);
        return NULL;
    }

    fp = fopen(path, "r");
    ASSERT_ERR("fopen()", fp);

    // HANDLE h_outfile = CreateFileA(path,
    //                         GENERIC_READ,
    //                         FILE_SHARE_READ,
    //                         NULL,
    //                         OPEN_ALWAYS,
    //                         FILE_ATTRIBUTE_NORMAL,
    //                         NULL);
    // if (h_outfile == INVALID_HANDLE_VALUE) {
    //     return RES_WINAPI_ERR;
    // }
    // if (h_outfile)
    //     CloseHandle(h_outfile);


    section_arr = calloc(num_of_sections, sizeof(struct section));
    ASSERT_ERR("calloc()", section_arr);

    /* TODO: can make prety */
    while (!feof(fp)) {
        line_cnt = 0;
        section_arr[i].start = char_cnt;
        while (line_cnt < lines_in_section) {
            c = fgetc(fp);
            char_cnt++;
            if (c == '\n') {
                line_cnt++;
                char_cnt++;
            }
            else if (c == EOF) { 
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
        // char_cnt += 1;
        i++;
    }
 
    res = fclose(fp);
    ASSERT_ERR("fclose()", (res == 0));

    return section_arr;
}