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
#include "basic_content_handler.h"
#include <cassert>

namespace binary_log {

Content_handler::Content_handler () {}
Content_handler::~Content_handler () {}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Query_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Rows_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Table_map_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Xid_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::User_var_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Incident_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Rotate_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Intvar_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Format_description_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Start_event_v3 *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Stop_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Ignorable_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Rows_query_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Gtid_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Previous_gtids_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Heartbeat_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Unknown_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Execute_load_query_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Load_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Create_file_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Delete_file_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Execute_load_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Append_block_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Begin_load_query_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Rand_event *ev)
{
  return ev;
}
binary_log::Binary_log_event *Content_handler::
process_event(binary_log::Binary_log_event *ev)
{
  return ev;
}

Injection_queue *Content_handler::
get_injection_queue(void)
{
  return m_reinject_queue;
}

void Content_handler::
set_injection_queue(Injection_queue *queue)
{
  m_reinject_queue= queue;
}

binary_log::Binary_log_event*
Content_handler::internal_process_event(binary_log::Binary_log_event *ev)
{
 binary_log::Binary_log_event *processed_event= 0;
 switch(ev->header ()->type_code) {
 case QUERY_EVENT:
   processed_event= process_event(static_cast<binary_log::Query_event*>(ev));
   break;
 case WRITE_ROWS_EVENT:
 case WRITE_ROWS_EVENT_V1:
 case UPDATE_ROWS_EVENT:
 case UPDATE_ROWS_EVENT_V1:
 case DELETE_ROWS_EVENT:
 case DELETE_ROWS_EVENT_V1:
   processed_event= process_event(static_cast<binary_log::Rows_event*>(ev));
   break;
 case USER_VAR_EVENT:
   processed_event= process_event(static_cast<binary_log::User_var_event *>(ev));
   break;
 case ROTATE_EVENT:
   processed_event= process_event(static_cast<binary_log::Rotate_event *>(ev));
   break;
 case INCIDENT_EVENT:
   processed_event= process_event(static_cast<binary_log::Incident_event *>(ev));
   break;
 case XID_EVENT:
   processed_event= process_event(static_cast<binary_log::Xid_event *>(ev));
   break;
 case TABLE_MAP_EVENT:
   processed_event= process_event(static_cast<binary_log::Table_map_event *>(ev));
   break;
 case FORMAT_DESCRIPTION_EVENT:
   if (ev != NULL)
     ev= dynamic_cast<binary_log::Format_description_event*>(ev);
   processed_event= process_event(ev);
   break;
 case BEGIN_LOAD_QUERY_EVENT:
   if (ev != NULL)
     ev= dynamic_cast<binary_log::Begin_load_query_event*>(ev);
   processed_event= process_event(ev);
   break;
 case EXECUTE_LOAD_QUERY_EVENT:
   if (ev != NULL)
     ev= dynamic_cast<binary_log::Execute_load_query_event*>(ev);
   processed_event= process_event(ev);
   break;
 case INTVAR_EVENT:
   processed_event= process_event(static_cast<binary_log::Intvar_event*>(ev));
   break;
 case STOP_EVENT:
   processed_event= process_event(static_cast<binary_log::Stop_event*>(ev));
   break;
 case RAND_EVENT:
   processed_event= process_event(static_cast<binary_log::Rand_event*>(ev));
   break;
 default:
   processed_event= process_event(ev);
   break;
 }
 return processed_event;
}


binary_log::Binary_log_event*
Content_stream_handler::handle_event(binary_log::Binary_log_event **event)
{
  // A NULL pointer should not be passed to the method
  assert (*event != NULL);

  Content_handler_pipeline::iterator it= m_content_handlers.begin();

  for(; it != m_content_handlers.end(); it++)
  {
    if (*event)
      *event= (*it)->internal_process_event(*event);
  }
  return *event;
}

} // end namespace
