# Copyright (c) 2011, 2016, Oracle and/or its affiliates. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

include_directories(${CMAKE_SOURCE_DIR}/libbinlogevents/include)
link_directories(${PROJECT_BINARY_DIR}/libbinlogevents/lib)

# This configuration file builds both the static and shared version of
# the library.
set(replication_sources
    access_method_factory.cpp
    binlog.cpp
    tcp_driver.cpp
    file_driver.cpp
    decoder.cpp
    value.cpp
    decimal.cpp
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
