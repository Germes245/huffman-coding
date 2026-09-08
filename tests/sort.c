#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

void _1byte_buble_sort(uint8_t array[], size_t length){ // length -- длина массива
    for(size_t i = 0; i < length-1; i++){
        for(size_t j = 0; j < length-i-1; j++){
            if(array[j] > array[j+1]){
                uint8_t temp = array[j+1];
                array[j+1] = array[j];
                array[j] = temp;
            }
        }
    }
}

int main(){
    uint8_t array[] = {255, 200, 100};
    _1byte_buble_sort(array, sizeof(array)/sizeof(array[0]));
    for(size_t i = 0; i < sizeof(array)/sizeof(array[0]); i++){
        printf("%d\n", array[i]);
    }
}