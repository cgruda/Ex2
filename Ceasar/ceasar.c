
/**
 * ceasar.c
 * Ceasar project
 * ISP_HW_2_2020
 * 
 * TODO:
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
 * GLOBAL VARIABLES
 ******************************************************************************
 */

/*
 ******************************************************************************
 * FUNCTION DEFENITIONS
 ******************************************************************************
 */

/**
 ******************************************************************************
 * @brief modulu
 * @param a
 * @param b
 * @param modulu(a,b)
 ******************************************************************************
 */
int mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

/**
 ******************************************************************************
 * @brief check input, open files
 * @param env pointer to enviroment struct to hold global vars
 * @param argc from main
 * @param argv from main
 ******************************************************************************
 */
void init(struct enviroment *env, int argc, char **argv)
{
    if (argc != ARGC)
    {
        printf("\nusage:\n\tceasar.exe [file-to-decrypt] [key]\n");
        return -1;
    }

    env->infptr  = fopen(argv[1], "r");
    env->outfptr = fopen(OUTPUT_FILE_NAME, "w");
    ASSERT_ERR("fopen()", env->infptr);
    ASSERT_ERR("fopen()", env->outfptr);

    env->key = strtol(argv[2], NULL, 10);
    // TODO: error handling
}

/**
 ******************************************************************************
 * @brief decrypt a symbol based on given key
 * @param symbol
 * @param key
 * @return decrypted symbol
 ******************************************************************************
 */
char decrypt_symbol(char symbol, int key)
{
    if (isdigit(symbol))
        return ('0' + mod(symbol - '0' - key, 10));
    else if (islower(symbol))
        return ('a' + mod(symbol - 'a' - key, 26));
    else if (isupper(symbol))
        return ('A' + mod(symbol - 'A' - key, 26));
    else if (symbol != EOF)
        return symbol;
    else
        return 0;
}

/**
 ******************************************************************************
 * @brief decrypt an encrypted file
 * @param enc_fptr encrypted (input) file pointer
 * @param dec_fptr decrypted (output) file pointer
 * @param key for decryption
 * @return 0 on sucess, -1 on failure
 ******************************************************************************
 */
int decrypt_file(FILE *enc_fptr, FILE *dec_fptr, int key)
{
    char symbol;
    char enc_symbol, dec_symbol;

    if (!enc_fptr || !dec_fptr)
        return -1;

    while(!feof(enc_fptr))
    {
        enc_symbol = fgetc(enc_fptr);
        dec_symbol = decrypt_symbol(enc_symbol, key);
        if (dec_symbol != 0)
            fputc(dec_symbol, dec_fptr);
    }

    return 0;
}

