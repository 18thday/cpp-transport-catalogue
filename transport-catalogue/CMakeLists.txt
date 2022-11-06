cmake_minimum_required(VERSION 3.10)

project(PhoneBook CXX)
set(CMAKE_CXX_STANDARD 17)

if (CMAKE_SYSTEM_NAME MATCHES "^MINGW")
    set(SYSTEM_LIBS -lstdc++)
else()
    set(SYSTEM_LIBS)
endif()

set(CMAKE_BUILD_TYPE Debug)

#set(CMAKE_PREFIX_PATH "C:\\Users\\Fedot\\Desktop\\cpp\\workspace_yap\\14_4_protobuf\\package")

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto map_renderer.proto svg.proto transport_router.proto)

set(TC_FILES transport_catalogue.h transport_catalogue.cpp 
             transport_catalogue.proto map_renderer.proto svg.proto transport_router.proto
             request_handler.h request_handler.cpp
             domain.h domain.cpp geo.h geo.cpp 
             graph.h router.h transport_router.h transport_router.cpp
             json.h json.cpp json_builder.h json_builder.cpp json_reader.h json_reader.cpp
             ranges.h
             svg.h svg.cpp map_renderer.h map_renderer.cpp
             serialization.h serialization.cpp
             main.cpp
)

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${TC_FILES})
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads ${SYSTEM_LIBS})