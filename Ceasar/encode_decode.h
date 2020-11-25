/** 
 * encode_decode.h
 * Ceasar project
 * ISP_HW_2_2020
 * 
 * this header file is part of encode_decode module
 * 
 * by: Chaim Gruda
 *     Nir Beiber
 */

#ifndef __ENCODE_DECODE_H__
#define __ENCODE_DECODE_H__
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
#define START_THREADS_EVT "start_threads_event"
#define MAX_WAIT_END_TIME_MS   7000
#define MAX_WAIT_START_TIME_MS 3000
#define DEC 'd'
#define ENC 'e'

/*
 ******************************************************************************
 * STRUCTURES
 ******************************************************************************
 */
struct section
{
    int start;
    int length;
};

struct thread_args
{
    char *inpath;
    char *outpath;
    int   key;
    struct section *section;
    HANDLE *start_evt;
};

/*
 ******************************************************************************
 * DECLARATIONS
 ******************************************************************************
 */
char         encode_decode_char  (char c, int key);
void         encode_decode_buffer(char *buffer, int buffer_len, int key);
int          encode_decode_file  (char *in_path, char command, int key, int n_threads);
DWORD WINAPI encode_decode_thread(struct thread_args *args);

#endif // __ENCODE_DECODE_H__
