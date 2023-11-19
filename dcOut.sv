`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 11/13/2023 09:08:24 PM
// Design Name: 
// Module Name: dcOut
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


module dcOut(
        input clk,
        input clk_sampling,             // sampling clock pulse at 50Khz
        input enableA,
        input enableB,
        
        input reg signed[15:0] dc_ofsA,
        input reg signed[15:0] dc_ofsB,
        output reg  [11:0] dacA_dc_fin,
        output reg  [11:0] dacB_dc_fin
    );
        
    //CALIBRATION VARS
    reg signed [15:0] sampleA_signed; // USE THESE SIGNALS TO PASS INTO DIFF MODULES
    reg signed [15:0] sampleB_signed;

    reg [15:0] sampleA_unsigned; 
    reg [15:0] sampleB_unsigned;

    reg [11:0] dacA_word_fin;
    reg [11:0] dacB_word_fin;

    reg signed [15:0] dacA_slope_fp = 16'd1961;
    reg signed [15:0] dacB_slope_fp = 16'd1947;
    reg [11:0] dac_A_intercept = 12'd24;
    reg [11:0] dac_B_intercept = 12'd33;
    reg [7:0] calibration_scale = 8'd11;

    reg [31:0] sampleA_gained;
    reg [31:0] sampleB_gained;

//DC OFFSET BLOCK

    always_ff @(posedge clk) begin
        if (clk_sampling) begin
            if (enableA) begin
                //DC OFFSET IMPLEMENTATION
                sampleA_signed <= dc_ofsA >>>3;   // Convert a 16383 swing and Limit it to a swing of -2048 to 2048
        //Run samples A and B through calibration module
                sampleA_gained <= ((sampleA_signed*dacA_slope_fp)>>calibration_scale);
                sampleA_unsigned <= sampleA_gained + dac_A_intercept +16'd2048;  // Map to 0-4096 for the DAC words  
                dacA_word_fin <= sampleA_unsigned[11:0];   // Grab the last 11 bits to send to SPI
            end
        end

    end
    always_ff @(posedge clk) begin
        if (clk_sampling) begin
            if (enableB) begin
                //DC OFFSET IMPLEMENTATION
                sampleB_signed <= dc_ofsB >>>3;   // Divide by 8
        //Run samples A and B through calibration module
                sampleB_gained <= ((sampleB_signed*dacB_slope_fp)>>calibration_scale);
                sampleB_unsigned <= sampleB_gained + dac_B_intercept + 16'd2048;
                dacB_word_fin <= sampleB_unsigned[11:0];
            end
        end

    end

    assign dacA_dc_fin = dacA_word_fin;
    assign dacB_dc_fin = dacB_word_fin;

endmodule
