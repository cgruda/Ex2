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
    struct section *s;
    init(&env, argc, argv);
    
    s = file_2_section_arr(argv[1], env.thread_cnt);
    if (!s) {
        printf("files2sections error\n");
        exit(1);
    }

    for (int i = 0; i < env.thread_cnt; ++i) {
        printf("start=%d; length=%d\n",s[i].start,s[i].length);
    }
    //int l = count_lines_in_file(argv[1]);
    // printf("there are %d lines in file\n", l);

    //decrypt_section(env.infptr, env.outfptr, env.key, 5, 20);
    
    //decrypt_file(env.infptr, env.outfptr, env.key);


    return 0;
}