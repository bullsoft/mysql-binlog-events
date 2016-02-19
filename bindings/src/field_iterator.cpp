/*
   Copyright (c) 2014, 2016, Oracle and/or its affiliates. All rights reserved.

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

#include "field_iterator.h"

namespace binary_log
{

template<class Iterator_value_type>
bool Row_event_iterator< Iterator_value_type>::
is_null(unsigned char *bitmap, int index)
{
  unsigned char *byte= bitmap + (index / 8);
  unsigned bit= 1 << ((index) & 7);
  return ((*byte) & bit) != 0;
}

/**
  Return the number of columns getting modified.
*/
unsigned int get_total_set_bit(std::vector<uint8_t> cols_bitmap, unsigned int colcnt)
{
  unsigned int i= 0;
  for (unsigned col_no= 0; col_no < colcnt; ++col_no)
  {
    if ((cols_bitmap[col_no/8] >> col_no % 8) & 0x01)
      i++;
  }
  return i;
}

template<class Iterator_value_type>
uint32_t Row_event_iterator< Iterator_value_type>::
extract_metadata(const Table_map_event *map, int col_no)
{
  int offset= 0;

  for (int i= 0; i < col_no; ++i)
  {
    unsigned int type= (unsigned int)map->m_coltype[i] & 0xFF;
    offset += lookup_metadata_field_size((enum_field_types)type);
  }

  uint16_t metadata= 0;
  unsigned int type= (unsigned int)map->m_coltype[col_no] & 0xFF;
  switch (lookup_metadata_field_size((enum_field_types)type))
  {
  case 1:
    metadata= map->m_field_metadata[offset];
  break;
  case 2:
    {
      switch(type)
      {
      case MYSQL_TYPE_VARCHAR:
        {
          uint16_t tmp= 0;
          memcpy(&tmp, (char*)&(map->m_field_metadata[offset]), 2);
          metadata+= le16toh(tmp);
          break;
        }
      case MYSQL_TYPE_SET:
      case MYSQL_TYPE_ENUM:
      case MYSQL_TYPE_STRING:
      case MYSQL_TYPE_NEWDECIMAL:
        {
          /*
            The metadata is arranged like this <TYPE><PACK_LENGTH>
            type being in the most significant byte and pack length in the
            least significant byte.
          */
          metadata= (map->m_field_metadata[offset] << 8U) |
                     (map->m_field_metadata[offset + 1]);
          break;
        }
      case MYSQL_TYPE_BIT:
        {
          metadata= map->m_field_metadata[offset] |
                     (map->m_field_metadata[offset + 1] << 8U);
          break;
        }
      }
    }
  break;
  }
  return metadata;
}


template<class Iterator_value_type>
int Row_event_iterator< Iterator_value_type>::
lookup_metadata_field_size(enum_field_types field_type)
{
  switch (field_type)
  {
    case MYSQL_TYPE_DOUBLE:
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_BLOB:
#if MYSQL_VERSION_ID >= 50604
    case MYSQL_TYPE_DATETIME2:
    case MYSQL_TYPE_TIMESTAMP2:
    case MYSQL_TYPE_TIME2:
#endif
    case MYSQL_TYPE_GEOMETRY:
     return 1;
    case MYSQL_TYPE_BIT:
    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_NEWDECIMAL:
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_ENUM:
     return 2;
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_SET:
    case MYSQL_TYPE_YEAR:
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_LONGLONG:
    case MYSQL_TYPE_NULL:
    case MYSQL_TYPE_NEWDATE:
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    default:
      return 0;
  }
}

template <class Iterator_value_type >
Iterator_value_type Row_event_iterator<Iterator_value_type>::operator*()
{ // dereferencing
  Iterator_value_type fields_vector;
  /*
   * Remember this offset if we need to increate the row pointer
   */
  m_new_field_offset_calculated= fields(fields_vector);
  return fields_vector;
}


