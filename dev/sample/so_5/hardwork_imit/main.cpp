/*
 * Sample for demonstrating results of some hard work on different
 * dispatchers.
 */

#include <iostream>
#include <chrono>
#include <cstdlib>

#include <so_5/all.hpp>

struct msg_do_hardwork : public so_5::rt::message_t
{
	unsigned int m_index;
	unsigned int m_milliseconds;

	msg_do_hardwork(
		unsigned int index,
		unsigned int milliseconds )
		:	m_index( index )
		,	m_milliseconds( milliseconds )
	{}
};

struct msg_hardwork_done : public so_5::rt::message_t
{
	unsigned int m_index;

	msg_hardwork_done( unsigned int index )
		:	m_index( index )
	{}
};

struct msg_check_hardwork : public so_5::rt::message_t
{
	unsigned int m_index;
	unsigned int m_milliseconds;

	msg_check_hardwork(
		unsigned int index,
		unsigned int milliseconds )
		:	m_index( index )
		,	m_milliseconds( milliseconds )
	{}
};

struct msg_hardwork_checked : public so_5::rt::message_t
{
	unsigned int m_index;

	msg_hardwork_checked(
		unsigned int index )
		:	m_index( index )
	{}
};

class a_manager_t : public so_5::rt::agent_t
{
	public :
		a_manager_t(
			so_5::rt::environment_t & env,
			const so_5::rt::mbox_ref_t & worker_mbox,
			const so_5::rt::mbox_ref_t & checker_mbox,
			unsigned int requests,
			unsigned int milliseconds )
			:	so_5::rt::agent_t( env )
			,	m_worker_mbox( worker_mbox )
			,	m_checker_mbox( checker_mbox )
			,	m_requests( requests )
			,	m_milliseconds( milliseconds )
		{}

		virtual void
		so_define_agent() override
		{
			so_subscribe( so_direct_mbox() )
				.event( &a_manager_t::evt_hardwork_done )
				.event( &a_manager_t::evt_hardwork_checked );
		}

		virtual void
		so_evt_start() override
		{
			m_start_time = std::chrono::steady_clock::now();

			for( unsigned int i = 0; i != m_requests; ++i )
			{
				m_worker_mbox->deliver_message(
						new msg_do_hardwork { i, m_milliseconds } );
			}
		}

		void
		evt_hardwork_done( const msg_hardwork_done & evt )
		{
			m_checker_mbox->deliver_message(
					new msg_check_hardwork { evt.m_index, m_milliseconds } );
		}

		void
		evt_hardwork_checked( const msg_hardwork_checked & evt )
		{
			++m_processed;

			if( m_processed == m_requests )
			{
				auto finish_time = std::chrono::steady_clock::now();

				auto duration =
						std::chrono::duration_cast< std::chrono::milliseconds >(
								finish_time - m_start_time ).count() / 1000.0;

				std::cout << "Working time: " << duration << "s" << std::endl;

				so_environment().stop();
			}
		}

	private :
		const so_5::rt::mbox_ref_t m_worker_mbox;
		const so_5::rt::mbox_ref_t m_checker_mbox;

		const unsigned int m_requests;
		unsigned int m_processed = 0;

		const unsigned int m_milliseconds;

		std::chrono::steady_clock::time_point m_start_time;
};

so_5::rt::agent_coop_unique_ptr_t
create_test_coop(
	so_5::rt::environment_t & env,
	so_5::rt::disp_binder_unique_ptr_t disp_binder,
	unsigned int requests,
	unsigned int milliseconds )
{
	auto c = env.create_coop( "test", std::move( disp_binder ) );

	auto worker_mbox = env.create_local_mbox();
	auto checker_mbox = env.create_local_mbox();

	auto a_manager = c->add_agent(
			new a_manager_t(
					env,
					worker_mbox,
					checker_mbox,
					requests,
					milliseconds ) );

	c->define_agent()
		.event( worker_mbox,
				[a_manager]( const msg_do_hardwork & evt )
				{
					std::this_thread::sleep_for(
							std::chrono::milliseconds( evt.m_milliseconds ) );

					a_manager->so_direct_mbox()->deliver_message(
							new msg_hardwork_done { evt.m_index } );
				},
				so_5::thread_safe );

	c->define_agent()
		.event( checker_mbox,
				[a_manager]( const msg_check_hardwork & evt )
				{
					std::this_thread::sleep_for(
							std::chrono::milliseconds( evt.m_milliseconds ) );

					a_manager->so_direct_mbox()->deliver_message(
							new msg_hardwork_checked { evt.m_index } );
				},
				so_5::thread_safe );

	return c;
}

