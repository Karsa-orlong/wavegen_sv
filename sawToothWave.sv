`timescale 1ns / 1ps

module sawToothWave (

        input clk,
        input clk_sampling,             // sampling clock pulse at 50Khz
        input enableA,
        input enableB,

        input reg signed[15:0] dc_ofsA,
        input reg signed[15:0] dc_ofsB,
        input reg [15:0] maxStepCountA,    // Number of writes we can do for desired freq. wave in (1/f_sampling)(s) time
        input reg [15:0] maxStepCountB,
        input reg signed [15:0] stepValA,  // StepValue increments for writing the sawtooth
        input reg signed [15:0] stepValB, 

        output reg  [11:0] dacA_saw_fin,
        output reg  [11:0] dacB_saw_fin
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


//SAWTOOTH WAVE VARIABLES
    reg signed [15:0] valA;
    reg signed [15:0] valB;

    reg signed [15:00] accumulatorA = 0;
    reg  [15:00] counterA = 0;
    reg signed [15:00] accumulatorB = 0;
    reg  [15:00] counterB = 0;
// CHANNEL A
    always_ff @ (posedge clk) begin
        if (clk_sampling) begin

            if (enableA) begin
                accumulatorA <= accumulatorA + stepValA;
                counterA <= counterA + 1;

                if (counterA == maxStepCountA)
                begin
                    accumulatorA <= 0;
                    counterA <= 0;
                end
                //DC OFFSET IMPLEMENTATION
                sampleA_signed <= (accumulatorA + dc_ofsA) >>>3;   // Convert a 16383 swing and Limit it to a swing of -2048 to 2048
    //CALIBRATION MODULE
                sampleA_gained <= ((sampleA_signed*dacA_slope_fp)>>calibration_scale);
                sampleA_unsigned <= sampleA_gained + dac_A_intercept +16'd2048;  // Map to 0-4096 for the DAC words  
                dacA_word_fin <= sampleA_unsigned[11:0];   // Grab the last 11 bits to send to SPI
            end
        end
    end
//CHANNEL B

    always_ff @(posedge clk) begin
        if (clk_sampling) begin
            if (enableB) begin
                accumulatorB <= accumulatorB + stepValB;
                counterB <= counterB + 1;

                if (counterB == maxStepCountB)
                begin
                    accumulatorB <= 0;
                    counterB <= 0;
                end
                //DC OFFSET IMPLEMENTATION
                sampleB_signed <= (accumulatorB + dc_ofsB) >>>3;   // Divide by 
    //CALIBRATION MODULE
            sampleB_gained <= ((sampleB_signed*dacB_slope_fp)>>calibration_scale);
                sampleB_unsigned <= sampleB_gained + dac_B_intercept + 16'd2048;
                dacB_word_fin <= sampleB_unsigned[11:0];
            end
        end
    end

    assign dacA_saw_fin = dacA_word_fin;
    assign dacB_saw_fin = dacB_word_fin;

endmodule
