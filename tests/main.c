/**
 * @file main.c
 * @brief Подсчёт частот байтов, сортировка символов по частоте и построение дерева Хаффмана.
 *
 * В файле реализованы вспомогательные функции для кодирования Хаффмана:
 * подсчёт частот символов, сортировка индексов по частотам и сборка дерева.
 */

//#include <cstddef>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
//#include <bool.h>
#include <stdlib.h>
#include <sys/stat.h>

/**
 * @def length_of_buffer
 * @brief Длина входного файла в байтах.
 *
 * Макрос обращается к полю st_size структуры file_info.
 * Предполагается, что переменная file_info объявлена в области видимости.
 */
#define length_of_buffer file_info.st_size

/**
 * @brief Выделяет память заданного размера или завершает программу при ошибке.
 *
 * @param size Количество байт для выделения.
 * @return Указатель на выделенный блок памяти.
 */
void *xmalloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "xmalloc: out of memory (%zu bytes)\n", size);
        exit(1);
    }
    return ptr;
}

// HE -- Huffman Encoding

/**
 * @brief Подсчитывает частоты появления байтов во входном массиве.
 *
 * @param[out] hash_table_for_frequency_of_symbols Массив из 256 счётчиков частот.
 * @param[in] array_of_symbols Входной массив байтов.
 * @param[in] length_of_array_of_symbols Количество байтов в массиве.
 */
void HE_count_frequency(uint64_t *hash_table_for_frequency_of_symbols, uint8_t *array_of_symbols, size_t length_of_array_of_symbols){
    for(size_t i = 0; i < length_of_array_of_symbols; i++){
        hash_table_for_frequency_of_symbols[array_of_symbols[i]]++;
    }
}

/**
 * @brief Сортирует индексы символов по возрастанию частоты пузырьковой сортировкой.
 *
 * @param[in,out] indexes Массив индексов 0..255, который сортируется по частотам.
 * @param[in] hash_table_for_frequency_of_symbols Таблица частот символов.
 * @return Индекс последнего нулевого элемента в отсортированном массиве.
 *
 * @note Переменная index_of_last_zero обновляется только при обмене элементов,
 *       поэтому в некоторых случаях её значение требует дополнительной проверки.
 */
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

/**
 * @enum is_leaf_
 * @brief Признак того, является ли узел дерева листом.
 */
enum is_leaf_{
    NO,  ///< Узел не является листом.
    YES  ///< Узел является листом.
};

/**
 * @struct huffman_node
 * @brief Узел дерева Хаффмана.
 */
typedef struct huffman_node {
    uint8_t symbol;           ///< Символ (если узел — лист).
    uint64_t frequency;       ///< Частота (вес) узла.
    struct huffman_node *left;///< Левый потомок.
    struct huffman_node *right;///< Правый потомок.
    enum is_leaf_ is_leaf;    ///< Признак листа.
} huffman_node;

/**
 * @struct huffman_code
 * @brief Код Хаффмана для символа.
 */
typedef struct{
    uint8_t code[16];         ///< Битовая последовательность кода.
    uint8_t length_of_code;   ///< Длина кода в битах.
} huffman_code;

/**
 * @brief Рекурсивно выводит узел дерева Хаффмана в stdout.
 *
 * @param node Узел для вывода.
 */
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

