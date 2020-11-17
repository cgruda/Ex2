
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
 * @return mod(a,b)
 ******************************************************************************
 */
int mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

/**
 ******************************************************************************
 * @brief check valid input
 * @param env pointer to enviroment struct to hold global vars
 * @param argc from main
 * @param argv from main
 * @return ERR on failure, !ERR on success
 ******************************************************************************
 */
int init(struct enviroment *env, int argc, char **argv)
{
    if (argc != ARGC)
    {
        printf("\nusage:\n\tceasar.exe [path] [key] [n] -[command]\n\n"
                        "\t[path]    - path of file to dec/encrypt\n"
                        "\t[key]     - dec/encryption key         \n"
                        "\t[n]       - number of threads          \n"
                        "\t[command] - e: encrypt, d: decrypt     \n\n");
        return ERR;
    }

    if(!PathFileExistsA(argv[1])) {
        printf("%s not found (WinAPI Error 0x%X)", argv[1], GetLastError());
        return ERR;
    }

    env->key = strtol(argv[2], NULL, 10);
    if (errno == ERANGE) { // FIXME: not enough
        printf("invalid number of threads value [n], must be int > 0.\n");
        return ERR;
    }

    env->n_thread = strtol(argv[3], NULL, 10);
    if ((env->n_thread < 1) || (errno == ERANGE)) {
        printf("invalid number of threads, [n] must be integer > 0.\n");
        return ERR;
    }
    
    if (strcmp(argv[4],"-d") != 0 && strcmp(argv[4],"-e")) {
        printf("'%s' is not a valid command. try '-d' or '-e'.\n", argv[4]);
        return ERR;
    } else {
        env->command = argv[4][1];
    }

    return (!ERR);
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
    else
        return symbol;
}

/* encrypt is logically equivalent to decrypt */
#define encrypt_symbol decrypt_symbol

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
    char enc_symbol, dec_symbol;

    if (!enc_fptr || !dec_fptr)
        return ERR;

    while(!feof(enc_fptr))
    {
        enc_symbol = fgetc(enc_fptr);
        dec_symbol = decrypt_symbol(enc_symbol, key);
        if (dec_symbol != EOF)
            fputc(dec_symbol, dec_fptr);
    }

    return !ERR;
}

/**
 ******************************************************************************
 * @brief decrypt an file section
 * @param infp   encrypted (input) file pointer
 * @param outfp  decrypted (output) file pointer
 * @param key    decryption key
 * @param start  position in file from where to start decryption
 * @param length section len - how many chars to decrypt
 * @return 0 on sucess, -1 on failure
 ******************************************************************************
 */
int decrypt_section(FILE *infp, FILE *outfp, int key, int start, int length)
{
    char symbol;

    // sanity
    if (!infp || !outfp)
        return ERR;

    // sanity
    if (start < 0 || length < 0)
        return ERR;

    // seek positions
    if(!!fseek(infp,  start, SEEK_SET) ||
       !!fseek(outfp, start, SEEK_SET))
       return ERR;

    while (length--) {
        symbol = fgetc(infp);
        symbol = decrypt_symbol(symbol, key);
        if (symbol != EOF)
            fputc(symbol, outfp);
    }

    return !ERR;
}

/* encrypt is logically equivalent to decrypt */
#define encrypt_section decrypt_section

/**
 ******************************************************************************
 * @brief decrypt an file section
 * @param path path to file
 * @return number of lines in file
 ******************************************************************************
 */
int count_lines_in_file(char *path)
{
    FILE *fptr = NULL;
    int res;
    int lines = 0;
    char c;

    fptr = fopen(path, "r");
    ASSERT_ERR("fopen()", fptr);

    while(!feof(fptr)) {
        c = fgetc(fptr);
        if (c == '\n' || c == EOF)
            lines++;
    }

    res = fclose(fptr);
    ASSERT_ERR("fclose()", (res == 0));

    return lines;
}


/**
 ******************************************************************************
 * @brief calculate file sections start and length
 * @param path path to file
 * @param num_of_sections
 * @return pointer to sections array
 ******************************************************************************
 */
struct section *file_2_section_arr(char *path, int num_of_sections)
{
    FILE   *fp = NULL;
    struct section *section_arr = NULL;
    int    lines_in_file, lines_in_section, lines_remained;
    int    line_cnt, char_cnt = 0, i = 0;
    int    res;
    char   c;

    // sanity
    if (num_of_sections < 1)
        return NULL;

    lines_in_file    = count_lines_in_file(path);
    lines_in_section = lines_in_file / num_of_sections;
    lines_remained   = lines_in_file % num_of_sections;

    // sanity
    if (num_of_sections > lines_in_file) {
        return NULL;
    }

    fp = fopen(path, "r");
    ASSERT_ERR("fopen()", fp);

    section_arr = calloc(num_of_sections, sizeof(struct section));
    ASSERT_ERR("calloc()", section_arr);

    // TODO: can make prety
    while (!feof(fp)) {
        line_cnt = 0;
        section_arr[i].start = char_cnt;
        while (line_cnt < lines_in_section) {
            c = fgetc(fp);
            char_cnt++;
            if (c == '\n' || c == EOF)
                line_cnt++;
        }
        if (lines_remained != 0) {
            while (1) {
                c = fgetc(fp);
                char_cnt++;
                if (c == '\n')
                    break;
            }
            lines_remained--;
        }
        section_arr[i].length = char_cnt - section_arr[i].start;
        char_cnt += 1;
        i++;
    }
 
    res = fclose(fp);
    ASSERT_ERR("fclose()", (res == 0));

    return section_arr;
}