#include <stdio.h>
#include <string.h>
#include "types.h"
#include "decode.h"

Status open_file(DecodeInfo *decInfo)
{
    // Src Image file
    decInfo->fptr_src_image = fopen(decInfo->src_image_fname, "r");
    // Do Error handling
    if (decInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->src_image_fname);
    	return e_failure;
    }
    // move file pointer ahead to BMP header
    fseek(decInfo->fptr_src_image, 54, SEEK_SET);

    // No failure return e_success
    return e_success;
}

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if(argv[2] != NULL)
    {
        decInfo->src_image_fname = argv[2];
        char *src_extn = strstr(argv[2], ".bmp");
        if(src_extn != NULL && strcmp(src_extn, ".bmp") == 0)
        {
            if(argv[3] != NULL)
            {
                if(strstr(argv[3], ".txt") != NULL || strstr(argv[3], ".csv") != NULL || strstr(argv[3], ".c") != NULL || strstr(argv[3], ".py") != NULL)
                {
                    char *secret_file = strtok(argv[3], ".");
                    strcpy(decInfo->secret_fname, secret_file);
                }
                else
                {
                    printf("Error! Secret file not valid\n");
                    return e_failure;
                }
            }
            else
            {   
                strcpy(decInfo->secret_fname, "decoded");
            }
        }
        else
        {
            printf("Error! Invalid .bmp source file\n");
            return e_failure;
        }
    }
    else
    {
        return e_failure;
    }
    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    printf("Do decoding\n");

    char str[100];
    printf("Enter the magic string: ");
    scanf(" %[^\n]",str);
    decInfo->magic_str = str;
    decInfo->size_magic_str = strlen(str);
    if(open_file(decInfo) == e_success)
    {
        printf("File opened succesfully\n");
    }
    else
    {
        printf("Error! File not opened\n");
        return e_failure;
    }
    if(decode_magic_string_size(decInfo) == e_success)
    {
        printf("Magic string size decoded succesfully\n");
    }
    else
    {
        printf("Error! Magic string size decoding failed\n");
        return e_failure;
    }
    if(decode_magic_string(decInfo) == e_success)
    {
        printf("Magic string decoded succesfully\n");
    }
    else
    {
        printf("Error! Magic string decoding failed\n");
        return e_failure;
    }
    if(decode_secret_file_extn_size(decInfo) == e_success)
    {
        printf("Secret extension file size decoded succesfully\n");
    }
    else
    {
        printf("Error! Secret extension file size decoding failed\n");
        return e_failure;
    }
    if(decode_secret_file_extn(decInfo) == e_success)
    {
        printf("Secret extension file decoded succesfully\n");
    }
    else
    {
        printf("Error! Secret extension file decoding failed\n");
        return e_failure;
    }
    if(decode_secret_file_size(decInfo) == e_success)
    {
        printf("Secret file size decoded succesfully\n");
    }
    else
    {
        printf("Error! Secret file size decoding failed\n");
        return e_failure;
    }
    if(decode_secret_file_data(decInfo) == e_success)
    {
        printf("Secret file data decoded succesfully\n");
    }
    else
    {
        printf("Error! Secret file data decoding failed\n");
        return e_failure;
    }
    return e_success;
}

/* Decode magic string size */
Status decode_magic_string_size(DecodeInfo *decInfo)
{
    char buffer[32];
    fread(buffer, sizeof(char[32]), 1, decInfo->fptr_src_image);
    decInfo->size_magic_str = decode_integer(buffer);
    return e_success;
}

/* decod Magic String */
Status decode_magic_string(DecodeInfo *decInfo)
{
    char buffer[8];
    char dec_magic_str[decInfo->size_magic_str + 1];
    for(int i=0;i<decInfo->size_magic_str;i++)
    {
        fread(buffer, sizeof(char[8]), 1, decInfo->fptr_src_image);
        dec_magic_str[i] = decode_character(buffer);
    }
    dec_magic_str[decInfo->size_magic_str] = '\0';
    if(strcmp(decInfo->magic_str, dec_magic_str) != 0)
    {
        printf("Magic string mismatched\n");
        return e_failure;
    }
    return e_success;
}

/* decode secret  file extension size */
Status decode_secret_file_extn_size( DecodeInfo *decInfo)
{
    char buffer[32];
    fread(buffer, sizeof(char[32]), 1, decInfo->fptr_src_image);
    decInfo->size_secret_file_extn = decode_integer(buffer);
    return 0;
}

/* decode secret file extenstion */
Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char buffer[8];
    char dec_extn[decInfo->size_secret_file_extn + 1];
    for(int i=0;i<decInfo->size_secret_file_extn;i++)
    {
        fread(buffer, sizeof(char[8]), 1, decInfo->fptr_src_image);
        dec_extn[i] = decode_character(buffer);
    }
    dec_extn[decInfo->size_secret_file_extn] = '\0';
    strcat(decInfo->secret_fname, dec_extn);
    return e_success;
}

/* decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];
    fread(buffer, sizeof(char[32]), 1, decInfo->fptr_src_image);
    decInfo->size_secret_file = decode_integer(buffer);
    return 0;
}

/* decode secret file data*/
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];
    char dec_data[decInfo->size_secret_file + 1];
    decInfo->fptr_secret = fopen(decInfo->secret_fname, "w");
    if(decInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->secret_fname);
        return e_failure;
    }
    for(int i=0;i<decInfo->size_secret_file;i++)
    {
        fread(buffer, sizeof(char[8]), 1, decInfo->fptr_src_image);
        dec_data[i] = decode_character(buffer);
    }
    dec_data[decInfo->size_secret_file] = '\0';
    fprintf(decInfo->fptr_secret, "%s\n", dec_data);
    fclose(decInfo->fptr_secret);
    return e_success;
}

/* decode a byte into LSB of image data array */
char decode_character(char *image_buffer)
{
    char ch = 0;
    for(int i=0;i<8;i++)
    {
        int get_bit = image_buffer[i] & 1;
        ch = ch | (get_bit << i);
    }
    return ch;
}

/* decode a size into LSB of image data array */
int decode_integer(char *image_buffer)
{
    uint result = 0;
    for(int i=0;i<32;i++)
    {
        uint get_bit = image_buffer[i] & 1;
        result = result | (get_bit << i);
    }
    return result;
} 