`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 11/14/2023 12:08:55 AM
// Design Name: 
// Module Name: sineWave
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


module sineWave(
    input clk,
    input clk_sampling,
    input enableA,
    input enableB,

    input reg signed[15:0] dc_ofsA,
    input reg signed[15:0] dc_ofsB,

	input reg signed[15:0] ampl_A,
	input reg signed[15:0] ampl_B,

    input reg [15:0] phaseA_offset,  // Comes from AXI bus in fixed point
    input reg [15:0] phaseB_offset,

    input reg [63:0] delta_phaseA,      // Calculate and convert it to fixed point in top module. 
    input reg [63:0] delta_phaseB,      // Real values of delta_phase will be in range of 0-1

    output reg  [11:0] dacA_sine_fin,
    output reg  [11:0] dacB_sine_fin
    );


//CALIBRATION VARS
    reg signed [15:0] sampleA_signed; // USE THESE SIGNALS TO PASS INTO DIFF MODULES
    reg signed [15:0] sampleB_signed;

    reg signed [15:0]  sampleA_unsigned; 
    reg signed [15:0]  sampleB_unsigned;

    reg [11:0] dacA_word_fin;
    reg [11:0] dacB_word_fin;

    reg signed [15:0] dacA_slope_fp = 16'd1961;
    reg signed [15:0] dacB_slope_fp = 16'd1947;
    reg [11:0] dac_A_intercept = 12'd24;
    reg [11:0] dac_B_intercept = 12'd33;

    reg [31:0] sampleA_gained;
    reg [31:0] sampleB_gained;
    
    reg [7:0] calibration_scale = 8'd11;
    reg [7:0] default_fp_scale = 8'd14;

//SINE VARIABLES
    reg signed [15:0] LUT_dataA;
    reg signed [15:0] LUT_dataB;
    reg signed [31:0] dataA_gained;
    reg signed [31:0] dataB_gained;

    reg [31:0] accumulatorA = 32'd0;
    reg [31:0] accumulatorB = 32'd0;
    reg [11:0] LUT_indexA;
    reg [11:0] LUT_indexB;

//    assign accumulatorA = 32'd0;
//    assign accumulatorB = 32'd0;

    always_ff @(posedge clk) begin
        //Phase accumulator
        if (clk_sampling) begin
            accumulatorA <= accumulatorA + delta_phaseA[31:0];
            LUT_indexA <= accumulatorA >> 20; // only need 12 bits so discard the lower bits
            dataA_gained <= ((LUT_dataA*ampl_A)>>>default_fp_scale);

            //DC OFFSET IMPLEMENTATION
            sampleA_signed <= (dataA_gained + dc_ofsA) >>>3;   // Convert a 16383 swing and Limit it to a swing of -2048 to 2048
//CALIBRATION MODULE
            sampleA_gained <= ((sampleA_signed*dacA_slope_fp)>>calibration_scale);
            sampleA_unsigned <= sampleA_gained + dac_A_intercept +16'd2048;  // Map to 0-4096 for the DAC words  
            dacA_word_fin <= sampleA_unsigned[11:0];   // Grab the last 11 bits to send to SPI
        end
    end

    always_ff @(posedge clk) begin
        //Phase accumulator
        if (clk_sampling) begin
            accumulatorB <= accumulatorB + delta_phaseB[31:0];
            LUT_indexB <= accumulatorB >> 20; // only need 12 bits so discard the lower bits
            dataB_gained <= ((LUT_dataB*ampl_B)>>default_fp_scale);
            //DC OFFSET IMPLEMENTATION
            sampleB_signed <= (dataB_gained + dc_ofsB) >>>3;   // Divide by 
//CALIBRATION MODULE
            sampleB_gained <= ((sampleB_signed*dacB_slope_fp)>>calibration_scale);
            sampleB_unsigned <= sampleB_gained + dac_B_intercept + 16'd2048;
            dacB_word_fin <= sampleB_unsigned[11:0];
        end
    end


    //----------- Begin Cut here for INSTANTIATION Template ---// INST_TAG
blk_mem_gen_1 coe (
  .clka(clk),    // input wire clka
  .addra(LUT_indexA),  // input wire [11 : 0] addra
  .douta(LUT_dataA),  // output wire [15 : 0] douta
  .clkb(clk),    // input wire clkb
  .addrb(LUT_indexB),  // input wire [11 : 0] addrb
  .doutb(LUT_dataB)  // output wire [15 : 0] doutb
);


ila_0 sine (
	.clk(clk), // input wire clk
	.probe0(dacA_word_fin), // input wire [15:0]  probe0  
	.probe1(dacB_word_fin),
    .probe2(clk_sampling) // input wire [15:0]  probe1
);

// ila_0 lut (
// 	.clk(clk), // input wire clk
// 	.probe0(LUT_dataA), // input wire [15:0]  probe0  
// 	.probe1(LUT_dataB),
//     .probe2(pulse_50KHz) // input wire [15:0]  probe1
// );

    assign dacA_sine_fin = dacA_word_fin;
    assign dacB_sine_fin = dacB_word_fin; 

endmodule



