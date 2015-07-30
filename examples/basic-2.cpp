/*
Copyright (c) 2013, Oracle and/or its affiliates. All rights
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

#include <iostream>
#include <map>
#include <string>

/*
  Here is a basic system using the event loop to fetch context events
  and store them in an associative array.
 */
using binary_log::Binary_log;
using binary_log::system::create_transport;
using binary_log::system::Binary_log_driver;
using binary_log::User_var_event;
using binary_log::system::Binlog_file_driver;

/**
 * Class to maintain variable values.
 */
template <class AssociativeContainer>
class Save_variables : public Content_handler {
public:
  Save_variables(AssociativeContainer& container)
  : m_var(container)
  {
  }

  Binary_log_event *process_event(User_var_event *event) {
    m_var[event->name] = event->val;
    return NULL;
  }

private:
  AssociativeContainer &m_var;
};


template <class AssociativeContainer>
class Replace_variables : public Content_handler {
public:
  Replace_variables(AssociativeContainer& variables)
    : m_var(variables)
  {
  }

  Binary_log_event *process_event(Query_event *event) {
    std::string query = event->query;
    size_t start, end = 0;
    while (true) {
      start = query.find_first_of("@", end);
      if (start == std::string::npos)
        break;
      end = query.find_first_not_of("abcdefghijklmnopqrstuvwxyz", start+1);
      std::string key = query.substr(start + 1, end - start - 1);
      query.replace(start, end - start, "'" + m_var[key] + "'");
    }
    event->query= query.c_str();
    return event;
  }
private:
  AssociativeContainer &m_var;
};


int main(int argc, char** argv) {
  typedef std::map<std::string, std::string> Map;
  const char *base_name, *ptr;
  if (argc != 2) {
    ptr= strrchr(argv[0], '/');
    base_name= (ptr == NULL? argv[0]: ptr + 1);
    std::cerr << "Usage: " << base_name << " <uri>" << std::endl;
    return 1;
  }

  Binary_log_driver *drv= create_transport(argv[1]);
  if (drv == NULL)
    return 1;

  Binary_log binlog(drv);
  if (binlog.connect() != ERR_OK)
  {
    delete drv;
    return 1;
  }
  binlog.set_position(4);

  Map variables;
  Save_variables<Map> save_variables(variables);
  Content_stream_handler handler;

  handler.add_listener(save_variables);
  Replace_variables<Map> replace_variables(variables);
  handler.add_listener(replace_variables);

  Binary_log_event *event;
  Decoder decode;
  std::pair<unsigned char *, size_t> buf_and_len;
  while (true)
  {
    int error_number;
    error_number= drv->get_next_event(&buf_and_len);
    if (error_number == ERR_OK)
    {
      const char *error= NULL;
      if (!(event= decode.decode_event((char*)buf_and_len.first, buf_and_len.second,
                                        &error, 0)))
      {
         std::cerr << error << std::endl;
         break;
      }
      if (argv[1][0]== 'f')
        if ((error_number= static_cast<Binlog_file_driver*>(drv)->update_pos(event))
                           != ERR_OK)
        {
          const char* msg=  str_error(error_number);
          std::cerr << msg << std::endl;
          break;
        }
    }
    else
    {
      const char* msg=  str_error(error_number);
      std::cerr << msg << std::endl;
      break;
    }
    switch (event->get_event_type()) {
    case QUERY_EVENT:
      std::cout << static_cast<Query_event*>(event)->query
                << std::endl;
      break;
    }
    delete event;
  }
  delete drv;
  return 0;
}
