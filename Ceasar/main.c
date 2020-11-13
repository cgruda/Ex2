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

    init(&env, argc, argv);
    decrypt_file(env.infptr, env.outfptr, env.key);

    return 0;
}