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

#ifdef _WIN32
#define snprintf _snprintf
#endif

using namespace binary_log;
namespace binary_log {

/* Constructor of Date class */
Date::Date(int32_t day_arg, int32_t month_arg, int32_t year_arg)
  : day(day_arg), month(month_arg), year(year_arg)
{}

/* Constructor of Date_time class */
Date_time::Date_time(int32_t day_arg, int32_t month_arg, int32_t year_arg,
                     int32_t hour_arg, int32_t min_arg, int32_t sec_arg)
  : day(day_arg), month(month_arg), year(year_arg), hour(hour_arg),
    min(min_arg), sec(sec_arg)
{}

/* Constrctor of Time class */
Time::Time(int32_t hour_arg, int32_t min_arg, int32_t sec_arg)
  : hour(hour_arg), min(min_arg), sec(sec_arg)
{}

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
  m_is_bit_set= val.m_is_bit_set;
}

Value &Value::operator=(const Value &val)
{
  m_size= val.m_size;
  m_storage= val.m_storage;
  m_type= val.m_type;
  m_metadata= val.m_metadata;
  m_is_null= val.m_is_null;
  m_is_bit_set= val.m_is_bit_set;
  return *this;
}

bool Value::operator==(const Value &val) const
{
  return (m_size == val.m_size) &&
         (m_storage == val.m_storage) &&
         (m_type == val.m_type) &&
         (m_metadata == val.m_metadata) &&
         (m_is_bit_set == val.m_is_bit_set);
}

bool Value::operator!=(const Value &val) const
{
  return !operator==(val);
}

unsigned char *Value::as_c_str(unsigned long &size) const
{
  if (m_is_null || m_size == 0)
  {
    size= 0;
    return 0;
  }
  if (m_type == MYSQL_TYPE_STRING)
  {
    if (m_size < 256)
    {
       size= *m_storage;
       return const_cast<unsigned char *>(m_storage + 1);
    }
    else
    {
      unsigned long tmp= 0;
      memcpy(&tmp, m_storage, 2);
      size= le16toh(tmp);
      return const_cast<unsigned char *>(m_storage + 2);
    }
  }

  else if ( m_type == MYSQL_TYPE_BLOB)
  {
    switch (m_metadata)
    {
    // The number here shows the value of metadata and also it shows the
    // number of bytes needed to store the size of data.
    case 1:
    {
      size= *m_storage;
      return const_cast<unsigned char *>(m_storage + 1);
    }
    case 2:
    {
      unsigned long tmp= 0;
      memcpy(&tmp, m_storage, 2);
      size= le16toh(tmp);
      return const_cast<unsigned char *>(m_storage + 2);
    }
    case 3:
    {
      unsigned long tmp= 0;
      memcpy(&tmp, m_storage, 3);
      size= le32toh(tmp);
      return const_cast<unsigned char *>(m_storage + 3);
    }
    case 4:
    {
      unsigned long tmp= 0;
      memcpy(&tmp, m_storage, 4);
      size= le32toh(tmp);
      return const_cast<unsigned char *>(m_storage + 4);
    }
    }
  }
  /*
   Length encoded; First byte is length of string.
  */
  int metadata_length= m_metadata >= 256 ? 2: 1;
  /*
   Size is length of the character string; not of the entire storage
  */
  size= m_size - metadata_length;
  return const_cast<unsigned char *>(m_storage + metadata_length);
}

std::string Value::as_bitstring(unsigned int nbits) const
{
  std::string str;
  unsigned int bitnum;
  unsigned int nbits8= ((nbits + 7) / 8) * 8;
  unsigned int skip_bits= nbits8 - nbits;

  str.append("b'");
  for (bitnum= skip_bits ; bitnum < nbits8; bitnum++)
  {
    int is_set= (m_storage[(bitnum) / 8] >> (7 - bitnum % 8))  & 0x01;
    str.append(is_set ? "1" : "0");
  }
  str.append("'");
  return str;
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
  assert (!m_is_null);
  uint32_t to_int= 0;
  memcpy(&to_int, m_storage, m_size);
  to_int= le32toh(to_int);
  return to_int;
}

int8_t Value::as_int8() const
{
  assert (!m_is_null);
  int32_t to_int= 0;
  memcpy(&to_int, m_storage, m_size);
  return le32toh(to_int);
}

int16_t Value::as_int16() const
{
  assert (!m_is_null);
  int16_t to_int= 0;
  memcpy(&to_int, m_storage, m_size);
  return le16toh(to_int);
}

