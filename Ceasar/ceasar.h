/** 
 * ceasar.h
 * Ceasar project
 * ISP_HW_2_2020
 * 
 * TODO:
 * 
 * by: Chaim Gruda
 *     Nir Beiber
 */

#ifndef __CEASAR_H__
#define __CEASAR_H__

#pragma comment(lib, "Shlwapi.lib")


/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define _CRT_SECURE_NO_WARNINGS

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include <windows.h>
#include "Shlwapi.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <tchar.h>

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define ARGC    5 // exe, path, key, threads, e/d
#define OUTPUT_FILE_NAME    "decrypted.txt"

enum result
{
    RES_OK,
    RES_ERR,
    RES_WINAPI_ERR,
    RES_STDLIB_ERR,
};

#define ERR ((int)(1))
#define OK  ((int)(0))

#define DEC 'd'
#define ENC 'e'

/*
 ******************************************************************************
 * MACROS
 ******************************************************************************
 */

/**
 ******************************************************************************
 * @brief  custom assert for system calls, and try find error message/code
 * @param  func system function call
 * @param  ok value or expression to test for error
 ******************************************************************************
 */
#define ASSERT_ERR(func, ok) do {                                               \
    if (!(ok)) {                                                                \
        if (errno)                                                              \
            printf("%s failed: %s\n", func, strerror(errno));                   \
        else if (GetLastError())                                                \
            printf("%s failed: WinError 0x%X\n", func, GetLastError());         \
        else                                                                    \
            printf("%s failed: unknown error\n", func);                         \
        exit(EXIT_FAILURE);                                                     \
    }                                                                           \
} while (0)

// #define PRINT_ERR(err_code) do {                                                \
//         switch (err_code) {                                                     \
//         case RES_STDLIB_ERR:                                                    \
//             printf("%s failed: %s\n", func, strerror(errno));                   \
//             break;                                                              \
//         case RES_WINAPI_ERR:                                                    \
//             printf("WinAPI error: %s\n", func, strerror(errno));                   \
//             break;                                                              \
//         else if (GetLastError())                                                \
//             printf("%s failed: WinError 0x%X\n", func, GetLastError());         \
//         else                                                                    \
//             printf("%s failed: unknown error\n", func);                         \
        
//     }                                                                           \
// } while (0)



/*
 ******************************************************************************
 * ENUMERATIONS
 ******************************************************************************
 */


/*
 ******************************************************************************
 * STRUCTURES
 ******************************************************************************
 */

struct enviroment
{
    char *path;
    int   key;
    int   n_thread;
    char  command;
};

struct section
{
    int start;
    int length;
};

struct thread_args
{
    char *inpath;
    char *outpath;
    int key;
    int start;
    int length;
};

/*
 ******************************************************************************
 * DECLARATIONS
 ******************************************************************************
 */
int decrypt_file(char *path, int key, int n_threads,
                                      struct section *section_arr);
char decrypt_symbol(char symbol, int key);
int init(struct enviroment* env, int argc, char** argv);
DWORD WINAPI decrypt_thread(struct thread_args *args);
int count_lines_in_file(char *path);
struct section *file_2_section_arr(char *path, int num_of_sections);


#endif // __CEASAR_H__
