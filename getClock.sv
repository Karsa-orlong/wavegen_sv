`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 11/11/2023 07:02:10 PM
// Design Name: 
// Module Name: getClock
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module getClock(
    input wire clk,
    input reg[15:0] count_to_freq,
    output reg out_clk1
    );

    reg [15:0]countUp = 16'd0;
    reg [15:0]count2M = 16'd0;



    always_ff @(posedge clk) begin
        if (countUp == count_to_freq) begin          // get a 2Mhz clock
            countUp <= 16'd0;
            out_clk1 <= ~out_clk1;    // Toggle the clock every 2000 counts to get a 50 Khz clock
        end 
        else begin
            countUp <= countUp + 1;
        end
    end

endmodule