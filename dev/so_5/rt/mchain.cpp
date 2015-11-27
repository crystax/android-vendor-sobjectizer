/*
 * SObjectizer-5
 */

/*!
 * \since v.5.5.13
 * \file
 * \brief Public part of message chain related stuff.
 */

#include <so_5/rt/h/mchain.hpp>

namespace so_5 {

namespace rt {

//
// abstract_message_chain_t
//
abstract_message_chain_t::abstract_message_chain_t()
	{}
abstract_message_chain_t::~abstract_message_chain_t()
	{}

mbox_t
abstract_message_chain_t::as_mbox()
	{
		return mbox_t{ this };
	}

} /* namespace rt */

} /* namespace so_5 */

