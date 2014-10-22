/*
	SObjectizer 5.
*/

/*!
	\since v.5.2.3 
	\file
	\brief Agent cooperation notifications definition.
*/

#pragma once

#include <so_5/rt/h/agent_coop.hpp>
#include <so_5/rt/h/mbox.hpp>

#if defined( SO_5_MSVC )
	#pragma warning(push)
	#pragma warning(disable: 4251)
#endif

namespace so_5
{

namespace rt
{

//
// msg_coop_registered
//
/*!
 * \since v.5.2.3
 * \brief Message about cooperation registration completion.
 */
struct SO_5_TYPE msg_coop_registered : public message_t
{
	msg_coop_registered(
		const std::string & coop_name );
	virtual ~msg_coop_registered();

	const std::string m_coop_name;
};

//
// msg_coop_deregistered
//
/*!
 * \since v.5.2.3
 * \brief Message about cooperation deregistration completion.
 */
struct SO_5_TYPE msg_coop_deregistered : public message_t
{
	msg_coop_deregistered(
		const std::string & coop_name,
		coop_dereg_reason_t reason );
	virtual ~msg_coop_deregistered();

	const std::string m_coop_name;
	const coop_dereg_reason_t m_reason;
};

//
// make_coop_reg_notificator
//
/*!
 * \since v.5.2.3
 * \brief Create notificator about cooperation registration completion.
 */
SO_5_FUNC coop_reg_notificator_t
make_coop_reg_notificator(
	//! A mbox to which msg_coop_registered will be sent.
	const mbox_t & mbox );

//
// make_coop_dereg_notificator
//
/*!
 * \since v.5.2.3
 * \brief Create notificator about cooperation deregistration completion.
 */
SO_5_FUNC coop_dereg_notificator_t
make_coop_dereg_notificator(
	//! A mbox to which msg_coop_deregistered will be sent.
	const mbox_t & mbox );

} /* namespace rt */

} /* namespace so_5 */

#if defined( SO_5_MSVC )
	#pragma warning(pop)
#endif

