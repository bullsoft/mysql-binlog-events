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
#include <strings.h>
/**
  @file basic-1
  @author Mats Kindahl <mats.kindahl@oracle.com>

  This is a basic example that just opens a binary log either from a
  file or a server and print out what events are found.  It uses a
  simple event loop and checks information in the events using a
  switch.
 */

using binary_log::Binary_log;
using binary_log::system::create_transport;
using binary_log::system::Binary_log_driver;
using binary_log::system::Binlog_file_driver;
int main(int argc, char** argv) {
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
    std::cout << "Found event of type "
              << event->get_event_type()
              << std::endl;
    delete event;
  }
  delete drv;
  return 0;
}
