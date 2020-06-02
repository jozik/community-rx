package require turbine 1.1.0

namespace eval crx_model {

    proc crx_model_tcl { outputs inputs args } {
      set z [ lindex $outputs 0 ]
      set config [ lindex $inputs 0 ]
		  set params [ lindex $inputs 1 ]
      rule $config "crx_model::crx_model_tcl_body $z $config $params" {*}$args type $turbine::WORK
    }

    proc report_time { msg rank t } {
      puts [ format "%s %i %0.2f" $msg $rank [ expr $t / 1000.0 ] ]
    }

    proc crx_model_tcl_body { z config params } {

      set config [ retrieve_string $config ]
      set params [ retrieve_string $params ]
      # Look up MPI information
      set comm [ turbine::c::task_comm ]
      set rank [ adlb::rank $comm]

      #if { $rank == 0 } {
      #  set world_rank [ adlb::rank ]
      #  set start [ clock milliseconds ]
      #  report_time "TASK START:" $world_rank $start
      #}

      # Run the user code
      set z_value [ crx_model_run $comm $config $params ]

      if { $rank == 0 } {
      	store_string $z $z_value

        #set stop [ clock milliseconds ]
        #set ms [ expr $stop - $start ]
        #report_time "TASK STOP:" $world_rank $stop
        #report_time "TASK TIME:" $world_rank [ expr $stop - $start ]
      }
    }
}