/**
 * @brief Строит дерево Хаффмана по отсортированным частотам символов.
 *
 * @param[in] pointers_for_numbers_in_hash_table_for_frequency_of_symbols Массив индексов символов,
 *            отсортированных по частоте.
 * @param[in] array_of_frequences Таблица частот символов.
 * @param[in] index_of_last_zero Индекс последнего нулевого элемента в массиве индексов.
 * @return Указатель на корень построенного дерева Хаффмана.
 *
 * @note Входной массив индексов должен быть отсортирован по возрастанию частоты.
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

/**
 * @brief Заполняет массив кодов Хаффмана для всех символов дерева.
 *
 * Функция обходит дерево Хаффмана от корня к листьям без рекурсии,
 * используя ручной стек, накапливая путь в буфере path[].
 *
 * Каждый бит кода «растягивается» в целый байт:
 *   - ветвь налево  → 255 (0xFF), что играет роль бита 1;
 *   - ветвь направо →   0 (0x00), что играет роль бита 0.
 *
 * Байты записываются в code[0], code[1], ... в порядке обхода
 * (первое ответвление от корня — в code[0], второе — в code[1], ...).
 *
 * @param[out] codes Массив размером 256, где codes[symbol] содержит код
 *                   для символа symbol.
 * @param[in]  root  Корень дерева Хаффмана.
 *
 * @note Максимальная длина кода ограничена размером массива code[16],
 *       т.е. 16 «растянутыми» битами. Если глубина листа больше 16,
 *       лишние биты не сохраняются, а length_of_code обрезается до 16.
 */
void get_codes_of_symbols(huffman_code codes[], huffman_node *root) {
    /* Инициализация: обнуляем все коды */
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 16; j++) {
            codes[i].code[j] = 0;
        }
        codes[i].length_of_code = 0;
    }

    if (root == NULL) {
        return;
    }

    /* Кадр ручного стека для итеративного обхода в глубину */
    typedef struct {
        huffman_node *node;  /* текущий узел */
        uint8_t depth;       /* глубина узла (длина префикса) */
        uint8_t state;       /* 0 — вошли, 1 — левый обработан, 2 — правый обработан */
    } dfs_frame;

    /* Буфер пути: 255 — налево, 0 — направо.
       Максимальная глубина бинарного дерева с 256 листьями — 255. */
    uint8_t path[256];

    /* Стек обхода: в худшем случае 256 кадров (корень + 255 уровней). */
    dfs_frame stack[256];
    int top = 0;

    /* Кладём корень */
    stack[top].node  = root;
    stack[top].depth = 0;
    stack[top].state = 0;
    top++;

    while (top > 0) {
        dfs_frame *frame = &stack[top - 1];
        huffman_node *node = frame->node;
        uint8_t depth = frame->depth;

        if (frame->state == 0) {
            /* Первое посещение узла */
            if (node->is_leaf) {
                /* Копируем накопленный путь в код символа.
                   Каждый элемент path[i] — уже целый байт: 255 или 0.
                   Обрезаем по размеру code[16]. */
                huffman_code *code = &codes[node->symbol];
                uint8_t n = (depth < 16) ? depth : 16;
                code->length_of_code = n;
                for (uint8_t i = 0; i < n; i++) {
                    code->code[i] = path[i];
                }
                /* Лист — потомков нет, сразу снимаем со стека */
                top--;
            } else {
                /* Спуск влево: бит = 1 = 255 */
                path[depth] = 255;
                stack[top].node  = node->left;
                stack[top].depth = depth + 1;
                stack[top].state = 0;
                top++;
                frame->state = 1;  /* при возврате пойдём вправо */
            }
        } else if (frame->state == 1) {
            /* Левый потомок обработан — идём в правый: бит = 0 */
            path[depth] = 0;
            stack[top].node  = node->right;
            stack[top].depth = depth + 1;
            stack[top].state = 0;
            top++;
            frame->state = 2;  /* при возврате узел уже полностью обработан */
        } else {
            /* Оба потомка обработаны — снимаем узел со стека */
            top--;
        }
    }
}

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

    huffman_code codes_of_symbols[256];

    huffman_node *root = build_huffman_tree(indexes, hash_table_for_frequency_of_symbols, index_of_last_zero);
    get_codes_of_symbols(codes_of_symbols, root);

    for (uint16_t i = 0; i < 256; i++) {
        huffman_code current_el = codes_of_symbols[i];
        if(current_el.length_of_code){
            printf("length: %d, index: %d\n", current_el.length_of_code, i);
            for(uint8_t i = 0; i < current_el.length_of_code; i++){
                printf("%d ", current_el.code[indexes[i]]);
            }
            putchar('\n');
        }
    }
}