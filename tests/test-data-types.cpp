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

#include "global_vars.h"
#include "data_type_checks.h"
#include <gtest/gtest.h>

#define MAX_TIMESTAMP 2147444047
#define UTF8 33
#define LOG_HEADER_LEN 19
#define BINLOG_VERSION 4

using binary_log::Binary_log;
using binary_log::system::create_transport;
using binary_log::system::Binary_log_driver;
using binary_log::system::Binlog_file_driver;
using binary_log::Rows_event;
using binary_log::Row_event_set;
using binary_log::Row_of_fields;
using binary_log::Converter;
using binary_log::Date;
using binary_log::Time;
using binary_log::Date_time;
using namespace std;

const char* uri_arg;

class TestDataTypes : public ::testing::Test
{
public:
  TestDataTypes()
  {
  }

  virtual ~TestDataTypes()
  {
  }

  virtual void SetUp()
  {
    using binary_log::system::create_transport;
    using binary_log::system::Binary_log_driver;
    using binary_log::system::Binlog_tcp_driver;
    driver= create_transport(uri_arg);

    binlog= new binary_log::Binary_log(driver);
    ASSERT_EQ(binlog->connect(), 0);
    binlog->set_position(4);
  }

  virtual void TearDown()
  {
    binlog->disconnect();
    delete binlog;
  }
  void TestWriteRowsData(binary_log::Write_rows_event *write_rows_event,
                         binary_log::Table_map_event *table_map_event)
  {
    ComparisonValues expected_store;
    expected_store.assign_field_value();
    Row_event_set rows(write_rows_event, table_map_event);
    Row_event_set::iterator it= rows.begin();
    do {
      Row_of_fields fields= *it;
      Row_of_fields::iterator field_it= fields.begin();
      do {
        switch((*field_it).type())
        {
        case MYSQL_TYPE_TIMESTAMP:
        case MYSQL_TYPE_TIMESTAMP2:
        case MYSQL_TYPE_DATETIME2:
        case MYSQL_TYPE_TIME2:
        case MYSQL_TYPE_GEOMETRY:
        case MYSQL_TYPE_NEWDATE:
        case MYSQL_TYPE_DECIMAL:
        case MYSQL_TYPE_NEWDECIMAL:
          {
            //Not yet implemented
            break;
          }
        case MYSQL_TYPE_TINY:
          {
            string val= expected_store.expected_field_val[MYSQL_TYPE_TINY];
            int test= atoi(val.c_str());
            m_type.test_mysql_type_tiny(field_it, test);
            break;
          }
        case MYSQL_TYPE_SHORT:
          {
            string val= expected_store.expected_field_val[MYSQL_TYPE_SHORT];
            int test= atoi(val.c_str());
            m_type.test_mysql_type_short(field_it, test);
            break;
          }
        case MYSQL_TYPE_INT24:
          {
            string val= expected_store.expected_field_val[MYSQL_TYPE_INT24];
            int test= atoi(val.c_str());
            m_type.test_mysql_type_medium_int(field_it, test);
            break;
          }
        case MYSQL_TYPE_LONG:
          {
            string val= expected_store.expected_field_val[MYSQL_TYPE_LONG];
            long int test= atol(val.c_str());
            m_type.test_mysql_type_long(field_it, test);
            break;
          }
        case MYSQL_TYPE_LONGLONG:
          {
            string val= expected_store.expected_field_val[MYSQL_TYPE_LONGLONG];
            long long int test= atoll(val.c_str());
            m_type.test_mysql_type_longlong(field_it, test);
            break;
          }
        case MYSQL_TYPE_BIT:
          {
            string test= expected_store.expected_field_val[MYSQL_TYPE_BIT];
            m_type.test_mysql_type_bit(field_it, test);
            break;
          }
        case MYSQL_TYPE_FLOAT:
          {
            std::string val= expected_store.expected_field_val[MYSQL_TYPE_FLOAT];
            double test= atof(val.c_str());
            m_type.test_mysql_type_float(field_it, test);
            break;
          }
        case MYSQL_TYPE_DOUBLE:
          {
            string val= expected_store.expected_field_val[MYSQL_TYPE_DOUBLE];
            double test= atof(val.c_str());
            m_type.test_mysql_type_double(field_it, test);
            break;
          }
        case MYSQL_TYPE_VARCHAR:
          {
            string test= expected_store.expected_field_val[MYSQL_TYPE_VARCHAR];
            m_type.test_mysql_type_varchar(field_it, test);
            break;
          }
        case MYSQL_TYPE_STRING:
          {
            string test= expected_store.expected_field_val[MYSQL_TYPE_STRING];
            m_type.test_mysql_type_string(field_it, test);
            break;
          }
        case MYSQL_TYPE_TINY_BLOB:
          {
            string test= expected_store.expected_field_val[MYSQL_TYPE_TINY_BLOB];
            m_type.test_mysql_type_tiny_blob(field_it, test);
            break;
          }
        case MYSQL_TYPE_MEDIUM_BLOB:
          {
            string test= expected_store.expected_field_val[MYSQL_TYPE_MEDIUM_BLOB];
            m_type.test_mysql_type_medium_blob(field_it, test);
            break;
          }
        case MYSQL_TYPE_LONG_BLOB:
          {
            string test= expected_store.expected_field_val[MYSQL_TYPE_LONG_BLOB];
            m_type.test_mysql_type_long_blob(field_it, test);
            break;
          }
        case MYSQL_TYPE_BLOB:
          {
            string test= expected_store.expected_field_val[MYSQL_TYPE_BLOB];
            m_type.test_mysql_type_blob(field_it, test);
            break;
          }
        case MYSQL_TYPE_DATE:
          {
            string val= expected_store.expected_field_val[MYSQL_TYPE_DATE];
            char *cstr= new char[val.length() + 1];
            strcpy(cstr, val.c_str());
            int date[3];
            char *part= strtok(cstr, "-");
            int i= 0;
            while (part != NULL)
            {
              date[i++]= atoi(part);
              part= strtok(NULL, "-");
            }
            Date test(date[2], date[1], date[0]);
            m_type.test_mysql_type_date(field_it, test);
            delete [] cstr;
            break;
          }
        case MYSQL_TYPE_DATETIME:
          {
            string val= expected_store.expected_field_val[MYSQL_TYPE_DATETIME];
            char *cstr= new char[val.length() + 1];
            strcpy(cstr, val.c_str());
            int date_time[6];
            char *part= strtok(cstr, "- :");
            int i = 0;
            while (part != NULL)
            {
              date_time[i++]= atoi(part);
              part= strtok(NULL, "- :");
            }
            Date_time test(date_time[2], date_time[1], date_time[0],
                           date_time[3], date_time[4], date_time[5]);
            m_type.test_mysql_type_datetime(field_it, test);
            delete [] cstr;
            break;
          }
        case MYSQL_TYPE_TIME:
          {
            string val= expected_store.expected_field_val[MYSQL_TYPE_TIME];
            char *cstr= new char[val.length() + 1];
            strcpy(cstr, val.c_str());
            int time[3];
            char *part= strtok(cstr, ":");
            int i= 0;
            while (part != NULL)
            {
              time[i++]= atoi(part);
              part= strtok(NULL, ":");
            }
            Time test(time[0], time[1], time[2]);
            m_type.test_mysql_type_time(field_it, test);
            delete [] cstr;
            break;
          }
        case MYSQL_TYPE_YEAR:
          {
            string val= expected_store.expected_field_val[MYSQL_TYPE_YEAR];
            int test= atoi(val.c_str());
            m_type.test_mysql_type_year(field_it, test);
            break;
          }
        case MYSQL_TYPE_ENUM:
          {
            string test= expected_store.expected_field_val[MYSQL_TYPE_ENUM];
            m_type.test_mysql_type_blob(field_it, test);
            break;
          }
        case MYSQL_TYPE_SET:
          {
            string test= expected_store.expected_field_val[MYSQL_TYPE_SET];
            m_type.test_mysql_type_set(field_it, test);
            break;
          }
        case MYSQL_TYPE_VAR_STRING:
          {
            string test= expected_store.expected_field_val[MYSQL_TYPE_VAR_STRING];
            m_type.test_mysql_type_var_string(field_it, test);
            break;
          }
        default:
          {
            break;
          }
        }
      } while (++field_it != fields.end());
  } while (++it != rows.end());
  return;
  }

