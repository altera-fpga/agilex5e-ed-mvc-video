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


module sdi_phy_adapter #(
    parameter   VIDEO_STANDARD = "mr",
    parameter   DIRECTION = "du"
)(
    input  logic                                            tx_reset,
    input  logic                                            rx_reset,
    input  logic                                            tx_syspll_clkout,
    input  logic                                            rx_syspll_clkout,
    input  logic                                            tx_vid_clkout,
    input  logic                                            rx_vid_clkout,
    input  logic                                            gxb_tx_ready,
    input  logic                                            gxb_rx_ready,
    input  logic                                            tx_cadence,
    input  logic [60*(VIDEO_STANDARD == "mr"  | 
                      VIDEO_STANDARD == "twelveg")+20-1:0]  sdi_txdata,
    input  logic [79:0]                                     rx_parallel_data,
    input  logic  [2:0]                                     rx_vid_std,

    output logic [79:0]                                     tx_parallel_data,
    output logic [60*(VIDEO_STANDARD == "mr" | 
                      VIDEO_STANDARD == "twelveg")+20-1:0]  rxdata_to_sdi
);

`ifndef __TILE_IP__
//----------------------------------------------------------------------------------------
// Generate for Tx
//----------------------------------------------------------------------------------------
generate if (DIRECTION == "tx" || DIRECTION == "du")
begin : tx_comp_gen
    logic                                           txfifo_rdreq;
    logic                                           tx_fifo_data_valid;
    logic                                           gxb_tx_ready_sync;
    logic                                           tx_rst_sync;
    logic [2*(VIDEO_STANDARD == "mr"  | 
              VIDEO_STANDARD == "twelveg")+4-1:0]   txfifo_rdusedw;
    logic [39:0]                                    txfifo_dataout;

    altera_reset_controller #(
        .NUM_RESET_INPUTS          (1),
        .RESET_REQ_WAIT_TIME       (1),
        .MIN_RST_ASSERTION_TIME    (3),
        .RESET_REQ_EARLY_DSRT_TIME (1)
    ) tx_rst_sync_inst (
    // Input Clock and reset
        .reset_in0      (tx_reset),
        .clk            (tx_syspll_clkout),
    // Outputs
        .reset_out      (tx_rst_sync)
    );

    altera_std_synchronizer #(
        .depth(3)
    ) gxb_tx_ready_sync_inst (
        .clk(tx_vid_clkout),
        .reset_n(1'b1),
        .din(gxb_tx_ready),
        .dout(gxb_tx_ready_sync)
    );

    txdata_fifo txdata_dcfifo_inst (
    // Input Clocks and reset
        .aclr  (tx_reset),
        .rdclk (tx_syspll_clkout),
        .wrclk (tx_vid_clkout),
    // Inputs
        .data (sdi_txdata),
        .rdreq (txfifo_rdreq),
        .wrreq (gxb_tx_ready_sync),
    // Outputs
        .q (txfifo_dataout),
        .rdempty (),
        .rdusedw (txfifo_rdusedw),
        .wrfull (),
        .wrusedw ()
    );

    always @(posedge tx_syspll_clkout) begin
        tx_fifo_data_valid  <= txfifo_rdreq;
    end

    if (VIDEO_STANDARD == "mr" | VIDEO_STANDARD == "twelveg")
    begin : mr_fifo_rdreq_gen
        //----------------------------------------------------------------------------------------
        // Generate Fifo Rdreq for Multi rate mode
        //----------------------------------------------------------------------------------------
        logic tx_cadence_dly;

        always @(posedge tx_syspll_clkout) begin
            tx_cadence_dly <= tx_cadence;
        end

        always @(posedge tx_syspll_clkout) begin
            if (tx_rst_sync) begin
                txfifo_rdreq <= 1'b0;
            end else if (txfifo_rdusedw[5]) begin
                txfifo_rdreq <= tx_cadence || tx_cadence_dly;
            end
        end
    end else
    begin : non_mr_fifo_rdreq_gen
        //----------------------------------------------------------------------------------------
        // Generate Fifo Rdreq for Triple rate / Single rate HD or 3G mode
        //----------------------------------------------------------------------------------------
        logic [1:0] tx_cadence_cnt;
        assign txfifo_rdreq = tx_cadence_cnt[1];

        always @(posedge tx_syspll_clkout) begin
            if (tx_rst_sync) begin
                tx_cadence_cnt <= 2'd0;
            end else if (txfifo_rdusedw[3]) begin
                if (tx_cadence_cnt[1]) begin
                    tx_cadence_cnt  <= tx_cadence;
                end else begin
                    tx_cadence_cnt  <= tx_cadence_cnt + tx_cadence;
                end
            end
        end
    end

    assign tx_parallel_data = {20'd0, txfifo_dataout[39:20], 1'b0, tx_fifo_data_valid, 18'd0, txfifo_dataout[19:0]};
end else 
begin : txdata_empty_gen
    assign tx_parallel_data = 80'd0;
end
endgenerate

//----------------------------------------------------------------------------------------
// Generate for Rx
//----------------------------------------------------------------------------------------
generate if (DIRECTION == "rx" || DIRECTION == "du")
begin : rx_comp_gen
    logic                                           gxb_rx_ready_sync;
    logic                                           rx_rst_sync;
    logic [39:0]                                    rx_parallel_data_reg;
    logic                                           rx_parallel_data_valid;
    logic [19:0]                                    rx_3ghd_fifo_dataout;
    logic [39:0]                                    rx_6g_fifo_dataout;
    logic [79:0]                                    rx_12g_fifo_dataout;

    altera_reset_controller #(
        .NUM_RESET_INPUTS          (1),
        .RESET_REQ_WAIT_TIME       (1),
        .MIN_RST_ASSERTION_TIME    (3),
        .RESET_REQ_EARLY_DSRT_TIME (1)
    ) rx_rst_sync_inst (
    // Input Clock and reset
        .reset_in0      (rx_reset),
        .clk            (rx_vid_clkout),
    // Outputs
        .reset_out      (rx_rst_sync)
    );

    altera_std_synchronizer #(
        .depth(3)
    ) gxb_rx_ready_sync_inst (
        .clk(rx_syspll_clkout),
        .reset_n(1'b1),
        .din(gxb_rx_ready),
        .dout(gxb_rx_ready_sync)
    );

    // M20K block may be placed far from transceiver hard block in certain compilation.
    // Placing flops in between PHY and FIFO to relax timing
    always @ (posedge rx_syspll_clkout)
    begin
        rx_parallel_data_reg <= {rx_parallel_data[59:40],rx_parallel_data[19:0]};
        rx_parallel_data_valid <= gxb_rx_ready_sync & rx_parallel_data[38];
    end

    if (VIDEO_STANDARD != "twelveg")
    begin : rxdata_3ghd_gen
        logic       rx_3ghd_fifo_rdreq;
        logic [5:0] rx_3ghd_fifo_rdusedw;

        rxdata_3ghd_fifo rxdata_3ghd_fifo_inst (
        // Input Clocks and reset
            .aclr       (rx_reset),
            .rdclk      (rx_vid_clkout),
            .wrclk      (rx_syspll_clkout),
        // Inputs
            .data       (rx_parallel_data_reg),
            .wrreq      (rx_parallel_data_valid),
            .rdreq      (rx_3ghd_fifo_rdreq),
        // Outputs
            .q          (rx_3ghd_fifo_dataout),
            .rdempty    (),
            .rdusedw    (rx_3ghd_fifo_rdusedw),
            .wrfull     (),
            .wrusedw    ()
        );

        // Wait until FIFO half full before start reading
        always @ (posedge rx_vid_clkout)
        begin
            if (rx_rst_sync) begin
                rx_3ghd_fifo_rdreq <= 1'b0;
            end else begin
                if (rx_3ghd_fifo_rdusedw[5]) begin
                    rx_3ghd_fifo_rdreq <= 1'b1;
                end
            end
        end
    end else
    begin : rxdata_3ghd_empty_gen
        assign rx_3ghd_fifo_dataout = 20'd0;
    end

    if (VIDEO_STANDARD == "mr")
    begin : rxdata_6g_gen
        logic       rx_6g_fifo_rdreq;
        logic [4:0] rx_6g_fifo_rdusedw;

        rxdata_6g_fifo rxdata_6g_fifo_inst (
        // Input Clocks and reset
            .aclr       (rx_reset),
            .rdclk      (rx_vid_clkout),
            .wrclk      (rx_syspll_clkout),
        // Inputs
            .data       (rx_parallel_data_reg),
            .wrreq      (rx_parallel_data_valid),
            .rdreq      (rx_6g_fifo_rdreq),
        // Outputs
            .q          (rx_6g_fifo_dataout),
            .rdempty    (),
            .rdusedw    (rx_6g_fifo_rdusedw),
            .wrfull     (),
            .wrusedw    ()
        );

        // Wait until FIFO half full before start reading
        always @ (posedge rx_vid_clkout)
        begin
            if (rx_rst_sync) begin
                rx_6g_fifo_rdreq <= 1'b0;
            end else begin
                if (rx_6g_fifo_rdusedw[4]) begin
                    rx_6g_fifo_rdreq <= 1'b1;
                end
            end
        end
    end else
    begin : rxdata_6g_empty_gen
        assign rx_6g_fifo_dataout = 40'd0;
    end

    if (VIDEO_STANDARD == "mr" | VIDEO_STANDARD == "twelveg")
    begin : rxdata_12g_gen
        logic           rx_12g_fifo_rdreq;
        logic [3:0]     rx_12g_fifo_rdusedw;

        rxdata_12g_fifo rxdata_12g_fifo_inst (
        // Input Clocks and reset
            .aclr       (rx_reset),
            .rdclk      (rx_vid_clkout),
            .wrclk      (rx_syspll_clkout),
        // Inputs
            .data       (rx_parallel_data_reg),
            .wrreq      (rx_parallel_data_valid),
            .rdreq      (rx_12g_fifo_rdreq),
        // Outputs
            .q          (rx_12g_fifo_dataout),
            .rdempty    (),
            .rdusedw    (rx_12g_fifo_rdusedw),
            .wrfull     (),
            .wrusedw    ()
        );

        // Wait until FIFO half full before start reading
        always @ (posedge rx_vid_clkout)
        begin
            if (rx_rst_sync) begin
                rx_12g_fifo_rdreq <= 1'b0;
            end else begin
                if (rx_12g_fifo_rdusedw[3]) begin
                    rx_12g_fifo_rdreq <= 1'b1;
                end
            end
        end
    end else
    begin : rxdata_12g_empty_gen
        assign rx_12g_fifo_dataout = 80'd0;
    end

    if (VIDEO_STANDARD == "mr")
    begin : rxdata_to_mr_sdi_gen
        logic [79:0]    rxdata_tmp;
        always @ (posedge rx_vid_clkout)
        begin
            if (rx_vid_std[2:1] == 2'b11) begin
                rxdata_tmp <= rx_12g_fifo_dataout;
            end else if (rx_vid_std[2:1] == 2'b10) begin
                rxdata_tmp <= {40'd0, rx_6g_fifo_dataout};
            end else begin
                rxdata_tmp <= {60'd0, rx_3ghd_fifo_dataout};
            end
        end

        assign rxdata_to_sdi = rxdata_tmp;
    end else if (VIDEO_STANDARD == "twelveg")
    begin : rxdata_to_twelveg_sdi_gen
        assign rxdata_to_sdi = rx_12g_fifo_dataout;
    end else
    begin : rxdata_to_3ghd_sdi_gen
        assign rxdata_to_sdi = rx_3ghd_fifo_dataout;
    end
end else
begin : rxdata_empty_gen
    assign rxdata_to_sdi = 80'd0;
end
endgenerate
`endif

