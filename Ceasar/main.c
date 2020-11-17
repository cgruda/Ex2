/**
 * main.c
 * Ceasar project
 * ISP_HW_2_2020
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
 * MAIN
 ******************************************************************************
 */
int main(int argc, char **argv)
{
    struct enviroment env = {0};
    struct section *section_arr;
    
    if(init(&env, argc, argv) == ERR)
        return 1;
    
    section_arr = file_2_section_arr(argv[1], env.n_thread);
    if (section_arr == NULL) {
        printf("error: cant split file into %d sections\n", env.n_thread);
        return 1;
    }

    /* section arr */

    for (int i = 0; i < env.n_thread; ++i) {
        printf("start=%d; length=%d\n",section_arr[i].start,section_arr[i].length);
    }
    //int l = count_lines_in_file(argv[1]);
    // printf("there are %d lines in file\n", l);

    //decrypt_section(env.infptr, env.outfptr, env.key, 5, 20);
    
    //decrypt_file(env.infptr, env.outfptr, env.key);

    free(section_arr);

    return 0;
}