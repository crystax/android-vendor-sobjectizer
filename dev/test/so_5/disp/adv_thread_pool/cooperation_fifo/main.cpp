/*
 * A simple test for adv_thread_pool dispatcher.
 */

#include <iostream>
#include <set>
#include <vector>
#include <exception>
#include <stdexcept>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <sstream>

#include <so_5/all.hpp>

#include <various_helpers_1/time_limited_execution.hpp>

struct msg_shutdown : public so_5::rt::signal_t {};

class a_test_t : public so_5::rt::agent_t
{
	public:
		a_test_t(
			so_5::rt::environment_t & env,
			const so_5::rt::mbox_ref_t & shutdowner_mbox )
			:	so_5::rt::agent_t( env )
			,	m_shutdowner_mbox( shutdowner_mbox )
		{
		}

		void
		so_evt_start()
		{
			auto w = ++m_workers;
			if( w > 1 )
			{
				std::cerr << "too many workers: " << w << std::endl;
			}

			std::this_thread::sleep_for( std::chrono::milliseconds( 250 ) );

			w = --m_workers;
			if( w != 0 )
			{
				std::cout << "expected no more workers, but: " << w << std::endl;
			}

			m_shutdowner_mbox->deliver_signal< msg_shutdown >();
		}

	private :
		static std::atomic_uint m_workers;

		const so_5::rt::mbox_ref_t m_shutdowner_mbox;
};

std::atomic_uint a_test_t::m_workers = ATOMIC_VAR_INIT( 0 );

class a_shutdowner_t : public so_5::rt::agent_t
{
	public :
		a_shutdowner_t(
			so_5::rt::environment_t & env,
			std::size_t working_agents )
			:	so_5::rt::agent_t( env )
			,	m_working_agents( working_agents )
		{}

		virtual void
		so_define_agent()
		{
			so_subscribe( so_direct_mbox() )
				.event( so_5::signal< msg_shutdown >,
					[this]() {
						--m_working_agents;
						if( !m_working_agents )
							so_environment().stop();
					} );
		}

	private :
		std::size_t m_working_agents;
};

const std::size_t thread_count = 4;

void
run_sobjectizer()
{
	so_5::launch(
		[&]( so_5::rt::environment_t & env )
		{
			so_5::rt::mbox_ref_t shutdowner_mbox;
			{
				auto c = env.create_coop( "shutdowner" );
				auto a = c->add_agent( new a_shutdowner_t( env, thread_count ) );
				shutdowner_mbox = a->so_direct_mbox();
				env.register_coop( std::move( c ) );
			}

			so_5::disp::adv_thread_pool::params_t params;
			auto c = env.create_coop( "test_agents",
					so_5::disp::adv_thread_pool::create_disp_binder(
							"thread_pool",
							params ) );
			for( std::size_t i = 0; i != thread_count; ++i )
			{
				c->add_agent( new a_test_t( env, shutdowner_mbox ) );
			}

			env.register_coop( std::move( c ) );
		},
		[]( so_5::rt::environment_params_t & params )
		{
			params.add_named_dispatcher(
					"thread_pool",
					so_5::disp::adv_thread_pool::create_disp( thread_count ) );
		} );
}

int
main( int argc, char * argv[] )
{
	try
	{
		run_with_time_limit(
			[]()
			{
				run_sobjectizer();
			},
			5,
			"cooperation_fifo test" );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

