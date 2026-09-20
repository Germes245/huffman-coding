#include <stddef.h>
#include <stdint.h>

typedef struct{
    uint8_t *array;
    size_t bytes;
    size_t bits;
} uint8_t_array;

uint8_t_array HE_compress_buffer(uint8_t array[], size_t length_in_bytes);
uint8_t_array HE_decompress_buffer(uint8_t array[], size_t length_in_bits);

/*
 * возвращает 0 если успешно отрабатывает
*/
int HE_compress_file(char *input_file, char *output_file);
int HE_decompress_file(char *input_file, char *output_file);