struct dispatcher_factories_t
{
	std::function< so_5::rt::dispatcher_unique_ptr_t() > m_disp_factory;
	std::function< so_5::rt::disp_binder_unique_ptr_t() > m_binder_factory;
};

dispatcher_factories_t
make_dispatcher_factories(
	const std::string & type,
	const std::string & name )
{
	dispatcher_factories_t res;

	if( "active_obj" == type )
	{
		using namespace so_5::disp::active_obj;
		res.m_disp_factory = create_disp;
		res.m_binder_factory = [name]() { return create_disp_binder( name ); };
	}
	else if( "thread_pool" == type )
	{
		using namespace so_5::disp::thread_pool;
		res.m_disp_factory = []() { return create_disp(); };
		res.m_binder_factory = 
			[name]() {
				return create_disp_binder(
						name,
						[]( params_t & p ) { p.fifo( fifo_t::individual ); } );
			};
	}
	else if( "adv_thread_pool" == type )
	{
		using namespace so_5::disp::adv_thread_pool;
		res.m_disp_factory = []() { return create_disp(); };
		res.m_binder_factory = 
			[name]() {
				return create_disp_binder(
						name,
						[]( params_t & p ) { p.fifo( fifo_t::individual ); } );
			};
	}
	else if( "one_thread" == type )
	{
		using namespace so_5::disp::one_thread;

		res.m_disp_factory = create_disp;
		res.m_binder_factory = [name]() { return create_disp_binder( name ); };
	}
	else
		throw std::runtime_error( "unknown type of dispatcher: " + type );


	return res;
}

struct config_t
{
	dispatcher_factories_t m_factories;
	unsigned int m_requests;
	unsigned int m_milliseconds;

	static const std::string dispatcher_name;
};

const std::string config_t::dispatcher_name = "dispatcher";

config_t
parse_params( int argc, char ** argv )
{
	if( 1 == argc )
		throw std::runtime_error( "no arguments given!\n\n"
				"usage:\n\n"
				"sample.so_5.hardwork_imit <disp_type> [requests] [worktime_ms]" );

	config_t r {
			make_dispatcher_factories( argv[ 1 ], config_t::dispatcher_name ),
			200,
			15
		};

	if( 2 < argc )
		r.m_requests = std::atoi( argv[ 2 ] );
	if( 3 < argc )
		r.m_milliseconds = std::atoi( argv[ 3 ] );

	std::cout << "Config:\n"
		"\t" "dispatcher: " << argv[ 1 ] << "\n"
		"\t" "requests: " << r.m_requests << "\n"
		"\t" "worktime (ms): " << r.m_milliseconds << std::endl;

	return r;
}

int main( int argc, char ** argv )
{
	try
	{
		const config_t config = parse_params( argc, argv );

		so_5::launch(
			[config]( so_5::rt::environment_t & env )
			{
				env.register_coop(
						create_test_coop(
								env,
								config.m_factories.m_binder_factory(),
								config.m_requests,
								config.m_milliseconds ) );
			},
			[config]( so_5::rt::environment_params_t & params )
			{
				params.add_named_dispatcher(
						config_t::dispatcher_name,
						config.m_factories.m_disp_factory() );
			} );

		return 0;
	}
	catch( const std::exception & x )
	{
		std::cerr << "Exception: " << x.what() << std::endl;
	}

	return 2;
}

