include(ExternalProject)

ExternalProject_Add(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.8.0
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
    INSTALL_COMMAND "")

ExternalProject_Get_Property(googletest source_dir)
ExternalProject_Get_Property(googletest binary_dir)

add_library(gtest INTERFACE)
add_dependencies(gtest googletest)
target_include_directories(gtest INTERFACE
    "${source_dir}/googletest/include")
target_link_libraries(gtest INTERFACE
    "${binary_dir}/googlemock/gtest/libgtest.a"
    pthread)

add_library(gmock INTERFACE)
add_dependencies(gmock googletest)
target_include_directories(gmock INTERFACE
    "${source_dir}/googlemock/include")
target_link_libraries(gmock INTERFACE
    "${binary_dir}/googlemock/libgmock.a")
