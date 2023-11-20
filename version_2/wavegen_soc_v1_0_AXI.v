
`timescale 1 ns / 1 ps

module wavegen_soc_v1_0_AXI #
	(
		// Bit width of S_AXI address bus
        parameter integer C_S_AXI_ADDR_WIDTH = 5
    )
    (
        // Ports to top level module (what makes this the register IP module)
        output wire [31:00] mode_W_O,               // Mode Wire Output
        output wire [31:00] runn_W_O,               // Run Wire Output
        output wire [31:00] frqA_W_O,               // Frequency A Wire Output
        output wire [31:00] frqB_W_O,               // Frequency B Wire Output
        output wire [31:00] ofst_W_O,               // Offset Wire Output
        output wire [31:00] ampl_W_O,               // Amplitude Wire Output
        output wire [31:00] dCyc_W_O,               // Duty Cycels Wire Output
        output wire [31:00] cycl_W_O,               // Cycles Wire Output

        input wire S_AXI_ACLK,                      // AXI Clock
        input wire S_AXI_ARESETN,                   // AXI reset

        // AXI write channel
        // address:  add, protection, valid, ready
        // data:     data, byte enable strobes, valid, ready
        // response: response, valid, ready
        input wire [C_S_AXI_ADDR_WIDTH - 1:00] S_AXI_AWADDR,
        input wire [02:00] S_AXI_AWPROT,
        input wire S_AXI_AWVALID,
        output wire S_AXI_AWREADY,

        input wire [31:00] S_AXI_WDATA,
        input wire [03:00] S_AXI_WSTRB,
        input wire S_AXI_WVALID,
        output wire  S_AXI_WREADY,

        output wire [01:00] S_AXI_BRESP,
        output wire S_AXI_BVALID,
        input wire S_AXI_BREADY,

        /* AXI read channel
         * address: add, protection, valid, ready
         * data:    data, resp, valid, ready
         */
        input wire [C_S_AXI_ADDR_WIDTH-1:0] S_AXI_ARADDR,
        input wire [02:00] S_AXI_ARPROT,
        input wire S_AXI_ARVALID,
        output wire S_AXI_ARREADY,

        output wire [31:00] S_AXI_RDATA,
        output wire [01:00] S_AXI_RRESP,
        output wire S_AXI_RVALID,
        input wire S_AXI_RREADY
    );

    // Internal registers
    reg [31:0] mode_R_I_WR;                         // Mode                 Register Internal Write/Read
    reg [31:0] runn_R_I_WR;                         // Run                  Register Internal Write/Read
    reg [31:0] frqA_R_I_WR;                         // Frequency Channel A  Register Internal Write/Read
    reg [31:0] frqB_R_I_WR;                         // Frequency Channel B  Register Internal Write/Read
    reg [31:0] ofst_R_I_WR;                         // Offset               Register Internal Write/Read
    reg [31:0] ampl_R_I_WR;                         // Amplitude            Register Internal Write/Read
    reg [31:0] dCyc_R_I_WR;                         // Duty Cycle           Register Internal Write/Read
    reg [31:0] cycl_R_I_WR;                         // Cycles               Register Internal Write/Read

    // Register numbers
    localparam integer MODE_REG_P = 3'b000;         // Register to hold mode value
    localparam integer RUN__REG_P = 3'b001;         // Register to hold run value
    localparam integer FRQA_REG_P = 3'b010;         // Register to hold frequency Ch A value
    localparam integer FRQB_REG_P = 3'b011;         // Register to hold frequency Ch A value
    localparam integer OFST_REG_P = 3'b100;         // Register to hold offset value
    localparam integer AMPL_REG_P = 3'b101;         // Register to hold amplitude value
    localparam integer DCYC_REG_P = 3'b110;         // Register to hold duty cycle value
    localparam integer CYCL_REG_P = 3'b111;         // Register to hold cycles value

    // AXI4-lite signals
    reg axi_awready;
    reg axi_wready;
    reg [01:00] axi_bresp;
    reg axi_bvalid;
    reg axi_arready;
    reg [31:00] axi_rdata;
    reg [01:00] axi_rresp;
    reg axi_rvalid;

    // friendly clock, reset, and bus signals from master
    wire axi_clk            = S_AXI_ACLK;
    wire axi_resetn         = S_AXI_ARESETN;
    wire axi_awvalid        = S_AXI_AWVALID;
    wire axi_wvalid         = S_AXI_WVALID;
    wire axi_bready         = S_AXI_BREADY;
    wire axi_arvalid        = S_AXI_ARVALID;
    wire axi_rready         = S_AXI_RREADY;
    wire [31:00] axi_awaddr = S_AXI_AWADDR;
    wire [31:00] axi_araddr = S_AXI_ARADDR;
    wire [03:00] axi_wstrb  = S_AXI_WSTRB;

    // assign bus signals to master to internal reg names
    assign S_AXI_AWREADY = axi_awready;
    assign S_AXI_WREADY  = axi_wready;
    assign S_AXI_BRESP   = axi_bresp;
    assign S_AXI_BVALID  = axi_bvalid;
    assign S_AXI_ARREADY = axi_arready;
    assign S_AXI_RDATA   = axi_rdata;
    assign S_AXI_RRESP   = axi_rresp;
    assign S_AXI_RVALID  = axi_rvalid;

    /*
     * Assert address ready handshake (axi_awready)
     * - after address is valid (axi_awvalid)
     * - after data is valid (axi_wvalid)
     * - while configured to receive a write (aw_en)
     * De-assert ready (axi_awready)
     * - after write response channel ready handshake received (axi_bready)
     * - after this module sends write response channel valid (axi_bvalid)
     */
    wire wr_add_data_valid = axi_awvalid && axi_wvalid;
    reg aw_en;
    always_ff @ (posedge axi_clk)
    begin
        if (axi_resetn == 1'b0)
        begin
            axi_awready <= 1'b0;
            aw_en <= 1'b1;
        end
        else
        begin
            if (wr_add_data_valid && ~axi_awready && aw_en)
            begin
                axi_awready <= 1'b1;
                aw_en <= 1'b0;
            end

            else if (axi_bready && axi_bvalid)
            begin
                aw_en <= 1'b1;
                axi_awready <= 1'b0;
            end

            else
            begin
                axi_awready <= 1'b0;
            end
        end
    end

    /*
     * Capture the write address (axi_awaddr) in the first clock (~axi_awready)
     * - after write address is valid (axi_awvalid)
     * - after write data is valid (axi_wvalid)
     * - while configured to receive a write (aw_en)
     */
    reg [C_S_AXI_ADDR_WIDTH-1:0] waddr;
    always_ff @ (posedge axi_clk)
    begin
        if (axi_resetn == 1'b0)                                 waddr <= 0;
        else if (wr_add_data_valid && ~axi_awready && aw_en)    waddr <= axi_awaddr;
    end

    /* Output write data ready handshake (axi_wready) generation for one clock
     * - after address is valid (axi_awvalid)
     * - after data is valid (axi_wvalid)
     * - while configured to receive a write (aw_en)
     */
    always_ff @ (posedge axi_clk)
    begin
        if (axi_resetn == 1'b0) axi_wready <= 1'b0;
        else                    axi_wready <= (wr_add_data_valid && ~axi_wready && aw_en);
    end

    /* Write data to internal registers
     * - after address is valid (axi_awvalid)
     * - after write data is valid (axi_wvalid)
     * - after this module asserts ready for address handshake (axi_awready)
     * - after this module asserts ready for data handshake (axi_wready)
     * write correct bytes in 32-bit word based on byte enables (axi_wstrb)
     */
     wire wr = wr_add_data_valid && axi_awready && axi_wready;
    integer byte_index;
    always_ff @ (posedge axi_clk)
    begin
        if (axi_resetn == 1'b0)
        begin
            mode_R_I_WR <= 32'd0;
            runn_R_I_WR <= 32'd0;
            frqA_R_I_WR <= 32'd0;
            frqB_R_I_WR <= 32'd0;
            ofst_R_I_WR <= 32'd0;
            ampl_R_I_WR <= 32'd0;
            dCyc_R_I_WR <= 32'd0;
            cycl_R_I_WR <= 32'd0;
        end
        else
        begin
            if (wr)
            begin
                case (axi_awaddr[4:2])
                    MODE_REG_P:
                    begin
                        for (byte_index = 0; byte_index <= 3; byte_index = byte_index+1)
                            if ( axi_wstrb[byte_index] == 1)
                                mode_R_I_WR[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                    end

                    RUN__REG_P:
                    begin
                        for (byte_index = 0; byte_index <= 3; byte_index = byte_index+1)
                            if (axi_wstrb[byte_index] == 1)
                                runn_R_I_WR[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                    end

                    FRQA_REG_P:
                    begin
                        for (byte_index = 0; byte_index <= 3; byte_index = byte_index+1)
                            if (axi_wstrb[byte_index] == 1)
                                frqA_R_I_WR[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                    end

                    FRQB_REG_P:
                    begin
                        for (byte_index = 0; byte_index <= 3; byte_index = byte_index+1)
                            if (axi_wstrb[byte_index] == 1)
                                frqB_R_I_WR[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                    end

                    OFST_REG_P:
                    begin
                        for (byte_index = 0; byte_index <= 3; byte_index = byte_index+1)
                            if (axi_wstrb[byte_index] == 1)
                                ofst_R_I_WR[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                    end

                    AMPL_REG_P:
                    begin
                        for (byte_index = 0; byte_index <= 3; byte_index = byte_index+1)
                            if (axi_wstrb[byte_index] == 1)
                                ampl_R_I_WR[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                    end
                    DCYC_REG_P:
                    begin
                        for (byte_index = 0; byte_index <= 3; byte_index = byte_index+1)
                            if (axi_wstrb[byte_index] == 1)
                                dCyc_R_I_WR[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                    end

                    CYCL_REG_P:
                    begin
                        for (byte_index = 0; byte_index <= 3; byte_index = byte_index+1)
                            if (axi_wstrb[byte_index] == 1)
                                cycl_R_I_WR[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                    end
                endcase
            end
        end
    end

    /* Send write response (axi_bvalid, axi_bresp)
     * - after address is valid (axi_awvalid)
     * - after write data is valid (axi_wvalid)
     * - after this module asserts ready for address handshake (axi_awready)
     * - after this module asserts ready for data handshake (axi_wready)
     * Clear write response valid (axi_bvalid) after one clock
     */
    wire wr_add_data_ready = axi_awready && axi_wready;
    always_ff @ (posedge axi_clk)
    begin
        if (axi_resetn == 1'b0)
        begin
            axi_bvalid  <= 0;
            axi_bresp   <= 2'b0;
        end
        else
        begin
            if (wr_add_data_valid && wr_add_data_ready && ~axi_bvalid)
            begin
                axi_bvalid <= 1'b1;
                axi_bresp  <= 2'b0;
            end

            else if (S_AXI_BREADY && axi_bvalid)
            begin
                axi_bvalid <= 1'b0;
            end
        end
    end

    /* In the first clock (~axi_arready) that the read address is valid
     * - capture the address (axi_araddr)
     * - output ready (axi_arready) for one clock
     */
    reg [C_S_AXI_ADDR_WIDTH-1:0] raddr;
    always_ff @ (posedge axi_clk)
    begin
        if (axi_resetn == 1'b0)
        begin
            axi_arready <= 1'b0;
            raddr <= 32'b0;
        end

        else
        begin
            // if valid, pulse ready (axi_rready) for one clock and save address
            if (axi_arvalid && ~axi_arready)
            begin
                axi_arready <= 1'b1;
                raddr  <= axi_araddr;
            end

            else
            begin
                axi_arready <= 1'b0;
            end
        end
    end

    /* Update register read data
     * - after this module receives a valid address (axi_arvalid)
     * - after this module asserts ready for address handshake (axi_arready)
     * - before the module asserts the data is valid (~axi_rvalid)
     *   (don't change the data while asserting read data is valid)
     */
    wire rd = axi_arvalid && axi_arready && ~axi_rvalid;
    always_ff @ (posedge axi_clk)
    begin
        if (axi_resetn == 1'b0)
        begin
            axi_rdata <= 32'b0;
        end

        else
        begin
            if (rd)
            begin
                // Address decoding for reading registers
                case (raddr[04:02])
                    MODE_REG_P: axi_rdata <= mode_R_I_WR;
                    RUN__REG_P: axi_rdata <= runn_R_I_WR;
                    FRQA_REG_P: axi_rdata <= frqA_R_I_WR;
                    FRQB_REG_P: axi_rdata <= frqB_R_I_WR;
                    OFST_REG_P: axi_rdata <= ofst_R_I_WR;
                    AMPL_REG_P: axi_rdata <= ampl_R_I_WR;
                    DCYC_REG_P: axi_rdata <= dCyc_R_I_WR;
                    CYCL_REG_P: axi_rdata <= cycl_R_I_WR;
                endcase
            end
        end
    end

    /* Assert data is valid for reading (axi_rvalid)
     * - after address is valid (axi_arvalid)
     * - after this module asserts ready for address handshake (axi_arready)
     * De-assert data valid (axi_rvalid)
     * - after master ready handshake is received (axi_rready)
     */
    always_ff @ (posedge axi_clk)
    begin
        if (axi_resetn == 1'b0)
        begin
            axi_rvalid <= 1'b0;
        end

        else
        begin
            if (axi_arvalid && axi_arready && ~axi_rvalid)
            begin
                axi_rvalid <= 1'b1;
                axi_rresp <= 2'b0;
            end
            else if (axi_rvalid && axi_rready)
            begin
                axi_rvalid <= 1'b0;
            end
        end
    end

    // Assign outputs
    assign mode_W_O = mode_R_I_WR;
    assign runn_W_O = runn_R_I_WR;
    assign frqA_W_O = frqA_R_I_WR;
    assign frqB_W_O = frqB_R_I_WR;
    assign ofst_W_O = ofst_R_I_WR;
    assign ampl_W_O = ampl_R_I_WR;
    assign dCyc_W_O = dCyc_R_I_WR;
    assign cycl_W_O = cycl_R_I_WR;
endmodule