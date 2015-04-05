require 'mxx_ru/cpp'

MxxRu::Cpp::composite_target {
  path = 'test/so_5/samples_as_unit_tests'

  required_prj "#{path}/chameneos_prealloc_msgs.ut.rb"
  required_prj "#{path}/chameneos_simple.ut.rb"
  required_prj "#{path}/coop_listener.ut.rb"
  required_prj "#{path}/coop_notification.ut.rb"
  required_prj "#{path}/coop_user_resources.ut.rb"
  required_prj "#{path}/custom_error_logger.ut.rb"
  required_prj "#{path}/exception_logger.ut.rb"
  required_prj "#{path}/exception_reaction.ut.rb"
  required_prj "#{path}/hello_all.ut.rb"
  required_prj "#{path}/hello_evt_handler.ut.rb"
  required_prj "#{path}/hello_evt_lambda.ut.rb"
  required_prj "#{path}/hello_world.ut.rb"
  required_prj "#{path}/ping_pong_minimal.ut.rb"
  required_prj "#{path}/private_dispatcher_for_children.ut.rb"
  required_prj "#{path}/private_dispatcher_hello.ut.rb"
  required_prj "#{path}/subscriptions.ut.rb"
  required_prj "#{path}/svc_exceptions.ut.rb"
  required_prj "#{path}/svc_hello.ut.rb"

}