endmodule
`ifdef QUESTA_INTEL_OEM
`pragma questa_oem_00 "FIa0bKgJc9xKKmOUhV4QULS7HBT5OYtvwZgp8PMF+Yp9WqzKy+jBcTGtWs2h7fdIqAKwq5puOWhjiBc6x96v7V/wduz3P5sftuXki0lNs/B5QdNbqLUpNCe7g7CIUJgkMPKKZLE3VA7bUT6XrN9XKsHWCzGo59cUGiL54lkSCJ1OFFlmrPKvZkFd/2NF15ReDiLmQ8wSkO8I58bfHsKPSDAYKD3tKVz6FWiV8H6e3+Bk5mt0MoTsL2pezKw8Tr/UzbPItWMdECEwerCPmxVf8cd3AouYZ/1nnUrbLQX/JP47XkNwHt6h4cxttkWXsK5apng8kCwREz6I8qniFWUNFN4xVKbA0hBeG8ZgUcIuF23gwi5tyArp/1N1skhmpa3BsIPJn8OsxgvLz9PxXo6K2VsFygt9cho3YEbr+LziCJYatMGz6tGuMf/XgDAxkRbUlT6Zp+4DFJ/QMq0ERlXG7QhhuTbElvLEFw+J5GlV0wDSJKK1I2Walohbgy4g9hsm5XiP03Lk/nF7JltBUrKal2syAyUXj1nDMnzT3mu8V1azj3Drr2w8OXrilEr1rBBf8hA70Vf4YvQcGxFgRGq19x9aPPfA19CuAQgGtJA4ixbg6U+aad1KYmmAybJIctZshjN2m+L5mUvy930JVlGUrU/2OulOjL6jXRof7GVrhYvD7sSa96yT7FdZJdQAuDFReno8fr1xgFNfBZcqJWV7gOnMO4Qer9/qkF0gEeLXsPfIJkDt0zJwhHRZxv5XmkLLWUZICMVocuNq+xLXVOzQlwi0w3zsW6mfGrBaPCF1z8u2oO06qdhsNgU1XW4xPNhP8LfcMffQcFyS0xqNvju51+uHjC9ZfLlEXrKv6R/dIAApBoGQMr7rGzbsJ8rOFnNkY0W1teYKvd0MFI43NIP6FqfXYxKe6GCdFMxpnsZ0sS09WZsYCopVMxrDUKUS1krM/YoLnfMKrC4dNOmQFjNW6PeSjvSsBh5u7vH6VUks70zhIN5miyTmKQOR7XjiOIYI"
`endif