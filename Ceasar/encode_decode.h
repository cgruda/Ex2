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

#define MAX_WAIT_TIME_MS 7000
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
};

/*
 ******************************************************************************
 * DECLARATIONS
 ******************************************************************************
 */
char         enc_dec_char  (char c, int key);
void         enc_dec_buffer(char *buffer, int buffer_len, int key);
int          enc_dec_file  (char *in_path, char *out_path, int key, int n_threads);
int          enc_dec_thread_call(struct thread_args *args);
DWORD WINAPI enc_dec_thread(struct thread_args *args);

#endif // __ENCODE_DECODE_H__
