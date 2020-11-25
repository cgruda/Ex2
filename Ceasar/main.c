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

/*
 ******************************************************************************
 * MAIN
 ******************************************************************************
 */
int main(int argc, char **argv)
{
    struct arguments args;
    int rc;

    if(!init(&args, argc, argv))
        return ERR;

    rc = encode_decode_file(args.path, args.command, args.key, args.n_thread);

    // OK  on success
    // ERR on any failure
    return rc;
}