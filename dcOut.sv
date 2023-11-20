`timescale 1ns / 1ps

module dcOut (
        input clk,                                  // System Clock (100MHz)
        input clk_sampling,                         // Sampling clock pulse at 50Khz
        input enableA,                              // Enable channel
        input enableB,                              // Enable channel

        input reg signed[15:00] dc_ofsA,            // Inputs from the AXI modules
        input reg signed[15:00] dc_ofsB,            // Inputs from the AXI modules
        output reg  [11:00] dacA_dc_fin,            // Output to the SPI module
        output reg  [11:00] dacB_dc_fin             // Output to the SPI module
    );

    reg signed [15:00] sampleA_signed;              // Signed A Channel value
    reg signed [15:00] sampleB_signed;              // Signed B Channel value

    reg [15:00] sampleA_unsigned;                   // Unsigned A Channel value
    reg [15:00] sampleB_unsigned;                   // Unsigned B Channel value

    reg [11:00] dacA_word_fin;                      // Value output from the module
    reg [11:00] dacB_word_fin;                      // Value output from the module

    // Caliberation Variables
    reg signed [15:00] dacA_slope_fp = 16'd1961;    // Caliberation slope value
    reg signed [15:00] dacB_slope_fp = 16'd1947;    // Caliberation slope value
    reg [11:00] dac_A_intercept      = 12'd24;      // Caliberation intercept value
    reg [11:00] dac_B_intercept      = 12'd33;      // Caliberation intercept value
    reg [07:00] calibration_scale    = 8'd11;       // Caliberation scale

    reg [31:00] sampleA_gained;                     // Gain corrected value
    reg [31:00] sampleB_gained;                     // Gain corrected value

    // Channel A
    always_ff @(posedge clk)
    begin
        if (clk_sampling)
        begin
            if (enableA)
            begin
                sampleA_signed      <= dc_ofsA >>> 3;                                           // Limit swing from 16383 to -2048-2048

                // Calibration
                sampleA_gained      <= ((sampleA_signed * dacA_slope_fp) >> calibration_scale);
                sampleA_unsigned    <= sampleA_gained + dac_A_intercept +16'd2048;              // Map to 0-4096 for the DAC words
                dacA_word_fin       <= sampleA_unsigned[11:00];                                 // Grab the last 11 bits to send to SPI
            end
        end
    end

    // Channel B
    always_ff @ (posedge clk)
    begin
        if (clk_sampling)
        begin
            if (enableB)
            begin
                sampleB_signed      <= dc_ofsB >>> 3;                                           // Limit swing from 16383 to -2048-2048

                // Calibration
                sampleB_gained      <= ((sampleB_signed * dacB_slope_fp) >> calibration_scale);
                sampleB_unsigned    <= sampleB_gained + dac_B_intercept + 16'd2048;             // Map to 0-4096 for the DAC words
                dacB_word_fin       <= sampleB_unsigned[11:00];                                 // Grab the last 11 bits to send to SPI
            end
        end
    end

    assign dacA_dc_fin = dacA_word_fin;
    assign dacB_dc_fin = dacB_word_fin;
endmodule