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

set(MySQL_SERVER_TESTS test-basic test-content-handlers)
set(MySQL_SIMPLE_TESTS test-transport)
set(MySQL_DATA_TYPE_TESTS test-event)

foreach(test ${MySQL_SERVER_TESTS} ${MySQL_SIMPLE_TESTS} ${MySQL_DATA_TYPE_TESTS})
  message("Adding test ${test}")
  if(${MySQL_DATA_TYPE_TESTS})
    add_executable(${test} ${test}.cpp data_type_checks.cpp test-data-types.cpp)
  else()
    add_executable(${test} ${test}.cpp)
  endif(${MySQL_DATA_TYPE_TESTS})
  target_link_libraries(${test} replication_static ${GTEST_LIBRARIES}
                        ${MYSQL_LIBRARIES})
endforeach()

if(WITH_SERVER_TESTS)
  add_test(ServerTests ${MySQL_SERVER_TESTS})
endif(WITH_SERVER_TESTS)
add_test(BasicTests ${MySQL_SIMPLE_TESTS})

