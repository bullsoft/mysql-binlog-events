/*
Copyright (c) 2011, 2015, Oracle and/or its affiliates. All rights
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

#ifndef FILE_DRIVER_INCLUDED
#define	FILE_DRIVER_INCLUDED

/*
  We need to include protocol.h before binlog_api.h, for there is dependency
  between mysqlclient and the header value.h for the enu enum_field_types
*/
#include "binlog.h"
#include "binlog_driver.h"
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#define MAGIC_NUMBER_SIZE 4

namespace binary_log {
namespace system {

class Binlog_file_driver
  : public Binary_log_driver
{
public:
  template <class TFilename>
  Binlog_file_driver(const TFilename& filename = TFilename(),
                     unsigned int offset = 0)
    : Binary_log_driver(filename, offset)
  {
  }

    int connect();
    int disconnect();
    int set_position(const std::string &str, unsigned long position);
    int get_position(std::string *str, unsigned long *position);
    int connect(const std::string &filename, unsigned long offset);
    /**
      Fetches a buffer containing the event data and its length
      and store them in a C++ pair.
    */
    int get_next_event(std::pair<unsigned char *, size_t> *);
    /**
     To update the position of binlog_file and position after decode_event
     @param   event    pointer to the event returned by decode_event

     @retval  ERR_OK   position is updated successfully
             >ERR_OK   error occured while updating position in file
    */
    int update_pos(binary_log::Binary_log_event *event);

    /**
      Returns the size of the binlog file currently being read

      @retval   size of file
    */
    size_t file_size() const;
private:

    unsigned long m_binlog_file_size;

    /*
      Bytes that has been read so for from the file.
      Updated after every event is read.
    */
    unsigned long m_bytes_read;

    std::ifstream m_binlog_file;
    Log_event_header m_event_log_header;
};

} // namespace binary_log::system
} // namespace binary_log

#endif	/* FILE_DRIVER_INCLUDED */
