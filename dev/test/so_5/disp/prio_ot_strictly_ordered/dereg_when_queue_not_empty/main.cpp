/*
 * A test of deregistration of cooperation when demand queue for
 * dispatcher is not empty.
 */

#include <so_5/all.hpp>

#include <various_helpers_1/time_limited_execution.hpp>

struct send_next : public so_5::rt::signal_t {};
struct stop : public so_5::rt::signal_t {};

void
fill_coop(
	so_5::rt::agent_coop_t & coop )
	{
		using namespace so_5::rt;

		auto a = coop.define_agent(
				coop.make_agent_context() +
				agent_t::limit_then_drop< send_next >( 100 ) +
				agent_t::limit_then_drop< stop >( 1 ) +
				so_5::prio::p0 );
		auto m = a.direct_mbox();

		a.on_start( [m, &coop] {
				so_5::send< send_next >( m );
				so_5::send_delayed< stop >(
						coop.environment(),
						m,
						std::chrono::milliseconds( 350 ) );
			} )
		.event< send_next >( m, [m] {
				so_5::send< send_next >( m );
				so_5::send< send_next >( m );
			} )
		.event< stop >( m, [&coop] {
				coop.environment().deregister_coop(
						coop.query_coop_name(),
						dereg_reason::normal );
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
				so_5::launch(
					[]( so_5::rt::environment_t & env )
					{
						using namespace so_5::disp::prio::common_thread;

						env.introduce_coop(
								create_private_disp( env )->binder(),
								fill_coop );
					} );
			},
			5,
			"deregistration of coop on prio::common_thread dispatcher test" );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

