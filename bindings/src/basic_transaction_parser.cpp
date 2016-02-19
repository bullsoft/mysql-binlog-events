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

#include "binlog_event.h"
#include "basic_transaction_parser.h"
#include "value.h"
#include "field_iterator.h"
#include <iostream>

namespace binary_log {

binary_log::Binary_log_event *Basic_transaction_parser::
process_event(binary_log::Query_event *qev)
{
  if (strncmp(qev->query, "BEGIN", strlen("BEGIN")) == 0)
  {
    m_transaction_state= STARTING;
  }
  else if (strncmp(qev->query, "COMMIT", strlen("COMMIT")) == 0)
  {
    m_transaction_state= COMMITTING;
  }

  return process_transaction_state(qev);
}

binary_log::Binary_log_event *Basic_transaction_parser::
process_event(binary_log::Xid_event *ev)
{
  m_transaction_state= COMMITTING;
  return process_transaction_state(ev);
}

binary_log::Binary_log_event *Basic_transaction_parser::
process_event(binary_log::Table_map_event *ev)
{
  if(m_transaction_state == IN_PROGRESS)
  {
    m_event_stack.push_back(ev);
    return 0;
  }
  return ev;
}

binary_log::Binary_log_event *Basic_transaction_parser::
process_event(binary_log::Rows_event *ev)
{
  if(m_transaction_state == IN_PROGRESS)
  {
    m_event_stack.push_back(ev);
    return 0;
  }
  return ev;
}

binary_log::Binary_log_event *Basic_transaction_parser::
process_transaction_state(binary_log::Binary_log_event *incomming_event)
{
  switch(m_transaction_state)
  {
    case STARTING:
    {
      m_transaction_state= IN_PROGRESS;
      m_start_time= incomming_event->header()->when.tv_sec;
      delete incomming_event;// drop the begin event
      return 0;
    }
    case COMMITTING:
    {
      delete incomming_event; // drop the commit event

      /**
       * Propagate the start time for the transaction to the newly created
       * event.
       */
      binary_log::Transaction_log_event *trans=
      binary_log::create_transaction_log_event();
      trans->header()->when.tv_sec= m_start_time;

      while( m_event_stack.size() > 0)
      {
        binary_log::Binary_log_event *event= m_event_stack.front();
        m_event_stack.pop_front();
        switch(event->get_event_type())
        {
          case binary_log::TABLE_MAP_EVENT:
          {
            /*
             Index the table name with a table id to ease lookup later.
            */
            binary_log::Table_map_event *tm=
            static_cast<binary_log::Table_map_event *>(event);
            trans->m_table_map.insert(binary_log::
                                      Event_index_element(tm->m_table_id,tm));
            trans->m_events.push_back(event);
          }
          break;
          case binary_log::WRITE_ROWS_EVENT:
          case binary_log::WRITE_ROWS_EVENT_V1:
          case binary_log::DELETE_ROWS_EVENT:
          case binary_log::DELETE_ROWS_EVENT_V1:
          case binary_log::UPDATE_ROWS_EVENT:
          case binary_log::UPDATE_ROWS_EVENT_V1:
          {
            trans->m_events.push_back(event);
             /*
             * Propagate last known next position
             */
            trans->header()->log_pos= event->header()->log_pos;
          }
          break;
          default:
            delete event;
         }
      } // end while
      m_transaction_state= NOT_IN_PROGRESS;
      return(trans);
    }
    case NOT_IN_PROGRESS:
    default:
      return incomming_event;
  }

}

Transaction_log_event *create_transaction_log_event(void)
{
    Transaction_log_event *trans= new Transaction_log_event();
    //This is a hack, to define an event which is not in the stream currently.
    trans->header()->type_code= binary_log::ENUM_END_EVENT;
    return trans;
};

Transaction_log_event::~Transaction_log_event()
{
  Int_to_Event_map::iterator it;
  for(it = m_table_map.begin(); it != m_table_map.end();)
  {
    /* No need to delete the event here; it happens in the next iteration */
    m_table_map.erase(it++);
  }

  while (m_events.size() > 0)
  {
    Binary_log_event *event= m_events.back();
    m_events.pop_back();
    delete(event);
  }

}

} // end namespace
