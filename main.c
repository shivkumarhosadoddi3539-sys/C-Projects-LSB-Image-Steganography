#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;
    DecodeInfo decInfo;
    uint img_size;

    if (argc < 2)
    {
        printf("Error! No arguments passed through command line\n");
        return e_failure;
    }
    
    if(check_operation_type(argv) == e_encode)          // for Encoding
    {
        if(argc > 5 || argc < 4 )
        {
            printf("Error! Invalide number of arguments passed\n");
            return e_failure;
        }
        if (read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            printf("Read and validation executed successfully\n");
            if (do_encoding(&encInfo) == e_success)
            {
                printf("Encoding is completed\n");
            }
            else
            {
                printf("Error! Encoding failed\n");
            }
        }
        else
        {
            printf("Can't encode\n");
            return e_failure;
        }
    }
    else if(check_operation_type(argv) == e_decode)         // for Decoding
    {
        if(argc > 4 || argc < 3 )
        {
            printf("Error! Invalide number of arguments passed\n");
            return e_failure;
        }
        if (read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            printf("Read and validation executed successfully\n");
            if (do_decoding(&decInfo) == e_success)
            {
                printf("Decoding is completed\n");
            }
            else
            {
                printf("Error! Decoding failed\n");
            }
        }
        else
        {
            printf("Can't decode\n");
            return e_failure;
        }
    }
    else
    {
        printf("Unsupported operation type\n");
    }
    
    return 0;
}

OperationType check_operation_type(char *argv[])
{
    if(strcmp(argv[1], "-e") == 0)
    {
        return e_encode;
    }
    else if(strcmp(argv[1], "-d") == 0)
    {
        return e_decode;
    }
    return e_unsupported;
}

