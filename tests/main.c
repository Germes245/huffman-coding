#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
//#include <bool.h>
#include <sys/stat.h>

#define length_of_buffer file_info.st_size

/*
 * подсчитывает частотность символов
*/
void count_frequency(uint64_t *hash_table_for_frequency_of_symbols, uint8_t *array_of_symbols, size_t length_of_array_of_symbols){
    for(size_t i = 0; i < length_of_array_of_symbols; i++){
        hash_table_for_frequency_of_symbols[array_of_symbols[i]]++;
    }
}

void sort_indexes(uint8_t indexes[], uint64_t hash_table_for_frequency_of_symbols[]){
    uint8_t i = 0;
    while (1) {
        uint8_t j = 0;
        while (1) {
            //if(hash_table_for_frequency_of_symbols[j] > array[j+1])
            printf("i = %d, j = %d\n", i, j);
            if(j == 254) break;
            j++;
        }
        if(i == 254) break;
        i++;
    }
}

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
    uint64_t hash_table_for_frequency_of_symbols[255] = {0}; // символы здесь это числа в одном байте данных, которые бывают от 0 до 2^8 - 1 включительно
    count_frequency(hash_table_for_frequency_of_symbols, buffer, length_of_buffer);

    for(size_t i = 0; i < 255; i++){
        printf("%d: %d\n", i, hash_table_for_frequency_of_symbols[i]);
    }

    uint8_t indexes[256];
    uint8_t i = 0;

    while(1){
        indexes[i] = i;
        if(i == 255) break;
        i++;
    }

    sort_indexes(indexes, hash_table_for_frequency_of_symbols);
    
}