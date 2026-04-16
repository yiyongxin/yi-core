include(FetchContent)

FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest.git
	GIT_TAG        release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# ── Binary 序列化测试 ────────────────────────────────────────────────────────
add_executable(test_binary test_binary.cpp)
target_link_libraries(test_binary PRIVATE yi_core GTest::gtest_main)
add_test(NAME binary_serialize COMMAND test_binary)

# ── JSON 序列化测试 ──────────────────────────────────────────────────────────
add_executable(test_json test_json.cpp)
target_link_libraries(test_json PRIVATE yi_core GTest::gtest_main)
add_test(NAME json_serialize COMMAND test_json)
