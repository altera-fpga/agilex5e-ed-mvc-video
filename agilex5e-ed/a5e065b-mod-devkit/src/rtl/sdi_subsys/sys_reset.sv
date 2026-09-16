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


module sys_reset #(
    parameter   DELAY_CNTR_BITS = 16,
    parameter   RST_CHAIN_DEPTH = 2
)(
    input   logic clk,
    input   logic clk_100mhz,
    input   logic async_rstn,
    output  logic ninit_done_sync,
    output  logic out_reset
);

logic                       ninit_done;
logic                       sync_rstn;
(*preserve_syn_only*) logic [RST_CHAIN_DEPTH-1:0] reset_sync_chain;

// ----------------------------------------------------------------------------------
// Reset Release IP for S10
// - This IP outputs nINIT_DONE after finishing device initalization.
// - User mode initialization can begin as soon as the nINIT_DONE signal de-asserts.
// ----------------------------------------------------------------------------------
reset_release s10_rst_release_inst (
    .ninit_done (ninit_done)
);

altera_std_synchronizer #(.depth(3)) u_ninit_done_sync   (.clk(clk_100mhz),.reset_n(1'b1),.din(ninit_done),.dout(ninit_done_sync));

//-------------------------------------------------------------------------------
// Synchronize async reset (As a debouncer as well for push button reset)
//-------------------------------------------------------------------------------
alt_reset_delay #(
    .CNTR_BITS  (DELAY_CNTR_BITS)
) async_rst_delay_inst (
    .clk        (clk),
    .ready_in   (async_rstn & ~ninit_done),
    .ready_out  (sync_rstn)
);

always @(posedge clk) begin
    reset_sync_chain[RST_CHAIN_DEPTH-1] <= ~sync_rstn;
    reset_sync_chain[RST_CHAIN_DEPTH-2:0] <= reset_sync_chain[RST_CHAIN_DEPTH-1:1];
    out_reset <= reset_sync_chain[0];
end

endmodule
`ifdef QUESTA_INTEL_OEM
`pragma questa_oem_00 "RD71bOkSrivegSkkp4I5+wDzCX2kXlX2aNpk/av3AGMum78fc0IH/D4D78z2PRJQvYxe1MAtGOZSq8lK1cGI/lriEdpS0q0WC4kxXTqXS6yX8taAoFd1MuJ+o4YnXLtnnY1KkVApm+y0e3pMzif6iKdYG6nEqYMpXcyVCrvdgVLYujco/9OJR1g2LoIMzmPPFZ68UJVexCndOypfappXOx88EvwA8I/Yuis7m8owvBiNa8be0URCw/jAx8kHCFJU/He/eqttHH67IYwub03dfGUgp2DNcIlvcVb6FI83RCxi9XGKhK+NEpv9GisEGegM2SyqEMvuyENLO4BJ64oticgWpep8yQkld4mHGfQiiKpDSLQ9l/VcU43Mc8Xq8kG9ZBw/D/eBcoaeUN/gaovbOimhg6pTv7Q6FEhc69FYR52gME0UWMl+VevdbImMO5LtevOmcWMtZPdepAV5v2H2av/pVosNsPExlRaDBnoNsiYjKmoEXlmqtnneV5a8MjBO1EjJWjFACqETNT8GtSQA5nfeFKLbVppKqE+O/bN4PrRglF2ghphgYN2mxIEYgOq4DeU0feCYG+LX103xoUGTivmfBzr9YP5VsI9zmcTgnLWD5xiBKziCT6UzoK5qvEiIupqdWiDXNfMtQL0L5E3M2xcIqACLmPDkLVm4hX8HvBZf8jkutx+oA0MPrqy6rdqvWVs8mqhAybQHY8WR++uDgUum1euAKC7ez8AC+AxPDnpQhhp/iGQz01T3QOZiQs/JGbezlFAolDhsNN3NI9Qy0ntQnJ/S17ywPp4dtKCg52xacdF03cXUbakvNoXCBWrHFDnViDMVKNpgi4MoyjpyPtyohVo91ENf+BUKRMxe1VdRswbysZn+aBWHw2fIi0nkbqHOas7+pnUWYfFpeXEStGT7y/Gfk6x/OUapv+SR8w5H4hQ6wAB2/0J0p8vrpPc1HjIB69YoANGIQ5KIw3dxuoPFGANx4XPfJAWXxb7eyxwfCEnGflbLs5BVgB+QA0nw"
`endif