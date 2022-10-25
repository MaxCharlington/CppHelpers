#ifndef TESTING_HPP
#define TESTING_HPP

#include <functional>
#include <string>
#include <string_view>
#include <cstdio>

// Полезны для тестов
#include <type_traits>
#include <concepts>

/*
Пример написания теста

TESTS_BEGIN
{"Sample suit", {
    {
        "Sample test",
        []{
            return 1 == 1;
        }
    },
    {
        "Sample test 2",
        []{}
    }
}}
TESTS_END

Тесты пишутся вне всех функций и классов и прогоняются при объявлении -DRUN_TESTS при компиляции
*/
namespace TESTING {
    int TEST_RESULT = EXIT_SUCCESS;
    unsigned TESTS_PASSED = 0;
    unsigned TESTS_FAILED = 0;

    struct TEST_CASE {
        std::string_view name;
        std::function<bool()> func;

        TEST_CASE(std::string_view test_name, bool(*test_func)())
            : name{test_name}, func{test_func} {}

        TEST_CASE(std::string_view test_name, void(*test_func)())
            : name{test_name}, func{[test_func]{test_func(); return true;}} {}

        bool run() const {
            return func();
        }
    };

    struct TEST_SUIT_IMPL {
        TEST_SUIT_IMPL(std::string_view suit_name, std::initializer_list<TEST_CASE> tests) {
    #ifdef RUN_TESTS
            printf("\nTest suit: \"%s\"\n\n", suit_name.data());
            size_t num{};
            size_t failed_count{};
            size_t success_count{};

            for (const auto &test : tests) {
                printf("%lu. %s", ++num, test.name.data());
                bool success_status = false;
                try
                {
                    success_status = test.run();
                }
                catch(...) {}

                if (success_status) {
                    success_count++;
                    puts(" -> Passed");
                    TESTS_PASSED += 1;
                }
                else {
                    failed_count++;
                    puts(" -> Failed");
                    TESTS_FAILED += 1;
                    TEST_RESULT = EXIT_FAILURE;
                }
            }
            printf("\nSUMMARY for \"%s\": %lu passed and %lu failed\n\n", suit_name.data(), success_count, failed_count);
    #endif  // RUN_TESTS
        }
    };
}  // namespace TESTING


/*
Макрос для прогона тестов из нескольких хедеров сразу
*/
#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)
#define TESTS_BEGIN \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wold-style-cast\"") \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wnonnull\"") \
    TESTING::TEST_SUIT_IMPL CONCAT(test, __COUNTER__)

#define TESTS_END ;\
    _Pragma("GCC diagnostic pop") \
    _Pragma("GCC diagnostic pop")


#ifdef RUN_TESTS
    #define TEST_MAIN main
#endif  // RUN_TESTS

int TEST_MAIN() {
    printf("All tests SUMMARY: ");
    if (TESTING::TEST_RESULT == EXIT_FAILURE) {
        printf("Some tests failed: %u passed, %u failed\n\n", TESTING::TESTS_PASSED, TESTING::TESTS_FAILED);
    }
    else {
        printf("All tests passed: %u passed\n\n", TESTING::TESTS_PASSED);
    }
    return TESTING::TEST_RESULT;
}

#endif  // TESTING_HPP
