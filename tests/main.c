/**
 * @file main.c
 * @brief Подсчёт частот байтов, сортировка символов по частоте и построение дерева Хаффмана.
 *
 * В файле реализованы вспомогательные функции для кодирования Хаффмана:
 * подсчёт частот символов, сортировка индексов по частотам и сборка дерева.
 */
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

/**
 * @def length_of_buffer
 * @brief Длина входного файла в байтах.
 */
#define length_of_buffer file_info.st_size

/**
 * @brief Выделяет память заданного размера или завершает программу при ошибке.
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
 */
void HE_count_frequency(uint64_t *hash_table_for_frequency_of_symbols,
                        uint8_t *array_of_symbols,
                        size_t length_of_array_of_symbols) {
    for (size_t i = 0; i < length_of_array_of_symbols; i++) {
        hash_table_for_frequency_of_symbols[array_of_symbols[i]]++;
    }
}

/**
 * @brief Сортирует индексы символов по возрастанию частоты пузырьковой сортировкой.
 *
 * @param[in,out] indexes Массив индексов 0..255, который сортируется по частотам.
 * @param[in] hash_table_for_frequency_of_symbols Таблица частот символов.
 *
 * @return Индекс последнего нулевого элемента в отсортированном массиве,
 *         либо -1, если ни у одного символа частота не равна нулю.
 *
 * @note После сортировки все символы с нулевой частотой оказываются в начале
 *       массива, поэтому достаточно найти последний индекс, у которого частота
 *       равна нулю, и остановиться при первом ненулевом.
 */
int16_t HE_sort_indexes(uint8_t indexes[],
                        uint64_t hash_table_for_frequency_of_symbols[]) {
    /* Пузырьковая сортировка индексов по возрастанию частоты. */
    for (size_t i = 0; i < 256; i++) {
        for (size_t j = 0; j < 256 - i - 1; j++) {
            if (hash_table_for_frequency_of_symbols[indexes[j]] >
                hash_table_for_frequency_of_symbols[indexes[j + 1]]) {
                uint8_t temp = indexes[j + 1];
                indexes[j + 1] = indexes[j];
                indexes[j] = temp;
            }
        }
    }

    /* После сортировки нули стоят в начале. Ищем последний нуль. */
    int16_t index_of_last_zero = -1;
    for (int i = 0; i < 256; i++) {
        if (hash_table_for_frequency_of_symbols[indexes[i]] == 0) {
            index_of_last_zero = (int16_t)i;
        } else {
            break;  /* Дальше нулей быть не может — массив отсортирован. */
        }
    }
    return index_of_last_zero;
}

/*
 * Дерево строится так, что в *left указывается текущий лист, а в *right
 * указывается следующая ветвь листов. Можно представить так:
 *
 * (A)    (B)   (C)    (D)   (E)   (F)
 *   \    /       \    /      \    /
 *    (AB)         (CD)        (EF)
 *        \       /           /
 *         (ABCD)            /
 *            \             /
 *             \           /
 *              \         /
 *               \       /
 *                (ABCDEF)
 */

/**
 * @enum is_leaf_
 * @brief Признак того, является ли узел дерева листом.
 */
enum is_leaf_ {
    NO,  ///< Узел не является листом.
    YES  ///< Узел является листом.
};

/**
 * @struct huffman_node
 * @brief Узел дерева Хаффмана.
 */
typedef struct huffman_node {
    uint8_t symbol;            ///< Символ (если узел — лист).
    uint64_t frequency;        ///< Частота (вес) узла.
    struct huffman_node *left; ///< Левый потомок.
    struct huffman_node *right;///< Правый потомок.
    enum is_leaf_ is_leaf;     ///< Признак листа.
} huffman_node;

/**
 * @struct huffman_code
 * @brief Код Хаффмана для символа.
 */
typedef struct {
    uint8_t code[16];       ///< Битовая последовательность кода.
    uint8_t length_of_code; ///< Длина кода в битах.
} huffman_code;

/**
 * @brief Рекурсивно выводит узел дерева Хаффмана в stdout.
 */
