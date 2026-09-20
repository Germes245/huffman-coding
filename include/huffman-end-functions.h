/**
 * @file huffman-end-functions.h
 * @brief Конечный (высокоуровневый) API библиотеки кодирования Хаффмана.
 *
 * Позволяет сжимать/распаковывать буферы и файлы, не заглядывая внутрь
 * дерева Хаффмана. Сжатый поток самодостаточен: содержит сериализованное
 * дерево (или таблицу частот), число значимых бит и упакованные коды.
 */

#ifndef HUFFMAN_END_FUNCTIONS_H
#define HUFFMAN_END_FUNCTIONS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct uint8_t_array
 * @brief Буфер байтов с длиной в байтах и (опционально) в битах.
 *
 * Для сжатого буфера:
 *   - array[0..bytes-1] содержит упакованные биты, выровненные по байтам;
 *   - bits — число значимых бит, bits <= bytes * 8;
 *   - незначащие биты последнего байта (если есть) не определены и должны
 *     игнорироваться.
 *
 * Для распакованного буфера:
 *   - array[0..bytes-1] содержит исходные байты;
 *   - bits всегда равно bytes * 8 (либо 0, если bytes == 0).
 *
 * Владение памятью: array выделяется внутри библиотеки и должен быть
 * освобождён вызывающим через HE_free_buffer() (или напрямую free()).
 * Признак ошибки — array == NULL (в этом случае bytes и bits равны 0).
 */
typedef struct {
    uint8_t *array;
    size_t   bytes;
    size_t   bits;
} uint8_t_array;

/**
 * @brief Освобождает буфер, полученный от HE_compress_buffer/HE_decompress_buffer.
 *
 * Безопасно вызывать для пустой структуры (array == NULL): просто ничего
 * не делает. После вызова структура обнуляется.
 */
void HE_free_buffer(uint8_t_array *buf);

/**
 * @brief Сжимает буфер с помощью кодирования Хаффмана.
 *
 * @param[in]  array            Исходные данные. Может быть NULL, если
 *                              length_in_bytes == 0.
 * @param[in]  length_in_bytes  Длина исходных данных в байтах.
 *
 * @return Структура с упакованными данными. При успехе:
 *           array != NULL (или bytes == 0 для пустого входа),
 *           bits  == число значимых бит,
 *           bytes == (bits + 7) / 8.
 *         При ошибке (нехватка памяти, некорректные аргументы):
 *           array == NULL, bytes == 0, bits == 0.
 *
 * @note Пустой вход — валидный случай: возвращается корректный сжатый
 *       поток с нулём значимых бит (то есть, как минимум, с заголовком
 *       дерева).
 */
uint8_t_array HE_compress_buffer(const uint8_t *array, size_t length_in_bytes);

/**
 * @brief Распаковывает буфер, сжатый HE_compress_buffer.
 *
 * @param[in] array          Упакованные данные (выход HE_compress_buffer).
 * @param[in] length_in_bytes Размер массива array в байтах. Нужен для
 *                            проверки границ; может быть больше, чем
 *                            (length_in_bits + 7) / 8, если буфер имеет
 *                            запас, но не меньше.
 * @param[in] length_in_bits  Число значимых бит в упакованном потоке,
 *                            полученное от HE_compress_buffer.
 *
 * @return Структура с распакованными данными. При успехе:
 *           array != NULL (или bytes == 0 для пустого результата),
 *           bytes == длина исходного сообщения,
 *           bits  == bytes * 8.
 *         При ошибке (повреждённые данные, нехватка памяти, несоответствие
 *         границ, оборванный код):
 *           array == NULL, bytes == 0, bits == 0.
 *
 * @note Функция ожидает самодостаточный поток: дерево Хаффмана и число
 *       значимых бит уже содержатся в array как часть формата.
 */
uint8_t_array HE_decompress_buffer(const uint8_t *array,
                                   size_t length_in_bytes,
                                   size_t length_in_bits);

/**
 * @brief Сжимает файл и записывает результат в другой файл.
 *
 * @param[in] input_file  Путь к исходному файлу.
 * @param[in] output_file Путь к файлу для сжатых данных. Если файл
 *                        существует, он перезаписывается.
 *
 * @return 0 при успехе, ненулевой код при ошибке (errno-подобный).
 *
 * @note Входной и выходной пути не должны совпадать — поведение в этом
 *       случае не определено.
 */
int HE_compress_file(const char *input_file, const char *output_file);

/**
 * @brief Распаковывает файл и записывает результат в другой файл.
 *
 * @param[in] input_file  Путь к сжатому файлу (выход HE_compress_file).
 * @param[in] output_file Путь к файлу для исходных данных. Если файл
 *                        существует, он перезаписывается.
 *
 * @return 0 при успехе, ненулевой код при ошибке (errno-подобный).
 */
int HE_decompress_file(const char *input_file, const char *output_file);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* HUFFMAN_END_FUNCTIONS_H */