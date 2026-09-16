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

// -----------------------------------------------------------------------------
// reset_extender
//
// Extends an active-low reset that is already synchronized to clk.
//
// resetn_ext asserts (goes low) as soon as resetn asserts, and deasserts
// EXTENSION_CYCLES clk cycles after resetn deasserts. EXTENSION_CYCLES = 1 gives
// the minimum extension of a single register stage; 512 is the maximum.
//
// resetn is expected to come from a reset synchronizer, i.e. asynchronously
// asserted and synchronously deasserted with respect to clk. The same style is
// preserved at the output so that downstream logic sees an immediate assertion.
// -----------------------------------------------------------------------------

`timescale 1ns / 1ps

module reset_extender #(
    parameter int EXTENSION_CYCLES = 512
) (
    input logic clk,
    input logic resetn,
    // MAX_FANOUT lets the fitter duplicate this register to spread the reset
    // tree. Do not add a preserve attribute here, as that blocks duplication.
    (* altera_attribute = "-name MAX_FANOUT 50" *)
    output logic resetn_ext
);

  localparam int CntWidth = (EXTENSION_CYCLES > 1) ? $clog2(EXTENSION_CYCLES) : 1;
  localparam logic [CntWidth-1:0] CntLoad = EXTENSION_CYCLES - 1;

  logic [CntWidth-1:0] count;

  always_ff @(posedge clk or negedge resetn) begin
    if (!resetn) begin
      count      <= CntLoad;
      resetn_ext <= 1'b0;
    end else if (count != '0) begin
      count      <= count - 1'b1;
      resetn_ext <= 1'b0;
    end else begin
      resetn_ext <= 1'b1;
    end
  end

endmodule






