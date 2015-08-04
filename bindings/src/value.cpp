/*
   Copyright (c) 2011, 2015, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "byteorder.h"
#include "value.h"
#include <iomanip>
#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <sstream>
#include <string.h>

using namespace binary_log;
namespace binary_log {

/*
Value::Value(Value &val)
{
  m_size= val.length();
  m_storage= val.storage();
  m_type= val.type();
  m_metadata= val.metadata();
  m_is_null= val.is_null();
}
*/

Value::Value(const Value& val)
{
  m_size= val.m_size;
  m_storage= val.m_storage;
  m_type= val.m_type;
  m_metadata= val.m_metadata;
  m_is_null= val.m_is_null;
}

Value &Value::operator=(const Value &val)
{
  m_size= val.m_size;
  m_storage= val.m_storage;
  m_type= val.m_type;
  m_metadata= val.m_metadata;
  m_is_null= val.m_is_null;
  return *this;
}

bool Value::operator==(const Value &val) const
{
  return (m_size == val.m_size) &&
         (m_storage == val.m_storage) &&
         (m_type == val.m_type) &&
         (m_metadata == val.m_metadata);
}

bool Value::operator!=(const Value &val) const
{
  return !operator==(val);
}

char *Value::as_c_str(unsigned long &size) const
{
  if (m_is_null || m_size == 0)
  {
    size= 0;
    return 0;
  }

  if (m_size == 1) {
    size = 0;
    return const_cast<char *>(m_storage);
  }
  
  /*
   Length encoded; First byte is length of string.
  */
  uint32_t maxlen = 0;
  
  if(m_type == MYSQL_TYPE_STRING) {
    uint8_t  lower  = m_metadata & 0xFF;
    uint8_t  higher = m_metadata >> 8U;
    if ((lower & 0x30) != 0x30) {
      maxlen = (((lower & 0x30) ^ 0x30) << 4) | higher;
    } else {
      maxlen = higher;
    }
  } else {
    maxlen = m_metadata;
  }
  
  int metadata_length = maxlen > 255 ? 2: 1;
  
  /*
   Size is length of the character string; not of the entire storage
  */
  size = m_size - metadata_length;
  return const_cast<char *>(m_storage + metadata_length);
}

unsigned char *Value::as_blob(unsigned long &size) const
{
  if (m_is_null || m_size == 0)
  {
    size= 0;
    return 0;
  }

  /*
   Size was calculated during construction of the object and only inludes the
   size of the blob data, not the metadata part which also is stored in the
   storage. For blobs this part can be between 1-4 bytes long.
  */
  size= m_size - m_metadata;

  /*
   Adjust the storage pointer with the size of the metadata.
  */
  return (unsigned char*)(m_storage + m_metadata);
}

int32_t Value::as_int32() const
{
  if (m_is_null)
  {
    return 0;
  }
  uint32_t to_int= 0;
  memcpy(&to_int, m_storage, m_size);
  to_int= le32toh(to_int);
  return to_int;
}

int8_t Value::as_int8() const
{
  if (m_is_null)
  {
    return 0;
  }
  int32_t to_int= 0;
  memcpy(&to_int, m_storage, m_size);
  return le32toh(to_int);
}

int16_t Value::as_int16() const
{
  if (m_is_null)
  {
    return 0;
  }
  int16_t to_int= 0;
  memcpy(&to_int, m_storage, m_size);
  return le16toh(to_int);
}

int64_t Value::as_int64() const
{
  if (m_is_null)
  {
    return 0;
  }
  int64_t to_int= 0;
  memcpy(&to_int, m_storage, m_size);
  return le64toh(to_int);
}

float Value::as_float() const
{
  // TODO
  return *((const float *)storage());
}

double Value::as_double() const
{
  // TODO
  return *((const double *)storage());
}

