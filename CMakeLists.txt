# CMakeLists.txt для проекта Huffman-compress (динамическая библиотека)
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
#   либо напрямую: ./build/huffman_test

cmake_minimum_required(VERSION 3.13)

project(Huffman-compress
    VERSION 0.1.0
    DESCRIPTION "Huffman coding library and test program"
    LANGUAGES C)

# --- Стандарт C -----------------------------------------------------------
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# --- Динамическая библиотека ---------------------------------------------
# Вместо STATIC используем SHARED. CMake сам подставит нужный флаг
# (-fPIC для GCC/Clang на Linux, /LD для MSVC и т.п.).
add_library(huffman-coding SHARED
    src/huffman-coding.c
)

# Версия библиотеки: VERSION — версия API, SOVERSION — версия ABI.
# На Linux это даст libhuffman-coding.so.0.1.0, libhuffman-coding.so.0
# и симлинк libhuffman-coding.so. Если ABI совместим между релизами,
# SOVERSION не меняем, а VERSION обновляем.
set_target_properties(huffman-coding PROPERTIES
    VERSION   ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    # Явно выставляем PIC, хотя для SHARED CMake включает его сам.
    # Полезно, если таргет когда-нибудь захотят линковать статически
    # в другой shared-объект.
    POSITION_INDEPENDENT_CODE ON
)

# На Windows при экспорте символов из DLL по умолчанию ничего не видно.
# WINDOWS_EXPORT_ALL_SYMBOLS автоматически экспортирует все не-static
# функции — этого достаточно для учебного проекта.
set_target_properties(huffman-coding PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
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

# Чтобы тест находил .so без LD_LIBRARY_PATH, пропишем rpath на каталог
# сборки. После установки библиотеки в систему или в CMAKE_INSTALL_PREFIX
# тест всё равно найдёт её по стандартным путям.
set_target_properties(huffman_test PROPERTIES
    BUILD_RPATH   "$<TARGET_FILE_DIR:huffman-coding>"
    INSTALL_RPATH "$<TARGET_FILE_DIR:huffman-coding>"
)

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

# --- Установка ------------------------------------------------------------
# Опционально: позволяет сделать `cmake --install build --prefix ...`.
# Заголовок кладём в include, библиотеку — в lib. При установке в системный
# префикс тест не поставляем.
include(GNUInstallDirs)

install(TARGETS huffman-coding
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES include/huffman-coding.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)