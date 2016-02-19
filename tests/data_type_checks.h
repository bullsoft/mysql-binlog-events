/*
   Copyright (c) 2015, Oracle and/or its affiliates. All rights
   reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
   02110-1301  USA
   */

#ifndef DATA_TYPE_CHECK_INCLUDED
#define DATA_TYPE_CHECK_INCLUDED

#include "binlog.h"
#include <gtest/gtest.h>
#include <iostream>
#include <cstdlib>
#include <cstring>

using binary_log::Row_of_fields;
using binary_log::Converter;
using binary_log::Date;
using binary_log::Time;
using binary_log::Date_time;

class TestDataTypeUtility
{
private:
  Converter converter;

public:
  // Numeric datatypes
  void test_mysql_type_decimal(Row_of_fields::iterator& field_it,
                               std::string& expected);
  void test_mysql_type_newdecimal(Row_of_fields::iterator& field_it,
                                  std::string& expected);
  void test_mysql_type_tiny(Row_of_fields::iterator& field_it,
                            int expected);
  void test_mysql_type_short(Row_of_fields::iterator& field_it,
                             int expected);
  void test_mysql_type_long(Row_of_fields::iterator& field_it,
                            long expected);
  void test_mysql_type_float(Row_of_fields::iterator& field_it,
                             float expected);
  void test_mysql_type_double(Row_of_fields::iterator& field_it,
                              double expected);
  void test_mysql_type_longlong(Row_of_fields::iterator& field_it,
                                long long expected);
  void test_mysql_type_medium_int(Row_of_fields::iterator& field_it,
                                  int expected);

  // Date and Time datatypes
  void test_mysql_type_date(Row_of_fields::iterator& field_it,
                            Date& expected);
  void test_mysql_type_newdate(Row_of_fields::iterator& field_it,
                               std::string& expected);
  void test_mysql_type_time(Row_of_fields::iterator& field_it,
                            Time& expected);
  void test_mysql_type_datetime(Row_of_fields::iterator& field_it,
                                Date_time& expected);
  void test_mysql_type_year(Row_of_fields::iterator& field_it,
                            long expected);

  // String datatypes
  void test_mysql_type_varchar(Row_of_fields::iterator& field_it,
                               std::string& expected);
  void test_mysql_type_bit(Row_of_fields::iterator& field_it,
                           std::string& expected);
  void test_mysql_type_enum(Row_of_fields::iterator& field_it,
                            std::string& expected);
  void test_mysql_type_set(Row_of_fields::iterator& field_it,
                           std::string& expected);
  void test_mysql_type_tiny_blob(Row_of_fields::iterator& field_it,
                                 std::string& expected);
  void test_mysql_type_medium_blob(Row_of_fields::iterator& field_it,
                                   std::string& expected);
  void test_mysql_type_long_blob(Row_of_fields::iterator& field_it,
                                 std::string& expected);
  void test_mysql_type_blob(Row_of_fields::iterator& field_it,
                            std::string& expected);
  void test_mysql_type_var_string(Row_of_fields::iterator& field_it,
                                  std::string& expected);
  void test_mysql_type_string(Row_of_fields::iterator& field_it,
                              std::string& expected);
  void test_mysql_type_json(Row_of_fields::iterator& field_it,
                            std::string& expected);

};

#endif
