`timescale 1ns / 1ps

module wavegen_system_top(
        input CLK100,
    output [9:0] LED,       // RGB1, RGB0, LED 9..0 placed from left to right
    output [2:0] RGB0,
    output [2:0] RGB1,
    output [3:0] SS_ANODE,   // Anodes 3..0 placed from left to right
    output [7:0] SS_CATHODE, // Bit order: DP, G, F, E, D, C, B, A
    input [11:0] SW,         // SWs 11..0 placed from left to right
    input [3:0] PB,          // PBs 3..0 placed from left to right
    inout [19:0] GPIO,       // PMODA-C 1P, 1N, ... 3P, 3N order
    output [3:0] SERVO,      // Servo outputs
    output PDM_SPEAKER,      // PDM signals for mic and speaker
    input PDM_MIC_DATA,
    output PDM_MIC_CLK,
    output ESP32_UART1_TXD,  // WiFi/Bluetooth serial interface 1
    input ESP32_UART1_RXD,
    output IMU_SCLK,         // IMU spi clk
    output IMU_SDI,          // IMU spi data input
    input IMU_SDO_AG,        // IMU spi data output (accel/gyro)
    input IMU_SDO_M,         // IMU spi data output (mag)
    output IMU_CS_AG,        // IMU cs (accel/gyro)
    output IMU_CS_M,         // IMU cs (mag)
    input IMU_DRDY_M,        // IMU data ready (mag)
    input IMU_INT1_AG,       // IMU interrupt (accel/gyro)
    input IMU_INT_M,         // IMU interrupt (mag)
    output IMU_DEN_AG,       // IMU data enable (accel/gyro)
    inout [14:0]DDR_addr,
    inout [2:0]DDR_ba,
    inout DDR_cas_n,
    inout DDR_ck_n,
    inout DDR_ck_p,
    inout DDR_cke,
    inout DDR_cs_n,
    inout [3:0]DDR_dm,
    inout [31:0]DDR_dq,
    inout [3:0]DDR_dqs_n,
    inout [3:0]DDR_dqs_p,
    inout DDR_odt,
    inout DDR_ras_n,
    inout DDR_reset_n,
    inout DDR_we_n,
    inout FIXED_IO_ddr_vrn,
    inout FIXED_IO_ddr_vrp,
    inout [53:0]FIXED_IO_mio,
    inout FIXED_IO_ps_clk,
    inout FIXED_IO_ps_porb,
    inout FIXED_IO_ps_srstb,

    output CS_,
    output CLK_SPI,
    output SDI,
    output LDAC_
    );

        // Terminate all of the unused outputs or i/o's
    // assign LED = 10'b0000000000;
    //assign RGB0 = 3'b000;
    assign RGB1 = 3'b000;
    // assign SS_ANODE = 4'b0000;
    // assign SS_CATHODE = 8'b11111111;
    // assign GPIO = 24'bzzzzzzzzzzzzzzzzzzzzzzzz;
    assign SERVO = 4'b0000;
    assign PDM_SPEAKER = 1'b0;
    assign PDM_MIC_CLK = 1'b0;
    assign ESP32_UART1_TXD = 1'b0;
    assign IMU_SCLK = 1'b0;
    assign IMU_SDI = 1'b0;
    assign IMU_CS_AG = 1'b1;
    assign IMU_CS_M = 1'b1;
    assign IMU_DEN_AG = 1'b0;

    // display g (gpio) on left seven segment display
    assign SS_ANODE = 4'b0111;
    assign SS_CATHODE = 8'b10010000;

//REGISTERS FROM AXI BUS EXPOSED TO TOP MODULE
    wire [31:00] ampl_W_I;
    wire [31:00] cycl_W_I;
    wire [31:00] dCyc_W_I;
    wire [31:00] frqA_W_I;
    wire [31:00] frqB_W_I;
    wire [31:00] mode_W_I;
    wire [31:00] ofst_W_I;
    wire [31:00] runn_W_I;

    wire clk = CLK100;

    vio_0 axitest (
        .clk(clk),              // input wire clk
        .probe_in0(ampl_W_I),  // input wire [31 : 0] probe_in0
        .probe_in1(cycl_W_I),  // input wire [31 : 0] probe_in1
        .probe_in2(dCyc_W_I),  // input wire [31 : 0] probe_in2
        .probe_in3(frqA_W_I),  // input wire [31 : 0] probe_in3
        .probe_in4(frqB_W_I),  // input wire [31 : 0] probe_in4
        .probe_in5(mode_W_I),  // input wire [31 : 0] probe_in5
        .probe_in6(ofst_W_I),  // input wire [31 : 0] probe_in6
        .probe_in7(runn_W_I),  // input wire [31 : 0] probe_in7
        .probe_in8(dacA_Val),
        .probe_in9(dacB_Val)
    );

//AXI bus register space
    reg [5:0] mode_regVal;
    reg [2:0] modeA;
    reg [2:0] modeB;


    reg signed [31:0] offset_regVal;
    reg signed [15:0] offset_dc_A;
    reg signed [15:0] offset_dc_B;

    reg signed [31:0] ampl_regVal;
    reg signed [15:0] amplA_R;
    reg signed [15:0] amplB_R;

    reg signed [31:0] dutyCyc_regVal;
    reg signed [15:0] dutyCycA;
    reg signed [15:0] dutyCycB;

    reg signed [31:0] cycles_regVal;
    reg signed [15:0] cyclesA;
    reg signed [15:0] cyclesB;

    reg [31:0] freqA_count;
    reg [31:0] freqB_count;

    assign mode_regVal      = mode_W_I;
    assign run_regVal       = runn_W_I;
    assign freqA_regVal     = frqA_W_I;
    assign freqB_regVal     = frqB_W_I;
    assign offset_regVal    = ofst_W_I;
    assign ampl_regVal      = ampl_W_I;
    assign dutyCyc_regVal   = dCyc_W_I;
    assign cycles_regVal    = cycl_W_I;

    assign modeA            = mode_regVal [02:00];
    assign modeB            = mode_regVal [05:03];
    assign runA             = run_regVal;
    assign runB             = run_regVal;
    assign offset_dc_A      = offset_regVal [15:00];
    assign offset_dc_B      = offset_regVal [31:16];
    assign amplA            = ampl_regVal [15:0];
    assign amplB            = ampl_regVal [31:16];
    assign dutyCycA         = dutyCyc_regVal [15:0];
    assign dutyCycB         = dutyCyc_regVal [31:16];
    assign cyclesA          = cycles_regVal [15:00];
    assign cyclesB          = cycles_regVal [31:16];

    assign freqA_count = 100000000 / freqA_regVal; // 100Mhz/desired frequency
    assign freqB_count = 100000000 / freqB_regVal;


// SPI WIRES TO DAC BLOCK
    wire cs_connect;
    wire sdi_connect;
    wire ldac_connect;

//WAVE VARIABLES
//SINE VARIABLES
    wire [15:00] Asin_w;

    wire CLK50K;
    reg clk_spibus;
    reg [15:00] CLK50K_load = 16'd1000;
    reg[15:0] spi_clk_count = 16'd25;   // 2Mhz clock

    reg [11:00] addrA = 12'h0, addrB = 12'h800;
    reg [15:00] cosFromCoe, sinFromCoe;

//SAWTOOTH,TRIANGLE,SQUARE VARIABLES
    reg signed [15:00] sawTW_A, sawTW_B;        // Outputs from the sawtooth module
    reg signed [15:00] trgW_A, trgW_B;          // Outputs from the triangle module
    reg [11:00] dacA_Val, dacB_Val;

//DC OFFSET VARIABLES
    reg [31:00] ofst_R_E;

//50KHZ SAMPLING CLOCK AND SPI WRITE TO DAC
    getClock CLK50K_I (.clk(clk), .count_to_freq(CLK50K_load),.out_clk1(CLK50K));

    getClock spi(.clk(clk), .count_to_freq(spi_clk_count),.out_clk1(clk_spibus)); // Get a 2Mhz  Spi Clk

    // Connect the wires coming from the spiModule to the Top Module ports
    assign CS_      = cs_connect;
    assign CLK_SPI  = clk_spibus;
    assign SDI      = sdi_connect;
    assign LDAC_    = ldac_connect;
//SPI WRITE ENDS

//PROBES

    reg enableA_probe;
    reg enableB_probe;

//CLOCKS
    reg clk_variableA;
    reg clk_variableB;


    getClock clks_for_wavesA (.clk(clk), .count_to_freq(freqA_count), .out_clk1(clk_variableA));
    getClock clks_for_wavesB (.clk(clk), .count_to_freq(freqB_count), .out_clk1(clk_variableB));

//SYNCHRONIZATION WITH 50Khz sampling frequency
    reg clk_50KHz_del;
    wire pulse_50KHz;
    always_ff@(posedge clk)
    begin
       clk_50KHz_del <=  CLK50K ;
    end

    assign pulse_50KHz   =  CLK50K & ~clk_50KHz_del;

//SYNC END
    //MODE SELECTION FOR CHANNEL A AND B AND CHANNEL ENABLES
    reg dcOffsetEnable_A, sineEnable_A, sawtoothEnable_A, triangleEnable_A, squareEnable_A, arbitaryEnable_A;
    reg dcOffsetEnable_B, sineEnable_B, sawtoothEnable_B, triangleEnable_B, squareEnable_B, arbitaryEnable_B;

    // Square Wave Registers
    reg [11:0] dacA_sq;   // Declare register for dacA_sq
    reg [11:0] dacB_sq;   // Declare register for dacB_sq

    // Sawtooth Wave Registers
    reg [11:0] dacA_saw;  // Declare register for dacA_saw
    reg [11:0] dacB_saw;  // Declare register for dacB_saw

    // Triangle Wave Registers
    reg [11:0] dacA_tri;  // Declare register for dacA_tri
    reg [11:0] dacB_tri;  // Declare register for dacB_tri

    // Sine Wave Registers
    reg [11:0] dacA_sine; // Declare register for dacA_sine
    reg [11:0] dacB_sine; // Declare register for dacB_sine

    // DC Offset Registers
    reg [11:0] dacA_dc;   // Declare register for dacA_dc
    reg [11:0] dacB_dc;   // Declare register for dacB_dc

    reg [63:0] deltaPhaseA;
    reg [63:0] deltaPhaseB;

    reg [15:0] stepcountA;
    reg [15:0] stepcountB;

    reg [15:0] stepValA;
    reg [15:0] stepValB;

    // reg [15:0] freqA_count;
    // reg [15:0] freqB_count;

    reg [31:0] mul_resultA;
    reg [31:0] mul_resultB;


    reg [31:0] freqA_regVal;
    reg [31:0] freqB_regVal;

    reg signed [15:0] amplA;
    reg signed [15:0] amplB;

    reg [2:0] modeA_R;
    reg [2:0] modeB_R;


    // Enable the respective modules for Channel A
    always_ff @(posedge clk)
    begin
        if (pulse_50KHz)
        begin
            // Default: Disable all modules for Channel A
            dcOffsetEnable_A    <= 1'b0;
            sineEnable_A        <= 1'b0;
            sawtoothEnable_A    <= 1'b0;
            triangleEnable_A    <= 1'b0;
            squareEnable_A      <= 1'b0;
            arbitaryEnable_A    <= 1'b0;

            case (modeA)
                6'd0:
                begin
                    dcOffsetEnable_A    <= 1'b1;                                        // Enable DC Offset for Channel A
                    dacA_Val            <= dacA_dc;
                end
                6'd1:
                begin
                    sineEnable_A        <= 1'b1;                                        // Enable Sine for Channel A
                    deltaPhaseA         <= ((freqA_regVal * 32'hFFFFFFFF) / 50000);     // 32 bit accumulator step value
                    dacA_Val            <= dacA_sine;
                end
                6'd2:
                begin
                    sawtoothEnable_A    <= 1'b1;                                        // Enable Sawtooth for Channel A
                    stepcountA          <= 50000 / freqA_regVal;
                    stepValA            <= ((2 * amplA )/ stepcountA);
                    dacA_Val            <= dacA_saw;
                end
                6'd3:
                begin
                    triangleEnable_A    <= 1'b1;                                        // Enable Triangle for Channel A
                    stepcountA          <= 25000 / freqA_regVal;
                    stepValA            <= ((2 * amplA) / stepcountA);
                    dacA_Val            <= dacA_tri;
                end
                6'd4:
                begin
                    squareEnable_A      <= 1'b1;                                        // Enable Square for Channel A
                    dacA_Val            <= dacA_sq;
                end
                6'd5:
                begin
                    arbitaryEnable_A    <= 1'b1;                                        // Enable Arbitrary for Channel A
                    // dacA_Val <= dacA_arb;
                end
            endcase
        end
    end
    // Enable the respective modules for Channel B
    always_ff @(posedge clk)
    begin
        if (pulse_50KHz)
        begin
            // Default: Disable all modules for Channel B
            dcOffsetEnable_B    <= 1'b0;
            sineEnable_B        <= 1'b0;
            sawtoothEnable_B    <= 1'b0;
            triangleEnable_B    <= 1'b0;
            squareEnable_B      <= 1'b0;
            arbitaryEnable_B    <= 1'b0;
                // For modeB
            case (modeB)
                6'd0:
                begin
                    dcOffsetEnable_B    <= 1'b1;                                        // Enable DC Offset for Channel B
                    dacB_Val            <= dacB_dc;
                end
                6'd1:
                begin
                    sineEnable_B        <= 1'b1;                                        // Enable Sine for Channel B
                    deltaPhaseB         <= ((freqB_regVal * 32'hFFFFFFFF) / 50000);     // 32 bit accumulator step value
                    dacB_Val            <= dacB_sine;
                end
                6'd2:
                begin
                    sawtoothEnable_B    <= 1'b1;                                        // Enable Sawtooth for Channel B
                    stepcountB          <= 50000 / freqB_regVal;
                    stepValB            <= ((2 * amplB) / stepcountB);
                    dacB_Val            <= dacB_saw;
                end
                6'd3:
                begin
                    triangleEnable_B    <= 1'b1;                                        // Enable Triangle for Channel B
                    stepcountB          <= 25000 / freqB_regVal;
                    stepValB            <= ((2 * amplB) / stepcountB);
                    dacB_Val            <= dacB_tri;
                end
                6'd4:
                begin
                    squareEnable_B      <= 1'b1;                                        // Enable Square for Channel B
                    dacB_Val            <= dacB_sq;
                end
                6'd5:
                begin
                    arbitaryEnable_B    <= 1'b1;                                        // Enable Arbitrary for Channel B
                    // dacB_Val <= dacB_arb;
                end
            endcase
        end
    end

    // Instantiate system wrapper
    system_wrapper system (
        .DDR_addr(DDR_addr),
        .DDR_ba(DDR_ba),
        .DDR_cas_n(DDR_cas_n),
        .DDR_ck_n(DDR_ck_n),
        .DDR_ck_p(DDR_ck_p),
        .DDR_cke(DDR_cke),
        .DDR_cs_n(DDR_cs_n),
        .DDR_dm(DDR_dm),
        .DDR_dq(DDR_dq),
        .DDR_dqs_n(DDR_dqs_n),
        .DDR_dqs_p(DDR_dqs_p),
        .DDR_odt(DDR_odt),
        .DDR_ras_n(DDR_ras_n),
        .DDR_reset_n(DDR_reset_n),
        .DDR_we_n(DDR_we_n),
        .FIXED_IO_ddr_vrn(FIXED_IO_ddr_vrn),
        .FIXED_IO_ddr_vrp(FIXED_IO_ddr_vrp),
        .FIXED_IO_mio(FIXED_IO_mio),
        .FIXED_IO_ps_clk(FIXED_IO_ps_clk),
        .FIXED_IO_ps_porb(FIXED_IO_ps_porb),
        .FIXED_IO_ps_srstb(FIXED_IO_ps_srstb),
        .ampl_W_O(ampl_W_I),                        // Get register values from lower levels
        .cycl_W_O(cycl_W_I),                        // Get register values from lower levels
        .dCyc_W_O(dCyc_W_I),                        // Get register values from lower levels
        .frqA_W_O(frqA_W_I),                        // Get register values from lower levels
        .frqB_W_O(frqB_W_I),                        // Get register values from lower levels
        .mode_W_O(mode_W_I),                        // Get register values from lower levels
        .ofst_W_O(ofst_W_I),                        // Get register values from lower levels
        .runn_W_O(runn_W_I)                         // Get register values from lower levels
    );

    // Instantiate dcOut module
    dcOut dc_inst(
        .clk(clk),
        .clk_sampling(pulse_50KHz),
        .enableA(dcOffsetEnable_A),
        .enableB(dcOffsetEnable_B),
        .dc_ofsA(offset_dc_A),
        .dc_ofsB(offset_dc_B),
        .dacA_dc_fin(dacA_dc),
        .dacB_dc_fin(dacB_dc)
    );

    // Instantiate squareWave module for Channel A
    squareWave square_inst(
        .clk(clk),
        .clk_sampling(pulse_50KHz),
        .clkA(clk_variableA),
        .clkB(clk_variableB),

        .enableA(squareEnable_A),
        .enableB(squareEnable_B),

        .dc_ofsA(offset_dc_A),
        .dc_ofsB(offset_dc_B),
        .ampl_A(amplA),
        .ampl_B(amplB),
        .dutyA(16'd1),
        .dutyB(16'd1),

        .dacA_sq_signed(dacA_sq),
        .dacB_sq_signed(dacB_sq)
    );

    sawToothWave saw_inst (
        .clk(clk),                          // Provide the main clock
        .clk_sampling(pulse_50KHz),         // Provide the sampling clock
        .enableA(sawtoothEnable_A),
        .enableB(sawtoothEnable_B),
        .ampl_A(amplA),
        .ampl_B(ampl_B),
        .dc_ofsA(offset_dc_A),
        .dc_ofsB(offset_dc_B),
        .maxStepCountA(stepcountA),         // Number of writes for desired frequency A
        .maxStepCountB(stepcountB),         // Number of writes for desired frequency B
        .stepValA(stepValA),                // Step value for A
        .stepValB(stepValB),                // Step value for B

        .dacA_saw_fin(dacA_saw),            // Output for DAC A
        .dacB_saw_fin(dacB_saw)             // Output for DAC B
    );

    triangleWave triangle_inst (
        .clk(clk),
        .clk_sampling(pulse_50KHz),
        .enableA(triangleEnable_A),
        .enableB(triangleEnable_B),
        .ampl_A(amplA),
        .ampl_B(ampl_B),
        .dc_ofsA(offset_dc_A),
        .dc_ofsB(offset_dc_B),
        .halfMaxStepCountA(stepcountA),
        .halfMaxStepCountB(stepcountB),
        .stepValA(stepValA),
        .stepValB(stepValB_probe),
        .dacA_tri_fin(dacA_tri),
        .dacB_tri_fin(dacB_tri)
    );


    sineWave sine_inst (
        .clk(clk),
        .clk_sampling(pulse_50KHz),
        .enableA(sineEnable_A),
        .enableB(sineEnable_B),
        .dc_ofsA(offset_dc_A),
        .dc_ofsB(offset_dc_B),
        .ampl_A(amplA),
        .ampl_B(amplB),
        .phaseA_offset(16'd0),
        .phaseB_offset(16'd0),
        .delta_phaseA(deltaPhaseA),
        .delta_phaseB(deltaPhaseB),
        .dacA_sine_fin(dacA_sine),
        .dacB_sine_fin(dacB_sine)
    );

    spiModule spiwrite (
        .clk(clk),
        .clk_spi(clk_spibus),
        .dacA_in(dacA_Val),         // dacA_Val and dacB_Val will be final values output from the modules
        .dacB_in(dacB_Val),
        .chipselect(cs_connect),
        .sdi(sdi_connect),
        .ldac(ldac_connect)
    );
endmodule
