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

#ifndef BINLOG_DRIVER_INCLUDED
#define	BINLOG_DRIVER_INCLUDED

#include "binlog_event.h"
#include <utility>

namespace binary_log {
namespace system {

class Binary_log_driver
{
public:
  template <class FilenameT>
  Binary_log_driver(const FilenameT& filename = FilenameT(),
                    unsigned int offset = 0)
  : m_binlog_file_name(filename), m_binlog_offset(offset), buf(NULL),
    last_event_len(0)
  {
  }

  virtual ~Binary_log_driver()
  {
    if (buf)
      bapi_free(buf);
  }

  /**
   * Connect to the binary log using previously declared connection parameters
   * @return Success or error code
   * @retval 0 Success
   * @retval >0 Error code (to be specified)
   */
  virtual int connect()= 0;
  virtual int connect(const std::string &filename, unsigned long position)= 0;

  /**
   * Set the reader position
   * @param str The file name
   * @param position The file position
   *
   * @return False on success and True if an error occurred.
   */
  virtual int set_position(const std::string &str, unsigned long position)= 0;

  /**
   * Get the read position.
   *
   * @param[out] string_ptr Pointer to location where the
     filename will be stored.
   * @param[out] position_ptr Pointer to location where
     the position will be stored.
   *
   * @retval 0 Success
   * @retval >0 Error code
   */
  virtual int get_position(std::string *filename_ptr,
                           unsigned long *position_ptr) = 0;

  /**
   * Get the file size of Binary Log file.
   * @retval   Size of file
   */
  virtual size_t file_size() const= 0;

  virtual int disconnect()= 0;

  /**
    Blocking attempt to get the next event buffer
    @param   buffer_buflen  It stores the buffer and the buffer_len in a C++ pair
    @retval  0              Success
             >0             Error code
  */
  virtual int get_next_event(std::pair<unsigned char *, size_t> *buffer_buflen)=0;
protected:
  unsigned char *buf;
  /**
   We dont have to allocate memory inside get_next_event
   every time we receive an event, we can use the previously allocated memory,
   when current event has an event_len less than the last_event_len.
   And if that condition fails then we will allocate a new chunk.
   The function is not thread-safe and the caller is expected to copy the buffer
   if they want to retain it between successive calls to get_next_event()
  */
  size_t last_event_len;
protected:
  std::string m_binlog_file_name;
  /**
   * Used each time the client reconnects to the server to specify an
   * offset position.
   */
  unsigned long m_binlog_offset;
};

} // namespace binary_log::system
} // namespace binary_log
#endif	/* BINLOG_DRIVER_INCLUDED */
