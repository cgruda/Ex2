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


char *out_file_path[] =
{
    [DEC] = "decrypted.txt",
    [ENC] = "encrypted.txt",
};
/*
 ******************************************************************************
 * MAIN
 ******************************************************************************
 */
int main(int argc, char **argv)
{
    struct enviroment env = {0};
    //int res;
    struct section *section_arr;

    if(init(&env, argc, argv) == ERR)
        return ERR;

    section_arr = file_2_section_arr(env.path, env.n_thread);
    if (!section_arr)
        return ERR;
    
    for (int i = 0; i < env.n_thread; ++i) {
        printf("start=%d; length=%d\n",section_arr[i].start,section_arr[i].length);
    }


    decrypt_file(env.path, env.key, env.n_thread, section_arr);
    
    // res = decrypt_thread(env.path, out_file_path[env.command], env.key, 0, 50);
    // if (res == ERR)
    //     printf("decrypt_thread error\n");
    
    
    
    printf("exit\n");
    /* section arr */

    // call threads (env,section_arr)

    
    


    //free(section_arr);

    return 0;
}