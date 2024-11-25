cmake_minimum_required(VERSION 3.14)  # Требуется для использования FetchContent
project(FunctionLibrary)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

# Загружаем GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/release-1.12.1.zip
)

# Для Windows: предотвращаем переопределение настроек компилятора/компоновщика родительского проекта
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Добавляем библиотеку с исходными файлами
add_library(FunctionLibrary
    TFunction.cpp
    IdentFunc.cpp
    ConstFunc.cpp
    PowerFunc.cpp
    ExpFunc.cpp
    PolynomialFunc.cpp
    FunctionFactory.cpp
    Operators.cpp
    GradientDescent.cpp
)

target_include_directories(FunctionLibrary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Создаем исполняемый файл для тестов
add_executable(FunctionTest FunctionTest.cpp)
target_link_libraries(FunctionTest FunctionLibrary GTest::gtest_main)

# Добавляем исполняемый файл main
add_executable(main main.cpp)
target_link_libraries(main FunctionLibrary)

include(GoogleTest)
gtest_discover_tests(FunctionTest)
