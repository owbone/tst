add_executable(tst_set_test tst_set_test.cpp)
target_link_libraries(tst_set_test gtest gmock)
add_test(tst_set_test tst_set_test)