void Converter::to(std::string &str, const Value &val) const
{
  char buffer[20];
  if (val.is_null())
  {
    str= "(NULL)";
    return;
  }

  switch(val.type())
  {
    case MYSQL_TYPE_DECIMAL:
      str= "not implemented";
      break;
    case MYSQL_TYPE_TINY:
      sprintf(buffer, "%i", val.as_int8());
      str= buffer;
      break;
    case MYSQL_TYPE_SHORT:
      sprintf(buffer, "%i", val.as_int16());
      str= buffer;
      break;
    case MYSQL_TYPE_LONG:
      sprintf(buffer, "%i", val.as_int32());
      str= buffer;
      break;
    case MYSQL_TYPE_FLOAT:
      sprintf(buffer, "%g", val.as_float());
      str= buffer;
      break;
    case MYSQL_TYPE_DOUBLE:
      sprintf(buffer, "%g", val.as_double());
      str= buffer;
      break;
    case MYSQL_TYPE_NULL:
      str= "not implemented";
      break;
    case MYSQL_TYPE_TIMESTAMP:
      sprintf(buffer, "%i", val.as_int32());
      str= buffer;
      break;

    case MYSQL_TYPE_LONGLONG:
      sprintf(buffer, "%lld", (long long int)val.as_int64());
      str= buffer;
      break;
    case MYSQL_TYPE_INT24:
      str= "not implemented";
      break;
    case MYSQL_TYPE_DATE:
      str= "not implemented";
      break;
    case MYSQL_TYPE_DATETIME:
    {
      long long unsigned int timestamp= val.as_int64();
      long long unsigned int d= timestamp / 1000000;
      long long unsigned int t= timestamp % 1000000;
      char buf[19]= {0};
      sprintf(buf, "%llu-%llu-%llu %llu:%llu:%llu",
              d / 10000, (d % 10000) / 100,
              d % 100, t / 10000, (t % 10000) / 100, t % 100);
      str= buf;
    }
      break;
    case MYSQL_TYPE_TIME:
      str= "not implemented";
      break;
    case MYSQL_TYPE_YEAR:
      str= "not implemented";
      break;
    case MYSQL_TYPE_NEWDATE:
      str= "not implemented";
      break;
    case MYSQL_TYPE_VARCHAR:
    {
      unsigned long size;
      char *ptr= val.as_c_str(size);
      str.append(ptr, size);
    }
      break;
    case MYSQL_TYPE_VAR_STRING:
    {
      str.append(val.storage(), val.length());
    }
    break;
    case MYSQL_TYPE_STRING:
    {
      unsigned long size;
      char *ptr= val.as_c_str(size);
      str.append(ptr, size);
    }
      break;
    case MYSQL_TYPE_BIT:
      str= "not implemented";
      break;
    case MYSQL_TYPE_NEWDECIMAL:
      str= "not implemented";
      break;
    case MYSQL_TYPE_ENUM:
      str= "not implemented";
      break;
    case MYSQL_TYPE_SET:
      str= "not implemented";
      break;
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
    {
      unsigned long size;
      unsigned char *ptr= val.as_blob(size);
      str.append((const char *)ptr, size);
    }
      break;
    case MYSQL_TYPE_GEOMETRY:
      str= "not implemented";
      break;
    default:
      str= "not implemented";
      break;
  }
}

void Converter::to(float &out, const Value &val) const
{
  switch(val.type())
  {
  case MYSQL_TYPE_FLOAT:
    out= val.as_float();
    break;
  default:
    out= 0;
  }
}

void Converter::to(long &out, const Value &val) const
{
  switch(val.type())
  {
    case MYSQL_TYPE_DECIMAL:
      // TODO
      out= 0;
      break;
    case MYSQL_TYPE_TINY:
      out= val.as_int8();
      break;
    case MYSQL_TYPE_SHORT:
      out= val.as_int16();
      break;;
    case MYSQL_TYPE_LONG:
      out= (long)val.as_int32();
      break;
    case MYSQL_TYPE_FLOAT:
      out= 0;
      break;
    case MYSQL_TYPE_DOUBLE:
      out= (long)val.as_double();
    case MYSQL_TYPE_NULL:
      out= 0;
      break;
    case MYSQL_TYPE_TIMESTAMP:
      out=(uint32_t)val.as_int32();
      break;

    case MYSQL_TYPE_LONGLONG:
      out= (long)val.as_int64();
      break;
    case MYSQL_TYPE_INT24:
      out= 0;
      break;
    case MYSQL_TYPE_DATE:
      out= 0;
      break;
    case MYSQL_TYPE_TIME:
      out= 0;
      break;
    case MYSQL_TYPE_DATETIME:
      out= (long)val.as_int64();
      break;
    case MYSQL_TYPE_YEAR:
      out= 0;
      break;
    case MYSQL_TYPE_NEWDATE:
      out= 0;
      break;
    case MYSQL_TYPE_VARCHAR:
      out= 0;
      break;
    case MYSQL_TYPE_BIT:
      out= 0;
      break;
    case MYSQL_TYPE_NEWDECIMAL:
      out= 0;
      break;
    case MYSQL_TYPE_ENUM:
      out= 0;
      break;
    case MYSQL_TYPE_SET:
      out= 0;
      break;
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
      out= 0;
      break;
    case MYSQL_TYPE_VAR_STRING:
    {
      std::string str;
      str.append(val.storage(), val.length());
      out= atol(str.c_str());
    }
      break;
    case MYSQL_TYPE_STRING:
      out= 0;
      break;
    case MYSQL_TYPE_GEOMETRY:
      out= 0;
      break;
    default:
      out= 0;
      break;
  }
}


} // end namespace binary_log
