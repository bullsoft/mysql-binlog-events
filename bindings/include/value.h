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

#ifndef VALUE_INCLUDED
#define VALUE_INCLUDED

#include "binary_log_funcs.h"
#include "my_time.h"
#include "decimal.h"
#include <stdint.h>
#include <string.h>
#include <iostream>


namespace binary_log {

/**
  @class Date

   A date object encapsulates a tuple
   (day, month, year).

   It can be used to decode MYSQL_TYPE_DATE.
*/
class Date
{
public:
  Date(int32_t day, int32_t month, int32_t year);
  int32_t day;
  int32_t month;
  int32_t year;
};

/**
  @class Date_time

   A date_time object encapsulates a tuple
   (day, month, year: this represents a date
    hour, min, year: this represnts time).

   It can be used to decode MYSQL_TYPE_DATETIME.
*/
class Date_time
{
public:
  Date_time(int32_t day, int32_t month, int32_t year, int32_t hour, int32_t min,
            int32_t sec);
  int32_t day;
  int32_t month;
  int32_t year;
  int32_t hour;
  int32_t min;
  int32_t sec;
};

/**
  @class Time

   A Time object encapsulates a tuple
   (hour, minute, second).

   It can be used to decode MYSQL_TYPE_TIME.
*/
class Time
{
public:
  Time(int32_t hour, int32_t min, int32_t sec);
  int32_t hour;
  int32_t min;
  int32_t sec;
};

/**
  @class Value

  A value object class which encapsluate a tuple
  (value type, metadata, storage)
  and provide for views to this storage through a well defined interface.

  Can be used with a Converter to convert between different Values.
*/
class Value
{
public:
  Value(enum_field_types type, uint32_t metadata, const unsigned char *storage)
  : m_type(type), m_storage(storage), m_metadata(metadata), m_is_null(false),
    m_is_bit_set(true)
  {
    m_size= calc_field_size((unsigned char)type, storage, metadata);
    switch (m_type)
    {
    case MYSQL_TYPE_STRING:
    {
      int real_type= metadata >> 8;
      if (real_type == MYSQL_TYPE_ENUM || real_type == MYSQL_TYPE_SET)
        m_type= static_cast<enum_field_types>(real_type);
      break;
    }

    /*
      This type has not been used since before row-based replication,
      so we can safely assume that it really is MYSQL_TYPE_NEWDATE.
    */
    case MYSQL_TYPE_DATE:
      m_type= MYSQL_TYPE_NEWDATE;
      break;

    default:
      /* Do nothing */
      break;
    }
  };

  Value()
  : m_size(0), m_storage(0), m_metadata(0), m_is_null(false),
    m_is_bit_set(false)
  { }

  /**
   * Copy constructor
   */
  Value(const Value& val);

  Value &operator=(const Value &val);
  bool operator==(const Value &val) const;
  bool operator!=(const Value &val) const;

  ~Value() {}

  void set_null_bit(bool s) { m_is_null= s; }
  bool is_null(void) const { return m_is_null; }

  void set_exists_bit(bool s) { m_is_bit_set= s; }
  bool is_bit_set(void) const { return m_is_bit_set; }
  const unsigned char *storage() const { return m_storage; }

  /**
   * Get the length in bytes of the entire storage (any metadata part +
   * actual data)
   */
  uint32_t length() const { return m_size; }
  enum_field_types type() const { return m_type; }
  uint32_t metadata() const { return m_metadata; }

  /**
   * Returns the integer representation of a storage of a pre-specified
   * type.
   * Asserts if the value passed for decoding is NULL, check for null has
   * to be done before calling this method.
   */
  int32_t as_int32() const;

  /**
   * Returns the integer representation of a storage of pre-specified
   * type.
   * Asserts if the value passed for decoding is NULL, check for null has
   * to be done before calling this method.
   */
  int64_t as_int64() const;

  /**
   * Returns the integer representation of a storage of pre-specified
   * type.
   * Asserts if the value passed for decoding is NULL, check for null has
   * to be done before calling this method.
   */
  int8_t as_int8() const;

  /**
   * Returns the integer representation of a storage of pre-specified
   * type.
   * Asserts if the value passed for decoding is NULL, check for null has
   * to be done before calling this method.
   */
  int16_t as_int16() const;

  /**
   * Returns the integer representation of a storage of pre-specified
   * type.
   * Asserts if the value passed for decoding is NULL, check for null has
   * to be done before calling this method.
   */
  int32_t as_int24() const;

  /**
   * Returns a pointer to the character data of a string type stored
   * in the pre-defined storage.
   * @note The position is an offset of the storage pointer determined
   * by the metadata and type.
   *
   * @param[out] size The size in bytes of the character string.
   *
   */
  unsigned char *as_c_str(unsigned long &size) const;

  /**
   * Returns a pointer to the byte data of a blob type stored in the pre-
   * defined storage.
   * @note The position is an offset of the storage pointer determined
   * by the metadata and type.
   *
   * @param[out] size The size in bytes of the blob data.
   */
  unsigned char *as_blob(unsigned long &size) const;

  /**
   * Initializes a string, where each character is either 0 or 1 depending
   * on if that bit is on or off in the pre-defined storage.
   * @param[out] str    The string containing the decoded data
   * @param nbits       Number of bits
   * @retval            The serialized string.
   */
  std::string as_bitstring(unsigned int nbits) const;

  /**
   * Returns a Date object which contain the information about day, month
   * and year.
   * @retval      The Date oject.
   */
  Date as_date() const;

  /**
   * Returns a Date object which contain the information about day, month
   * and year.
   * It represents the DATE typed used after 5.0.
   * This uses 3 bytes whereas MYSQL_TYPE_DATE uses 4 bytes to store the data
   */
  std::string as_newdate(int value) const;

  /**
   * Returns a Date_time object which contain the information about the date
   * and time.
   * @retval      The Date_time oject.
   */
  Date_time as_date_time() const;
  
  Date_time as_date_time2() const;

  /**
   * Returns a Time object which contain the information about hour, minute
   * and second.
   * @retval      The Time oject.
   */
  Time as_time() const;

  /**
   * Returns the year which is a integer representation of a storage of
   * pre-specified type.
   */
  int32_t as_year() const;

  float as_float() const;
  double as_double() const;

private:
  enum_field_types m_type;
  uint32_t m_size;
  const unsigned char *m_storage;
  uint32_t m_metadata;
  bool m_is_null;
  bool m_is_bit_set;
};

class Converter
{
public:
  /**
   * Converts and copies the sql value to a std::string object.
   * @param[out] str The target string
   * @param[in] val The value object to be converted
   */
  void to(std::string &str, const Value &val) const;

  /**
   * Converts and copies the sql value to a long integer.
   * @param[out] out The target variable
   * @param[in] val The value object to be converted
   */
  void to(long &out, const Value &val) const;

  /**
   * Converts and copies the sql value to a floating point number.
   * @param[out] out The target variable
   * @param[in] val The value object to be converted
   */
  void to(float &out, const Value &val) const;
};

}
#endif
