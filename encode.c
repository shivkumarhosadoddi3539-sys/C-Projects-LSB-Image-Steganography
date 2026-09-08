#include <stdio.h>
#include <string.h>
#include "types.h"
#include "encode.h"

/*
================================================================================
Function : get_image_size_for_bmp
Purpose  : Calculate how much data a BMP image can hold for encoding
Why      : Before encoding secret data, we must ensure the image has enough space
How      :
    - BMP stores width at byte offset 18
    - Height is stored immediately after width
    - Each pixel uses 3 bytes (R, G, B)
================================================================================
*/
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;

    // Move file pointer to byte offset 18 (BMP header format)
    fseek(fptr_image, 18, SEEK_SET);

    // Read image width (4 bytes)
    fread(&width, sizeof(int), 1, fptr_image);

    // Read image height (next 4 bytes)
    fread(&height, sizeof(int), 1, fptr_image);

    // Total capacity = width × height × 3 bytes per pixel
    return width * height * 3;
}

/*
================================================================================
Function : open_files
Purpose  : Open all files required for encoding
Files    :
    1. Source image (BMP)  -> read mode
    2. Secret file         -> read mode
    3. Stego image output  -> write mode
Why      : Encoding requires simultaneous reading and writing
================================================================================
*/
Status open_files(EncodeInfo *encInfo)
{
    // Open source BMP image
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    if (encInfo->fptr_src_image == NULL)
    {
        perror("Source image fopen failed");
        return e_failure;
    }

    // Open secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    if (encInfo->fptr_secret == NULL)
    {
        perror("Secret file fopen failed");
        return e_failure;
    }

    // Open stego image file for writing encoded output
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("Stego image fopen failed");
        return e_failure;
    }

    // All files opened successfully
    return e_success;
}

/*
================================================================================
Function : read_and_validate_encode_args
Purpose  : Validate command line arguments
Checks   :
    - Source image must be .bmp
    - Secret file must have supported extension
    - Destination image must be .bmp (optional)
Why      : Prevent invalid input before encoding starts
================================================================================
*/
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // Check if source image argument exists
    if (argv[2] == NULL)
        return e_failure;

    // Assign source image filename
    encInfo->src_image_fname = argv[2];

    // Check for .bmp extension
    char *src_ext = strstr(argv[2], ".bmp");
    if (src_ext == NULL || strcmp(src_ext, ".bmp") != 0)
        return e_failure;

    // Check if secret file argument exists
    if (argv[3] == NULL)
        return e_failure;

    // Assign secret filename
    encInfo->secret_fname = argv[3];

    // Extract secret file extension
    char *secret_ext = strrchr(argv[3], '.');

    // Store extension for encoding
    strcpy(encInfo->extn_secret_file, secret_ext);

    // Destination image handling
    if (argv[4] != NULL)
        encInfo->stego_image_fname = argv[4];
    else
        encInfo->stego_image_fname = "stego.bmp";

    return e_success;
}

/*
================================================================================
Function : do_encoding
Purpose  : Master function that controls entire encoding process
Flow     :
    1. Read magic string
    2. Open files
    3. Check capacity
    4. Copy BMP header
    5. Encode metadata
    6. Encode secret data
    7. Copy remaining image bytes
================================================================================
*/
Status do_encoding(EncodeInfo *encInfo)
{
    char mg_str[100];

    // Read magic string from user
    printf("Enter the Magic string: ");
    scanf(" %[^\n]", mg_str);

    // Store magic string pointer
    encInfo->magic_str = mg_str;

    // Open all required files
    if (open_files(encInfo) == e_failure)
        return e_failure;

    // Ensure image can hold secret data
    if (check_capacity(encInfo) == e_failure)
        return e_failure;

    // Copy first 54 bytes (BMP header remains unchanged)
    copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image);

    // Encode length of magic string
    encode_magic_string_size(strlen(encInfo->magic_str), encInfo);

    // Encode magic string characters
    encode_magic_string(encInfo->magic_str, encInfo);

    // Encode secret file extension length
    encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo);

    // Encode secret file extension characters
    encode_secret_file_extn(encInfo->extn_secret_file, encInfo);

    // Encode size of secret file
    encode_secret_file_size(encInfo->size_secret_file, encInfo);

    // Encode secret file contents
    encode_secret_file_data(encInfo);

    // Copy remaining image data as-is
    copy_remaining_img_data(encInfo->fptr_src_image,
                            encInfo->fptr_stego_image);

    return e_success;
}

/*
================================================================================
Function : check_capacity
Purpose  : Verify whether image has enough space to store secret data
Logic    :
    - Each byte of data requires 8 image bytes (LSB encoding)
    - Additional fixed sizes:
        * Magic string size (4 bytes)
        * Extension size (4 bytes)
        * Secret file size (4 bytes)
        => Total = 12 bytes
================================================================================
*/
Status check_capacity(EncodeInfo *encInfo)
{
    // Calculate image capacity
    encInfo->image_capacity =
        get_image_size_for_bmp(encInfo->fptr_src_image);

    // Get secret file size
    encInfo->size_secret_file =
        get_file_size(encInfo->fptr_secret);

    // Total bytes to encode
    uint enc_things_size =
        strlen(encInfo->magic_str) +
        strlen(encInfo->extn_secret_file) +
        encInfo->size_secret_file +
        12;

    // Check if capacity is sufficient
    if (encInfo->image_capacity >= (enc_things_size * 8) + 54)
        return e_success;

    return e_failure;
}

/*
================================================================================
Function : get_file_size
Purpose  : Find size of secret file in bytes
================================================================================
*/
uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);
    return ftell(fptr);
}

/*
================================================================================
Function : copy_bmp_header
Purpose  : Copy first 54 bytes (BMP header) unchanged
Why      : Header contains metadata and must not be altered
================================================================================
*/
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    rewind(fptr_src_image);

    char header[54];

    fread(header, 54, 1, fptr_src_image);
    fwrite(header, 54, 1, fptr_dest_image);

    return e_success;
}

/*
================================================================================
Function : encode_integer
Purpose  : Encode a 32-bit integer into image using LSB method
Logic    :
    - Take one bit from integer
    - Store it in LSB of each image byte
================================================================================
*/
Status encode_integer(int size, char *image_buffer)
{
    for (int i = 0; i < 32; i++)
    {
        uint bit = (size >> i) & 1;      // Extract ith bit
        image_buffer[i] =
            (image_buffer[i] & 0xFE) | bit;
    }
    return e_success;
}

/*
================================================================================
Function : encode_character
Purpose  : Encode a single character into 8 image bytes
================================================================================
*/
Status encode_character(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        uint bit = (data >> i) & 1;
        image_buffer[i] =
            (image_buffer[i] & 0xFE) | bit;
    }
    return e_success;
}

/*
================================================================================
Function : encode_data_to_image
Purpose  : Encode string data into image byte-by-byte
================================================================================
*/
Status encode_data_to_image(char *data, int size,
                            FILE *fptr_src_image,
                            FILE *fptr_stego_image)
{
    char buffer[8];

    for (int i = 0; i < size; i++)
    {
        fread(buffer, 8, 1, fptr_src_image);
        encode_character(data[i], buffer);
        fwrite(buffer, 8, 1, fptr_stego_image);
    }
    return e_success;
}