template< class Iterator_value_type >
Row_event_iterator< Iterator_value_type >&
  Row_event_iterator< Iterator_value_type >::operator++()
{ // preﬁx
  if (m_field_offset == UINT_MAX)
    throw std::logic_error("Field type is unrecognized");

  if (m_field_offset < m_row_event->row.size() - 1)
  {
    /*
     * If we requested the fields in a previous operations
     * we also calculated the new offset at the same time.
     */
    if (m_new_field_offset_calculated != 0)
    {
      m_field_offset= m_new_field_offset_calculated;
      //m_field_offset += m_row_event->n_bits_len;
      m_new_field_offset_calculated= 0;
      if (m_field_offset >= m_row_event->row.size() - 1)
        m_field_offset= 0;
      return *this;
    }

    /*
     * Advance the field offset to the next row
     */
    int row_field_col_index= 0;
    std::vector<uint8_t> nullbits(m_row_event->get_null_bits_len());
    std::copy(m_row_event->row.begin() + m_field_offset,
              m_row_event->row.begin() +
              (m_field_offset + m_row_event->get_null_bits_len()),
              nullbits.begin());
    m_field_offset += m_row_event->get_null_bits_len();
    for (unsigned col_no= 0; col_no < m_table_map->m_colcnt; ++col_no)
    {
      ++row_field_col_index;
      binary_log::Value val((enum_field_types)m_table_map->m_coltype[col_no],
                            m_table_map->m_field_metadata[col_no],
                            (const unsigned char*)(&m_row_event->row[m_field_offset]));
      int is_null= (nullbits[col_no/8] >> (col_no % 8)) & 0x01;
      if (!is_null)
      {
        m_field_offset += val.length();
      }
    }

    return *this;
  }

  m_field_offset= 0;
  return *this;
}


template <class Iterator_value_type >
Row_event_iterator< Iterator_value_type >
  Row_event_iterator< Iterator_value_type >::operator++(int)
{ // postﬁx
  Row_event_iterator temp = *this;
  ++*this;
  return temp;
}


template <class Iterator_value_type >
bool Row_event_iterator< Iterator_value_type >::
     operator==(const Row_event_iterator& x) const
{
  return m_field_offset == x.m_field_offset;
}


template <class Iterator_value_type >
bool Row_event_iterator< Iterator_value_type >::
     operator!=(const Row_event_iterator& x) const
{
  return m_field_offset != x.m_field_offset;
}


template <class Iterator_value_type>
uint32_t Row_event_iterator<Iterator_value_type>::
       fields(Iterator_value_type& fields_vector)
{
  uint32_t field_offset= m_field_offset;
  int null_bit_index= 0;;
  std::vector<uint8_t> nullbits(m_row_event->get_null_bits_len());
  std::copy(m_row_event->row.begin() + m_field_offset,
            m_row_event->row.begin() +
            (m_field_offset + m_row_event->get_null_bits_len()),
            nullbits.begin());

  field_offset += (get_total_set_bit(m_row_event->columns_before_image,
                                     m_table_map->m_colcnt) + 7) / 8;
  for (unsigned col_no= 0; col_no < m_table_map->m_colcnt; ++col_no)
  {
    unsigned int type= m_table_map->m_coltype[col_no] & 0xFF;
    uint32_t metadata= extract_metadata(m_table_map, col_no);

    if (!((m_row_event->columns_before_image[col_no / 8] >> col_no % 8) & 0x01))
    {
       /*
        Here we create the default constructor of Value, this is a placeholder
        and it is required to print the correct row number in case of Delete
        event.
        Lets say if rows 1,2 and 4 are modified by a delete event in a table,
        then for row 3 we will have this placeholder,
        so that while printing we have this format.
        @1= old_value
        @2= old_value
        @4= old_value

        If we will not create the placeholder than the output will be like
        @1= old_value
        @2= old_value
        @3= old_value

        Notice the last row in both cases.
       */
       binary_log::Value val;
       val.set_exists_bit(false);
       fields_vector.push_back(val);
       continue;
    }
    binary_log::Value val((enum_field_types)type, metadata,
                          (const unsigned char*)(&m_row_event->row[field_offset]));

    int is_null= (nullbits[null_bit_index/8] >> (null_bit_index % 8)) & 0x01;
    if (is_null)
    {
      val.set_null_bit(true);
    }
    if (!is_null)
    {
      if (val.length() == UINT_MAX)
        throw std::logic_error("Field type is unrecognized");
      field_offset += val.length();
    }
    null_bit_index++;
    fields_vector.push_back(val);
  }
  return field_offset;
}

/*
  Explicit instantiation of Row_event_iterator, so that the compiler finds
  the implementaion for member methods the template Row_event_iterator,
  implememnted in this source file.
*/
template class Row_event_iterator<binary_log::Row_of_fields>;
} // end namespace binary_log