int32_t Value::as_int24() const
{
  assert (!m_is_null);
  uint32_t to_int= 0;
  memcpy(&to_int, m_storage, 3);
  to_int= le32toh(to_int);
  if (m_storage[2] & 128)
    to_int= to_int | ((uint32_t) 255L << 24);
  return to_int;
}

int64_t Value::as_int64() const
{
  assert (!m_is_null);
  int64_t to_int= 0;
  memcpy(&to_int, m_storage, m_size);
  return le64toh(to_int);
}

float Value::as_float() const
{
  return *((const float *)storage());
}

double Value::as_double() const
{
  return *((const double *)storage());
}

/**
  The date data type is stored in YYYY-MM-DD format
  The first 5 byte is used to store day,
  next 4 byte is used for storing month,
  rest bytes are used for storing the year.
*/
Date Value::as_date() const
{
  int32_t i32= this->as_int24();
  int32_t day= i32 % 32;
  int32_t month= (i32 / 32) % 16;
  int32_t year= i32 / (16 * 32);
  Date dt(day, month, year);
  return dt;
}

/**
  The datetime is stored in this format YYYY-MM-DD HH:MM:SS
  we divide this complete stream in two parts:
  date: here we trim the first 6 digit from right and then left with 8 digits
        first four digit from left is for year
        next two digit for month
        last two of eight digits is for day

  time: here we take out the first 6 digit from right
        first two digit from left is for hour
        next two digit for minute
        last two of six digits is for seconds
*/
Date_time Value::as_date_time() const
{
  int64_t timestamp= this->as_int64();
  size_t date= timestamp / 1000000;
  size_t time= timestamp % 1000000;
  //decode the date in YYYY-MM-DD format
  int32_t year= date / 10000;
  int32_t month= (date % 10000) / 100;
  int32_t day= date % 100;
  // decode the time HH:MM:SS format
  int32_t hour= time / 10000;
  int32_t min= (time % 10000) / 100;
  int32_t sec= time % 100;
  Date_time dt(day, month, year, hour, min, sec);
  return dt;
}


Date_time Value::as_date_time2() const
{
  longlong packed = my_datetime_packed_from_binary(m_storage, m_metadata); 
  MYSQL_TIME ltime;
  TIME_from_longlong_datetime_packed(&ltime, packed);
  Date_time dt(ltime.day, ltime.month, ltime.year, ltime.hour, ltime.minute, ltime.second);
  return dt;
}

/**
  The time data type is stored in HH:MM:SS format
  first two digit from left is for hour
  next two digit for minute
  last two of six digits is for seconds
*/
Time Value::as_time() const
{
  int32_t i32= this->as_int24();
  int32_t hour= i32 / 10000;
  int32_t min= (i32 % 10000) / 100;
  int32_t sec= i32 % 100;
  Time t(hour, min, sec);
  return t;
}

int32_t Value::as_year() const
{
  // MySQL supports a range of 1901 to 2155 for the data type MYSQL_TYPE_YEAR
  uint32_t i32= *m_storage;
  return i32 + 1900;
}

std::string Value::as_newdate(int tmp) const
{
  int part;
  char buf[11];
  char *pos= &buf[10];
  *pos--= 0;
  part= (int) (tmp & 31);
  *pos--= (char) ('0' + part % 10);
  *pos--= (char) ('0' + part / 10);
  *pos--= ':';
  part=(int) (tmp >> 5 & 15);
  *pos--= (char) ('0' + part % 10);
  *pos--= (char) ('0' + part / 10);
  *pos--= ':';
  part=(int) (tmp >> 9);
  *pos--= (char) ('0' + part % 10); part/= 10;
  *pos--= (char) ('0' + part % 10); part/= 10;
  *pos--= (char) ('0' + part % 10); part/= 10;
  *pos= (char) ('0' + part);
  std::string str(buf);
  return str;
}

/**
  Prints a quoted string to string stream
  Control characters are displayed as hex sequence, e.g. \x00
  length       Length of the string
  ptr          pointer to string
*/
std::string write_quoted(unsigned long length, const unsigned char* ptr)
{
  std::ostringstream oss;
  for (; length > 0; ptr++, length--)
  {
     if (*ptr < 0X1F)
     {
       char hex[10];
       snprintf((char*)hex, sizeof(hex), "%s%02x", "\\x", *ptr);
       for (int j= 0; j < sizeof(hex) && hex[j] != '\0'; j++)
         oss << hex[j];
     }
     else
       oss << *ptr;
  }
  return oss.str();
}

