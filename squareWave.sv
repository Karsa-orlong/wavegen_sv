`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 11/13/2023 06:50:44 PM
// Design Name: 
// Module Name: squareWave
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


module squareWave(

	input clk,
    input clk_sampling,             // sampling clock pulse at 50Khz
    input clkA,                     // Clocks based on the frequencies desired for A and B
    input clkB,
    input enableA,
    input enableB,

    input reg signed[15:0] dc_ofsA,
    input reg signed[15:0] dc_ofsB,
	input reg signed[15:0] ampl_A,
	input reg signed[15:0] ampl_B,
	input reg[15:0] dutyA,
	input reg[15:0] dutyB,

	output reg [11:0] dacA_sq_signed,
	output reg [11:0] dacB_sq_signed
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


//SYNCHRONIZATION WITH 50Khz sampling frequency
    reg clkA_del;
    reg clkB_del;
    wire pulseA_rising; 
    wire pulseA_falling; 
    wire pulseB_rising; 
    wire pulseB_falling; 

    always_ff@(posedge clk)
    begin
        clkA_del <=  clkA ;
        clkB_del <=  clkB ;
    end 

    assign pulseA_rising   =  clkA & ~clkA_del   ; 
    assign pulseA_falling   =  clkA_del & ~clkA   ; 
    assign pulseB_rising   =  clkB & ~clkB_del   ; 
    assign pulseB_falling   =  clkB_del & ~clkB   ; 
//SYNC END



// Write the DAC values at the frequency desired and swing between -Amplitude to +Amplitude
//SQUARE WAVE VARS
	reg signed [15:0] valA;
	reg signed [15:0] valB;

    always_ff @(posedge clk) begin
        //Channel A square wave
        if (enableA) begin
            if(pulseA_rising) begin
                valA <= ampl_A;
            end
            if(pulseA_falling) begin
                valA <= -1*ampl_A;
            end
        end
    end
	

    always_ff @(posedge clk) begin
        if (enableB) begin
        //Channel B square wave
            if(pulseB_rising) begin
                valB <= ampl_B;
            end
            if(pulseB_falling) begin
                valB <= -1*ampl_B;
            end
        end

    end
	
    always_ff @(posedge clk) begin
        if (clk_sampling) begin
            if (enableA) begin
                sampleA_signed <= (valA + dc_ofsA) >>>3;   // Convert a 16383 swing and Limit it to a swing of -2048 to 2048
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
                sampleB_signed <= (valB + dc_ofsB) >>>3;   // Divide by 8 to limit swing between -2028 to 2047
    //Run samples A and B through calibration module
                sampleB_gained <= ((sampleB_signed*dacB_slope_fp)>>calibration_scale);
                sampleB_unsigned <= sampleB_gained + dac_B_intercept + 16'd2048;
                dacB_word_fin <= sampleB_unsigned[11:0];
            end
        end
    end

    assign dacA_sq_signed = dacA_word_fin;
    assign dacB_sq_signed = dacB_word_fin;
endmodule
