# Copyright (c) 2011, 2015, Oracle and/or its affiliates. All rights reserved.
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

project(examples)

link_directories(${PROJECT_BUILD_DIR}/lib
                 ${CMAKE_SOURCE_DIR}/libbinlogevents/lib
                 ${CMAKE_SOURCE_DIR}/lib)

include_directories(${PROJECT_BUILD_DIR}/include
                    /usr/local/include/mysql-binary-log-events/
                    /Users/guweigang/tmp/mysql-5.7.7-rc/include
                    ${CMAKE_SOURCE_DIR}/libbinlogevents/include)

# Create build rules for all the simple examples that only require a
# single file.

foreach(prog basic-1 basic-2 binlog-browser)
  ADD_EXECUTABLE(${prog} ${prog}.cpp)
  TARGET_LINK_LIBRARIES(${prog} replication_static binlogevents_static
                        mysqlclient)
endforeach()
