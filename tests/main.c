#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
//#include <bool.h>
#include <sys/stat.h>

#define length_of_buffer file_info.st_size

// HE -- Huffman Encodiung

/*
 * подсчитывает частотность символов
*/
void count_frequency(uint64_t *hash_table_for_frequency_of_symbols, uint8_t *array_of_symbols, size_t length_of_array_of_symbols){
    for(size_t i = 0; i < length_of_array_of_symbols; i++){
        hash_table_for_frequency_of_symbols[array_of_symbols[i]]++;
    }
}

uint8_t sort_indexes(uint8_t indexes[], uint64_t hash_table_for_frequency_of_symbols[]){
    uint8_t index_of_last_zero;
    for(size_t i = 0; i < 256; i++){
        for(size_t j = 0; j < 256-i-1; j++){ // возможно что последний нуль будет на 244 индексе
            if(hash_table_for_frequency_of_symbols[indexes[j]] > hash_table_for_frequency_of_symbols[indexes[j+1]]){
                index_of_last_zero = j;
                //printf("j = %ld, left = %ld, right = %ld\n", j, hash_table_for_frequency_of_symbols[indexes[j]], hash_table_for_frequency_of_symbols[indexes[j+1]]);
                uint8_t temp = indexes[j+1];
                indexes[j+1] = indexes[j];
                indexes[j] = temp;
            }
        }
        //getchar();
    }
    return index_of_last_zero;
}

typedef struct huffman_node {
    uint8_t symbol;           // символ (если лист)
    uint64_t frequency;       // частота (вес)
    struct huffman_node *left;
    struct huffman_node *right;
    uint8_t is_leaf;              // 1, если это лист
} huffman_node;

huffman_node build_huffman_tree

int main(){
    char* name_of_file = "input";

    // чтение файла

    struct stat file_info;
    if(stat(name_of_file, &file_info) && length_of_buffer == 0){
        perror("Error reading file");
        return 1;
    }

    FILE *file = fopen("input", "rb");
    if(!file){
        perror("ошибка открытия файла\n");
        return 1;
    }
    uint8_t buffer[length_of_buffer];
    fread(buffer, length_of_buffer, 1, file);
    fclose(file);

    // конец чтения файла

    // подсчёт частотности символов
    uint64_t hash_table_for_frequency_of_symbols[256] = {0}; // символы здесь это числа в одном байте данных, которые бывают от 0 до 2^8 - 1 включительно
    count_frequency(hash_table_for_frequency_of_symbols, buffer, length_of_buffer);

    /*for(size_t i = 0; i < 255; i++){
        printf("%d: %d\n", i, hash_table_for_frequency_of_symbols[i]);
    }*/

    uint8_t indexes[256];
    uint8_t i = 0;

    while(1){
        indexes[i] = i;
        if(i == 255) break;
        i++;
    }

    for(size_t i = sort_indexes(indexes, hash_table_for_frequency_of_symbols); i < 256; i++){
        printf("i = %d, in index array: %d, value = %ld\n", i, indexes[i], hash_table_for_frequency_of_symbols[indexes[i]]);
    }

    // начало создания кодов

    
}