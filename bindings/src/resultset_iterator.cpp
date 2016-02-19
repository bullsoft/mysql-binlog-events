/*
Copyright (c) 2011, 2014, Oracle and/or its affiliates. All rights
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

#include "resultset_iterator.h"
#include "row_of_fields.h"
#include <sstream>

using namespace binary_log;

namespace binary_log {

Result_set::iterator Result_set::begin()
{
  return iterator(this);
}
Result_set::iterator Result_set::end()
{
  return iterator();
}
Result_set::const_iterator Result_set::begin() const
{
  return const_iterator(const_cast<Result_set *>(this));
}
Result_set::const_iterator Result_set::end() const
{
  return const_iterator();
}
} // end namespace system, binary_log
