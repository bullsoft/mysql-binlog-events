/*
Copyright (c) 2015, 2016 Oracle and/or its affiliates. All rights
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

#ifndef GLOBAL_VARS_INCLUDED
#define GLOBAL_VARS_INCLUDED

#include "binlog.h"
#include <map>

extern const char* uri_arg;

class ComparisonValues
{
public:
  typedef std::map<enum_field_types, std::string> Map;
  typedef std::map<std::string, std::string> VarMap;
  Map expected_field_val;
  VarMap expected_server_var;

  void assign_field_value();
};

void ComparisonValues::assign_field_value()
{
  expected_field_val[MYSQL_TYPE_TINY]= "127";
  expected_field_val[MYSQL_TYPE_SHORT]= "32767";
  expected_field_val[MYSQL_TYPE_LONG]= "2147483647";
  expected_field_val[MYSQL_TYPE_FLOAT]= "4.50933e+27";
  expected_field_val[MYSQL_TYPE_DOUBLE]= "4.17201e-309";
  expected_field_val[MYSQL_TYPE_LONGLONG]= "9223372036854775807";
  expected_field_val[MYSQL_TYPE_DATETIME]= "9999-12-31 23:59:59";
  expected_field_val[MYSQL_TYPE_VARCHAR]= "BAPI";
  expected_field_val[MYSQL_TYPE_STRING]= "mysql_type_string";
  expected_field_val[MYSQL_TYPE_TINY_BLOB]= "mysql_type_blob";
  expected_field_val[MYSQL_TYPE_MEDIUM_BLOB]= "mysql_type_blob";
  expected_field_val[MYSQL_TYPE_LONG_BLOB]= "mysql_type_blob";
  expected_field_val[MYSQL_TYPE_BLOB]= "mysql_type_blob";
  expected_field_val[MYSQL_TYPE_YEAR]= "2012";
  expected_field_val[MYSQL_TYPE_ENUM]= "2";
  expected_field_val[MYSQL_TYPE_SET]= "b'00000100'";
  expected_field_val[MYSQL_TYPE_VAR_STRING]= "mysql_type_string";
  expected_field_val[MYSQL_TYPE_INT24]= "8388607";
  expected_field_val[MYSQL_TYPE_DATE]= "9999-12-31";
  expected_field_val[MYSQL_TYPE_TIME]= "838:59:59";
  expected_field_val[MYSQL_TYPE_BIT]= "b'11111111'";
  // Not yet implemented
  expected_field_val[MYSQL_TYPE_TIMESTAMP]= "2038-01-19 03:14:07";
  expected_field_val[MYSQL_TYPE_DECIMAL]= "818.98";
  expected_field_val[MYSQL_TYPE_NEWDECIMAL]= "818.98";
}
#endif