void Converter::to(std::string &str, const Value &val) const
{
  char buffer[320];

  if (!val.is_bit_set())
    return;

  if (val.is_null())
  {
    str= "NULL";
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
    case MYSQL_TYPE_INT24:
      sprintf(buffer, "%i", val.as_int24());
      str= buffer;
      break;
    case MYSQL_TYPE_FLOAT:
      sprintf(buffer, "%g", val.as_float());
      str= buffer;
      break;
    case MYSQL_TYPE_DOUBLE:
      sprintf(buffer, "%-.20g", val.as_double());
      str= buffer;
      break;
    case MYSQL_TYPE_NULL:
      str= "NULL";
      break;
    case MYSQL_TYPE_TIMESTAMP:
      sprintf(buffer, "%i", val.as_int32());
      str= buffer;
      break;
    case MYSQL_TYPE_TIMESTAMP2:
    {
      struct timeval tm;
      my_timestamp_from_binary(&tm, val.storage(), val.metadata());
      int buflen= my_timeval_to_str(&tm, buffer, val.metadata());
      str = buffer;
    }
    break;
    case MYSQL_TYPE_LONGLONG:
      sprintf(buffer, "%lld", (long long int)val.as_int64());
      str= buffer;
      break;
    case MYSQL_TYPE_DATE:
    {
      Date dt= val.as_date();
      sprintf(buffer , "'%04d:%02d:%02d'", (dt.year), (dt.month), (dt.day));
      str= buffer;
      break;
    }
    case MYSQL_TYPE_DATETIME:
    {
      Date_time dt= val.as_date_time();
      char buf[19]= {0};
      sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", dt.year, dt.month,
              dt.day, dt.hour, dt.min, dt.sec);
      str= buf;
    }
      break;
    case MYSQL_TYPE_DATETIME2:
    {
      Date_time dt= val.as_date_time2();
      char buf[19]= {0};
      sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", dt.year, dt.month,
              dt.day, dt.hour, dt.min, dt.sec);
      str= buf;
    }
      break;
    case MYSQL_TYPE_TIME:
    {
      Time t= val.as_time();
      sprintf(buffer, "'%02d:%02d:%02d'", t.hour, t.min, t.sec);
      str= buffer;
      break;
    }
    case MYSQL_TYPE_YEAR:
    {
      sprintf(buffer, "%04d", val.as_year());
      str= buffer;
      break;
    }
    case MYSQL_TYPE_NEWDATE:
    {
      int tmp= val.as_int24();
      str= val.as_newdate(tmp);
      break;
    }
    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_STRING:
    {
      unsigned long size;
      const unsigned char *ptr= val.as_c_str(size);
      str= write_quoted(size, ptr);
    }
      break;
    case MYSQL_TYPE_BIT:
    {
      /* Meta-data: bit_len, bytes_in_rec, 2 bytes */
      unsigned int nbits= ((val.metadata() >> 8) * 8) +
                           (val.metadata() & 0xFF);
      str= val.as_bitstring(nbits);
      break;
    }
    case MYSQL_TYPE_NEWDECIMAL:
    {
      char s[200];
      int len = sizeof(s) - 1;

      decimal_digit_t buff[10];
      decimal_t dec;
      dec.buf = buff;
      dec.len = 10;

      bin2decimal(val.storage(), &dec, val.metadata() >> 8, val.metadata() & 0xFF);
      decimal2string(&dec, s, &len, 0,0,0);

      s[len] = 0;
      str = std::string(s);
      break;
    }
    case MYSQL_TYPE_ENUM:
    switch (val.metadata() & 0xFF) {
    case 1:
    {
      sprintf(buffer, "%i", val.as_int32());
      str= buffer;
    }
      break;
    case 2:
    {
      sprintf(buffer, "%i", val.as_int16());
      str= buffer;
    }
      break;
    default:
      str="0";
    }
      break;
    case MYSQL_TYPE_SET:
    {
      unsigned int nbits= (val.metadata() & 0xFF) * 8;
      str= val.as_bitstring(nbits);
      break;
    }

    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
    {
      unsigned long size;
      const unsigned char *ptr= val.as_c_str(size);
      str= write_quoted(size, ptr);
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
      unsigned long size;
      const unsigned char *ptr= val.as_c_str(size);
      str= write_quoted(size, ptr);
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
