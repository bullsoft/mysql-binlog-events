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
#ifndef BASIC_CONTENT_HANDLER_INCLUDED
#define	BASIC_CONTENT_HANDLER_INCLUDED

#include "binary_log.h"

namespace binary_log {

class Injection_queue : public std::list<binary_log::Binary_log_event * >
{
public:
    Injection_queue() : std::list<binary_log::Binary_log_event * >() {}
    ~Injection_queue() {}
};

/**
 * A content handler accepts an event and returns the same event,
 * a new one or 0 (the event was consumed by the content handler).
 * The default behaviour is to return the event unaffected.
 * The generic event handler is used for events which aren't routed to
 * a dedicated member function, user defined events being the most
 * common case.
 */

class Content_handler {
public:
  Content_handler();
  Content_handler(const binary_log::Content_handler& orig);
  virtual ~Content_handler();

  virtual binary_log::Binary_log_event *process_event(binary_log::Query_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Rows_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Table_map_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Xid_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::User_var_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Incident_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Rotate_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Intvar_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Format_description_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Start_event_v3 *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Stop_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Ignorable_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Rows_query_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Rand_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Gtid_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Previous_gtids_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Heartbeat_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Unknown_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Execute_load_query_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Load_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Create_file_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Delete_file_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Execute_load_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Append_block_event *ev);
  virtual binary_log::Binary_log_event *process_event(binary_log::Begin_load_query_event *ev);

  /**
    Process any event which hasn't been registered yet.
   */
  virtual binary_log::Binary_log_event *process_event(binary_log::Binary_log_event *ev);

protected:
  /**
   * The Injection queue is emptied before any new event is pulled from
   * the Binary_log_driver. Injected events will pass through all content
   * handlers. The Injection_queue is a derived std::list.
   */
  Injection_queue *get_injection_queue();

private:
  Injection_queue *m_reinject_queue;
  void set_injection_queue(Injection_queue *injection_queue);
  binary_log::Binary_log_event *internal_process_event(binary_log::Binary_log_event *ev);

  friend class Content_stream_handler;
};

/**
  This class will maintain the list of registered content handlers.
  The default content handlers return the event unchanged. User application
  may use the content_stream_handler to register content handlers per event
  to process and modify the events.
*/
class Content_stream_handler
{
public:
  typedef std::list<Content_handler *> Content_handler_pipeline;
  /**
    Adds content handlers to the list
  */
  bool add_listener(Content_handler& handler)
  {
    m_content_handlers.push_back(&handler);
  }

  Content_handler_pipeline *get_content_handler_pipeline()
  {
    return &m_content_handlers;
  }

  /**
    Iterates over the registered content handlers, and calls the appropriate
    handler depending on the event type.
  */
  Binary_log_event* handle_event(Binary_log_event **event);

private:
  /**
    Inserts/removes content handlers in and out of the chain
  */
  Content_handler_pipeline m_content_handlers;
};
} // end namespace
#endif	/* BASIC_CONTENT_HANDLER_INCLUDED */
