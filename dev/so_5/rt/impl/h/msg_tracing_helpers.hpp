/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.9
 * \file
 * \brief Various helpers for message delivery tracing stuff.
 */

#pragma once

#include <so_5/h/msg_tracing.hpp>

#include <so_5/rt/h/mbox.hpp>
#include <so_5/rt/h/agent.hpp>

#include <so_5/rt/impl/h/internal_env_iface.hpp>

#include <so_5/details/h/invoke_noexcept_code.hpp>

#include <sstream>

namespace so_5 {

namespace rt {

namespace impl {

namespace msg_tracing_helpers {

namespace details {

struct overlimit_deep_t
	{
		unsigned int m_deep;
	};

struct mbox_identification_t
	{
		mbox_id_t m_id;
	};

struct composed_action_name_t
	{
		const char * m_1;
		const char * m_2;
	};

inline void
make_trace_to_1( std::ostream & s, mbox_identification_t id )
	{
		s << "[mbox_id=" << id.m_id << "]";
	}

inline void
make_trace_to_1( std::ostream & s, const abstract_message_box_t & mbox )
	{
		make_trace_to_1( s, mbox_identification_t{ mbox.id() } );
	}

inline void
make_trace_to_1( std::ostream & s, const std::type_index & msg_type )
	{
		s << "[msg_type=" << msg_type.name() << "]";
	}

inline void
make_trace_to_1( std::ostream & s, const agent_t * agent )
	{
		s << "[agent_ptr=" << agent << "]";
	}

inline void
make_trace_to_1( std::ostream & s, const state_t * state )
	{
		s << "[state=" << state->query_name() << "]";
	}

inline void
make_trace_to_1( std::ostream & s, const event_handler_data_t * handler )
	{
		s << "[evt_handler=";
		if( handler )
			s << handler;
		else
			s << "NONE";
		s << "]";
	}

inline void
make_trace_to_1(
	std::ostream & s,
	const so_5::rt::message_limit::control_block_t * limit )
	{
		s << "[limit_ptr=" << limit << "]";
	}

inline void
make_trace_to_1( std::ostream & s, const message_ref_t & message )
	{
		s << "[msg_ptr=" << message.get() << "]";
	}

inline void
make_trace_to_1( std::ostream & s, const overlimit_deep_t limit )
	{
		s << "[overlimit_deep=" << limit.m_deep << "]";
	}

inline void
make_trace_to_1( std::ostream & s, const composed_action_name_t name )
	{
		s << " " << name.m_1 << "." << name.m_2 << " ";
	}

inline void
make_trace_to( std::ostream & ) {}

template< typename A, typename... OTHER >
void
make_trace_to( std::ostream & s, A && a, OTHER &&... other )
	{
		make_trace_to_1( s, std::forward< A >(a) );
		make_trace_to( s, std::forward< OTHER >(other)... );
	}

template< typename... ARGS >
void
make_trace(
	so_5::msg_tracing::tracer_t & tracer,
	ARGS &&... args ) SO_5_NOEXCEPT
	{
		so_5::details::invoke_noexcept_code( [&] {
				std::ostringstream s;

				s << "[tid=" << query_current_thread_id() << "]";

				make_trace_to( s, std::forward< ARGS >(args)... );

				tracer.trace( s.str() );
			} );
	}

} /* namespace details */

//
// tracing_disabled_base_t
//
/*!
 * \since v.5.5.9
 * \brief Base class for an mbox for the case when message delivery
 * tracing is disabled.
 */
struct tracing_disabled_base_t
	{
		class deliver_op_tracer_t
			{
			public :
				deliver_op_tracer_t(
					const tracing_disabled_base_t &,
					const abstract_message_box_t &,
					const char *,
					const std::type_index &,
					const message_ref_t &,
					const unsigned int )
					{}

				void
				no_subscribers() const {}

				void
				push_to_queue( const agent_t * ) const {}

				void
				message_rejected(
					const agent_t *,
					const delivery_possibility_t ) const {}
			};
	};

//
// tracing_enabled_base_t
//
/*!
 * \since v.5.5.9
 * \brief Base class for an mbox for the case when message delivery
 * tracing is enabled.
 */
class tracing_enabled_base_t
	{
	private :
		so_5::msg_tracing::tracer_t & m_tracer;

	public :
		tracing_enabled_base_t( so_5::msg_tracing::tracer_t & tracer )
			:	m_tracer{ tracer }
			{}

		so_5::msg_tracing::tracer_t &
		tracer() const
			{
				return m_tracer;
			}

		class deliver_op_tracer_t
			{
			private :
				so_5::msg_tracing::tracer_t & m_tracer;
				const abstract_message_box_t & m_mbox;
				const char * m_op_name;
				const std::type_index & m_msg_type;
				const message_ref_t & m_message;
				const details::overlimit_deep_t m_overlimit_deep;

			public :
				deliver_op_tracer_t(
					const tracing_enabled_base_t & tracing_base,
					const abstract_message_box_t & mbox,
					const char * op_name,
					const std::type_index & msg_type,
					const message_ref_t & message,
					const unsigned int overlimit_reaction_deep )
					:	m_tracer{ tracing_base.tracer() }
					,	m_mbox{ mbox }
					,	m_op_name{ op_name }
					,	m_msg_type{ msg_type }
					,	m_message{ message }
					,	m_overlimit_deep{ overlimit_reaction_deep }
					{
					}

				void
				no_subscribers() const
					{
						details::make_trace(
								m_tracer,
								m_mbox,
								details::composed_action_name_t{
										m_op_name, "no_subscribers" },
								m_msg_type,
								m_message,
								m_overlimit_deep );
					}

				void
				push_to_queue( const agent_t * subscriber ) const
					{
						details::make_trace(
								m_tracer,
								m_mbox,
								details::composed_action_name_t{
										m_op_name, "push_to_queue" },
								m_msg_type,
								m_message,
								m_overlimit_deep,
								subscriber );
					}

				void
				message_rejected(
					const agent_t * subscriber,
					const delivery_possibility_t status ) const
					{
						if( delivery_possibility_t::disabled_by_delivery_filter
								== status )
							{
								details::make_trace(
										m_tracer,
										m_mbox,
										details::composed_action_name_t{
												m_op_name, "message_rejected" },
										m_msg_type,
										m_message,
										m_overlimit_deep,
										subscriber );
							}
					}

			};
	};

/*!
 * \since v.5.5.9
 * \brief Helper for tracing the result of event handler search.
 */
inline void
trace_event_handler_search_result(
	const execution_demand_t & demand,
	const char * context_marker,
	const event_handler_data_t * search_result )
	{
		details::make_trace(
			internal_env_iface_t{ demand.m_receiver->so_environment() }.msg_tracer(),
			demand.m_receiver,
			details::composed_action_name_t{ context_marker, "find_handler" },
			details::mbox_identification_t{ demand.m_mbox_id },
			demand.m_msg_type,
			demand.m_message_ref,
			&(demand.m_receiver->so_current_state()),
			search_result );
	}

} /* namespace msg_tracing_helpers */

} /* namespace impl */

} /* namespace rt */

} /* namespace so_5 */

