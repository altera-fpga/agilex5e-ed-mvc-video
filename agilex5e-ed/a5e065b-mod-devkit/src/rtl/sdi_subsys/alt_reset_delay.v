// (C) 2001-2026 Altera Corporation. All rights reserved.
// Your use of Altera Corporation's design tools, logic functions and other 
// software and tools, and its AMPP partner logic functions, and any output 
// files from any of the foregoing (including device programming or simulation 
// files), and any associated documentation or information are expressly subject 
// to the terms and conditions of the Altera Program License Subscription 
// Agreement, Altera IP License Agreement, or other applicable 
// license agreement, including, without limitation, that your use is for the 
// sole purpose of programming logic devices manufactured by Altera and sold by 
// Altera or its authorized distributors.  Please refer to the applicable 
// agreement for further details.


// Copyright 2010 Altera Corporation. All rights reserved.  
// Altera products are protected under numerous U.S. and foreign patents, 
// maskwork rights, copyrights and other intellectual property laws.  
//
// This reference design file, and your use thereof, is subject to and governed
// by the terms and conditions of the applicable Altera Reference Design 
// License Agreement (either as signed by you or found at www.altera.com).  By
// using this reference design file, you indicate your acceptance of such terms
// and conditions between you and Altera Corporation.  In the event that you do
// not agree with such terms and conditions, you may not use the reference 
// design file and please promptly destroy any copies you have made.
//
// This reference design file is being provided on an "as-is" basis and as an 
// accommodation and therefore all warranties, representations or guarantees of 
// any kind (whether express, implied or statutory) including, without 
// limitation, warranties of merchantability, non-infringement, or fitness for
// a particular purpose, are specifically disclaimed.  By making this reference
// design file available, Altera expressly does not recommend, suggest or 
// require that this reference design file be used in combination with any 
// other product not provided by Altera.
/////////////////////////////////////////////////////////////////////////////

`timescale 1 ps / 1 ps
// baeckler - 12-17-2009

// when not ready_in - immediately not ready_out
// when ready_in - wait for counter, then ready out synchronously

// DESCRIPTION
// 
// This is a more elaborate version of aclr_filter, typically used for bringing up SERDES pins or PLLs. When
// the input ready condition is not met the output is immediately driven to not ready. When the input
// ready becomes true the output will become ready after a programmable delay.
// 



// CONFIDENCE
// This is used very liberally in Altera test and demo designs
// 

module alt_reset_delay #(
	parameter CNTR_BITS = 16
)
(
	input clk,
	input ready_in,
	output ready_out
);

reg [2:0] rs_meta /* synthesis preserve dont_replicate */
/* synthesis ALTERA_ATTRIBUTE = "-name SDC_STATEMENT \"set_false_path -from [get_fanins -async *reset_delay*rs_meta\[*\]] -to [get_keepers *reset_delay*rs_meta\[*\]]\" " */;

always @(posedge clk or negedge ready_in) begin
	if (!ready_in) rs_meta <= 3'b000;
	else rs_meta <= {rs_meta[1:0],1'b1};
end
wire ready_sync = rs_meta[2];

reg [CNTR_BITS-1:0] cntr /* synthesis preserve */;
assign ready_out = cntr[CNTR_BITS-1];
always @(posedge clk or negedge ready_sync) begin
	if (!ready_sync) cntr <= {CNTR_BITS{1'b0}};
	else if (!ready_out) cntr <= cntr + 1'b1;
end

endmodule

// BENCHMARK INFO :  10AX115U2F45I2SGE2
// BENCHMARK INFO :  Quartus Prime Version 15.1.0 Internal Build 99 06/10/2015 TO Standard Edition
// BENCHMARK INFO :  Uses helper file :  alt_reset_delay.v
// BENCHMARK INFO :  Max depth :  3.4 LUTs
// BENCHMARK INFO :  Total registers : 19
// BENCHMARK INFO :  Total pins : 3
// BENCHMARK INFO :  Total virtual pins : 0
// BENCHMARK INFO :  Total block memory bits : 0
// BENCHMARK INFO :  Comb ALUTs :  17                 
// BENCHMARK INFO :  ALMs : 9 / 427,200 ( < 1 % )
// BENCHMARK INFO :  Worst setup path @ 468.75MHz : 0.796 ns, From cntr[15], To ready_out}
`ifdef QUESTA_INTEL_OEM
`pragma questa_oem_00 "RD71bOkSrivegSkkp4I5+wDzCX2kXlX2aNpk/av3AGMum78fc0IH/D4D78z2PRJQvYxe1MAtGOZSq8lK1cGI/lriEdpS0q0WC4kxXTqXS6yX8taAoFd1MuJ+o4YnXLtnnY1KkVApm+y0e3pMzif6iKdYG6nEqYMpXcyVCrvdgVLYujco/9OJR1g2LoIMzmPPFZ68UJVexCndOypfappXOx88EvwA8I/Yuis7m8owvBjtIJlvot5TO+FT0kzKQLhok0MPISZDjuSWGH1p9MhMJhp3RaRP6MuvWVay4SNgve0s/nSmvd1UufbZCt8xz464a0Mb9i6hHkotGqEMIhaVwu2ELxqTm2Mw+jkjQlezB63NuVgBOcEZBX2eDZJJFH5Bs3JcNtMINQhc3u9zB9C4ghA09W162BN3sW8ZXlRIUd823fq9q6clJhPiR5jZ13Ia352KLVYea7FiVuviOLWDMficKoJohXcHmjhCNPDTASoenbjucOBARREqN7EMYOmrvMrFYwcjXddBVWhYeelnKulmTsqTynqEZiwW5FXqA3bGQq7bzuUacAiKyMW2GntQy07VNkvX5UEq73/v9wbFLY5cMWXJyxWdlTjknyYD0KO6Q+5MNx9Jn6921KAzh1QxN2IhZT0KBLnf367O9ZuR/2BQloIVc8T7ZPs9FHsoJ9JQkNZBDdpp73bsvxbReDzvqY0pPu8RQdznDtL2d8UtsSZ0/2QntXXbjx+i5QWEFj2vRL0mb9r+RHspNmM/KCm+w0F19vOg2/5oY0dzzPyhn8ZT96gkJQckHN80E407jPFVXrkW0BhgzyrJAUnnyeno9XIpWI0EdtTpTKJDgnfO3ozZTCtIcTgiN1ljBqQN+7kzRdSX/jarDV7pSpyDZeuohZt12xIwPdmlUjy/9Cc8+VKNrJqSybDY15ihCj6pCGxKtJYi9sNQlKupT1V1C+DLoQpmMU+hoUaE+UESPrKWtdNyfnVdikyxxR3W3b1Mjpa3j6L1TgJJPtaLxVSThRbO"
`endif