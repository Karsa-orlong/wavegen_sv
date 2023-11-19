`timescale 1ns / 1ps

/*
    @module multiplier to perform fixed-point multiplication operations
    @input  reg signed [15:00] operand1 operand
    @input  reg signed [15:00] operand2 operand
    @output reg signed [15:00] product resulting product
*/
module multiplier (
    input clk,
    input reg signed [15:0] x_fixed_point,
    input reg signed [15:0] y_fixed_point,
    input reg [7:0] fp_scale_factor,
    output reg signed [15:0] mul_result_fp
    );


    reg signed [31:0] mul_var;
    reg signed[15:0]  result;
     
    /*
        Perform operation: (fixedPointOperand * 2^14 * fixedPointOperand * 2^14) >> 2^14
        equivalent to (fixedPointOperand * 2^14 * fixedPointOperand * 2^14) / 2^14
        equivalent to (fixedPointOperand * fixedPointOperand)
    */
     
    always_ff @(negedge clk) begin
        mul_var <= ((x_fixed_point*y_fixed_point)); // signb is preserved
        result <= (mul_var >> fp_scale_factor);
    end

    assign mul_result_fp = result; // return a 16 bit signed integer in fixed point representation

endmodule
