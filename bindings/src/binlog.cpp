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

#include "binlog.h"
#include <list>

using namespace binary_log;
using namespace binary_log::system;
namespace binary_log
{
/**
 *Errors you can get from the API
 */
const char *bapi_error_messages[]=
{
  "All OK",
  "End of File",
  "Unexpected failure",
  "Could not notify master about checksum awareness.\n"
  "Master returned no rows for the query\n"
  "SHOW VARIABLES LIKE 'BINLOG_CHECKSUM'",
  "Unable to set up connection",
  "Binlog Version not supported",
  "Error in packet length. Binlog checksums may be enabled on the master.\n"
  "Please set it to NONE.",
  "Error in executing MySQL Query on the server",
  ""
};

/*
  Get a string describing an error from BAPI.

  @param  error_no   the error number

  @retval buf        buffer containing the error message
*/
const char* str_error(int error_no)
{
  char *msg= NULL;
  if (error_no != ERR_OK)
  {
    if ((error_no > ERR_OK) && (error_no < ERROR_CODE_COUNT))
      msg= (char*)bapi_error_messages[error_no];
    else
      msg= (char*)"Unknown error";
   }
   return msg;
}

Binary_log::Binary_log(Binary_log_driver *drv) : m_binlog_position(4),
                                                 m_binlog_file("")
{
  if (drv == NULL)
  {
    m_driver= &m_dummy_driver;
  }
  else
   m_driver= drv;
}


int Binary_log::set_position(const std::string &filename,
                             unsigned long position)
{
  int status= m_driver->set_position(filename, position);
  if (status == ERR_OK)
  {
    m_binlog_file= filename;
    m_binlog_position= position;
  }
  return status;
}

int Binary_log::set_position(unsigned long position)
{
  std::string filename;
  m_driver->get_position(&filename, NULL);
  return this->set_position(filename, position);
}

unsigned long Binary_log::get_position(void)
{
  return m_binlog_position;
}

unsigned long Binary_log::get_position(std::string &filename)
{
  m_driver->get_position(&m_binlog_file, &m_binlog_position);
  filename= m_binlog_file;
  return m_binlog_position;
}

int Binary_log::connect(const std::string& binlog_filename, unsigned long pos)
{
  return m_driver->connect(binlog_filename, pos);
}

int Binary_log::disconnect()
{
  return m_driver->disconnect();
}
int Binary_log::connect()
{
  return m_driver->connect();
}
}
