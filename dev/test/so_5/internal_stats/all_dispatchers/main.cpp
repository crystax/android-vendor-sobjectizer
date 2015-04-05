/*
 * Demo application to show run-time monitoring information
 * from all types of standard dispatchers.
 */

#include <algorithm>
#include <iostream>
#include <map>
#include <exception>
#include <stdexcept>
#include <cstdlib>
#include <thread>
#include <chrono>

#include <so_5/all.hpp>

class a_worker_t : public so_5::rt::agent_t
	{
	public :
		static const std::size_t targets_count = 4;

		struct hello : public so_5::rt::signal_t {};

		a_worker_t( context_t ctx )
			:	so_5::rt::agent_t( ctx
					+ limit_then_drop< hello >( targets_count ) )
			{}

		void
		set_targets( const so_5::rt::mbox_t targets[ targets_count ] )
			{
				std::copy( targets, targets + targets_count, m_targets );
			}

		virtual void
		so_define_agent()
			{
				so_default_state().event< hello >( &a_worker_t::evt_hello );
			}

	private :
		so_5::rt::mbox_t m_targets[ targets_count ];

		void
		evt_hello()
			{
				using namespace std;

				for_each( begin( m_targets ), end( m_targets ),
						[]( const so_5::rt::mbox_t & t ) {
							so_5::send< hello >( t );
						} );
			}
	};

class a_controller_t : public so_5::rt::agent_t
	{
	public :
		a_controller_t( context_t ctx )
			:	so_5::rt::agent_t( ctx )
			{}

		virtual void
		so_define_agent() override
			{
				so_default_state().event(
						so_environment().stats_controller().mbox(),
						&a_controller_t::evt_monitor_quantity );

				so_default_state().event< finish >(
						[this] { so_deregister_agent_coop_normally(); } );
			}

		virtual void
		so_evt_start() override
			{
				create_child_coops();

				so_environment().stats_controller().set_distribution_period(
						std::chrono::milliseconds(500) );
				so_environment().stats_controller().turn_on();

				so_5::send_delayed_to_agent< finish >( *this,
						std::chrono::seconds( 6 ) );
			}

	private :
		struct finish : public so_5::rt::signal_t {};

		using workers_vector_t = std::vector< a_worker_t * >;

		void
		evt_monitor_quantity(
			const so_5::rt::stats::messages::quantity< std::size_t > & evt )
			{
				namespace stats = so_5::rt::stats;

				std::cout << evt.m_prefix.c_str()
						<< evt.m_suffix.c_str()
						<< ": " << evt.m_value << std::endl;
			}

		void
		create_child_coops()
			{
				auto coop = so_5::rt::create_child_coop(
						*this,
						so_5::autoname );

				workers_vector_t workers;
				workers.reserve( 100 );

				create_children_on_default_disp( *coop, workers );
				create_children_on_one_thread_disp( *coop, workers );
				create_children_on_active_obj_disp( *coop, workers );
				create_children_on_active_group_disp( *coop, workers );
				create_children_on_thread_pool_disp_1( *coop, workers );
				create_children_on_thread_pool_disp_2( *coop, workers );
				create_children_on_adv_thread_pool_disp_1( *coop, workers );
				create_children_on_adv_thread_pool_disp_2( *coop, workers );

				connect_workers( workers );

				so_environment().register_coop( std::move( coop ) );

				send_initial_hello( workers );
			}

		void
		create_children_on_default_disp(
			so_5::rt::agent_coop_t & coop,
			workers_vector_t & workers )
			{
				for( int i = 0; i != 5; ++i )
					workers.push_back( coop.make_agent< a_worker_t >() );
			}

		void
		create_children_on_one_thread_disp(
			so_5::rt::agent_coop_t & coop,
			workers_vector_t & workers )
			{
				auto disp = so_5::disp::one_thread::create_private_disp(
						so_environment() );

				create_children_on( coop, workers,
						[disp] { return disp->binder(); } );
			}

		void
		create_children_on_active_obj_disp(
			so_5::rt::agent_coop_t & coop,
			workers_vector_t & workers )
			{
				auto disp = so_5::disp::active_obj::create_private_disp(
						so_environment() );

				create_children_on( coop, workers,
						[disp] { return disp->binder(); } );
			}

		void
		create_children_on_active_group_disp(
			so_5::rt::agent_coop_t & coop,
			workers_vector_t & workers )
			{
				auto disp = so_5::disp::active_group::create_private_disp(
						so_environment() );

				unsigned int group_no = 0;

				create_children_on( coop, workers,
						[disp, &group_no] {
							return disp->binder( "group#" +
									std::to_string( ++group_no ) );
						} );
			}

		void
		create_children_on_thread_pool_disp_1(
			so_5::rt::agent_coop_t & coop,
			workers_vector_t & workers )
			{
				auto disp = so_5::disp::thread_pool::create_private_disp(
						so_environment() );

				create_children_on( coop, workers,
						[disp] {
								return disp->binder(
									so_5::disp::thread_pool::params_t{} );
						} );
			}

		void
		create_children_on_thread_pool_disp_2(
			so_5::rt::agent_coop_t & coop,
			workers_vector_t & workers )
			{
				using namespace so_5::disp::thread_pool;

				auto disp = create_private_disp( so_environment() );

				create_children_on( coop, workers,
						[disp] {
								return disp->binder(
									params_t{}.fifo( fifo_t::individual ) );
						} );
			}

		void
		create_children_on_adv_thread_pool_disp_1(
			so_5::rt::agent_coop_t & coop,
			workers_vector_t & workers )
			{
				using namespace so_5::disp::adv_thread_pool;

				auto disp = create_private_disp( so_environment() );

				create_children_on( coop, workers,
						[disp] {
								return disp->binder( params_t{} );
						} );
			}

		void
		create_children_on_adv_thread_pool_disp_2(
			so_5::rt::agent_coop_t & coop,
			workers_vector_t & workers )
			{
				using namespace so_5::disp::adv_thread_pool;

				auto disp = create_private_disp( so_environment() );

				create_children_on( coop, workers,
						[disp] {
								return disp->binder(
									params_t{}.fifo( fifo_t::individual ) );
						} );
			}

		template< typename LAMBDA >
		void
		create_children_on(
			so_5::rt::agent_coop_t & coop,
			workers_vector_t & workers,
			LAMBDA binder )
			{
				for( int i = 0; i != 5; ++i )
					workers.push_back(
							coop.make_agent_with_binder< a_worker_t >( binder() ) );
			}

		void
		connect_workers( const workers_vector_t & workers )
			{
				using namespace std;

				for( auto w : workers )
					{
						so_5::rt::mbox_t targets[ a_worker_t::targets_count ];

						generate( begin( targets ), end( targets ),
								[&workers] {
									return workers[ random_index( workers.size() ) ]
										->so_direct_mbox();
								} );

						w->set_targets( targets );
					}
			}

		void
		send_initial_hello( const workers_vector_t & workers )
			{
				so_5::send_to_agent< a_worker_t::hello >(
						*( workers[ random_index( workers.size() ) ] ) );
			}

		static std::size_t
		random_index( std::size_t max_size )
			{
				return static_cast< std::size_t >( std::rand() ) % max_size;
			}
	};

int
main()
{
	try
	{
		so_5::launch( []( so_5::rt::environment_t & env ) {
				env.register_agent_as_coop( so_5::autoname,
						env.make_agent< a_controller_t >() );
			} );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

