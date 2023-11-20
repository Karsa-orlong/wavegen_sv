`timescale 1ns / 1ps

module triangleWave (
        input clk,                                  // System clock (100MHz)
        input clk_sampling,                         // sampling clock pulse at 50Khz

        input enableA,                              // Enable channel
        input enableB,                              // Enable channel

        input reg signed [15:00] dc_ofsA,           // Offset requested by the AXI module
        input reg signed [15:00] dc_ofsB,           // Offset requested by the AXI module

        input reg signed [15:00] ampl_A,            // Amplitude requested by AXI
        input reg signed [15:00] ampl_B,            // Amplitude requested by AXI

        input reg [15:00] halfMaxStepCountA,        // Number of writes for desired freq. wave in (1/f_sampling)(s) time
        input reg [15:00] halfMaxStepCountB,        // Number of writes for desired freq. wave in (1/f_sampling)(s) time

        input reg signed [15:00] stepValA,          // StepValue increments for writing the sawtooth
        input reg signed [15:00] stepValB,          // StepValue increments for writing the sawtooth

        output reg [11:00] dacA_tri_fin,            // Output to the SPI module
        output reg [11:00] dacB_tri_fin             // Output to the SPI module
    );


    reg signed [15:00] sampleA_signed;              // Signed A Channel value
    reg signed [15:00] sampleB_signed;              // Signed B Channel value

    reg [15:0] sampleA_unsigned;                    // Unsigned A Channel value
    reg [15:0] sampleB_unsigned;                    // Unsigned B Channel value

    reg [11:0] dacA_word_fin;                       // Value output from the module
    reg [11:0] dacB_word_fin;                       // Value output from the module

    reg signed [15:0] dacA_slope_fp = 16'd1961;     // Caliberation slope value
    reg signed [15:0] dacB_slope_fp = 16'd1947;     // Caliberation slope value
    reg [11:00] dac_A_intercept = 12'd24;           // Caliberation intercept value
    reg [11:00] dac_B_intercept = 12'd33;           // Caliberation intercept value
    reg [07:00] calibration_scale = 8'd11;          // Caliberation scale

    reg [31:00] sampleA_gained;                     // Gain corrected value
    reg [31:00] sampleB_gained;                     // Gain corrected value

//TRIANGLE WAVE VARS
    reg signed [18:00] accumulator_A = -1 * ampl_A;
    reg signed [18:00] accumulator_B = -1 * ampl_B;

    reg signed [15:00] counterA = 16'd0;
    reg signed [15:00] counterB = 16'd0;

    reg direction_A = 1'b0, direction_B = 1'b0;


    // Channel A
    always_ff @ (posedge clk)
    begin
        if (clk_sampling)
        begin
            if (enableA)
            begin
                if (direction_A == 1'b0)    accumulator_A <= accumulator_A - stepValA;          // Decrement count
                else                        accumulator_A <= accumulator_A + stepValA;          // Increment count

                counterA <= counterA + 1;

                if (counterA == halfMaxStepCountA)                                              // Flip directions
                begin
                    counterA        <= 16'd0;                                                   // Reset count
                    direction_A     <= ~direction_A;
                end

                // DC Offset
                sampleA_signed <= (accumulator_A + dc_ofsA) >>> 3;                              // Limit 16383 swing to -2048-2048

                // Caliberation
                sampleA_gained      <= ((sampleA_signed * dacA_slope_fp) >> calibration_scale);
                sampleA_unsigned    <= sampleA_gained + dac_A_intercept + 16'd2048;             // Map to 0-4096 for the DAC words
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
                if (direction_B == 1'b0)    accumulator_B <= accumulator_B - stepValB;          // Decrement count
                else                        accumulator_B <= accumulator_B + stepValB;          // Increment count

                counterB <= counterB + 1;

                if (counterB == halfMaxStepCountB)                                              // Flip directions
                begin
                    counterB        <= 16'd0;                                                   // Reset count
                    direction_B     <= ~direction_B;
                end

                // DC Offset
                sampleB_signed <= (accumulator_B + dc_ofsB) >>>3;                               // Limit 16383 swing to -2048-2048

                // Caliberatopn
                sampleB_gained      <= ((sampleB_signed * dacB_slope_fp) >> calibration_scale);
                sampleB_unsigned    <= sampleB_gained + dac_B_intercept + 16'd2048;             // Map to 0-4096 for the DAC words
                dacB_word_fin       <= sampleB_unsigned[11:00];                                 // Grab the last 11 bits to send to SPI
            end
        end
    end

    assign dacA_tri_fin = dacA_word_fin;
    assign dacB_tri_fin = dacB_word_fin;

endmodule