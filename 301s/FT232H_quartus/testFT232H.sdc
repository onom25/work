# Clock
create_clock -name CLK -period 16.666 [get_ports {CLK}]

##derive_all_clocks
derive_clock_uncertainty
##set_clock_groups -asynchronous -group [get_clocks {BCLK EDACK WR}] -group [get_clocks {AD[10] CLK48 HLAT}]

# Input Port
set_input_delay -clock { CLK } 2 [get_ports {UISW1 UISW2 nRXF nTXE NRES}]

# Output Port
set_output_delay -clock { CLK } 2.0 [get_ports {nOE nPWRSAV nRD nWR nPWRSAV D* TP*}]