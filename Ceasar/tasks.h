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
    ERR,
    OK,
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
void print_error();
int mod(int a, int b);
int init(struct arguments *args, int argc, char **argv);
int count_lines_in_file(char *path, int *lines);
int file_2_sections(char *path, int num_of_sections, struct section *p_sections);
int wait_for_n_threads(HANDLE *p_h_threads, int n_threads);
int overwrite_file(char *path);
int create_n_threads(LPTHREAD_START_ROUTINE thread_func, HANDLE *p_h_threads,
                     int n_threads, struct thread_args *p_args);

#endif // __TASKS_H__
