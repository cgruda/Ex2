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
#include "tasks.h"
#include "encode_decode.h"

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
    struct arguments args = {0};
    int rc;

    if (init(&args, argc, argv) != RC_OK)
        return RC_ERR;

    rc = enc_dec_file(args.path, 
                      out_file_path[args.command],
                      args.key,
                      args.n_thread);

    if (rc != RC_OK)
        print_error(rc);

    return 0;
}