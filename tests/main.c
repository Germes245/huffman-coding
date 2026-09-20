#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include "huffman-coding.h"

/**
 * @def length_of_buffer
 * @brief Длина входного файла в байтах.
 */
#define length_of_buffer file_info.st_size

int main(void) {
    char *name_of_file = "one letter.txt";

    /* Чтение размера файла. */
    struct stat file_info;
    if (stat(name_of_file, &file_info) != 0) {
        perror("stat");
        return 1;
    }

    /* Пустой файл кодировать нечем. */
    if (length_of_buffer == 0) {
        fprintf(stderr, "Файл пуст, кодировать нечего\n");
        return 1;
    }

    /* Открытие файла и чтение данных. */
    FILE *file = fopen(name_of_file, "rb");
    if (!file) {
        perror("fopen");
        return 1;
    }
    uint8_t buffer[length_of_buffer];
    if (fread(buffer, 1, length_of_buffer, file) != (size_t)length_of_buffer) {
        fprintf(stderr, "Не удалось прочитать весь файл\n");
        fclose(file);
        return 1;
    }
    fclose(file);

    /* Подсчёт частот. */
    uint64_t hash_table_for_frequency_of_symbols[256] = {0};
    HE_count_frequency(hash_table_for_frequency_of_symbols, buffer, length_of_buffer);

    /* Начальный массив индексов 0..255. */
    uint8_t indexes[256];
    for (int k = 0; k < 256; k++) {
        indexes[k] = (uint8_t)k;
    }

    int16_t index_of_last_zero = HE_sort_indexes(indexes, hash_table_for_frequency_of_symbols);
    printf("index_of_last_zero: %d\n", index_of_last_zero);

    for (int k = (index_of_last_zero < 0 ? 0 : index_of_last_zero); k < 256; k++) {
        printf("i = %d, in index array: %d, value = %lu\n",
               k, indexes[k],
               (unsigned long)hash_table_for_frequency_of_symbols[indexes[k]]);
    }

    /* Построение дерева и кодов. */
    huffman_code codes_of_symbols[256];
    huffman_node *root = build_huffman_tree(indexes,
                                            hash_table_for_frequency_of_symbols,
                                            index_of_last_zero);
    if (root == NULL) {
        fprintf(stderr, "Не удалось построить дерево Хаффмана\n");
        return 1;
    }

    get_codes_of_symbols(codes_of_symbols, root);
    free_huffman_tree(root);

    /* Печать кодов символов (для отладки). */
    for (uint16_t k = 0; k < 256; k++) {
        huffman_code current_el = codes_of_symbols[k];
        if (current_el.length_of_code) {
            printf("symbol %d (0x%02X), length %d: ",
                   k, k, current_el.length_of_code);
            for (uint8_t b = 0; b < current_el.length_of_code; b++) {
                printf("%d", current_el.code[b] ? 1 : 0);
            }
            putchar('\n');
        }
    }

    /* Кодирование. Худший случай — 16 «растянутых» битов на символ,
       то есть 2 байта на входной байт. Оставляем запас +1. */
    size_t output_capacity = 2 * (size_t)length_of_buffer + 1;
    uint8_t *output_buffer = xmalloc(output_capacity);

    size_t total_bits = 0;
    size_t output_length = code_text(buffer, length_of_buffer,
                                     codes_of_symbols,
                                     output_buffer, &total_bits);

    printf("Закодировано: %zu байт → %zu байт (%zu значимых бит)\n",
           (size_t)length_of_buffer, output_length, total_bits);

    free(output_buffer);
    return 0;
}