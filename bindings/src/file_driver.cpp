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

#include "file_driver.h"
#define PROBE_HEADER_LEN (EVENT_LEN_OFFSET+4)
namespace binary_log { namespace system {
using namespace std;

int Binlog_file_driver::connect()
{
  struct stat stat_buff;

  unsigned char magic[]= {0xfe, 0x62, 0x69, 0x6e, 0};
  char magic_buf[MAGIC_NUMBER_SIZE];

  // Get the file size.
  if (stat(m_binlog_file_name.c_str(), &stat_buff) == -1)
    return ERR_FAIL;                          // Can't stat binlog file.
  m_binlog_file_size= stat_buff.st_size;

  m_binlog_file.exceptions(ifstream::failbit | ifstream::badbit |
                           ifstream::eofbit);
  try
  {
    // Check if the file can be opened for reading.
    m_binlog_file.open(m_binlog_file_name.c_str(), ios::in | ios::binary);

    // Check if a valid MySQL binlog file is provided, BINLOG_MAGIC.
    m_binlog_file.read(magic_buf, MAGIC_NUMBER_SIZE);

    if(memcmp(magic, magic_buf, MAGIC_NUMBER_SIZE))
      return ERR_FAIL;                        // Not a valid binlog file.

    // Reset the get pointer.

    m_bytes_read= MAGIC_NUMBER_SIZE;

    // Read the first event to check the binlog version
    if (m_bytes_read < m_binlog_file_size && m_binlog_file.good())
    {
      char header[PROBE_HEADER_LEN];
      m_binlog_file.read(header, PROBE_HEADER_LEN);
      //TODO: Create a FDE event
      m_event_log_header.type_code= (Log_event_type)header[EVENT_TYPE_OFFSET];

      if (m_event_log_header.type_code != FORMAT_DESCRIPTION_EVENT)
        return ERR_BINLOG_VERSION;
    }

  }
  catch (std::ifstream::failure)
  {
    std::cerr << "Exception opening/reading/closing file\n";
    return ERR_FAIL;
  }

  return ERR_OK;
}

int Binlog_file_driver::connect(const std::string &filename, unsigned long position)
{
  if (m_binlog_file.is_open())
  {
    m_binlog_file.close();
    m_binlog_file.clear();
  }
  m_binlog_file_name= filename;
  /*
   The code for connecting to a file and validating it is common for both
   the defintion of connect method.
   So we are re-using the above definition of connect().
  */
  if (connect() != ERR_OK)
    return ERR_FAIL;

  try
  {
    m_binlog_file.seekg(position, ios::beg );
  }
  catch (std::ifstream::failure)
  {
    std::cerr << "Exception opening/reading/closing file\n";
    return ERR_FAIL;
  }

  m_bytes_read= position;
  return ERR_OK;
}
int Binlog_file_driver::disconnect()
{
  m_binlog_file.close();
  return ERR_OK;
}


int Binlog_file_driver::set_position(const string &str, unsigned long position)
{
  m_binlog_file.exceptions(ifstream::failbit);
  try
  {
    m_binlog_file.seekg(position, ios::beg );
  }
  catch (std::ifstream::failure)
  {
    std::cerr << "Exception opening/reading/closing file\n";
    return ERR_FAIL;
  }

  m_bytes_read= position;

  return ERR_OK;
}


int Binlog_file_driver::get_position(string *str, unsigned long *position)
{
  m_binlog_file.exceptions(ifstream::failbit | ifstream::badbit |
                           ifstream::eofbit);
  *str= m_binlog_file_name;

  try
  {
    if(position)
    {
      std::streamoff binlog_file_pos= m_binlog_file.tellg();
      *position= (unsigned long)binlog_file_pos;
    }
  }
  catch (std::ifstream::failure)
  {
    std::cerr << "Exception opening/reading/closing file\n";
    return ERR_FAIL;
  }

  return ERR_OK;
}


int Binlog_file_driver::get_next_event(std::pair<unsigned char *, size_t> *buf_len_pair)
{
  assert(m_binlog_file.tellg() >= MAGIC_NUMBER_SIZE );
  m_binlog_file.exceptions(ifstream::failbit | ifstream::badbit |
                           ifstream::eofbit);
  size_t buf_len;
  try
  {
    if (m_bytes_read < m_binlog_file_size && m_binlog_file.good())
    {
      //TODO: read the file to create a header
      char head[LOG_EVENT_MINIMAL_HEADER_LEN];
      size_t header_size= LOG_EVENT_MINIMAL_HEADER_LEN;
      /*
       When the last_event_len == 0 it means this is the first event, which
       in case of file_driver should be FDE.
       if m_bytes_read > 4 it means that we have opted to read from the middle
       of a file, in that case we have to force the code to read FDE as the
       first event.
      */
      if (last_event_len == 0)
        if (m_bytes_read > MAGIC_NUMBER_SIZE)
        {
           m_binlog_file.seekg(MAGIC_NUMBER_SIZE);
        }
      m_binlog_file.read(head, header_size);
      size_t data_len= 0;
      memcpy(&data_len, head + EVENT_LEN_OFFSET, MAGIC_NUMBER_SIZE);
      data_len= le32toh(data_len);

      if (data_len > last_event_len)
        buf= (unsigned char*) realloc(buf, data_len + 1);
      buf[data_len]= 0;
      memcpy(buf, head, header_size);
      m_binlog_file.read((char*)(buf + header_size), data_len - header_size);
      buf_len= data_len;
      last_event_len=buf_len;
      *buf_len_pair= std::make_pair((unsigned char*)buf, buf_len);
      /*
       The code below is to adjust the offset of the file when reading from the
       middle of the file, and we have changed it above to read the FDE.
       If this is not done then the m_binlog_file will be seek to a wrong position
       in the method update_pos which will be called after the call to this
       method
      */
      if (head[EVENT_TYPE_OFFSET] == FORMAT_DESCRIPTION_EVENT && m_bytes_read > 4)
      {
        m_binlog_file.seekg(m_bytes_read);
        m_bytes_read-= buf_len;
      }
      return ERR_OK;
    }
    else
    {
      disconnect();
    }
  }
  catch (std::ifstream::failure)
  {
    std::cerr << "Exception opening/reading/closing file\n";
    return ERR_FAIL;
  }

  return ERR_EOF;
}

/*
  After we create the header we need to update the position of file pointer
*/
int Binlog_file_driver::update_pos(binary_log::Binary_log_event *event)
{
  std::streamoff binlog_file_pos= m_binlog_file.tellg();
  assert(binlog_file_pos >= MAGIC_NUMBER_SIZE );

  m_binlog_file.exceptions(ifstream::failbit);
  /*
   Correction.Except for the default case (above), this condition should
   always fail.
  */
  try
  {
    m_event_log_header= *(event->header());
    if (m_bytes_read + m_event_log_header.data_written !=
        (unsigned long)binlog_file_pos)
       m_binlog_file.seekg(m_bytes_read + m_event_log_header.data_written,
                           ios::beg);

    m_bytes_read= binlog_file_pos;
  }
  catch (std::ifstream::failure)
  {
    std::cerr << "Exception opening/reading/closing file\n";
    return ERR_FAIL;
  }

  return ERR_OK;
}

size_t Binlog_file_driver::file_size() const
{
  return m_binlog_file_size;
}
}// end namespace system
}// end namespace binary_log

