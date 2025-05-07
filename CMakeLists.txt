cmake_minimum_required(VERSION 3.10)

project(ProjectName VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(src)

find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

file(GLOB BOT_SRC_FILES src/bot/*.cpp)
file(GLOB NETWORK_SRC_FILES src/network/*.cpp)
file(GLOB PLATFORM_SRC_FILES src/platform/**/*.cpp)
file(GLOB UTILS_SRC_FILES src/utils/*.cpp)
file(GLOB MESSAGING_SRC_FILES src/messaging/*.cpp)
file(GLOB DATABASE_SRC_FILES src/database/*.cpp)
file(GLOB SECURITY_SRC_FILES src/security/*.cpp)
file(GLOB MONITORING_SRC_FILES src/monitoring/*.cpp)
file(GLOB REPORTING_SRC_FILES src/reporting/*.cpp)
file(GLOB API_SRC_FILES src/api/*.cpp)
file(GLOB CLI_SRC_FILES src/cli/*.cpp)
file(GLOB SELFDEFENSE_SRC_FILES src/selfdefense/*.cpp)
file(GLOB STEALTH_SRC_FILES src/stealth/*.cpp)
file(GLOB URLSHORTENER_SRC_FILES src/urlshortener/*.cpp)
file(GLOB USERDATA_SRC_FILES src/userdata/*.cpp)
file(GLOB CAPTCHA_SRC_FILES src/captcha/*.cpp)
file(GLOB CREATOR_SRC_FILES src/creator/*.cpp)
file(GLOB MALWARE_SRC_FILES src/hub/*.cpp)
file(GLOB SPIDER_SRC_FILES src/spider/*.cpp)

add_executable(malware_exe ${MALWARE_SRC_FILES})
add_executable(main src/main.cpp)

target_link_libraries(malware_exe PRIVATE
    ${BOT_SRC_FILES}
    ${NETWORK_SRC_FILES}
    ${PLATFORM_SRC_FILES}
    ${UTILS_SRC_FILES}
    ${MESSAGING_SRC_FILES}
    ${DATABASE_SRC_FILES}
    ${SECURITY_SRC_FILES}
    ${MONITORING_SRC_FILES}
    ${REPORTING_SRC_FILES}
    ${API_SRC_FILES}
    ${CLI_SRC_FILES}
    ${SELFDEFENSE_SRC_FILES}
    ${STEALTH_SRC_FILES}
    ${URLSHORTENER_SRC_FILES}
    ${USERDATA_SRC_FILES}
    ${CAPTCHA_SRC_FILES}
    ${CREATOR_SRC_FILES}
    ${SPIDER_SRC_FILES}
    OpenSSL::SSL
    CURL::libcurl
)

target_link_libraries(main PRIVATE
    ${BOT_SRC_FILES}
    ${NETWORK_SRC_FILES}
    ${PLATFORM_SRC_FILES}
    ${UTILS_SRC_FILES}
    ${MESSAGING_SRC_FILES}
    ${DATABASE_SRC_FILES}
    ${SECURITY_SRC_FILES}
    ${MONITORING_SRC_FILES}
    ${REPORTING_SRC_FILES}
    ${API_SRC_FILES}
    ${CLI_SRC_FILES}
    ${SELFDEFENSE_SRC_FILES}
    ${STEALTH_SRC_FILES}
    ${URLSHORTENER_SRC_FILES}
    ${USERDATA_SRC_FILES}
    ${CAPTCHA_SRC_FILES}
    ${CREATOR_SRC_FILES}
    ${SPIDER_SRC_FILES}
    OpenSSL::SSL
    CURL::libcurl
)

if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

enable_testing()
add_subdirectory(tests)

install(TARGETS malware_exe main DESTINATION bin)
install(DIRECTORY src/ DESTINATION include)

