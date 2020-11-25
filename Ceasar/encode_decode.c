/**
 * @file encode_decode.c
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

char encode_decode_char(char c, int key)
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

/*
 ******************************************************************************
 */

void encode_decode_buffer(char *buffer, int buffer_len, int key)
{
    for (int i = 0; i < buffer_len; ++i)
    {
        buffer[i] = encode_decode_char(buffer[i], key);
    }
}

/*
 ******************************************************************************
 */

DWORD WINAPI encode_decode_thread(struct thread_args *args)
{
    HANDLE h_infile  = NULL;
    HANDLE h_outfile = NULL;
    char *buffer     = NULL;
    DWORD rc         = ERR;
    DWORD wait_code;

    // wait for start event from main thread
    wait_code = WaitForSingleObject(*(args->start_evt), MAX_WAIT_START_TIME_MS);
    if (wait_code != WAIT_OBJECT_0)
    {
        if (wait_code == WAIT_FAILED)
            printf("encode_decode_thread: WinAPI Error 0x%X\n", GetLastError());
        
        ExitThread(ERR);
    }

    int start  = args->section->start;
    int length = args->section->length;

    // do-while(0) used for easier error cleanup
    do {
        // sanity
        if (start < 0 || length < 0)
            break;

        // sanity
        if (!args->inpath || !args->outpath)
            break;

        // input file handle
        h_infile = CreateFileA(args->inpath,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);
        if (h_infile == INVALID_HANDLE_VALUE)
            break;

        // output file handle
        h_outfile = CreateFileA(args->outpath,
                                GENERIC_WRITE,
                                FILE_SHARE_WRITE,
                                NULL,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
        if (h_outfile == INVALID_HANDLE_VALUE)
            break;

        // seek positions
        if ((SetFilePointer(h_infile,  start, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) ||
            (SetFilePointer(h_outfile, start, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER))
            break;

        // buffer alocation
        if (!(buffer = (char*)calloc(length, sizeof(char))))
            break;

        // read input file
        if (!ReadFile(h_infile, buffer, length, NULL, NULL))
            break;

        // encode/decode buffer
        encode_decode_buffer(buffer, length, args->key);

        // write output file
        if (!WriteFile(h_outfile, buffer, length, NULL, NULL))
            break;

        rc = OK;

    } while (0);

    if (rc == ERR)
        printf("encode_decode_thread: WinAPI Error 0x%X\n", GetLastError());

    // free thread resources
    if (h_infile)
        if (!CloseHandle(h_infile))
            rc = ERR;
    if (h_outfile)
        if (!CloseHandle(h_outfile))
            rc = ERR;
    if (buffer)
        free(buffer);

    ExitThread((DWORD)rc);
}

/*
 ******************************************************************************
 */

int encode_decode_file(char *in_path, char command, int key, int n_threads)
{
    HANDLE *p_h_threads        = NULL;
    HANDLE h_start_evt         = NULL;
    struct section *p_sections = NULL;
    struct thread_args *p_args = NULL;
    char *out_path             = NULL;
    int rc = ERR;

    // allocate and init mem for thread handles
    if (!(p_h_threads = calloc(n_threads, sizeof(HANDLE))))
        return ERR;
    for (int i = 0; i < n_threads; p_h_threads[i++] = NULL);

    // do/wile(0) for easier cleanup
    do {
        if (!out_path_file_generate(&out_path, in_path, command))
            break;

        // allocate mem for sections and thread args
        if (!(p_sections = calloc(n_threads, sizeof(*p_sections))) ||
            !(p_args     = calloc(n_threads, sizeof(*p_args))))
            break;

        // split file into n sctions
        if (!file_2_sections(in_path, n_threads, p_sections))
            break;

        // create output file
        if (!create_output_file(out_path))
            break;

        // create event that will start threads
        if (!(h_start_evt = CreateEventA(NULL, TRUE, FALSE, START_THREADS_EVT)))
            break;

        // prep aruments for threads
        for (int i = 0; i < n_threads; ++i)
        {
            p_args[i].inpath    = in_path;
            p_args[i].outpath   = out_path;
            p_args[i].key       = key;
            p_args[i].section   = &p_sections[i];
            p_args[i].start_evt = &h_start_evt;
        }

        // create n threads
        if (!create_n_threads(encode_decode_thread, p_h_threads, n_threads, p_args))
            break;

        // trigger threads start
        if (!SetEvent(h_start_evt))
            break;

        // wait for threads to end
        if (!wait_for_n_threads(p_h_threads, n_threads))
            break;

        rc = OK;

    } while (0);

    // cleanup
    for (int i = 0; i < n_threads; ++i)
        if (p_h_threads[i])
            if(!CloseHandle(p_h_threads[i]))
                rc = ERR;
    if (h_start_evt)
        if(!CloseHandle(h_start_evt))
            rc = ERR;
    if (p_h_threads)
        free(p_h_threads);
    if (p_sections)
        free(p_sections);
    if (p_args)
        free(p_args);
    if (out_path)
        free(out_path);

    return rc;
}
