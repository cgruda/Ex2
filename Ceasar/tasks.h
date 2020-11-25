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
#define ERR  0
#define OK   1

#define DECODE_OUT_PATH "decrypted.txt"
#define ENCODE_OUT_PATH "encrypted.txt"

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

/**
 ******************************************************************************
 * @brief print usage
 ******************************************************************************
 */
void print_usage();

/**
 ******************************************************************************
 * @brief modulu
 * @param a
 * @param b
 * @return mod(a,b)
 ******************************************************************************
 */
int mod(int a, int b);

/**
 ******************************************************************************
 * @brief check input validity
 * @param args pointer to arguments struct
 * @param argc from main
 * @param argv from main
 * @return ERR on failure, !ERR on success
 ******************************************************************************
 */
int init(struct arguments *args, int argc, char **argv);

/**
 ******************************************************************************
 * @brief count number of lines in file
 * @param path path to file
 * @param lines num of lines counted
 * @return number of lines in file
 ******************************************************************************
 */
int count_lines_in_file(char *path, int *lines);

/**
 ******************************************************************************
 * @brief calculate file sections start and length
 * @param path path to file
 * @param num_of_sections
 * @param p_sections pointer to sections array
 * @return ERR or OK
 ******************************************************************************
 */
int file_2_sections(char *path, int num_of_sections, struct section *p_sections);

/**
 ******************************************************************************
 * @brief waiting for threads to end and check exit codes
 * @param p_h_threads thread handles array
 * @param n_threads number of threads
 * @return ERR or OK
 ******************************************************************************
 */
int wait_for_n_threads(HANDLE *p_h_threads, int n_threads);

/**
 ******************************************************************************
 * @brief create new file ovrwriting if existing
 * @param path path to file to create
 * @return OK or ERR
 ******************************************************************************
 */
int create_output_file(char *path);

/**
 ******************************************************************************
 * @brief create n threads
 * @param thread_func function to br called
 * @param p_h_threads thread handles array
 * @param n_threads number of threads
 * @param p_args pointer to thread arguments array
 * @return ERR or OK
 ******************************************************************************
 */
int create_n_threads(LPTHREAD_START_ROUTINE thread_func, HANDLE *p_h_threads,
                     int n_threads, struct thread_args *p_args);

/**
 ******************************************************************************
 * @brief generate string of output path
 * @param out_path pointer to hold the string
 * @param in_path output path is based in that
 * @param command to know file name
 * @return ERR or OK
 ******************************************************************************
 */
int out_path_file_generate(char **out_path, char *in_path, char command);


#endif // __TASKS_H__
