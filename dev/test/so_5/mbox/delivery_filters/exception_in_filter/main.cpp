/*
 * A test for exception from delivery filter.
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

using data = so_5::tuple_as_message_t< so_5::mtag< 0 >, int >;

struct finish : public so_5::signal_t {};

class a_test_t : public so_5::agent_t
{
public :
	a_test_t( context_t ctx )
		:	so_5::agent_t( ctx )
		,	m_data_mbox( so_environment().create_mbox() )
	{}

	virtual void
	so_define_agent() override
	{
		so_set_delivery_filter( m_data_mbox,
			[]( const data & msg ) {
				if( 1 == std::get< 0 >( msg ) )
					throw std::runtime_error( "value is 1" );

				return true;
			} );

		so_default_state()
			.event( m_data_mbox, [this]( const data & msg ) {
				const auto value = std::get<0>( msg );
				if( 1 != value )
					throw std::runtime_error( "unexpected data value: " +
							std::to_string( value ) );
				++m_values_accepted;
			} )
			.event< finish >( [this] {
				if( 2 != m_values_accepted )
					throw std::runtime_error( "unexpected count of "
							"accepted data instances: " +
							std::to_string( m_values_accepted ) );

				so_deregister_agent_coop_normally();
			} );
	}

	virtual void
	so_evt_start() override
	{
		so_5::send< data >( m_data_mbox, 0 );
		so_5::send< data >( m_data_mbox, 1 );
		so_5::send< data >( m_data_mbox, 2 );
		so_5::send< data >( m_data_mbox, 3 );
		so_5::send< data >( m_data_mbox, 4 );
		so_5::send< data >( m_data_mbox, 0 );
		so_5::send< data >( m_data_mbox, 1 );
		so_5::send< data >( m_data_mbox, 2 );
		so_5::send< data >( m_data_mbox, 3 );
		so_5::send< data >( m_data_mbox, 4 );

		so_5::send_to_agent< finish >( *this );
	}

private :
	const so_5::mbox_t m_data_mbox;
	unsigned int m_values_accepted = 0;
};

void
init( so_5::environment_t & env )
{
	env.introduce_coop( []( so_5::coop_t & coop ) {
		coop.set_exception_reaction( so_5::shutdown_sobjectizer_on_exception );
		coop.make_agent< a_test_t >();
	} );
}

int
main()
{
	try
	{
		run_with_time_limit(
			[]()
			{
				so_5::launch( &init );
			},
			20,
			"exception in delivery filter test" );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

