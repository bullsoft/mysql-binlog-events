include_directories(${CMAKE_SOURCE_DIR}/libbinlogevents/include)
link_directories(${PROJECT_BINARY_DIR}/libbinlogevents/lib)

# This configuration file builds both the static and shared version of
# the library.
set(replication_sources
    access_method_factory.cpp
    binlog_driver.cpp
    binlog.cpp
    tcp_driver.cpp
    file_driver.cpp
    protocol.cpp
    decoder.cpp
    value.cpp
    row_of_fields.cpp
    field_iterator.cpp
    basic_transaction_parser.cpp
    basic_content_handler.cpp )

# Configure for building static library
add_library(replication_static STATIC ${replication_sources})
set_target_properties(replication_static PROPERTIES
                      VERSION 0.1 SOVERSION 1
                      OUTPUT_NAME "mysqlstream")
target_link_libraries(replication_static binlogevents_static ${MYSQL_LIBRARIES})

# Configure for building the shared library
add_library(replication_shared SHARED ${replication_sources})
set_target_properties(replication_shared PROPERTIES
                      VERSION 0.1 SOVERSION 1
                      OUTPUT_NAME "mysqlstream")
target_link_libraries(replication_shared binlogevents_shared ${MYSQL_LIBRARIES})

install(TARGETS replication_shared replication_static
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION lib)
