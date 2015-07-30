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

#include "global_vars.h"
#include "binlog.h"
#include "utility_methods.h"
#include <gtest/gtest.h>
#include <iostream>
#include <stdlib.h>

const char* uri_arg;

class MyContentHandler : public Content_handler
{
public:
  /*
    These methods override the process_event method
    defined in the class  Content_handler.
  */
  binary_log::Binary_log_event *process_event(binary_log::Rows_event *ev);
  binary_log::Binary_log_event *process_event(binary_log::Table_map_event *event);
  binary_log::Binary_log_event *process_event(binary_log::Query_event *event)
  {
    m_event.test_event(&event);
    return event;
  }

  binary_log::Binary_log_event *process_event(binary_log::Intvar_event *event)
  {
    m_event.test_event(&event);
    return event;
  }

  binary_log::Binary_log_event *process_event(binary_log::User_var_event *event)
  {
    m_event.test_event(&event);
    return event;
  }

  binary_log::Binary_log_event *process_event(binary_log::Xid_event *event)
  {
    m_event.test_event(&event);
    return event;
  }

  binary_log::Binary_log_event *process_event(binary_log::Format_description_event *event)
  {
    m_event.test_event(&event);
    return event;
  }

  binary_log::Binary_log_event *process_event(binary_log::Incident_event *event)
  {
    m_event.test_event(&event);
    return event;
  }

  binary_log::Binary_log_event *process_event(binary_log::Rotate_event *event)
  {
    m_event.test_event(&event);
    return event;
  }

  ~MyContentHandler ()
  {
    Int2event_map::iterator it= m_table_index.begin();
    do
    {
     if (it->second != NULL)
       delete it->second;
    }while (++it != m_table_index.end());
  }

  std::map<enum_field_types, std::string> fields_val;
  std::map<enum_field_types, std::string>::iterator field_it;

private:
  int m_table_id;
  typedef std::map<long int, binary_log::Table_map_event *> Int2event_map;
  Int2event_map m_table_index;
  TestEventUtility m_event;
};


binary_log::Binary_log_event* MyContentHandler::process_event(binary_log::Table_map_event *tm)
{
    m_event.test_event(&tm);

    m_table_index.insert(Int2event_map::value_type(tm->get_table_id(), tm));
    /* Consume this event so that it won't be deallocated */
    return 0;
}

binary_log::Binary_log_event* MyContentHandler::process_event(binary_log::Rows_event *event)
{
  m_event.test_event(&event);
  Int2event_map::iterator ti_it= m_table_index.find(event->get_table_id());
  EXPECT_TRUE (ti_it != m_table_index.end());
  if (ti_it != m_table_index.end())
  {
    binary_log::Row_event_set rows(event, ti_it->second);
    binary_log::Row_event_set::iterator row_it= rows.begin();
    do
    {
      binary_log::Row_of_fields fields= *row_it;

      binary_log::Row_of_fields::iterator field_it= fields.begin();
      binary_log::Converter converter;
      if (field_it != fields.end())
        do
        {
          std::string str;
          converter.to(str, *field_it);
          /*
            Storing results in a map, which contains pairs of
            field type and value
          */
          fields_val[(*field_it).type()]= str;
        } while (++field_it != fields.end());
    } while (++row_it != rows.end());
  } //end if
  return event;
}



class TestContentHandlers : public ::testing::Test
{
protected:
  TestContentHandlers()
  {
  }

  virtual ~TestContentHandlers()
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
    TestEventUtility m_event;

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

  binary_log::Binary_log* binlog;
  binary_log::system::Binary_log_driver *driver;
  binary_log::Decoder decode;
};

TEST_F(TestContentHandlers, InterfaceTest)
{
  MyContentHandler hndlr;
  Content_stream_handler handler;
  handler.add_listener(hndlr);
  binary_log::Binary_log_event *event;
  ComparisonValues expect;
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
    if (event->header()->type_code == binary_log::TABLE_MAP_EVENT)
    {
      handler.handle_event(&event);
      continue;
    }
    handler.handle_event(&event);
    if (event->header()->type_code == binary_log::QUERY_EVENT)
    {
      binary_log::Query_event *qev=
        static_cast<binary_log::Query_event *>(event);
      if (strncmp(qev->query, "DROP DATABASE BAPI_TEST",
                  strlen("DROP DATABASE BAPI_TEST")) == 0)
        break;
    }
    delete event;
  }
  // Checking field iterators
  expect.assign_field_value();
  std::map<enum_field_types, std::string>::iterator act_pos=
                                           hndlr.fields_val.begin();
  for (; act_pos != hndlr.fields_val.end(); act_pos++)
  {
    std::map<enum_field_types, std::string>::iterator exp_pos;
    exp_pos= expect.expected_field_val.find((*act_pos).first);
    std::string value;
    if (exp_pos != expect.expected_field_val.end())
    {
      // Field type found
      EXPECT_TRUE((*exp_pos).second == (*act_pos).second ||
                  (*act_pos).second == "not implemented");
    }
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  std::cout << "Important: Make sure that the binlog file is included in "
               "the repository and a relative path is set in CMakeLists.txt"
            << std::endl;
  uri_arg= argv[1];
  if (uri_arg == NULL)
  {
    std::cerr << "Cannot connect, no URI found";
    exit(1);
  }
  return RUN_ALL_TESTS();
}
