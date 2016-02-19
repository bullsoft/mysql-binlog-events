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

#ifndef BINLOG_API_INCLUDED
#define	BINLOG_API_INCLUDED

/*
  We need to include the driver headers before libbinlogevent because of
  the obscure dependency of libmysqlclient of libbinlogevent.
*/
#include "binlog_driver.h"
#include "tcp_driver.h"
#include "file_driver.h"
#include "access_method_factory.h"
#include "basic_content_handler.h"
#include "basic_transaction_parser.h"
#include "binary_log.h"
#include "field_iterator.h"
#include "rowset.h"
#include "decoder.h"
#include <iosfwd>
#include <list>
#include <cassert>
#include <algorithm>
#include <utility>

#define BAPI_STRERROR_SIZE (256)
namespace binary_log
{

/**
 * Error codes.
 */
enum Error_code {
  ERR_OK = 0,                                   /* All OK */
  ERR_EOF,                                      /* End of file */
  ERR_FAIL,                                     /* Unspecified failure */
  ERR_CHECKSUM_QUERY_FAIL,
  ERR_CONNECT,
  ERR_BINLOG_VERSION,
  ERR_PACKET_LENGTH,
  ERR_MYSQL_QUERY_FAIL,
  ERROR_CODE_COUNT
};

/**
 *Errors you can get from the API
 */
extern const char *bapi_error_messages[];

extern const char *str_error(int error_no);

/**
 * Returns true if the event is consumed
 */

class Dummy_driver : public system::Binary_log_driver
{
public:
  Dummy_driver() : Binary_log_driver("", 0) {}
  virtual ~Dummy_driver() {}

  virtual int connect()
  {
    return 1;
  }

  virtual int set_position(const std::string &str, unsigned long position)
  {
    return ERR_OK;
  }

  virtual int get_position(std::string *str, unsigned long *position)
  {
    return ERR_OK;
  }
  virtual int connect(const std::string &filename, unsigned long position)
  {
    return ERR_OK;
  }
  virtual int disconnect()
  {
    return ERR_OK;
  }
  /**
    Fetches a buffer containing the event data and its length,
    and store them in a C++ pair.
  */
  virtual int get_next_event(std::pair<unsigned char *, size_t> *)
  {
    return ERR_OK;
  }
  virtual size_t file_size() const
  {
    return ERR_OK;
  }
};


class Binary_log {
private:
  system::Binary_log_driver *m_driver;
  Dummy_driver m_dummy_driver;
  unsigned long m_binlog_position;
  std::string m_binlog_file;
public:
  Binary_log(system::Binary_log_driver *drv);
  ~Binary_log()
  {
    m_driver= NULL;
  }
  int connect();
  int connect(const std::string& binlog_filename, unsigned long position);


  /**
   * Set the binlog position (filename, position)
   *
   * @return Error_code
   *  @retval ERR_OK The position is updated.
   *  @retval ERR_EOF The position is out-of-range
   *  @retval >= ERR_CODE_COUNT An unspecified error occurred
   */
  int set_position(const std::string &filename, unsigned long position);

  /**
   * Set the binlog position using current filename
   * @param position Requested position
   *
   * @return Error_code
   *  @retval ERR_OK The position is updated.
   *  @retval ERR_EOF The position is out-of-range
   *  @retval >= ERR_CODE_COUNT An unspecified error occurred
   */
  int set_position(unsigned long position);

  /**
   * Fetch the binlog position for the current file
   */
  unsigned long get_position(void);

  /**
   * Fetch the current active binlog file name, and also the file position.
   * @param[out] filename
   * @return The current binlog file position
   */
  unsigned long get_position(std::string &filename);

  /**
   * Returns size of the binary log file.
   */
  size_t file_size() const;

  int disconnect();
};

}
#endif	/* BINLOG_API_INCLUDED */
