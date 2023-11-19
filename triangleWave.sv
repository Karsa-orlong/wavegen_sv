`timescale 1ns / 1ps

module triangleWave (
        input clk,
        input clk_sampling,             // sampling clock pulse at 50Khz
        input enableA,
        input enableB,

        input reg signed[15:0] dc_ofsA,
        input reg signed[15:0] dc_ofsB,
        
        input reg [15:0] halfMaxStepCountA,    // Number of writes we can do for desired freq. wave in (1/f_sampling)(s) time
        input reg [15:0] halfMaxStepCountB,
        input reg signed [15:0] stepValA,  // StepValue increments for writing the sawtooth
        input reg signed [15:0] stepValB, 

        output reg [11:0] dacA_tri_fin,
        output reg [11:0] dacB_tri_fin
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

//TRIANGLE WAVE VARS
    reg signed [18:00] accumulator_A = 16'd0, accumulator_B = 16'd0;
    reg signed [15:00] localTriangle_A = 16'd0, localTriangle_B = 16'd0;
    reg direction_A = 1'b0, direction_B = 1'b0;

    // Test with a starting accumulator value later

//CHANNEL A
    always_ff @ (posedge clk) begin
        if (clk_sampling) begin
            if (enableA)
            begin
                // Channel A
                if (direction_A == 1'b0)  accumulator_A <= accumulator_A - stepValA;
                else                    accumulator_A <= accumulator_A + stepValA;

                localTriangle_A <= localTriangle_A + 1;

                if (localTriangle_A == halfMaxStepCountA)   // Flip directions
                begin
                    localTriangle_A <= 6'd0;
                    direction_A <= ~direction_A;
                end
                //DC OFFSET IMPLEMENTATION
                sampleA_signed <= (accumulator_A + dc_ofsA) >>>3;   // Convert a 16383 swing and Limit it to a swing of -2048 to 2048
    //CALIBRATION MODULE
                sampleA_gained <= ((sampleA_signed*dacA_slope_fp)>>calibration_scale);
                sampleA_unsigned <= sampleA_gained + dac_A_intercept +16'd2048;  // Map to 0-4096 for the DAC words  
                dacA_word_fin <= sampleA_unsigned[11:0];  // Grab the last 11 bits to send to SPI
        
            end
        end
    end
//CHANNEL B
    always_ff @ (posedge clk) begin
        if (clk_sampling) begin
            if (enableB)
            begin
                // Channel B
                if (direction_B == 1'b0)  accumulator_B <= accumulator_B - stepValB;
                else                    accumulator_B <= accumulator_B + stepValB;

                localTriangle_B <= localTriangle_B + 1;

                if (localTriangle_B == halfMaxStepCountB)
                begin
                    localTriangle_B <= 6'd0;
                    direction_B <= ~direction_B;
                end
            end
            //DC OFFSET IMPLEMENTATION
                sampleB_signed <= (accumulator_B + dc_ofsB) >>>3;   // Divide by 
    //CALIBRATION MODULE
            sampleB_gained <= ((sampleB_signed*dacB_slope_fp)>>calibration_scale);
                sampleB_unsigned <= sampleB_gained + dac_B_intercept + 16'd2048;
                dacB_word_fin <= sampleB_unsigned[11:0];
        end
    end

    assign dacA_tri_fin = dacA_word_fin;
    assign dacB_tri_fin = dacB_word_fin;

endmodule