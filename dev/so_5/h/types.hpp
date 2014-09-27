/*
	SObjectizer 5.
*/

/*!
	\file
	\brief Various typedefs.
*/

#if !defined( _SO_5__TYPES_HPP_ )
#define _SO_5__TYPES_HPP_

#include <atomic>
#include <cstdint>

namespace so_5
{

//! Atomic counter type.
typedef std::atomic_ulong atomic_counter_t;


//! Atomic flag type.
typedef std::atomic_ulong atomic_flag_t;

//! A type for mbox indentifier.
typedef unsigned long long mbox_id_t;

/*!
 * \since v.5.4.0
 * \brief Thread safety indicator.
 */
enum class thread_safety_t : std::uint8_t
	{
		//! Not thread safe.
		unsafe = 0,
		//! Thread safe.
		safe = 1
	};

/*!
 * \since v.5.4.0
 * \brief Shorthand for thread unsafety indicator.
 */
const thread_safety_t not_thread_safe = thread_safety_t::unsafe;

/*!
 * \since v.5.4.0
 * \brief Shorthand for thread safety indicator.
 */
const thread_safety_t thread_safe = thread_safety_t::safe;

}

#endif
