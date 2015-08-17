/*
 * SObjectizer-5
 */

/*!
 * \since v.5.4.0
 * \file
 * \brief Helpers for disp_binder implementation.
 */

#pragma once

#include <so_5/h/ret_code.hpp>
#include <so_5/h/exception.hpp>

#include <so_5/rt/h/environment.hpp>

#include <functional>
#include <type_traits>
#include <utility>
#include <string>

namespace so_5
{

namespace disp
{

namespace reuse
{

/*!
 * \since v.5.5.4
 * \brief A helper method for casing dispatcher to the specified type
 * and performing some action with it.
 */
template< class DISPATCHER, class ACTION > 
auto
do_with_dispatcher_of_type(
	so_5::rt::dispatcher_t * disp_pointer,
	const std::string & disp_name,
	ACTION action )
	-> decltype(action(*static_cast<DISPATCHER *>(nullptr)))
	{
		// It should be our dispatcher.
		DISPATCHER * disp = dynamic_cast< DISPATCHER * >(
				disp_pointer );

		if( nullptr == disp )
			SO_5_THROW_EXCEPTION(
					rc_disp_type_mismatch,
					"type of dispatcher with name '" + disp_name +
					"' is not '" + typeid(DISPATCHER).name() + "'" );

		return action( *disp );
	}

/*!
 * \since v.5.4.0
 * \brief A helper method for extracting dispatcher by name,
 * checking its type and to some action.
 */
template< class DISPATCHER, class ACTION > 
auto
do_with_dispatcher(
	so_5::rt::environment_t & env,
	const std::string & disp_name,
	ACTION action )
	-> decltype(action(*static_cast<DISPATCHER *>(nullptr)))
	{
		so_5::rt::dispatcher_ref_t disp_ref = env.query_named_dispatcher(
				disp_name );

		// If the dispatcher is found then the agent should be bound to it.
		if( !disp_ref.get() )
			SO_5_THROW_EXCEPTION(
					rc_named_disp_not_found,
					"dispatcher with name '" + disp_name + "' not found" );

		return do_with_dispatcher_of_type< DISPATCHER >(
				disp_ref.get(),
				disp_name,
				action );
	}

//
// binder_for_public_disp_template_t
//
/*!
 * \since v.5.5.8
 * \brief A template of binder for a named dispatcher.
 *
 * \tparam DISPATCHER type of a dispatcher.
 * \tparam BINDER_MIXIN type of a mixin with implementation of
 * do_bind and do_unbind methods.
 * \tparam BINDER_MIXIN_ARGS arguments for constructor of BINDER_MIXIN class.
 */
template<
	typename DISPATCHER,
	typename BINDER_MIXIN >
class binder_for_public_disp_template_t
	:	public so_5::rt::disp_binder_t
	,	protected BINDER_MIXIN
	{
	public:
		template< typename... BINDER_MIXIN_ARGS >
		binder_for_public_disp_template_t(
			std::string disp_name,
			BINDER_MIXIN_ARGS &&... args )
			:	BINDER_MIXIN( std::forward<BINDER_MIXIN_ARGS>(args)... )
			,	m_disp_name( std::move( disp_name ) )
			{}

		virtual so_5::rt::disp_binding_activator_t
		bind_agent(
			so_5::rt::environment_t & env,
			so_5::rt::agent_ref_t agent ) override
			{
				return do_with_dispatcher< DISPATCHER >(
					env,
					m_disp_name,
					[this, agent]( DISPATCHER & disp )
					{
						return this->do_bind( disp, std::move( agent ) );
					} );
			}

		virtual void
		unbind_agent(
			so_5::rt::environment_t & env,
			so_5::rt::agent_ref_t agent ) override
			{
				using namespace so_5::disp::reuse;

				do_with_dispatcher< DISPATCHER >( env, m_disp_name,
					[this, agent]( DISPATCHER & disp )
					{
						this->do_unbind( disp, std::move( agent ) );
					} );
			}

	private:
		//! Name of the dispatcher to be bound to.
		const std::string m_disp_name;
	};

} /* namespace reuse */

} /* namespace disp */

} /* namespace so_5 */


