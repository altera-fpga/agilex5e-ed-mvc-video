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


module mr_rate_detect 
#(
    parameter [23:0] SLOW_CHECK = 24'd245000,
    parameter [23:0] CYC_MEASURE_CLK_IN_10_MSEC = 24'h0F4240 
// synopsys translate_off
/ 10000
// synopsys translate_on
) 
(
    input  wire        refclock,         
    input  wire        measure_clk,      
    input  wire        reset,            
    output reg  [23:0] refclock_measure,
    output reg         too_slow,
    output reg         valid
);

reg        refclock_rst_sync;
(* altera_attribute = {"-name SYNCHRONIZER_IDENTIFICATION \"OFF\";", 
                       "-name SDC_STATEMENT \"set_false_path -to [get_keepers {*phy_rate_detect:*|refclock_rst_meta}]\" "} *)
reg        refclock_rst_meta;
reg [23:0] tenms_count;
reg        tenms_en;
reg        tenms_en_d1;
reg        tgl_send;

(* altera_attribute = {"-name SYNCHRONIZER_IDENTIFICATION OFF;", 
                       "-name SDC_STATEMENT \"set_false_path -to [get_keepers {*phy_rate_detect:*|refclock_measure_meta*}]\" " } *) 
reg [23:0] refclock_measure_meta;
reg [23:0] refclock_count;
reg [23:0] refclock_count_lat;
(* altera_attribute = {"-name SYNCHRONIZER_IDENTIFICATION \"OFF\";", 
                       "-name SDC_STATEMENT \"set_false_path -to [get_keepers {*phy_rate_detect:*|tgl_echo_meta}]\" " } *) 
reg        tgl_echo_meta;
reg        tgl_echo;
reg        tgl_echo_d1;
(* altera_attribute = {"-name SYNCHRONIZER_IDENTIFICATION \"OFF\";", 
                       "-name SDC_STATEMENT \"set_false_path -to [get_keepers {*phy_rate_detect:*|tgl_rcvd_meta}]\" "} *) 
reg        tgl_rcvd_meta;
reg        tgl_rcvd;


  //------------------------------------------------------------------------------
  // CDC reset to refclock
  //------------------------------------------------------------------------------
  always @ (posedge refclock) begin
    refclock_rst_meta <= reset;
    refclock_rst_sync <= refclock_rst_meta;
    
  end

  //------------------------------------------------------------------------------
  // 10ms Reference timer
  //------------------------------------------------------------------------------
  always @ (posedge measure_clk or posedge reset) begin
    if (reset) begin
      tenms_count <= 24'd0;       
      tenms_en <= 1'b0;
    end else begin 
      if (tenms_count == CYC_MEASURE_CLK_IN_10_MSEC - 1) begin
        tenms_count <= 24'd0;
        tenms_en <= 1'b1;
      end else begin
        tenms_count <= tenms_count + 24'd1;
        tenms_en <= 1'b0;
      end
    end
  end
  
  //------------------------------------------------------------------------------
  // Refclock counter
  //------------------------------------------------------------------------------
  always @ (posedge refclock or posedge refclock_rst_sync) begin
    if (refclock_rst_sync) begin
      refclock_count     <= 24'd0;       
      refclock_count_lat <= 24'd0;
      tgl_echo_meta      <= 1'b0;
      tgl_echo           <= 1'b0;
      tgl_echo_d1        <= 1'b0;
    end else begin 
      tgl_echo_meta      <= tgl_send;
      tgl_echo           <= tgl_echo_meta;
      tgl_echo_d1        <= tgl_echo;
      if (tgl_echo ^ tgl_echo_d1) begin      
        refclock_count     <= 24'd1;       
        refclock_count_lat <= refclock_count;
      end else begin
        refclock_count <= refclock_count + 1'b1;       
      end
    end
  end

  //------------------------------------------------------------------------------
  // Measurement collector
  //------------------------------------------------------------------------------
  always @ (posedge measure_clk or posedge reset)
  begin
    if (reset) begin
      tgl_send              <= 1'b0;
      tgl_rcvd_meta         <= 1'b0;
      tgl_rcvd              <= 1'b0;
      refclock_measure_meta <= 24'd0;
      refclock_measure      <= 24'd0;
      valid                 <= 1'b0;
    end else begin
      tgl_rcvd_meta <= tgl_echo;
      tgl_rcvd <= tgl_rcvd_meta;
      refclock_measure_meta <= refclock_count_lat;
      tenms_en_d1 <= tenms_en;
      valid <= tenms_en_d1;
      
      if (tenms_en) begin
        // Handshake failure, refclock is dead.
        if (tgl_send ^ tgl_rcvd) begin
          refclock_measure <= 24'd0;
        end else begin
          refclock_measure <= refclock_measure_meta;
          tgl_send <= ~tgl_send;
        end
      end
      too_slow <= (refclock_measure < SLOW_CHECK);      
      
    end
  end

endmodule
`ifdef QUESTA_INTEL_OEM
`pragma questa_oem_00 "ZD7evmOPoSczPPlxTOD4Gku5RRkFOa0/kko01urmAjq6XMza9rUB+15P6t8DzcjhDbi3O9mxZLyEl+Ym+pgM1nYq0pdfi6Qi4DABIZJaA1ue7vzLVNZcV8vX+w+5IzcZs/657tlLQVDPyChgv5oSk0K0KjOPXrScr8hzhy1bu/voRDFs/H1wlLqvacCe+d5UdyXFPgaddNKaKBnqFIVsOjTS+GQG+3bXNFJJ5f9+RXRDJoW5hpXaBZ+ytNItCleDSWeMYexhfAUBlSXBhQ/pRO//VaBd/3lxVo0zG0xPcfxqiYrKDZDyPFhI+qMPxoC3oIUyIa+DjRnYwbmBf6tNAt6tbHZTBthCOPAuB7FumiDSGZIJYwg8mssZAGagQC14a2v4H72lVRdHTczRP4vXKUHhH94fCTFfOQS500iI9QlrtMNoqmFJS6uvsUsuF8rih03n7f9GpbTrbnDDujt6dBq2k6ABb2r8n94ScsFQesidKr6w3/589wLgdcp53KuYaP1Tez2kAl5Jlj/zGf31TTV3h2JngHmdd7413qCUSuztoWTiSdC9Iajb32xLb+EYqr0ts2p6y3BI/18rqy8HZ7yVOyYI9rAsdzxiNoqAnJt5GEIDCFwMZs8yumlDFHbK1Syrix2pn3O5uMkEpQ+4x+uqLBOd+0L9MC64zYSz8YERx4Nf29Ij5rpZECur78wGd/hiXRTTdVbdt8JktL6zyZXfT/PsiyNL6BQR4A6Jmfgwwxa6b4aS/n+tHC91HQoq5HA5RxQ2KcXz0TNTtvqLlDCDgZz4nTJEgQLfLdjI1JyM/yiukYUOBqojTZP9+n9T31l3KYqxkaKio5rlpqxlOq55sEHSfv1h65+0UbPoWhdhCb5t1SDU8oq7CQKm5+x2lAfHgUErj6bkFhZs4JKWpsDP6bmV6Sj7RCupwS9f7aigxukpuiPeCDx5I/h53KOQbPrF075mafV1GB2LW6smvkuAZ2bX19ORVwd+9PEOxxQKDCjYwYMkTjzo9/t/HRIF"
`endif