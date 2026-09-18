//#include <cstddef>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
//#include <bool.h>
#include <stdlib.h>
#include <sys/stat.h>

#define length_of_buffer file_info.st_size

void *xmalloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "xmalloc: out of memory (%zu bytes)\n", size);
        exit(1);
    }
    return ptr;
}

// HE -- Huffman Encoding

/*
 * подсчитывает частотность символов
 * hash_table_for_frequency_of_symbols -- указатель на массив фиксированой длины в 256 элементов
*/
void HE_count_frequency(uint64_t *hash_table_for_frequency_of_symbols, uint8_t *array_of_symbols, size_t length_of_array_of_symbols){
    for(size_t i = 0; i < length_of_array_of_symbols; i++){
        hash_table_for_frequency_of_symbols[array_of_symbols[i]]++;
    }
}

uint8_t HE_sort_indexes(uint8_t indexes[], uint64_t hash_table_for_frequency_of_symbols[]){
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

/*
 * дерево строится так что в *left указывается текущий лист, а в *rigth указывается следующая ветвь листов
 * можно представить так:
(A)    (B)   (C)    (D)   (E)   (F)
  \    /       \    /      \    /
   (AB)         (CD)        (EF)   
       \       /           /
        (ABCD)            /
           \             /
            \           /
             \         /
              \       /
               (ABCDEF)
ФИСВ
*/

enum is_leaf_{
    NO,
    YES
};

typedef struct huffman_node {
    uint8_t symbol;           // символ (если лист)
    uint64_t frequency;       // частота (вес)
    struct huffman_node *left;
    struct huffman_node *right;
    enum is_leaf_ is_leaf;              // 1, если это лист
} huffman_node;

typedef struct{
    uint8_t code[16];
    uint8_t length_of_code;
} huffman_code;

void print_node(huffman_node* node){
    if(node->is_leaf){
        printf("this is a leaf\nfrequency: %d\nsymbol: %d\n", node->frequency, node->symbol);
    }
    else{
        printf("this is not a leaf\npoints on leafs:\n\nleft:\n\n");
        print_node(node->left);
        printf("\nrigth:\n\n");
        print_node(node->right);
        putchar('\n');
    }
}
// elton john - a word in spanish
/*
 * @param index_of_last_zero -- индекс для массива, который указывает на число в массиве pointers_for_numbers_in_hash_table_for_frequency_of_symbols, которое является последним указателем в pointers_for_numbers_in_hash_table_for_frequency_of_symbols, указывающее на число 0 в хэш-таблице частот символов
*/
huffman_node* build_huffman_tree(uint8_t pointers_for_numbers_in_hash_table_for_frequency_of_symbols[], uint64_t *array_of_frequences, uint16_t index_of_last_zero){
    uint16_t length = 255 - index_of_last_zero;

    // создание листьев

    huffman_node* huffman_nodes[length];

    uint8_t j = index_of_last_zero + 1;
    for (uint16_t i = 0; i < length; i++) {
        huffman_nodes[i] = xmalloc(sizeof(huffman_node));
        huffman_nodes[i]->is_leaf = YES;
        huffman_nodes[i]->symbol = pointers_for_numbers_in_hash_table_for_frequency_of_symbols[j];
        huffman_nodes[i]->frequency = array_of_frequences[huffman_nodes[i]->symbol];
        j++;
    }
    
    // листья созданы, теперь строится первый слой веток

    if(length == 1){ // если нода одна
        return huffman_nodes[0];
    }

    huffman_node* stack_for_tree[length];
    uint16_t score_of_elements_in_stack = 0;

    uint16_t length_of_last_layer = length;
    uint16_t length_of_current_layer = length / 2;

    do{
        printf("last_layer: %d\n", length_of_last_layer);
        printf("current layer: %d\n", length_of_current_layer);
        printf("stack pos: %d\n", score_of_elements_in_stack);
        j = 0;
        if(length_of_last_layer % 2 == 1){
            stack_for_tree[score_of_elements_in_stack] = huffman_nodes[length_of_last_layer-1];
            score_of_elements_in_stack++;
        }
        for (uint16_t i = 0; i < length_of_last_layer;) {
            huffman_node *node = xmalloc(sizeof(huffman_node));
            node->is_leaf = NO;
            node->left = huffman_nodes[i];
            i++;
            node->right = huffman_nodes[i];
            i++;
            huffman_nodes[j] = node;
            j++;
        }
        length_of_last_layer = length_of_current_layer;
        length_of_current_layer /= 2;
    } while (length_of_last_layer > 1);

    printf("stack pos: %d\n", score_of_elements_in_stack);

    if(score_of_elements_in_stack != 0){
        uint16_t i = score_of_elements_in_stack;
        do{
            i--;
            printf("shya\n");
            huffman_node *new_root = xmalloc(sizeof(huffman_node));
            new_root->is_leaf = NO;
            new_root->left = huffman_nodes[0];
            new_root->right = stack_for_tree[i];
            huffman_nodes[0] = new_root;
        } while(i != 0);
    }
    return huffman_nodes[0];
}
/*
#define add_code(array, symbol){

}*/

enum direct{
    LEFT,
    RIGHT
};

typedef struct{
    huffman_node* node;
    enum direct direct;
} node_for_stack;

/*
 * возвращает текущий bypass_stack_current_element
*/
/*
static uint8_t go_to_left(node_for_stack bypass_stack[], uint8_t bypass_stack_current_element){
    while (1) {
        huffman_node* current_node = bypass_stack[bypass_stack_current_element].node;
        enum is_leaf_ is_leaf_ = current_node->is_leaf;
        if(is_leaf_ == NO){
            bypass_stack_current_element++;
            bypass_stack[bypass_stack_current_element].node = current_node->left;
            bypass_stack[bypass_stack_current_element].direct = RIGHT;
        }
        else if(is_leaf_ == YES){
            bypass_stack[bypass_stack_current_element].node = current_node;
            return bypass_stack_current_element;
        }
        //bypass_stack_current_element++;
    }
}*/

/*
 * выдаёт код для каждого символа, по кодированию Хаффана
 * @param 
*/
huffman_code_array get_codes_of_symbols(huffman_code , huffman_node* root_node);*/

int main(){
    char* name_of_file = "one letter.txt";//"input";

    // чтение файла

    struct stat file_info;
    if(stat(name_of_file, &file_info) && length_of_buffer == 0){
        perror("Error reading file");
        return 1;
    }

    FILE *file = fopen(name_of_file, "rb");
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
    HE_count_frequency(hash_table_for_frequency_of_symbols, buffer, length_of_buffer);

    /*for(size_t i = 0; i < 255; i++){
        printf("%d: %d\n", i, hash_table_for_frequency_of_symbols[i]);
    }*/

    uint8_t indexes[256]; // массив индексов, которые указывают адресс числа в массиве hash_table_for_frequency_of_symbols
    uint8_t i = 0;

    while(1){
        indexes[i] = i;
        if(i == 255) break;
        i++;
    }

    uint16_t index_of_last_zero = HE_sort_indexes(indexes, hash_table_for_frequency_of_symbols);
    printf("index_of_last_zero: %d\n", index_of_last_zero);

    for(size_t i = index_of_last_zero; i < 256; i++){
        printf("i = %d, in index array: %d, value = %ld\n", i, indexes[i], hash_table_for_frequency_of_symbols[indexes[i]]);
    }

    // начало создания кодов

    huffman_node *root = build_huffman_tree(indexes, hash_table_for_frequency_of_symbols, index_of_last_zero);
    //print_node(root);
    get_codes_of_symbols(root);
}