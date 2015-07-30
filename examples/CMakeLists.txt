project(examples)

link_directories(${PROJECT_BUILD_DIR}/lib
                 ${CMAKE_SOURCE_DIR}/libbinlogevents/lib
                 ${CMAKE_SOURCE_DIR}/lib)

include_directories(${PROJECT_BUILD_DIR}/include
                    ${CMAKE_SOURCE_DIR}/libbinlogevents/include)

# Create build rules for all the simple examples that only require a
# single file.

foreach(prog basic-1 basic-2 binlog-browser)
  ADD_EXECUTABLE(${prog} ${prog}.cpp)
  TARGET_LINK_LIBRARIES(${prog} replication_static binlogevents_static
                        mysqlclient)
endforeach()
