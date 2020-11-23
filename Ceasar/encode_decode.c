/**
 * encode_decode.c
 * Ceasar project
 * ISP_HW_2_2020
 * 
 * this module holds all encode_decode functions,
 * from encodeing symbol to encodeing file.
 * 
 * by: Chaim Gruda
 *     Nir Beiber
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "encode_decode.h"
#include "tasks.h"

/*
 ******************************************************************************
 * FUNCTION DEFENITIONS
 ******************************************************************************
 */

/**
 ******************************************************************************
 * @brief encode / decode a char by given key
 * @param c 
 * @param key
 * @return decrypted symbol
 ******************************************************************************
 */
char enc_dec_char(char c, int key)
{
    if (isdigit(c))
        return ('0' + mod(c - '0' - key, 10));
    else if (islower(c))
        return ('a' + mod(c - 'a' - key, 26));
    else if (isupper(c))
        return ('A' + mod(c - 'A' - key, 26));
    else
        return c;
}

/**
 ******************************************************************************
 * @brief encrpyt / decrypt buffer
 * @param buffer pointer to buffer
 * @param buffer_len length of buffer
 * @param key
 ******************************************************************************
 */
void enc_dec_buffer(char *buffer, int buffer_len, int key)
{
    for (int i = 0; i < buffer_len; ++i)
    {
        buffer[i] = enc_dec_char(buffer[i], key);
    }
}

/**
 ******************************************************************************
 * @brief thread for decrypting file
 * @param inpath encrypted (input) file path
 * @param outfp  decrypted (output) file path
 * @param key    decryption key
 * @param start  offset from begining of file from where to start decryption
 * @param length how many chars to decrypt
 * @return (!ERR) on sucess, (ERR) on failure
 ******************************************************************************
 */
DWORD WINAPI enc_dec_thread(struct thread_args *args)
{
    HANDLE h_infile  = NULL;
    HANDLE h_outfile = NULL;
    char *buffer     = NULL;
    int rc           = RC_OK;

    int start  = args->section->start;
    int length = args->section->length;

    // sanity
    if (start < 0 || length < 0)
        return RC_ERR;

    // sanity
    if (!args->inpath || !args->outpath)
        return RC_ERR;

    // do/while(0) used to make error cleanup easier
    do {
        // input file handle
        h_infile = CreateFileA(args->inpath,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);
        if (h_infile == INVALID_HANDLE_VALUE) {
            rc = RC_WINAPI_ERR;
            break;
        }

        // output file handle
        h_outfile = CreateFileA(args->outpath,
                                GENERIC_WRITE,
                                FILE_SHARE_WRITE,
                                NULL,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
        if (h_outfile == INVALID_HANDLE_VALUE) {
            rc = RC_WINAPI_ERR;
            break;
        }

        // seek positions
        if (SetFilePointer(h_infile, start, NULL, FILE_BEGIN) == 
                                                  INVALID_SET_FILE_POINTER) {
            rc = RC_WINAPI_ERR;
            break;
        }
        if (SetFilePointer(h_outfile, start, NULL, FILE_BEGIN) == 
                                                  INVALID_SET_FILE_POINTER) {
            rc = RC_WINAPI_ERR;
            break;
        }

        // buffer alocation
        if ((buffer = calloc(length, sizeof(char))) == NULL) {
            rc = RC_ERR;
            break;
        }

        // read from input file
        if (ReadFile(h_infile, buffer, length, NULL, NULL) == 0) {
            rc = RC_WINAPI_ERR;
            break;
        }

        // encode/decode buffer
        enc_dec_buffer(buffer, length, args->key);

        // write buffer to output file
        if (WriteFile(h_outfile, buffer, length, NULL, NULL) == 0) {
            rc = RC_WINAPI_ERR;
            break;
        }

    } while (0);

    // free thread resources
    if (h_infile)
        if (CloseHandle(h_infile) == 0)
            return RC_WINAPI_ERR;
    if (h_outfile)
        if (CloseHandle(h_outfile) == 0)
            return RC_WINAPI_ERR;
    if (buffer)
        free(buffer);

    ExitThread((DWORD)rc);
}

/**
 ******************************************************************************
 * @brief thread for decrypting file
 * @param args thread arguments
 * @return 
 ******************************************************************************
 */
int enc_dec_thread_call(struct thread_args *args)
{
    HANDLE h_thread;
    DWORD exit_code;
    int rc;

    // use do/while(0) for easier error cleanup
    do {
        // cerate the thread
        h_thread = CreateThread(NULL,
                                0,
                                enc_dec_thread,
                                args,
                                0,
                                NULL);
        if (h_thread == NULL)
            return RC_WINAPI_ERR;

        // end thread
        rc = WaitForSingleObject(h_thread, MAX_WAIT_TIME_MS);
        if (rc != WAIT_OBJECT_0) {
            rc = TerminateThread(h_thread, RC_WINAPI_ERR);
            if(rc == 0) {
                rc = RC_WINAPI_ERR;
                break;
            }
        }

        // get thread exit code
        if (GetExitCodeThread(h_thread, &exit_code) == 0) {
            rc = RC_WINAPI_ERR;
            break;
        }

        rc = (int)exit_code;

    } while (0);

    // close thread handle
    if (CloseHandle(h_thread) == 0) {
        return RC_WINAPI_ERR;
    }

    return rc;
}

/**
 ******************************************************************************
 * @brief decrypt an encrypted file
 * @param in_path path to input file
 * @param out_path path to output file
 * @param key for decryption
 * @param n_threads hoe many threads will be used
 * @return return_code
 ******************************************************************************
 */
int enc_dec_file(char *in_path, char *out_path, int key, int n_threads)
{
    HANDLE *h_outfile = NULL;
    struct section *section_arr = NULL;
    struct thread_args args;
    int rc = RC_OK, i = 0;

    // use do/wile(0) for easier error cleanup
    do {
        // allocate mem for sections
        section_arr = calloc(n_threads, sizeof(struct section));
        if (section_arr == NULL) {
            rc = RC_ERR;
            break;
        }

        // split file into n sctions
        rc = split_file_into_sections(in_path, n_threads, section_arr);
        if (rc != RC_OK) {
            printf("error: failed to split file into %d.\n", n_threads);
            break;
        }

        // create output file
        rc = create_overwrite_output_file(out_path);
        if (rc != RC_OK) {
            printf("error: failed to create output file.\n");
            break;
        }

        args.inpath  = in_path;
        args.outpath = out_path;
        args.key     = key;

        // execute threads
        for (int i = 0; i < n_threads; ++i) {
            args.section = &section_arr[i];
            rc = enc_dec_thread_call(&args);
            if (rc != RC_OK) {
                printf("error: thread %d failed.\n", i);
                break;
            }
        }
    } while (0);

    // free mem
    free(section_arr);

    return rc;
}
