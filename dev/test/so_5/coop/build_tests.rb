#!/usr/local/bin/ruby
require 'mxx_ru/cpp'

MxxRu::Cpp::composite_target {

	path = 'test/so_5/coop'

	required_prj( "#{path}/duplicate_name/prj.ut.rb" )
	required_prj( "#{path}/reg_some_and_stop_1/prj.ut.rb" )
	required_prj( "#{path}/reg_some_and_stop_2/prj.ut.rb" )
	required_prj( "#{path}/reg_some_and_stop_3/prj.ut.rb" )
	required_prj( "#{path}/throw_on_define_agent/prj.ut.rb" )
	required_prj( "#{path}/throw_on_bind_to_disp/prj.ut.rb" )
	required_prj( "#{path}/throw_on_bind_to_disp_2/prj.ut.rb" )
	required_prj( "#{path}/coop_notify_1/prj.ut.rb" )
	required_prj( "#{path}/coop_notify_2/prj.ut.rb" )
	required_prj( "#{path}/coop_notify_3/prj.ut.rb" )
	required_prj( "#{path}/parent_child_1/prj.ut.rb" )
	required_prj( "#{path}/parent_child_2/prj.ut.rb" )
	required_prj( "#{path}/parent_child_3/prj.ut.rb" )
	required_prj( "#{path}/parent_child_4/prj.ut.rb" )
	required_prj( "#{path}/user_resource/prj.ut.rb" )
	required_prj( "#{path}/build_coop/prj.ut.rb" )
}
