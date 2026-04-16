find_package(GTest REQUIRED)

# ── Binary 序列化测试 ────────────────────────────────────────────────────────
add_executable(test_binary test_binary.cpp)
target_link_libraries(test_binary PRIVATE yi::serialize GTest::GTest GTest::Main)
add_test(NAME binary_serialize COMMAND test_binary)

# ── JSON 序列化测试 ──────────────────────────────────────────────────────────
add_executable(test_json test_json.cpp)
target_link_libraries(test_json PRIVATE yi::serialize GTest::GTest GTest::Main)
add_test(NAME json_serialize COMMAND test_json)
