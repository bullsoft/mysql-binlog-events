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

#include "data_type_checks.h"

void TestDataTypeUtility::
test_mysql_type_decimal(Row_of_fields::iterator& field_it,
                        std::string& expected)
{
  // Not yet implemented
}

void TestDataTypeUtility::
test_mysql_type_newdecimal(Row_of_fields::iterator& field_it,
                           std::string& expected)
{
  // Not yet implemented
}

void TestDataTypeUtility::
test_mysql_type_tiny(Row_of_fields::iterator& field_it,
                     int expected)
{
  int val= static_cast<int>((*field_it).as_int8());
  EXPECT_EQ(expected, val);
}

void TestDataTypeUtility::
test_mysql_type_short(Row_of_fields::iterator& field_it,
                      int expected)
{
  int val= static_cast<int>((*field_it).as_int16());
  EXPECT_EQ(expected, val);
}

void TestDataTypeUtility::
test_mysql_type_long(Row_of_fields::iterator& field_it, long expected)
{
  long val= static_cast<long>((*field_it).as_int32());
  EXPECT_EQ(expected, val);
}

void TestDataTypeUtility::
test_mysql_type_float(Row_of_fields::iterator& field_it,
                      float expected)
{
  float val= (*field_it).as_float();
  EXPECT_FLOAT_EQ(expected, val);
}

void TestDataTypeUtility::
test_mysql_type_double(Row_of_fields::iterator& field_it,
                       double expected)
{
  double val= (*field_it).as_double();
  EXPECT_DOUBLE_EQ(expected, val);
}

void TestDataTypeUtility::
test_mysql_type_longlong(Row_of_fields::iterator& field_it,
                         long long expected)
{
  long long val= static_cast<long long>((*field_it).as_int64());
  EXPECT_EQ(expected, val);
}

void TestDataTypeUtility::
test_mysql_type_medium_int(Row_of_fields::iterator& field_it,
                           int expected)
{
  int val= static_cast<int>((*field_it).as_int24());
  EXPECT_EQ(expected, val);
}

void TestDataTypeUtility::
test_mysql_type_date(Row_of_fields::iterator& field_it,
                     Date& expected)
{
  Date val= (*field_it).as_date();
  EXPECT_EQ(val.day, expected.day);
  EXPECT_EQ(val.month, expected.month);
  EXPECT_EQ(val.year, expected.year);
}

void TestDataTypeUtility::
test_mysql_type_newdate(Row_of_fields::iterator& field_it,
                        std::string& expected)
{
  // Not yet implemented
}

void TestDataTypeUtility::
test_mysql_type_time(Row_of_fields::iterator& field_it,
                     Time& expected)
{
  Time val= (*field_it).as_time();
  EXPECT_EQ(val.hour, expected.hour);
  EXPECT_EQ(val.min, expected.min);
  EXPECT_EQ(val.sec, expected.sec);
}

void TestDataTypeUtility::
test_mysql_type_datetime(Row_of_fields::iterator& field_it,
                         Date_time& expected)
{
  Date_time val= (*field_it).as_date_time();
  EXPECT_EQ(val.day, expected.day);
  EXPECT_EQ(val.month, expected.month);
  EXPECT_EQ(val.year, expected.year);
  EXPECT_EQ(val.hour, expected.hour);
  EXPECT_EQ(val.min, expected.min);
  EXPECT_EQ(val.sec, expected.sec);
}

void TestDataTypeUtility::
test_mysql_type_year(Row_of_fields::iterator& field_it,
                     long expected)
{
  long val= (*field_it).as_year();
  EXPECT_EQ(expected, val);
}

void TestDataTypeUtility::
test_mysql_type_varchar(Row_of_fields::iterator& field_it,
                        std::string& expected)
{
  std::string val;
  converter.to(val, *field_it);
  EXPECT_STREQ(expected.c_str(), val.c_str());
}

void TestDataTypeUtility::
test_mysql_type_bit(Row_of_fields::iterator& field_it,
                    std::string& expected)
{
  std::string val;
  converter.to(val, *field_it);
  EXPECT_STREQ(expected.c_str(), val.c_str());
}

void TestDataTypeUtility::
test_mysql_type_enum(Row_of_fields::iterator& field_it,
                     std::string& expected)
{
  std::string val;
  converter.to(val, *field_it);
  EXPECT_STREQ(expected.c_str(), val.c_str());
}

void TestDataTypeUtility::
test_mysql_type_set(Row_of_fields::iterator& field_it,
                    std::string& expected)
{
  std::string val;
  converter.to(val, *field_it);
  EXPECT_STREQ(expected.c_str(), val.c_str());
}

void TestDataTypeUtility::
test_mysql_type_tiny_blob(Row_of_fields::iterator& field_it,
                          std::string& expected)
{
  std::string val;
  converter.to(val, *field_it);
  EXPECT_STREQ(expected.c_str(), val.c_str());
}

void TestDataTypeUtility::
test_mysql_type_medium_blob(Row_of_fields::iterator& field_it,
                            std::string& expected)
{
  std::string val;
  converter.to(val, *field_it);
  EXPECT_STREQ(expected.c_str(), val.c_str());
}

void TestDataTypeUtility::
test_mysql_type_long_blob(Row_of_fields::iterator& field_it,
                          std::string& expected)
{
  std::string val;
  converter.to(val, *field_it);
  EXPECT_STREQ(expected.c_str(), val.c_str());
}

void TestDataTypeUtility::
test_mysql_type_blob(Row_of_fields::iterator& field_it,
                     std::string& expected)
{
  std::string val;
  converter.to(val, *field_it);
  EXPECT_STREQ(expected.c_str(), val.c_str());
}

void TestDataTypeUtility::
test_mysql_type_var_string(Row_of_fields::iterator& field_it,
                           std::string& expected)
{
  std::string val;
  converter.to(val, *field_it);
  EXPECT_STREQ(expected.c_str(), val.c_str());
}

void TestDataTypeUtility::
test_mysql_type_string(Row_of_fields::iterator& field_it,
                       std::string& expected)
{
  std::string val;
  converter.to(val, *field_it);
  EXPECT_STREQ(expected.c_str(), val.c_str());
}

void TestDataTypeUtility::
test_mysql_type_json(Row_of_fields::iterator& field_it,
                     std::string& expected)
{
  // Not yet implemented
}
