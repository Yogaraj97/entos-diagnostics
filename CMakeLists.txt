cmake_minimum_required(VERSION 3.16)
project(entos_diagnostics VERSION 0.1.0 LANGUAGES CXX)
include(CTest)
include(GNUInstallDirs)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
find_library(JSONCPP_LIBRARY NAMES jsoncpp)
find_path(JSONCPP_INCLUDE_DIR NAMES json/json.h PATH_SUFFIXES jsoncpp)
if(NOT JSONCPP_LIBRARY OR NOT JSONCPP_INCLUDE_DIR)
    message(FATAL_ERROR "JsonCpp library and headers are required")
endif()
find_package(CURL REQUIRED)
option(ENTOS_ENABLE_YAML "Enable structured declared YAML configuration collection" OFF)
add_library(diagnostics_core 
    src/platform/ProcessSnapshot.cpp 
    src/core/ProviderRegistry.cpp 
    src/core/Snapshot.cpp 
    src/core/CollectionEngine.cpp 
    src/thunder/RpcClient.cpp 
    src/thunder/CurlTransport.cpp 
    src/thunder/PluginCollector.cpp 
    src/platform/PlatformCollector.cpp 
    src/application/AppCollector.cpp
    src/application/AppManagerDumpsys.cpp
    src/application/AppManagerFormatter.cpp
    src/application/DumpsysFormatter.cpp
    src/application/UnifiedDiagnostics.cpp)
target_include_directories(diagnostics_core PUBLIC include ${JSONCPP_INCLUDE_DIR})
target_link_libraries(diagnostics_core PUBLIC ${JSONCPP_LIBRARY} CURL::libcurl)
if(ENTOS_ENABLE_YAML)
    find_package(yaml-cpp CONFIG REQUIRED)
    if(TARGET yaml-cpp::yaml-cpp)
        target_link_libraries(diagnostics_core PUBLIC yaml-cpp::yaml-cpp)
    else()
        target_link_libraries(diagnostics_core PUBLIC yaml-cpp)
    endif()
    target_compile_definitions(diagnostics_core PUBLIC ENTOS_ENABLE_YAML=1)
endif()
target_compile_options(diagnostics_core PRIVATE -Wall -Wextra -Wpedantic)
add_executable(entos-diagnostics src/cli/main.cpp)
target_link_libraries(entos-diagnostics PRIVATE diagnostics_core)
target_compile_definitions(entos-diagnostics PRIVATE ENTOS_INSTALLED_PROFILE="${CMAKE_INSTALL_FULL_DATADIR}/entos-diagnostics/providers.json")
configure_file(config/providers.json providers.json COPYONLY)
install(TARGETS entos-diagnostics RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES config/providers.json config/providers.schema.json config/coverage.schema.json DESTINATION ${CMAKE_INSTALL_DATADIR}/entos-diagnostics)
if(BUILD_TESTING)
    find_package(Threads REQUIRED)
    add_executable(diagnostics_tests tests/DiagnosticsTests.cpp)
    target_link_libraries(diagnostics_tests PRIVATE diagnostics_core)
    target_compile_definitions(diagnostics_tests PRIVATE ENTOS_TEST_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/config/providers.json")
    add_test(NAME diagnostics_core COMMAND diagnostics_tests)
    add_test(NAME diagnostics_help COMMAND entos-diagnostics --help)
    add_test(NAME diagnostics_list COMMAND entos-diagnostics list --format json)
    add_executable(diagnostics_transport_tests tests/TransportTests.cpp)
    target_link_libraries(diagnostics_transport_tests PRIVATE diagnostics_core Threads::Threads)
    add_test(NAME diagnostics_transport COMMAND diagnostics_transport_tests)
    add_test(NAME diagnostics_cli COMMAND ${CMAKE_COMMAND} -DUTILITY=$<TARGET_FILE:entos-diagnostics> -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/CliTests.cmake)
endif()