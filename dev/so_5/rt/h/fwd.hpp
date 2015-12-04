/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.13
 * \file
 * \brief Forward declaration for SObjectizer run-time related classes.
 */

#pragma once

namespace so_5 {

namespace impl
{

class state_listener_controller_t;
class mpsc_mbox_t;
struct event_handler_data_t;
class delivery_filter_storage_t;

} /* namespace impl */

class state_t;
class environment_t;
class coop_t;
class agent_t;

} /* namespace so_5 */

