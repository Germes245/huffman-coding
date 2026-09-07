#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>

void count_frequency(uint64_t *hash_table_for_frequency_of_symbols, uint8_t *array_of_symbols, size_t length_of_array_of_symbols){

}

int main(){
    char* name_of_file = "input";

    // чтение файла

    struct stat file_info;
    if(stat(name_of_file, &file_info)){
        perror("Error reading file");
        return 1;
    }
    size_t size_of_file = stat(name_of_file);
    FILE *file = fopen("input", "rb");
    if(!file){
        perror("ошибка открытия файла\n");
        return 1;
    }
    uint64_t hash_table_for_frequency_of_symbols[255] = {0}; // символы здесь это числа в одном байте данных, которые бывают от 0 до 2^8 - 1 включительно
    count_frequency(hash_table_for_frequency_of_symbols, );
}