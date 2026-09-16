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


module en_refclk_buffer //#()
(
    input  logic        clk,
    input  logic        reset,
    input  logic        pdp_avmm_waitrequest,
    input  logic [31:0] pdp_avmm_readdata,
    input  logic        pdp_avmm_readdatavalid,
    output logic [3:0 ] pdp_byte_enable,
    output logic        pdp_avmm_write,
    output logic        pdp_avmm_read,
    output logic [19:0] pdp_avmm_address,
    output logic [31:0] pdp_avmm_writedata
);

//--------------------------------------
// state assignments
//--------------------------------------
localparam  [2:0]   IDLE                    = 3'd0;
localparam  [2:0]   PHY_RD                  = 3'd1; 
localparam  [2:0]   PHY_CHK_RDDATA          = 3'd2; 
localparam  [2:0]   PHY_WR                  = 3'd3; 
localparam  [2:0]   DONE                    = 3'd4;  // Added DONE state

//--------------------------------------
// signals
//--------------------------------------
logic [2:0]     next_state;
logic [2:0]     state;
logic           first_check;  // Added first_check signal

// state register
always @(posedge clk or posedge reset)
begin
    if (reset) begin
        state <= IDLE;
    end else begin
        state <= next_state;
    end
end

// First check logic
always_ff @(posedge clk or posedge reset) begin
    if (reset)
        first_check <= 1'b0;
    else if (state == DONE)
        first_check <= 1'b1;
end

// next state logic
always @ (*) begin
  case(state)
    IDLE: begin
        if (!first_check || pdp_avmm_readdata[15:8] != 8'h00 )
            next_state = PHY_RD;
        else
            next_state = IDLE;
    end
    PHY_RD: begin
        if (pdp_avmm_waitrequest)
            next_state = PHY_RD;
        else
            next_state = PHY_CHK_RDDATA;
    end
    PHY_CHK_RDDATA: begin
        if (pdp_avmm_readdatavalid)
            next_state = PHY_WR;
        else
            next_state = PHY_CHK_RDDATA;
    end
    PHY_WR: begin
        if (pdp_avmm_waitrequest)
            next_state = PHY_WR;
        else
            next_state = DONE;
    end
    DONE: begin
        next_state = IDLE;
    end
    default : next_state = IDLE;
  endcase
end

//********************************************************************************
//*****************Generate PHY_Access signals for single PDP Interface***************
//PHY_Access read
always @(posedge clk or posedge reset)
begin
    if (reset)
        pdp_avmm_read  <= 1'b0; 
    else begin
        if(next_state == PHY_RD)
            pdp_avmm_read  <= 1'b1; 
        else
            pdp_avmm_read  <= 1'b0; 
    end
end

//PHY_Access write
always @(posedge clk or posedge reset)
begin
    if (reset)
        pdp_avmm_write  <= 1'b0; 
    else begin
        if(next_state == PHY_WR)
            pdp_avmm_write  <= 1'b1; 
        else 
            pdp_avmm_write  <= 1'b0; 
    end
end

//PHY_Access writedata
always @(posedge clk or posedge reset)
begin
    if (reset) begin
        pdp_avmm_writedata  <= 32'h0;
    end else begin
        if(next_state == PHY_WR)
            pdp_avmm_writedata <= {pdp_avmm_readdata[31:24], pdp_avmm_readdata[15:8], pdp_avmm_readdata[15:0]};
    end
end

//PHY_Access address
always @ (posedge clk or posedge reset)
begin
    if (reset)
        pdp_avmm_address  <= 20'd0;
    else begin
        if (next_state == PHY_RD)
            pdp_avmm_address <= 20'hA6038;
    end
end

assign pdp_byte_enable = pdp_avmm_write ? 4'b0100 : 4'b1111;

endmodule

`ifdef QUESTA_INTEL_OEM
`pragma questa_oem_00 "RD71bOkSrivegSkkp4I5+wDzCX2kXlX2aNpk/av3AGMum78fc0IH/D4D78z2PRJQvYxe1MAtGOZSq8lK1cGI/lriEdpS0q0WC4kxXTqXS6yX8taAoFd1MuJ+o4YnXLtnnY1KkVApm+y0e3pMzif6iKdYG6nEqYMpXcyVCrvdgVLYujco/9OJR1g2LoIMzmPPFZ68UJVexCndOypfappXOx88EvwA8I/Yuis7m8owvBhg8oHXTlE3vKhhXTbbfOU8+XHFHjFRqIE1srHa/qzVlpeHM6X78IYPqm5S1ePk66Bllsl7hwtD1dNLfGAPNZ/pMPFlogpTCR/33hN90pWVJ+C0HHdlnwHRneEefNtoGoB6gDQXhFpcFF9qXuqKu8eXyl25yJx5nP+doaLGcrhwOsVOeLt98kca3fPF414PgWbDXZsxjWq/T8sF4MfjYZrSBYHjUIhahwDBfChuDsm6WuzN+aBAD/annKyDrW8RlnL6kW5mNvkfe17hzw7bTg4nY/SNcViowfkMi/qU7LxmPPvasTL8Fzp3L5ZTPRX8HSs7ZkKv7CojQe5mHEQvBaLQwirOirDN0zGZ9kmg7diSleygA0Mmq6ld6fSgxD8EIhtNX2dAUlhbr1TF5/IajZrTDjK6D9FUZOW/DK/0rz4vMQVhcD8spaGgy2sV5wCkL3irRaWhzGtJmPc/uU2yoi6xp1LgkCX4zijhF3RAXTUWQfGA2GJOomdPDC0zKtSlQpptIcxaKvd7siCxLijyRCSlwWYMdofIQe7uEsOKn/x8D9kiIBZ38iYAHF+5iIVJ8QPQOKDWhS2ud5cOUSxKp/RmJ/OjJMugsxpzSr/l9kD1cCQ3t7T3FLnuznEdpMVYtMmHsBa+GVCy2NyzkEQ+kk6K+SQCChcNJo2nsBzqsOBgowtx8b0aMA82MvdzwmdXSEU1nCeumHyDpHT0blJHIeUyajdY6saQLCbDSoVaTE+3O6Ew00+5+V2ERPdCUPjfzKGjjlD2eEuBAqNYjydA+mr7"
`endif