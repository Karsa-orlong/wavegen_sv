`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company:
// Engineer:
//
// Create Date: 11/11/2023 07:03:43 PM
// Design Name:
// Module Name: spiModule
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


module spiModule(
    input clk,  // Get the Spi clocks from inside this module
    input clk_spi,
    input reg[11:0] dacA_in,
    input reg[11:0] dacB_in,

    output reg chipselect,     // Connect these to the actual registers in the top module using wires
    // output reg clk_spi,
    output reg sdi,
    output reg ldac
    );


 

    // Idle , Start SPI, write 16 bits, End SPI, Start SPI, write 16 bits, End SPI, Pulse LDAC
    reg [7:0]state = 8'd0;      // ranges from 0 to 40
    reg [7:0]index = 8'd15;
    reg sentFrame  = 1'b0;
    reg firstFrame = 1'b1;      // This will only be true for the first frame and false for the rest

    reg [15:0]dacA;
    reg [15:0]dacB;

    // wire [11:0]dacA_signal;
    // wire [11:0]dacB_signal;

    // wire dacA_load;
    // wire dacB_load;

       always_ff@(posedge clk) begin
    // Get the data bits to be sent from SW[9:0] + PB[3:2] temporarily to get the 12 bits
     dacA <= {4'b0011, dacA_in};
     dacB <= {4'b1011, dacB_in};
    
    end

    always_ff @(negedge clk_spi) begin
        if (state == 0) begin
            // SPI frame A
            chipselect <= 1'b0; // State 0: CS_ goes low and first bit of data is sampled
            sdi <= dacA[index]; // Assign the most significant bit of dacA
            state <= state + 1;
            index <= index -1;
        end else if (state >= 1 && state <= 15) begin
            // State 1-15: Write data from dacA bit by bit to GPIO_18
            sdi <= dacA[index]; // Assign the most significant bit of dacA
            index <= index -1;
            state <= state + 1;
        end else if (state == 16) begin
            chipselect <= 1'b1; // State 16: CS_ goes high
            state <= state + 1;
            index <= 8'd15;
        end
            // SPI frame B
        else if (state == 17) begin
            chipselect <= 1'b0; // State 17: CS_ goes low
            sdi <= dacB[index]; // Assign the most significant bit of dacA
            state <= state + 1;
            index <= index -1;
        end else if (state >= 18 && state <= 32) begin
            // State 20-36: Write data from dacB bit by bit to GPIO_18
            sdi <= dacB[index]; // Assign the most significant bit of dacB
            index <= index -1;
            state <= state + 1;
        end else if (state == 33) begin
            chipselect <= 1; // State 33: CS_ goes high
            state <= state + 1;
            index <= 8'd15;
        end
            //LDAC pulse
        else if (state == 34) begin
            ldac <= 1; // State 34: LDAC_ goes high
            state <= state + 1;
        end else if (state == 35) begin
            ldac <= 0; // State 35: LDAC_ goes low
            state <= state + 1;
            sentFrame <= 1'b1; // Sent One complete frame (A + B)
            firstFrame <= 1'b0;
        end
        // Loopback at 50Khz rate. This is controlled by another always bloack at 50Khz rate
        else begin
            if(state == 40) begin
                state <= 8'd0;
                sentFrame <= 1'b0;      // Mark the upcoming new frame as unsent
            end
            else begin
            state <= state +1;
            end
        end
    end



endmodule