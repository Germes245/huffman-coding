# CMakeLists.txt для проекта Huffman-compress
#
# Структура проекта:
#   include/huffman-coding.h   — публичный заголовок библиотеки
#   src/huffman-coding.c       — реализация библиотеки
#   tests/main.c               — тестовая программа
#   tests/one letter.txt       — входные данные для теста
#
# Сборка:
#   cmake -S . -B build
#   cmake --build build
#
# Запуск теста:
#   cd build && ctest --output-on-failure
#   либо напрямую: ./build/huffman_test (из каталога build)

cmake_minimum_required(VERSION 3.13)

project(Huffman-compress
    VERSION 0.1.0
    DESCRIPTION "Huffman coding library and test program"
    LANGUAGES C)

# --- Стандарт C -----------------------------------------------------------
# В коде используются VLA и объявления переменных внутри for — это C99.
# C_EXTENSIONS OFF, чтобы не подмешивались GNU-расширения.
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# --- Библиотека -----------------------------------------------------------
add_library(huffman-coding STATIC
    src/huffman-coding.c
)

# Публичный include-каталог: подключающие увидят <huffman-coding.h>.
target_include_directories(huffman-coding
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Предупреждения для библиотеки.
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(huffman-coding PRIVATE -Wall -Wextra)
endif ()

# --- Тестовая программа ---------------------------------------------------
add_executable(huffman_test
    tests/main.c
)

target_link_libraries(huffman_test PRIVATE huffman-coding)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(huffman_test PRIVATE -Wall -Wextra)
endif ()

# --- Файл данных для теста ------------------------------------------------
# main.c открывает файл "one letter.txt" по относительному пути, поэтому
# копируем его в каталог сборки рядом с исполняемым файлом.
add_custom_command(TARGET huffman_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/one letter.txt"
        "$<TARGET_FILE_DIR:huffman_test>/one letter.txt"
    COMMENT "Копирование 'one letter.txt' в каталог сборки"
    VERBATIM
)

# --- CTest ----------------------------------------------------------------
enable_testing()

add_test(NAME huffman_test COMMAND huffman_test)
set_tests_properties(huffman_test PROPERTIES
    WORKING_DIRECTORY $<TARGET_FILE_DIR:huffman_test>
)