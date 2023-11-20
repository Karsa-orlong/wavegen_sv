`timescale 1ns / 1ps

module sawToothWave (

        input clk,                                  // System clock (100MHz)
        input clk_sampling,                         // sampling clock pulse at 50Khz
        input enableA,                              // Enable channel
        input enableB,                              // Enable channel

        input reg signed [15:00] dc_ofsA,           // Offset requested by the AXI module
        input reg signed [15:00] dc_ofsB,           // Offset requested by the AXI module

        input reg signed [15:00] ampl_A,            // Amplitude requested by AXI
        input reg signed [15:00] ampl_B,            // Amplitude requested by AXI

        input reg [15:00] maxStepCountA,            // Number of writes for desired freq. wave in (1/f_sampling)(s) time
        input reg [15:00] maxStepCountB,            // Number of writes for desired freq. wave in (1/f_sampling)(s) time

        input reg signed [15:00] stepValA,          // StepValue increments for writing the sawtooth
        input reg signed [15:00] stepValB,          // StepValue increments for writing the sawtooth

        output reg  [11:00] dacA_saw_fin,           // Output to the SPI module
        output reg  [11:00] dacB_saw_fin            // Output to the SPI module
    );


    reg signed [15:00] sampleA_signed;              // Signed A Channel value
    reg signed [15:00] sampleB_signed;              // Signed B Channel value

    reg [15:00] sampleA_unsigned;                   // Unsigned A Channel value
    reg [15:00] sampleB_unsigned;                   // Unsigned B Channel value

    reg [11:00] dacA_word_fin;                      // Value output from the module
    reg [11:00] dacB_word_fin;                      // Value output from the module

    reg signed [15:00] dacA_slope_fp = 16'd1961;    // Caliberation slope value
    reg signed [15:00] dacB_slope_fp = 16'd1947;    // Caliberation slope value
    reg [11:00] dac_A_intercept      = 12'd24;      // Caliberation intercept value
    reg [11:00] dac_B_intercept      = 12'd33;      // Caliberation intercept value
    reg [07:00] calibration_scale    = 8'd11;       // Caliberation scale

    reg [31:00] sampleA_gained;                     // Gain corrected value
    reg [31:00] sampleB_gained;                     // Gain corrected value


    reg signed [15:00] valA;
    reg signed [15:00] valB;

    reg signed [15:00] accumulatorA = -1 * ampl_A;
    reg signed [15:00] accumulatorB = -1 * ampl_B;

    reg  [15:00] counterA = 0;
    reg  [15:00] counterB = 0;

    // Channel A
    always_ff @ (posedge clk)
    begin
        if (clk_sampling)
        begin
            if (enableA)
            begin
                accumulatorA    <= accumulatorA + stepValA;
                counterA        <= counterA + 1;

                if (counterA == maxStepCountA)
                begin
                    accumulatorA <=  (-1 * ampl_A);
                    counterA     <= 0;
                end

                // DC Offset
                sampleA_signed  <= (accumulatorA - dc_ofsA) >>> 3;                          // Limit 16383 swing to -2048-2048

                // Caliberation
                sampleA_gained   <= ((sampleA_signed * dacA_slope_fp) >> calibration_scale);
                sampleA_unsigned <= sampleA_gained + dac_A_intercept + 16'd2048;            // Map to 0-4096 for the DAC words
                dacA_word_fin    <= sampleA_unsigned[11:00];                                // Grab the last 11 bits to send to SPI
            end
        end
    end

    // Channel B
    always_ff @(posedge clk)
    begin
        if (clk_sampling)
        begin
            if (enableB)
            begin
                accumulatorB    <= accumulatorB + stepValB;
                counterB        <= counterB + 1;

                if (counterB == maxStepCountB)
                begin
                    accumulatorB <= (-1 * ampl_B);
                    counterB     <= 0;
                end

                // DC Offset
                sampleB_signed  <= (accumulatorB - dc_ofsB) >>>3;                           // Limit 16383 swing to -2048-2048

                // Caliberation
                sampleB_gained   <= ((sampleB_signed * dacB_slope_fp) >> calibration_scale);
                sampleB_unsigned <= sampleB_gained + dac_B_intercept + 16'd2048;            // Map to 0-4096 for the DAC words
                dacB_word_fin    <= sampleB_unsigned[11:00];                                // Grab the last 11 bits to send to SPI
            end
        end
    end

    assign dacA_saw_fin = dacA_word_fin;
    assign dacB_saw_fin = dacB_word_fin;

endmodule