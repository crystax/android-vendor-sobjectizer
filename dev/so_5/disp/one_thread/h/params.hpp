/*
	SObjectizer 5.
*/

/*!
 * \since v.5.5.10
 * \file
 * \brief Parameters for %one_thread dispatcher.
 *
 * \note Parameters are described in separate header file to avoid
 * header file cycles (parameters needs for environment_params_t declared
 * in environment.hpp, which can be included in dispatcher definition file).
 */

#pragma once

#include <so_5/disp/mpsc_queue_traits/h/pub.hpp>

namespace so_5
{

namespace disp
{

namespace one_thread
{

/*!
 * \since v.5.5.10
 * \brief Alias for namespace with traits of event queue.
 */
namespace queue_traits = so_5::disp::mpsc_queue_traits;

//
// disp_params_t
//
/*!
 * \since v.5.5.10
 * \brief Parameters for one thread dispatcher.
 */
class disp_params_t
	{
	public :
		//! Default constructor.
		disp_params_t() {}
		//! Copy constructor.
		disp_params_t( const disp_params_t & o )
			:	m_queue_params{ o.m_queue_params }
			{}
		//! Move constructor.
		disp_params_t( disp_params_t && o )
			:	m_queue_params{ std::move(o.m_queue_params) }
			{}

		friend inline void swap( disp_params_t & a, disp_params_t & b )
			{
				swap( a.m_queue_params, b.m_queue_params );
			}

		//! Copy operator.
		disp_params_t & operator=( const disp_params_t & o )
			{
				disp_params_t tmp{ o };
				swap( *this, tmp );
				return *this;
			}
		//! Move operator.
		disp_params_t & operator=( disp_params_t && o )
			{
				disp_params_t tmp{ std::move(o) };
				swap( *this, tmp );
				return *this;
			}

		//! Setter for queue parameters.
		disp_params_t &
		set_queue_params( queue_traits::queue_params_t p )
			{
				m_queue_params = std::move(p);
				return *this;
			}

		//! Tuner for queue parameters.
		/*!
		 * Accepts lambda-function or functional object which tunes
		 * queue parameters.
			\code
			so_5::disp::one_thread::create_private_disp( env,
				"my_one_thread_disp",
				so_5::disp::one_thread::disp_params_t{}.tune_queue_params(
					[]( so_5::disp::one_thread::queue_traits::queue_params_t & p ) {
						p.lock_factory( so_5::disp::one_thread::queue_traits::simple_lock_factory() );
					} ) );
			\endcode
		 */
		template< typename L >
		disp_params_t &
		tune_queue_params( L tunner )
			{
				tunner( m_queue_params );
				return *this;
			}

		//! Getter for queue parameters.
		const queue_traits::queue_params_t &
		queue_params() const
			{
				return m_queue_params;
			}

	private :
		//! Queue parameters.
		queue_traits::queue_params_t m_queue_params;
	};

} /* namespace one_thread */

} /* namespace disp */

} /* namespace so_5 */


