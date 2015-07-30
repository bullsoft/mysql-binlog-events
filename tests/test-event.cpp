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

#include "binlog.h"
#include "utility_methods.h"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <map>
#define MAX_TIMESTAMP 2147444047
const char* uri_arg;
class TestEvent : public ::testing::Test
{
public:
  TestEvent()
  {
  }

  virtual ~TestEvent()
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

    if (!strncmp(uri_arg, "mysql:", 6))         //TCP connection
    {
      if (Binlog_tcp_driver *tcp_driver= dynamic_cast
                                         <Binlog_tcp_driver*>(driver))
        m_event.RunQueries(tcp_driver->user(), tcp_driver->password(),
                           tcp_driver->host(), tcp_driver->port());
      else
        ASSERT_TRUE(tcp_driver);
    }
  }

  virtual void TearDown()
  {
    binlog->disconnect();
    delete binlog;
  }

  void TestEventCommonHeader(binary_log::Binary_log_event *event)
  {
      EXPECT_TRUE(event->header()->data_written > 0);
      //Timestamp range: 1970-01-01 00:00:01' UTC to '2038-01-19 03:14:07' UTC
      // ROTATE Events don't have a timestamp
      EXPECT_TRUE((event->header()->when.tv_sec >= 0) &&
                  (event->header()->when.tv_sec <= MAX_TIMESTAMP));
      // Type code must be in the range of enum Log_event_type
      EXPECT_FALSE(event->header()->type_code >=  ENUM_END_EVENT);
      // flags are defined in binlog_event.h, to be replaced by def in log_event
      EXPECT_TRUE(event->header()->flags == 0 ||
                  event->header()->flags == LOG_EVENT_TIME_F ||
                  event->header()->flags == LOG_EVENT_FORCED_ROTATE_F ||
                  event->header()->flags == LOG_EVENT_BINLOG_IN_USE_F ||
                  event->header()->flags == LOG_EVENT_THREAD_SPECIFIC_F ||
                  event->header()->flags == LOG_EVENT_SUPPRESS_USE_F ||
                  event->header()->flags == LOG_EVENT_ARTIFICIAL_F ||
                  event->header()->flags == LOG_EVENT_RELAY_LOG_F ||
                  event->header()->flags == LOG_EVENT_IGNORABLE_F);
  }

  void TestEventData(binary_log::Binary_log_event *event)
  {
    switch (event->get_event_type())
    {
    case FORMAT_DESCRIPTION_EVENT:
      {
        if (Format_description_event *format_event= dynamic_cast<Format_description_event*>(event))
          m_event.test_event(&format_event);
        else
          ASSERT_TRUE(format_event);
        break;
      }
    case QUERY_EVENT:
      {
        if (Query_event *query_event= dynamic_cast<Query_event*>(event))
        {
          m_event.test_event(&query_event);
          m_event.test_server_variables(&query_event);
        }
        else
          ASSERT_TRUE(query_event);
        break;
      }
    case INCIDENT_EVENT:
      {
        if (Incident_event *incident_event= dynamic_cast<Incident_event*>(event))
          m_event.test_event(&incident_event);
        else
          ASSERT_TRUE(incident_event);
        break;
      }
    case INTVAR_EVENT:
      {
        if (Intvar_event *int_var_event= dynamic_cast<Intvar_event*>(event))
          m_event.test_event(&int_var_event);
        else
          ASSERT_TRUE(int_var_event);
        break;
      }
    case USER_VAR_EVENT:
      {
        if (User_var_event *user_var_event= dynamic_cast<User_var_event*>(event))
          m_event.test_event(&user_var_event);
        else
          ASSERT_TRUE(user_var_event);
        break;
      }
    case TABLE_MAP_EVENT:
      {
        if (Table_map_event *table_map_event= dynamic_cast<Table_map_event*>(event))
          m_event.test_event(&table_map_event);
        else
          ASSERT_TRUE(table_map_event);
        break;
      }
    case WRITE_ROWS_EVENT:
    case WRITE_ROWS_EVENT_V1:
    case UPDATE_ROWS_EVENT:
    case UPDATE_ROWS_EVENT_V1:
    case DELETE_ROWS_EVENT:
    case DELETE_ROWS_EVENT_V1:
      {
        if (Rows_event *row_event= dynamic_cast<Rows_event*>(event))
          m_event.test_event(&row_event);
        else
          ASSERT_TRUE(row_event);
        break;
      }
    case XID_EVENT:
      {
        if (Xid_event *xid_event= dynamic_cast<Xid_event*>(event))
          m_event.test_event(&xid_event);
        else
          ASSERT_TRUE(xid_event);
        break;
      }
    case ROTATE_EVENT:
      {
        if (Rotate_event *rotate_event= dynamic_cast<Rotate_event*>(event))
          m_event.test_event(&rotate_event);
        else
          ASSERT_TRUE(rotate_event);
        break;
      }
    default:
      {
         //std::cerr << "An unhandled event occured of type "
           //        << binary_log::system::get_event_type_str
             //                  ((event)->get_event_type());
        break;
      }
    }
  }
  binary_log::Binary_log* binlog;
  binary_log::system::Binary_log_driver *driver;
  binary_log::Decoder decode;
private:
  TestEventUtility m_event;
};

TEST_F(TestEvent, TestEventHeader)
{
  binary_log::Binary_log_event *event;
  bool quit= false;
  while (true)
  {
    int result;
    std::pair<unsigned char *, size_t> buffer_buflen;
    result= driver->get_next_event(&buffer_buflen);
      if (result == ERR_OK)
      {
        if (!(event= decode.decode_event((char*)buffer_buflen.first, buffer_buflen.second, NULL, 1)))
        {
          break;
        }
      }
    else
      break;
    TestEvent::TestEventCommonHeader(event);
    if (event->header()->type_code == binary_log::QUERY_EVENT)
    {
      const binary_log::Query_event *qev= static_cast
                                     <const binary_log::Query_event *>(event);

      if (strncmp(qev->query, "DROP DATABASE BAPI_TEST", strlen("DROP DATABASE BAPI_TEST")))
        break;
    }
    delete event;
  }
}

TEST_F(TestEvent, TestEventData)
{
  binary_log::Binary_log_event *event;
  binary_log::Decoder decode(0);
  bool quit= false;
  while (!quit)
  {
    int result;
    std::pair<unsigned char *, size_t> buffer_buflen;
    result= driver->get_next_event(&buffer_buflen);
      if (result == ERR_OK)
      {
        if (!(event= decode.decode_event((char*)buffer_buflen.first, buffer_buflen.second, NULL, 1)))
        {
          break;
        }
      }
    else
      break;
    TestEvent::TestEventData(event);
    if (event->header()->type_code == binary_log::QUERY_EVENT)
    {
      const binary_log::Query_event *qev=
      static_cast<const binary_log::Query_event *>(event);

      if (strncmp(qev->query, "DROP DATABASE BAPI_TEST", strlen("DROP DATABASE BAPI_TEST")))
        quit= true;
    }
    delete event;
  }
}

int main(int argc, char **argv) {
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
