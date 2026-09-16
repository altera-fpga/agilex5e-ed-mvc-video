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


//**********************************************************************************
// Edge Detector
//**********************************************************************************
`timescale 1 ns / 1 ps

module edge_detector #(
   parameter EDGE_DETECT = "POSEDGE"
) (
   input wire clk,
   input wire rst,
   input wire d,
   output reg q
);

reg d_reg;
always @ (posedge clk or posedge rst)
begin
   if (rst) begin
      d_reg <= 1'b0;
      q <= 1'b0;
   end else begin 
      d_reg <= d;

      if (EDGE_DETECT == "POSEDGE") begin
         q <= d & ~d_reg;
      end else if (EDGE_DETECT == "NEGEDGE") begin
         q <= ~d & d_reg;
      end else if (EDGE_DETECT == "DUAL_EDGE") begin
         q <= d ^ d_reg;
      end
   end
end

endmodule
`ifdef QUESTA_INTEL_OEM
`pragma questa_oem_00 "CzkW75+oN9UPDLm4KgJc7M1Yjfnd1OwjA0A725fAsrwWVG/JUBVeLyamtB1TtmUDi5839RVSXNqimaWxif4YaavEVawwBAbtiotnMCLr/D79qJWw8BVue0OW4NHTnpfCj4gTQwCdnI3i/N3ZZ927tcpRwhBwMhZNHbqg34N7sGcsLYM7rasMUJRTTH/4nk+xJbXNY78cjehd+5g9zTHBNO8/GRi+RymrJ0aLciuZvsYbm+q2JXuNLYund/sMgnyDQRYL4bw/F9c2bMww/r012cDiibL2B7VoEx8ravDQLGwdsSXIa4Q1VTeIHPfRxNhG3If7NDSxO1f0e9cvaMHUL12t695XQFSg9yR2gqErthhvjmuCjgpeF8KCmTWZ83OYCVW0Th0rTjJUrcaAcuirHLX547C52hBU1f9A2LefUUREtUSvjyOnHxH9Zu3UH2zfy7p74fjjqEz7cCV5IiIEE3lKNddIZgJTfZsslIoJwegqmHmufoZxL99Zr+eOGbTlf82FwxXYx+PEdJGDSmnVGAkySTsXWfdIOcauf7Jr45NEfRUnihnUU6OhtoTFn48WJ0Zt/SyqhptPRyvHzsl5QC1Nj7nP03nd3ZSUOInEAht/77CCZlB6OFOGgDvNcirOYU+tTKGN95yCj08MXnxd0LhofhynZHNj2evhlN4jUr+fbS3IRJz1O2EmpiYgKl2BKMepCLGFMeLj/wxH+BMWNxfaZcVAW2gQj56jWQtOsDSF3W08+Vw58r1/iPM+G+DRqAtvNUNuvHD9/3e3Lb3LG3NWiSkHCDCDoW1Ps9dKSPyIOUK9TrK0l4Y/ImFHS504gyGz+uEjn/eE3bGbbi4UbppZaeaQzHYqtZgUSnFunxV/Tv2ld39WSxQIChdiQPgAp1KbFlLn4WyEJ3yBnJ0zIb5lri5SXnHCHLmNFluNobMWVEWfSqUweI3lu8GURPHwUKkLs3cr9xpRoBhP8ImOS45cxNV3xcdqg2eNTGfd0VXcgt+Jk+cDBpzcsPeziw1H"
`endif