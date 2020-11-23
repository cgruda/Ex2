/** 
 * tasks.h
 * Ceasar project
 * ISP_HW_2_2020
 * 
 * this is part of tasks module, that holds functions
 * needed by the encode_decode module
 * 
 * by: Chaim Gruda
 *     Nir Beiber
 */

#ifndef __TASKS_H__
#define __TASKS_H__

#pragma comment(lib, "Shlwapi.lib")
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

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define ARGC 5

/*
 ******************************************************************************
 * MACROS
 ******************************************************************************
 */

/*
 ******************************************************************************
 * ENUMERATIONS
 ******************************************************************************
 */

// return codes are negative
enum return_code
{
    RC_OK,
    RC_ERR,
    RC_WINAPI_ERR,
};

/*
 ******************************************************************************
 * STRUCTURES
 ******************************************************************************
 */
struct arguments
{
    char *path;
    int   key;
    int   n_thread;
    char  command;
};

/*
 ******************************************************************************
 * DECLARATIONS
 ******************************************************************************
 */
void print_usage();
void print_error(int error_code);
int mod(int a, int b);
int init(struct arguments *args, int argc, char **argv);
int create_overwrite_output_file(char *path);
int count_lines_in_file(char *path, int *lines);
int split_file_into_sections(char *path, int num_of_sections, struct section *section_arr);

#endif // __TASKS_H__
