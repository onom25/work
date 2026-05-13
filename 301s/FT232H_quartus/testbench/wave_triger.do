onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /tb_testFT232H/CLK
add wave -noupdate /tb_testFT232H/nRES
add wave -noupdate /tb_testFT232H/nRXF
add wave -noupdate /tb_testFT232H/nTXE
add wave -noupdate -radix hexadecimal /tb_testFT232H/Din
add wave -noupdate /tb_testFT232H/UISW1
add wave -noupdate /tb_testFT232H/UISW2
add wave -noupdate -radix hexadecimal /tb_testFT232H/Dout
add wave -noupdate /tb_testFT232H/nRD
add wave -noupdate /tb_testFT232H/nWR
add wave -noupdate /tb_testFT232H/nOE
add wave -noupdate /tb_testFT232H/wrOE
add wave -noupdate /tb_testFT232H/nPOWSAV
add wave -noupdate /tb_testFT232H/nSIWU
add wave -noupdate /tb_testFT232H/i_mode_SyncFIFO/TRG1
add wave -noupdate /tb_testFT232H/i_mode_SyncFIFO/TRG2
add wave -noupdate /tb_testFT232H/i_mode_SyncFIFO/i_detect_uisw/SW2FLG
add wave -noupdate /tb_testFT232H/i_mode_SyncFIFO/i_detect_uisw/SW1FLG
add wave -noupdate -radix unsigned /tb_testFT232H/i_mode_SyncFIFO/i_detect_uisw/cnt
add wave -noupdate /tb_testFT232H/i_mode_SyncFIFO/i_detect_uisw/SYSW1
add wave -noupdate /tb_testFT232H/i_mode_SyncFIFO/i_detect_uisw/SYSW2
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {1420 ps} 0}
quietly wave cursor active 1
configure wave -namecolwidth 330
configure wave -valuecolwidth 48
configure wave -justifyvalue left
configure wave -signalnamewidth 0
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ps
update
WaveRestoreZoom {1260 ps} {2948 ps}
