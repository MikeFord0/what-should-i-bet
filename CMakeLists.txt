cmake_minimum_required(VERSION 3.16)
project(whatshouldibet LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# libcurl: makes the HTTPS request to the Claude API.
find_package(CURL REQUIRED)

# nlohmann/json: reads and writes JSON. CMake downloads it automatically.
include(FetchContent)
FetchContent_Declare(json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(json)

add_executable(daily_pick
  src/main.cpp
  src/claude_client.cpp
  src/prompt.cpp
  src/pick.cpp
  src/record.cpp
  src/history.cpp)

target_link_libraries(daily_pick PRIVATE CURL::libcurl nlohmann_json::nlohmann_json)
