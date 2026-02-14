cmake_minimum_required(VERSION 3.18)
project(neutron_rl_addon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Torch REQUIRED)

include_directories(${CMAKE_JS_INC})

execute_process(
    COMMAND node -p "require('node-addon-api').include"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/../..
    OUTPUT_VARIABLE NODE_ADDON_API_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})

add_library(${PROJECT_NAME} SHARED
    src/agent.cpp
    src/game_state.cpp
    src/mcts.cpp
    src/model_loader.cpp
    RlAddon.cpp
    RlAsyncWorker.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${NODE_ADDON_API_DIR}
    ${CMAKE_JS_INC}
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_JS_LIB} ${TORCH_LIBRARIES})

target_compile_definitions(${PROJECT_NAME} PRIVATE NAPI_VERSION=6 NAPI_CPP_EXCEPTIONS)

if (TORCH_CXX_FLAGS)
    separate_arguments(TORCH_CXX_FLAGS_LIST NATIVE_COMMAND ${TORCH_CXX_FLAGS})
    target_compile_options(${PROJECT_NAME} PRIVATE ${TORCH_CXX_FLAGS_LIST})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    PREFIX ""
    SUFFIX ".node"
    OUTPUT_NAME "neutron_rl_addon"
)
