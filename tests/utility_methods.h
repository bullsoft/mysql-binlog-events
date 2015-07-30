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

#ifndef UTILITY_METHODS_INCLUDED
#define	UTILITY_METHODS_INCLUDED

#include "binlog.h"
#include "global_vars.h"
//#include "utilities.h"
#include <gtest/gtest.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <vector>

#define UTF8 33
#define LOG_HEADER_LEN 19
#define BINLOG_VERSION 4

class TestEventUtility
{
private:
  std::set<long int> m_table_id;

public:
  ComparisonValues obj;
  std::vector<uint8_t> post_header_len;
  int log_header_len;
  void test_event(Format_description_event **event);
  void test_event(Query_event **event);
  void test_event(Incident_event **event);
  void test_event(Intvar_event **event);
  void test_event(User_var_event **event);
  void test_event(Table_map_event **event);
  void test_event(Rows_event **event);
  void test_event(Xid_event **event);
  void test_event(Rotate_event **event);
  void test_server_variables(Query_event **event);
  void RunQueries(const std::string&, const std::string&, const std::string&, uint);

};

void TestEventUtility::test_event(Format_description_event **event)
{
  EXPECT_EQ((*event)->server_version[0], '5');
  // Since BAPI support MySQL 5.x.x
  EXPECT_EQ((*event)->created, (*event)->header()->when.tv_sec);
  EXPECT_EQ((*event)->binlog_version, BINLOG_VERSION);
  EXPECT_EQ((*event)->common_header_len, LOG_HEADER_LEN);
}

void TestEventUtility::test_event(Query_event **event)
{
  EXPECT_EQ(strncmp((*event)->db, "BAPI_TEST", 9), 0);
  EXPECT_GT((*event)->thread_id, 0);
  EXPECT_TRUE((*event)->query_exec_time >= 0);
  EXPECT_EQ((*event)->error_code, 0);
  EXPECT_NE((*event)->query, "");
}

void TestEventUtility::test_event(Incident_event **event)
{
  EXPECT_NE((*event)->get_message(), "");
  EXPECT_TRUE((*event)->get_incident_type() == 0 ||
              (*event)->get_incident_type() == 1 ||
              (*event)->get_incident_type() == 2);
  /*
    type 0 is an incident event, but it is invalid
    type 1 is the case when there are possibly lost events
    in the replication stream.
    type 2 is the last event of the enumeration
    (defined in the file rpl_constants.h), to be replaced
  */
}

void TestEventUtility::test_event(Intvar_event **event)
{
  EXPECT_TRUE((*event)->type == (*event)->INVALID_INT_EVENT ||
              (*event)->type == (*event)->LAST_INSERT_ID_EVENT ||
              (*event)->type == (*event)->INSERT_ID_EVENT);
  //value given by the query fired in the constructor
  EXPECT_TRUE((*event)->val == 1);
}

void TestEventUtility::test_event(User_var_event **event)
{
  EXPECT_STRNE((*event)->name, "");
  EXPECT_TRUE((*event)->type == (*event)->STRING_TYPE ||
              (*event)->type == (*event)->REAL_TYPE ||
              (*event)->type == (*event)->INT_TYPE ||
              (*event)->type == (*event)->ROW_TYPE ||
              (*event)->type == (*event)->DECIMAL_TYPE ||
              (*event)->type == (*event)->VALUE_TYPE_COUNT);
  EXPECT_TRUE((*event)->is_null == 1 || (*event)->is_null == 0);
  EXPECT_EQ((*event)->charset_number, UTF8);
  //value is binary encoded dpending on the type, no assertions on them
}

void TestEventUtility::test_event(Table_map_event **event)
{
  m_table_id.insert((*event)->get_table_id());

  EXPECT_EQ((*event)->m_flags, 1);
  EXPECT_EQ((*event)->m_dbnam, "BAPI_TEST");
  EXPECT_FALSE(strncmp((*event)->m_tblnam.c_str(), "bapi_test", 9));
}

