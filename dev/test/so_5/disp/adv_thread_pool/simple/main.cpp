/*
 * A simple test for thread_pool dispatcher.
 */

#include <iostream>
#include <map>
#include <exception>
#include <stdexcept>
#include <cstdlib>
#include <thread>
#include <chrono>

#include <so_5/all.hpp>

#include <various_helpers_1/time_limited_execution.hpp>

struct msg_hello : public so_5::rt::signal_t {};

class a_test_t : public so_5::rt::agent_t
{
	public:
		a_test_t(
			so_5::rt::environment_t & env )
			:	so_5::rt::agent_t( env )
		{}

		virtual void
		so_define_agent()
		{
			so_subscribe_self().event< msg_hello >( &a_test_t::evt_hello );
		}

		virtual void
		so_evt_start()
		{
			so_direct_mbox()->deliver_signal< msg_hello >();
		}

		void
		evt_hello()
		{
			so_environment().stop();
		}
};

int
main( int argc, char * argv[] )
{
	try
	{
		run_with_time_limit(
			[]()
			{
				so_5::launch(
					[]( so_5::rt::environment_t & env )
					{
						env.register_agent_as_coop(
								"test",
								new a_test_t( env ),
								so_5::disp::adv_thread_pool::create_disp_binder(
										"thread_pool",
										so_5::disp::adv_thread_pool::params_t() ) );
					},
					[]( so_5::rt::environment_params_t & params )
					{
						params.add_named_dispatcher(
								"thread_pool",
								so_5::disp::adv_thread_pool::create_disp( 4 ) );
					} );
			},
			5,
			"simple adv_thread_pool dispatcher test" );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

