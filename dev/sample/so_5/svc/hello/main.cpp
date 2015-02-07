/*
 * A sample for service request handler.
 */

#include <iostream>
#include <exception>
#include <sstream>

#include <so_5/all.hpp>

class msg_hello_svc : public so_5::rt::signal_t {};

void
define_hello_service(
	so_5::rt::agent_coop_t & coop,
	const so_5::rt::mbox_t & self_mbox )
	{
		coop.define_agent().event< msg_hello_svc >( self_mbox,
			[]() -> std::string {
				std::cout << "svc_hello called" << std::endl;
				return "Hello, World!";
			} );
	}

struct msg_convert : public so_5::rt::message_t
	{
		int m_value;

		msg_convert( int value ) : m_value( value )
			{}
	};

void
define_convert_service(
	so_5::rt::agent_coop_t & coop,
	const so_5::rt::mbox_t & self_mbox )
	{
		coop.define_agent().event( self_mbox,
			[]( const msg_convert & msg ) -> std::string {
				std::cout << "svc_convert called: value=" << msg.m_value
						<< std::endl;

				std::ostringstream s;
				s << msg.m_value;

				return s.str();
			} );
	}

struct msg_shutdown : public so_5::rt::signal_t {};

void
define_shutdown_service(
	so_5::rt::agent_coop_t & coop,
	const so_5::rt::mbox_t & self_mbox )
	{
		auto & env = coop.environment();
		coop.define_agent().event< msg_shutdown >( self_mbox,
			[&env]() {
				std::cout << "svc_shutdown called" << std::endl;

				env.stop();
			} );
	};

class a_client_t
	:	public so_5::rt::agent_t
	{
	public :
		a_client_t(
			so_5::rt::environment_t & env,
			const so_5::rt::mbox_t & svc_mbox )
			:	so_5::rt::agent_t( env )
			,	m_svc_mbox( svc_mbox )
			{}

		virtual void
		so_evt_start() override
			{
				std::cout << "hello_svc: "
						<< m_svc_mbox->get_one< std::string >()
								.async< msg_hello_svc >().get()
						<< std::endl;

				std::cout << "convert_svc: "
						<< m_svc_mbox->get_one< std::string >()
								.make_async< msg_convert >( 42 ).get()
						<< std::endl;

				std::cout << "sync_convert_svc: "
						<< m_svc_mbox->get_one< std::string >()
								.wait_forever().make_sync_get< msg_convert >( 1020 )
						<< std::endl;

				// More complex case with conversion.
				auto svc_proxy = m_svc_mbox->get_one< std::string >();

				// These requests should be processed before next 'sync_request'...
				auto c1 = svc_proxy.make_async< msg_convert >( 1 );
				auto c2 = svc_proxy.make_async< msg_convert >( 2 );

				// Two previous request should be processed before that call.
				std::cout << "sync_convert_svc: "
						<< svc_proxy.wait_forever().make_sync_get< msg_convert >(3)
						<< std::endl;

				// But their value will be accessed only now.
				std::cout << "convert_svc: c2=" << c2.get() << std::endl;
				std::cout << "convert_svc: c1=" << c1.get() << std::endl;

				// Initiate shutdown via another synchonyous service.
				m_svc_mbox->run_one().wait_forever().sync_get< msg_shutdown >();
			}

	private :
		const so_5::rt::mbox_t m_svc_mbox;
	};

void
init(
	so_5::rt::environment_t & env )
	{
		auto coop = env.create_coop(
				"test_coop",
				so_5::disp::active_obj::create_disp_binder( "active_obj" ) );

		auto svc_mbox = env.create_local_mbox();

		define_hello_service( *coop, svc_mbox );
		define_convert_service( *coop, svc_mbox );
		define_shutdown_service( *coop, svc_mbox );

		coop->add_agent( new a_client_t( env, svc_mbox ) );

		env.register_coop( std::move( coop ) );
	}

int
main( int, char ** )
	{
		try
			{
				so_5::launch(
					&init,
					[]( so_5::rt::environment_params_t & p ) {
						p.add_named_dispatcher(
							"active_obj",
							so_5::disp::active_obj::create_disp() );
					} );
			}
		catch( const std::exception & ex )
			{
				std::cerr << "Error: " << ex.what() << std::endl;
				return 1;
			}

		return 0;
	}