  binary_log::Binary_log* binlog;
  binary_log::system::Binary_log_driver *driver;
  binary_log::Decoder decode;
private:
  TestDataTypeUtility m_type;
};


TEST_F(TestDataTypes, TestEventData)
{
  binary_log::Binary_log_event *event, *table_map_event;
  binary_log::Decoder decode(0);
  bool quit= false;
  while (!quit)
  {
    int result;
    std::pair<unsigned char *, size_t> buffer_buflen;
    result= driver->get_next_event(&buffer_buflen);
    if (result == ERR_OK)
    {
      if (!(event= decode.decode_event((char*)buffer_buflen.first,
                                        buffer_buflen.second,
                                        NULL, 1)))
      {
        break;
      }
    }
    else
      break;

    if (event->header()->type_code == binary_log::TABLE_MAP_EVENT)
    {
      if (!(table_map_event= decode.decode_event((char*)buffer_buflen.first,
                                                  buffer_buflen.second,
                                                  NULL, 1)))
        break;
    }
    else
    {
      if (event->header()->type_code == binary_log::WRITE_ROWS_EVENT ||
          event->header()->type_code == binary_log::WRITE_ROWS_EVENT_V1)
      {
        if (Write_rows_event *write_row_event= dynamic_cast
                                              <Write_rows_event*>(event))
        {
          TestDataTypes::TestWriteRowsData(write_row_event, static_cast
                                           <Table_map_event*>(table_map_event));
          delete table_map_event;
          quit = true;
        }
      }
    }
    delete event;
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  std::cout << "Important: Make sure that the binlog file is included in "
               "the repository and a relative path is set, or the tcp uri is"
               "included in CMakeLists.txt"
            << std::endl;
  uri_arg= argv[1];
  if (uri_arg == NULL)
  {
    std::cerr << "Cannot connect, no URI found";
    exit(1);
  }
  return RUN_ALL_TESTS();
}