void TestEventUtility::test_event(Rows_event **event)
{
  EXPECT_TRUE((*event)->get_table_id() >= 0);
  /*
    enum for flags are defined in log_event.h
    0x0001      end of statement
    0x0002      no foreign key checks
    0x0004      no unique key checks
    0x0008      row has a columns
  */
  if ((*event)->get_flags())
    EXPECT_FALSE((*event)->get_flags() &
                ~((*event)->STMT_END_F |
                  (*event)->NO_FOREIGN_KEY_CHECKS_F |
                  (*event)->RELAXED_UNIQUE_CHECKS_F |
                  (*event)->COMPLETE_ROWS_F));
  if ((*event)->get_table_id() == 0x00ffffff)      // It is a dummy event
  {
    // Flag should be End of stmt
    EXPECT_TRUE((*event)->get_flags() == (*event)->STMT_END_F);
  }
  EXPECT_TRUE((*event)->get_width() > 0);
  EXPECT_EQ((*event)->get_null_bits_len(), 1);
  if ((*event)->header()->type_code == WRITE_ROWS_EVENT ||
      (*event)->header()->type_code == DELETE_ROWS_EVENT ||
      (*event)->header()->type_code == UPDATE_ROWS_EVENT)
  {
    // Variable length header for row events in 5.6
     // var_header_len is protected now
    //EXPECT_FALSE((*event)->var_header_len < 2);
    //TODO: Write checks for extra_header_data
  }
  EXPECT_FALSE(std::find(m_table_id.begin(), m_table_id.end(),
                         (*event)->get_table_id()) == m_table_id.end());
}

void TestEventUtility::test_event(Xid_event **event)
{
  EXPECT_GE((*event)->xid, 1);
}

void TestEventUtility::test_event(Rotate_event **event)
{
  EXPECT_STRNE((*event)->new_log_ident, "");
  EXPECT_NE((*event)->pos, 0);
}

void TestEventUtility::test_server_variables(Query_event **event)
{
  EXPECT_EQ(strncmp((*event)->catalog, "std", 3), 0);
  EXPECT_EQ((*event)->flags2, 0);
  EXPECT_EQ((*event)->sql_mode, 1075839008);
  EXPECT_EQ((*event)->auto_increment_increment, 1);
  EXPECT_EQ((*event)->auto_increment_offset, 1);
  EXPECT_EQ((*event)->lc_time_names_number, 0);
  EXPECT_EQ((*event)->charset_database_number, 0);
}

void TestEventUtility::RunQueries(const std::string& user,
                                  const std::string& pass,
                                  const std::string& host, uint port)
{
  if (!strncmp(uri_arg, "mysql:", 6))       //TCP connection
  {
    MYSQL *conn;
    ASSERT_TRUE(conn= mysql_init(NULL));
    //Get an alternate connection to the server in order to send queries
    ASSERT_TRUE(mysql_real_connect(conn, host.c_str(), user.c_str(),
                                   pass.c_str(), "", port, 0, 0));

    std::string query[]= {
                           "DROP DATABASE IF EXISTS BAPI_TEST;",
                           "CREATE DATABASE BAPI_TEST;",
                           "USE BAPI_TEST;",
                           "set @@session.binlog_format='STATEMENT';",
                           "CREATE TABLE autoinc1 (col INT NOT NULL"
                           " AUTO_INCREMENT PRIMARY KEY);",
                           "SET @@auto_increment_increment=10",
                           // 'latin1_swedish_ci' by default
                           "SET @@session.collation_database="
                           "'utf8_general_ci'",
                           "INSERT INTO autoinc1 VALUES (NULL);",
                           "set @@session.binlog_format='ROW';",
                           "CREATE TABLE bapi_test2(i mediumint,"
                           "j bigint,k float,l double);",
                           "INSERT INTO bapi_test2 values (8388607,"
                           "9223372036854775807, "
                           "4.50933e27, 4.17201e-309);",
                           "CREATE TABLE bapi_test3(i datetime,"
                           "j timestamp, k char, l date, m time,"
                           "n  bit(8));",
                           "INSERT INTO bapi_test3 values "
                           "('9999-12-31 23:59:59',"
                           "'2038-01-19 03:14:07', '9', "
                           "'9999-12-31','838:59:59',b'11111111');",
                           "CREATE TABLE bapi_test1"
                           "( i int, j tinyint, k smallint);",
                           "INSERT INTO bapi_test1 values "
                           " (2147483647,127,32767);",
                           "DROP DATABASE BAPI_TEST;"
                         };

    int query_count= sizeof(query)/sizeof(query[0]);
    for (int i= 0; i < query_count; i++)
    {
      EXPECT_FALSE(mysql_query(conn, query[i].c_str()));
    }
    mysql_close(conn);
  }
}
#endif