void print_node(huffman_node *node) {
    if (node->is_leaf) {
        printf("this is a leaf\nfrequency: %lu\nsymbol: %d\n",
               (unsigned long)node->frequency, node->symbol);
    } else {
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
 * @param[in] pointers_for_numbers_in_hash_table_for_frequency_of_symbols
 *            Массив индексов символов, отсортированных по частоте.
 * @param[in] array_of_frequences Таблица частот символов.
 * @param[in] index_of_last_zero  Индекс последнего нулевого элемента в массиве
 *                                индексов, либо -1, если нулей нет.
 *
 * @return Указатель на корень построенного дерева Хаффмана,
 *         либо NULL, если кодировать нечего (нет ни одного ненулевого символа).
 *
 * @note Входной массив индексов должен быть отсортирован по возрастанию частоты.
 */
huffman_node *build_huffman_tree(
        uint8_t pointers_for_numbers_in_hash_table_for_frequency_of_symbols[],
        uint64_t *array_of_frequences,
        int16_t index_of_last_zero) {

    /* Если нулей нет вообще — index_of_last_zero == -1; это нормальный случай
       (все 256 байт встречаются). Если наоборот, все частоты нулевые —
       index_of_last_zero == 255, кодировать нечего. */
    if (index_of_last_zero >= 255) {
        return NULL;
    }

    uint16_t length = (uint16_t)(255 - index_of_last_zero);
    if (length == 0) {
        return NULL;
    }

    /* Создание листьев. */
    huffman_node *huffman_nodes[length];

    uint16_t j = (uint16_t)(index_of_last_zero + 1);
    for (uint16_t i = 0; i < length; i++) {
        huffman_nodes[i] = xmalloc(sizeof(huffman_node));
        huffman_nodes[i]->is_leaf   = YES;
        huffman_nodes[i]->symbol    = pointers_for_numbers_in_hash_table_for_frequency_of_symbols[j];
        huffman_nodes[i]->frequency = array_of_frequences[huffman_nodes[i]->symbol];
        huffman_nodes[i]->left      = NULL;
        huffman_nodes[i]->right     = NULL;
        j++;
    }

    /* Если лист единственный — он же корень. */
    if (length == 1) {
        return huffman_nodes[0];
    }

    /* Стек «отложенных» узлов для нечётных слоёв. */
    huffman_node *stack_for_tree[length];
    uint16_t score_of_elements_in_stack = 0;

    uint16_t length_of_last_layer    = length;
    uint16_t length_of_current_layer = length / 2;

    do {
        /* Если слой нечётный, последний узел откладываем в стек
           и исключаем его из спаривания. */
        uint16_t effective_length = length_of_last_layer;
        if (length_of_last_layer % 2 == 1) {
            stack_for_tree[score_of_elements_in_stack] =
                huffman_nodes[length_of_last_layer - 1];
            score_of_elements_in_stack++;
            effective_length = length_of_last_layer - 1;
        }

        /* Спариваем соседние узлы: (0,1), (2,3), ... */
        j = 0;
        for (uint16_t i = 0; i < effective_length / 2; i++) {
            huffman_node *node = xmalloc(sizeof(huffman_node));
            node->is_leaf   = NO;
            node->symbol    = 0;
            node->left      = huffman_nodes[j];
            j++;
            node->right     = huffman_nodes[j];
            j++;
            node->frequency = node->left->frequency + node->right->frequency;
            huffman_nodes[i] = node;
        }

        length_of_last_layer    = length_of_current_layer;
        length_of_current_layer /= 2;
    } while (length_of_last_layer > 1);

    /* Прикрепляем отложенные в стек узлы к корню. */
    if (score_of_elements_in_stack != 0) {
        uint16_t i = score_of_elements_in_stack;
        do {
            i--;
            huffman_node *new_root = xmalloc(sizeof(huffman_node));
            new_root->is_leaf   = NO;
            new_root->symbol    = 0;
            new_root->left      = huffman_nodes[0];
            new_root->right     = stack_for_tree[i];
            new_root->frequency = new_root->left->frequency + new_root->right->frequency;
            huffman_nodes[0] = new_root;
        } while (i != 0);
    }

    return huffman_nodes[0];
}

/**
 * @brief Освобождает память, занятую всеми узлами дерева Хаффмана.
 */
void free_huffman_tree(huffman_node *root) {
    if (root == NULL) {
        return;
    }

    huffman_node *stack[512];
    int top = 0;

    stack[top++] = root;

    while (top > 0) {
        huffman_node *node = stack[--top];

        if (node->left != NULL) {
            stack[top++] = node->left;
        }
        if (node->right != NULL) {
            stack[top++] = node->right;
        }

        free(node);
    }
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
 * @param[out] codes Массив размером 256, где codes[symbol] содержит код
 *                   для символа symbol.
 * @param[in]  root  Корень дерева Хаффмана.
 */
void get_codes_of_symbols(huffman_code codes[], huffman_node *root) {
    memset(codes, 0, 256 * sizeof(huffman_code));

    if (root == NULL) {
        return;
    }

    /* Особый случай: дерево состоит из одного листа. Тогда для того, чтобы
       кодирование/декодирование работали единообразно, выдаём этому символу
       код длины 1. Бит не важен: договоримся, что это 0. */
    if (root->is_leaf) {
        codes[root->symbol].length_of_code = 1;
        codes[root->symbol].code[0] = 0;
        return;
    }

    typedef struct {
        huffman_node *node;
        uint8_t depth;
        uint8_t state; /* 0 — вошли, 1 — левый обработан, 2 — правый обработан */
    } dfs_frame;

    uint8_t  path[256];
    dfs_frame stack[256];
    int top = 0;

    stack[top].node  = root;
    stack[top].depth = 0;
    stack[top].state = 0;
    top++;

    while (top > 0) {
        dfs_frame *frame = &stack[top - 1];
        huffman_node *node = frame->node;
        uint8_t depth = frame->depth;

        if (frame->state == 0) {
            if (node->is_leaf) {
                huffman_code *code = &codes[node->symbol];
                uint8_t n = (depth < 16) ? depth : 16;
                code->length_of_code = n;
                for (uint8_t k = 0; k < n; k++) {
                    code->code[k] = path[k];
                }
                top--;
            } else {
                path[depth] = 255; /* налево — бит 1 */
                stack[top].node  = node->left;
                stack[top].depth = depth + 1;
                stack[top].state = 0;
                top++;
                frame->state = 1;
            }
        } else if (frame->state == 1) {
            path[depth] = 0; /* направо — бит 0 */
            stack[top].node  = node->right;
            stack[top].depth = depth + 1;
            stack[top].state = 0;
            top++;
            frame->state = 2;
        } else {
            top--;
        }
    }
}

/**
 * @brief Кодирует входной буфер с помощью кодов Хаффмана.
 *
 * Биты укладываются MSB-first: первый бит кода записывается в старший бит
 * первого байта. Последний байт при неполной упаковке дополняется нулями.
 *
 * @param[in]  input_buffer        Исходные данные.
 * @param[in]  input_buffer_length Длина входного буфера в байтах.
 * @param[in]  codes_of_symbols    Массив из 256 кодов Хаффмана.
 * @param[out] output_buffer       Буфер для упакованных битов. Должен быть
 *                                 достаточно большим: не менее
 *                                 (input_buffer_length * MAX_CODE_LEN + 7) / 8
 *                                 байт.
 * @param[out] out_total_bits      Число значимых бит (без выравнивающего
 *                                 паддинга). Может быть NULL.
 *
 * @return Количество записанных байт, то есть (total_bits + 7) / 8.
 */
size_t code_text(const uint8_t input_buffer[], size_t input_buffer_length,
                 const huffman_code codes_of_symbols[], uint8_t output_buffer[],
                 size_t *out_total_bits) {
    size_t bit_pos = 0;

    for (size_t i = 0; i < input_buffer_length; i++) {
        const huffman_code *c = &codes_of_symbols[input_buffer[i]];

        for (uint8_t b = 0; b < c->length_of_code; b++) {
            size_t  byte_idx  = bit_pos >> 3;
            uint8_t bit_index = 7 - (uint8_t)(bit_pos & 7);
            uint8_t mask      = (uint8_t)(1u << bit_index);

            if ((bit_pos & 7) == 0) {
                output_buffer[byte_idx] = c->code[b] ? mask : 0;
            } else if (c->code[b]) {
                output_buffer[byte_idx] |= mask;
            }
            bit_pos++;
        }
    }

    if (out_total_bits != NULL) {
        *out_total_bits = bit_pos;
    }
    return (bit_pos + 7) >> 3;
}

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
    for (int k = 0; k < 256; k++) {
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