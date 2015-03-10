/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.3
 * \file
 * \brief A collector for agent tuning options.
 */

#pragma once

#include <so_5/rt/h/subscription_storage_fwd.hpp>
#include <so_5/rt/h/message_limit.hpp>

namespace so_5
{

namespace rt
{

/*
 * NOTE: copy and move constructors and copy operator is implemented
 * because Visual C++ 12.0 (MSVS2013) doesn't generate it by itself.
 */
//
// agent_tuning_options_t
//
/*!
 * \since v.5.5.3
 * \brief A collector for agent tuning options.
 */
class agent_tuning_options_t
	{
	public :
		agent_tuning_options_t()
			{}
		agent_tuning_options_t(
			const agent_tuning_options_t & o )
			:	m_subscription_storage_factory( o.m_subscription_storage_factory )
			,	m_message_limits( o.m_message_limits )
			{}
		agent_tuning_options_t(
			agent_tuning_options_t && o )
			:	m_subscription_storage_factory(
					std::move( o.m_subscription_storage_factory ) )
			,	m_message_limits( std::move( o.m_message_limits ) )
			{}

		void
		swap( agent_tuning_options_t & o )
			{
				std::swap( m_subscription_storage_factory,
						o.m_subscription_storage_factory );
				std::swap( m_message_limits, o.m_message_limits );
			}

		agent_tuning_options_t &
		operator=( agent_tuning_options_t o )
			{
				swap( o );
				return *this;
			}

		//! Set factory for subscription storage creation.
		agent_tuning_options_t &
		subscription_storage_factory(
			subscription_storage_factory_t factory )
			{
				m_subscription_storage_factory = std::move( factory );

				return *this;
			}

		const subscription_storage_factory_t &
		query_subscription_storage_factory() const
			{
				return m_subscription_storage_factory;
			}

		//! Default subscription storage factory.
		static subscription_storage_factory_t
		default_subscription_storage_factory()
			{
				return so_5::rt::default_subscription_storage_factory();
			}

		message_limit::description_container_t
		giveout_message_limits()
			{
				return std::move( m_message_limits );
			}

		template< typename... ARGS >
		agent_tuning_options_t &
		message_limits( ARGS &&... args )
			{
				message_limit::accept_indicators(
						m_message_limits,
						std::forward< ARGS >( args )... );

				return *this;
			}

	private :
		subscription_storage_factory_t m_subscription_storage_factory =
				default_subscription_storage_factory();

		message_limit::description_container_t m_message_limits;
	};

} /* namespace rt */

} /* namespace so_5 */

namespace std
{

inline void
swap(
	so_5::rt::agent_tuning_options_t & a,
	so_5::rt::agent_tuning_options_t & b )
	{
		a.swap( b );
	}

} /* namespace std */

