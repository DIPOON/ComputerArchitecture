#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int CheckEndian()
{
    int test = 0x12345678;
    if ((char)*(&test) == 0x12)
    {
        return 0;
    }
    // why this works..?
    else if ((char)*(&test) == 0x78)
    {
        return 1;
    }
    else
    {
        printf("brave new endian machine!\n");
        return 1;
    }
}

int main(int argc, char* argv[])
{
    // Processor Check
    int isLittleEndianProcessor = CheckEndian();

    // option
    FILE* fp = NULL;
    int nAlwaysTaken = -1;
    int isMMode = 0;
    int isDMode = 0;
    int isPMode = 0;
    int nAddr1 = 0;
    int nAddr2 = 0;
    int num_instruction = 0x7fffffff;
    for (int i = 1; i < argc; i++)
    {
        // <-atp>
        if (!strcmp(argv[i], "-atp"))
        {
            nAlwaysTaken = 1;            
        }
        // <-antp>
        else if (!strcmp(argv[i], "-antp"))
        {
            nAlwaysTaken = 0;
        }
        // option m
        else if (!strcmp(argv[i], "-m"))
        {
            isMMode = 1;
            // addr1:addr2
            i++;
            char* end;
            nAddr1 = strtol(argv[i], &end, 16);
            nAddr2 = strtol(end + 1, NULL, 16);
        }
        // option d
        else if (!strcmp(argv[i], "-d"))
        {
            isDMode = 1;
        }
        // option p
        else if (!strcmp(argv[i], "-p"))
        {
            isPMode = 1;
        }
        // option n
        else if (!strcmp(argv[i], "-n"))
        {
            i++;
            // num_instruction
            num_instruction = atoi(argv[i]);
        }
        // input file
        else if (i == argc - 1)
        {
            fp = fopen(argv[i], "r");
        }
        else
        {
            printf("Unknown Input : %s\n", argv[i]);
        }
    }
    if (nAlwaysTaken == -1)
    {
        printf("Branch Prediction Option missed.\n");
    }
    if (fp == NULL)
    {
        printf("File open fail\n");
    }

    // loading
    // initialize
    int ProgramCounter = 0x400000;
    int Registerfile[32] = { 0, };
    char Buffer[15]; // for one line buffer
    // text memory building
    fgets(Buffer, 15, fp);
    int TextSectionSize = strtol(Buffer, NULL, 16);
    char* TextMemory = malloc(TextSectionSize);
    // data memory building
    fgets(Buffer, 15, fp);
    int DataSectionSize = strtol(Buffer, NULL, 16);
    char* DataMemory = malloc(DataSectionSize);
    // text memory filling
    for (int i = 0; i < TextSectionSize; i = i + 4)
    {
        fgets(Buffer, 15, fp);
        for (int j = 0; j < 4; j++)
        {
            TextMemory[i + j] = (strtoul(Buffer, NULL, 16) >> (8 * (j % 4))) & 0xff;
        }
    }
    // data memory filling
    // if data except word size consist, dynamic allocation needed.
    for (int i = 0; i < DataSectionSize; i = i + 4)
    {
        fgets(Buffer, 15, fp);
        for (int j = 0; j < 4; j++)
        {
            DataMemory[i + j] = (strtoul(Buffer, NULL, 16) >> (8 * (j % 4))) & 0xff;
        }
    }

    // Processor Simulation
    int nClockCycleNumber = 0;
    int nCompleteInstructionNumber = 0;
    int nALUResult = 0;
    int nMuxOut = 0;
    int nIFInstruction = 0;
    int nIDInstruction = 0;
    int nIFPC = 0;
    int nIDPC = 0;
    int nEXPC = 0;
    int nMEMPC = 0;
    int nWBPC = 0;
    int isFlush = 0;
    // Circuit data
    // IF/ID
    int IFIDInstruction = 0;
    int IFIDNPC = 0;
    int IFIDRs = 0;
    int IFIDRt = 0;
    // ID/EX
    int IDEXRs = 0;
    int IDEXRt = 0;
    int IDEXRd = 0;
    int IDEXALUControlLines = 0;
    int IDEXFunctionField = 0;
    int IDEXShamt = 0;
    int IDEXRegDst = 0;
    int IDEXRegWrite = 0;
    int IDEXALUSrc = 0;
    int IDEXPCSrc = 0;
    int IDEXMemRead = 0;
    int IDEXMemWrite = 0;
    int IDEXBranch = 0;
    int IDEXMemtoReg = 0;
    int IDEXReadData1 = 0;
    int IDEXReadData2 = 0;
    int IDEXImmediateValue = 0;
    int IDEXNPC = 0;
    // EX/MEM
    int EXMEMALUOut = 0;
    int EXMEMMemRead = 0;
    int EXMEMImmediateValue = 0;
    int EXMEMMemtoReg = 0;
    int EXMEMRegWrite = 0;
    int EXMEMWriteRegister = 0;
    int EXMEMBranch = 0;
    int EXMEMMemWrite = 0;
    int EXMEMReadData2 = 0;
    int EXMEMBranchTargetAddress = 0;
    int EXMEMNPC = 0;
    // MEM/WB
    int MEMWBALUOut = 0;
    int MEMWBMemOut = 0;
    int MEMWBImmediateValue = 0;
    int MEMWBMemtoReg = 0;
    int MEMWBRegWrite = 0;
    int MEMWBWriteRegister = 0;

    while (nCompleteInstructionNumber < num_instruction)
    {
        nClockCycleNumber++;
        // WriteBack Stage
        nWBPC = nMEMPC;
        // Mux
        if (MEMWBMemtoReg == 0)
        {
            nMuxOut = MEMWBALUOut;
        }
        else if (MEMWBMemtoReg == 1)
        {
            nMuxOut = MEMWBMemOut;
        }
        else if (MEMWBMemtoReg == 2)
        {
            nMuxOut = MEMWBImmediateValue << 16;
        }
        else
        {
            printf("Undefined nMEMWBMemtoReg = %d\n", MEMWBMemtoReg);
        }
        // MEM/WB to EX Forwarding
        if (MEMWBRegWrite
            && (MEMWBWriteRegister != 0)
            && !(EXMEMRegWrite && (EXMEMWriteRegister != 0) && (EXMEMWriteRegister == IDEXRs))
            && (MEMWBWriteRegister == IDEXRs))
        {
            IDEXReadData1 = nMuxOut;
        }
        if (MEMWBRegWrite
            && (MEMWBWriteRegister != 0)
            && !(EXMEMRegWrite && (EXMEMWriteRegister != 0) && (EXMEMWriteRegister == IDEXRt))
            && (MEMWBWriteRegister == IDEXRt))
        {
            IDEXReadData2 = nMuxOut;
        }
        // Registers
        if (MEMWBRegWrite == 0)
        {
        }
        else if (MEMWBRegWrite == 1)
        {
            Registerfile[(MEMWBWriteRegister)] = nMuxOut;
        }
        // LUI need << 16
        else if (MEMWBRegWrite == 2)
        {
            Registerfile[(MEMWBWriteRegister)] = nMuxOut & 0xffffffff;
        }
        // LB
        else if (MEMWBRegWrite == 3)
        {
            Registerfile[MEMWBWriteRegister] = nMuxOut & 0xff;
        }
        // nRegWrite is wrong.
        else
        {
            printf("Undefined nMEMWBRegWrite = %d\n", MEMWBRegWrite);
        }
        if (nWBPC != 0)
        {
            nCompleteInstructionNumber++;
        }

        // Memory Stage - Load
        nMEMPC = nEXPC;
        // EX/MEM to EX Forwarding
        if (EXMEMRegWrite
            && (EXMEMWriteRegister != 0)
            && (EXMEMWriteRegister == IDEXRs))
        {
            IDEXReadData1 = nALUResult;
        }
        if (EXMEMRegWrite
            && (EXMEMWriteRegister != 0)
            && (EXMEMWriteRegister == IDEXRt))
        {
            IDEXReadData2 = nALUResult;
        }
         // LW
        int nReadData = 0;
        if (EXMEMMemRead == 1)
        {
            for (int i = 0; i < 4; i++)
            {
                nReadData = (nReadData << 8) + ((int)DataMemory[EXMEMALUOut - 0x10000000 + 3 - i] & 0xff);
            }
        }
        // LB need one byte sign-extended
        else if (EXMEMMemRead == 2)
        {
            if (isLittleEndianProcessor == 1)
            {
                int nAddress = 0;
                nAddress = ((EXMEMALUOut - 0x10000000) / 4) * 4 + (3 - ((EXMEMALUOut - 0x10000000) % 4));
                nReadData = ((int)DataMemory[nAddress]);
            }
            else
            {
                nReadData = ((int)DataMemory[EXMEMALUOut - 0x10000000]);
            }
            // nReadData is plus or zero
            if (((nReadData >> 7) & 0x1) == 0)
            {
                nReadData = nReadData & 0x000000ff;
            }
            // nReadData is minus. sign extended.
            else if (((nReadData >> 7) & 0x1) == 1)
            {
                nReadData = nReadData | 0xffffff00;
            }
            else
            {
                printf("why nReadData sign value is not binary?\n");
            }
        }
        // Memory Stage - Store
        // SW
        int nIFRead = 0;
        if (EXMEMMemWrite == 1)
        {
            for (int i = 0; i < 4; i++)
            {
                nIFRead = (nIFRead << 8) + ((int)DataMemory[EXMEMALUOut - 0x10000000 + 3 - i] & 0xff);
            }
            for (int j = 0; j < 4; j++)
            {
                
                DataMemory[EXMEMALUOut - 0x10000000 + j] = (char)(EXMEMReadData2 >> (8 * (j % 4))) & 0xff;
            }
        }
        // SB needs cutting
        if (EXMEMMemWrite == 2)
        {
            if (isLittleEndianProcessor == 1)
            {
                int nAddress = 0;
                nAddress = ((EXMEMALUOut - 0x10000000) / 4) * 4 + (3 - ((EXMEMALUOut - 0x10000000) % 4));
                nIFRead = ((int)DataMemory[nAddress]);
                DataMemory[nAddress] = (char)(EXMEMReadData2 & 0xff);
            }
            else
            {
                nIFRead = DataMemory[EXMEMALUOut - 0x10000000];
                DataMemory[EXMEMALUOut - 0x10000000] = (char)(EXMEMReadData2 & 0xff);
            }
        }
        // MEM/WB Pipeline State Register
        MEMWBALUOut = EXMEMALUOut;
        MEMWBMemOut = nReadData;
        MEMWBImmediateValue = EXMEMImmediateValue;
        MEMWBMemtoReg = EXMEMMemtoReg;
        MEMWBRegWrite = EXMEMRegWrite;
        MEMWBWriteRegister = EXMEMWriteRegister;
        // Branch PC modify
        if ((EXMEMBranch == 1 && EXMEMALUOut != 0 && nAlwaysTaken == 0) || (EXMEMBranch == 2 && EXMEMALUOut == 0 && nAlwaysTaken == 0))
        {
            ProgramCounter = EXMEMBranchTargetAddress;
        }
        // next PC
        if ((EXMEMBranch == 1 && EXMEMALUOut == 0 && nAlwaysTaken == 1) || (EXMEMBranch == 2 && EXMEMALUOut != 0 && nAlwaysTaken == 1))
        {
            ProgramCounter = EXMEMNPC;
        }
        // BNE and not equal but predict always not taken
        if ((EXMEMBranch == 1 && EXMEMALUOut != 0 && nAlwaysTaken == 0) ||
            // BNE and equal but predict always taken
            (EXMEMBranch == 1 && EXMEMALUOut == 0 && nAlwaysTaken == 1) ||
            // BEQ and equal but predict always not taken
            (EXMEMBranch == 2 && EXMEMALUOut == 0 && nAlwaysTaken == 0) ||
            // BEQ and not equal but predict always taken
            (EXMEMBranch == 2 && EXMEMALUOut != 0 && nAlwaysTaken == 1))
        {
            // 3 flushing
            // IF/ID
            nIFPC = 0;
            isFlush = 1;
            IFIDInstruction = 0;
            IFIDNPC = 0;
            IFIDRs = 0;
            IFIDRt = 0;
            // ID/EX
            nIDPC = 0;
            IDEXRs = 0;
            IDEXRt = 0;
            IDEXRd = 0;
            IDEXALUControlLines = 0;
            IDEXFunctionField = 0;
            IDEXShamt = 0;
            IDEXRegDst = 0;
            IDEXRegWrite = 0;
            IDEXALUSrc = 0;
            IDEXPCSrc = 0;
            IDEXMemRead = 0;
            IDEXMemWrite = 0;
            IDEXBranch = 0;
            IDEXMemtoReg = 0;
            IDEXReadData1 = 0;
            IDEXReadData2 = 0;
            IDEXImmediateValue = 0;
            // EX/MEM
            nEXPC = 0;
            EXMEMALUOut = 0;
            EXMEMMemRead = 0;
            EXMEMImmediateValue = 0;
            EXMEMMemtoReg = 0;
            EXMEMRegWrite = 0;
            EXMEMWriteRegister = 0;
            EXMEMBranch = 0;
            EXMEMMemWrite = 0;
            EXMEMReadData2 = 0;
        }
        else
        {
        }

        // Instruction Execution
        nEXPC = nIDPC;
         // ALU
          // R type function field check
        if (IDEXALUControlLines == 3)
        {
            // SLL
            if (IDEXFunctionField == 0x0)
            {
                IDEXALUControlLines = 4;
                nALUResult = IDEXReadData2 << IDEXShamt;
            }
            // SRL
            else if (IDEXFunctionField == 0x2)
            {
                IDEXALUControlLines = 4;
                nALUResult = IDEXReadData2 >> IDEXShamt;
            }
            // JR
            else if (IDEXFunctionField == 0x8)
            {
            }
            // ADDU
            else if (IDEXFunctionField == 0x21)
            {
                IDEXALUControlLines = 2;
            }
            // SUBU
            else if (IDEXFunctionField == 0x23)
            {
                IDEXALUControlLines = 6;
            }
            // AND
            else if (IDEXFunctionField == 0x24)
            {
                IDEXALUControlLines = 0;
            }
            // OR
            else if (IDEXFunctionField == 0x25)
            {
                IDEXALUControlLines = 1;
            }
            // NOR
            else if (IDEXFunctionField == 0x27)
            {
                IDEXALUControlLines = 12;
            }
            else if (IDEXFunctionField == 0x2b)
            {
                IDEXALUControlLines = 7;
            }
            else
            {
                printf("Not R type but why see function field?\n");
            }
        }
        // AND
        if (IDEXALUControlLines == 0)
        {
            nALUResult = IDEXReadData1 & (IDEXALUSrc ? IDEXImmediateValue : IDEXReadData2);
        }
        // OR
        else if (IDEXALUControlLines == 1)
        {
            nALUResult = IDEXReadData1 | (IDEXALUSrc ? IDEXImmediateValue : IDEXReadData2);
        }
        // add
        else if (IDEXALUControlLines == 2)
        {
            nALUResult = IDEXReadData1 + (IDEXALUSrc ? IDEXImmediateValue : IDEXReadData2);
        }
        // See Function Field
        else if (IDEXALUControlLines == 3)
        {
        }
        // Like Shift, need pass
        else if (IDEXALUControlLines == 4)
        {
        }
        // subtract
        else if (IDEXALUControlLines == 6)
        {
            nALUResult = IDEXReadData1 - (IDEXALUSrc ? IDEXImmediateValue : IDEXReadData2);
        }
        // lui
        else if (IDEXALUControlLines == 5)
        {
            nALUResult = (IDEXALUSrc ? IDEXImmediateValue : IDEXReadData2) << 16;
        }
        // set less than
        else if (IDEXALUControlLines == 7)
        {
            nALUResult = IDEXReadData1 < (IDEXALUSrc ? IDEXImmediateValue : IDEXReadData2);

        }
        // NOR
        else if (IDEXALUControlLines == 12)
        {
            for (int j = 31; j >= 0; j--)
            {
                if (((IDEXReadData1 >> j) & 0x1) == 0 && (((IDEXALUSrc ? IDEXImmediateValue : IDEXReadData2) >> j) & 0x1) == 0)
                {
                    nALUResult = (nALUResult << 1) + 1;
                }
                else
                {
                    nALUResult = (nALUResult << 1) + 0;
                }
            }
        }
        else
        {
            printf("Undefined nIDEXALUControlLines : %d\n", IDEXALUControlLines);
        }
        
        // EX/MEM Pipeline State Register
        EXMEMReadData2 = IDEXReadData2;
        EXMEMALUOut = nALUResult;
        EXMEMMemRead = IDEXMemRead;
        EXMEMImmediateValue = IDEXImmediateValue;
        EXMEMMemtoReg = IDEXMemtoReg;
        EXMEMRegWrite = IDEXRegWrite;
        EXMEMBranch = IDEXBranch;
        EXMEMMemWrite = IDEXMemWrite;
        EXMEMBranchTargetAddress = IDEXNPC + (IDEXImmediateValue << 2);
        EXMEMNPC = IDEXNPC;
        // RegDst Mux
        EXMEMWriteRegister = IDEXRegDst ? IDEXRd : IDEXRt;

        // Instruction Decode
        // Data Dependency by LoadLike instruction
        int isStall = 0;
        if (IDEXMemRead
            && ((IDEXRt == IFIDRs) || (IDEXRt == IFIDRt)))
        {
            // stall
            isStall = 1;
            nIDPC = 0;
            IDEXRs = 0;
            IDEXRt = 0;
            IDEXRd = 0;
            IDEXALUControlLines = 0;
            IDEXFunctionField = 0;
            IDEXShamt = 0;
            IDEXRegDst = 0;
            IDEXRegWrite = 0;
            IDEXALUSrc = 0;
            IDEXPCSrc = 0;
            IDEXMemRead = 0;
            IDEXMemWrite = 0;
            IDEXBranch = 0;
            IDEXMemtoReg = 0;
            IDEXReadData1 = 0;
            IDEXReadData2 = 0;
            IDEXImmediateValue = 0;
        }
        if (isStall == 0)
        {
            nIDPC = nIFPC;
            nIDInstruction = nIFInstruction;
            int nOpcode = (IFIDInstruction >> 26) & 0x3f;
            int nRs = (IFIDInstruction >> 21) & 0x1f;
            int nRt = (IFIDInstruction >> 16) & 0x1f;
            int nFunctionField = (IFIDInstruction) & 0x3f;
            int nOffset = 0;
            // IDEXImmediateValue is plus or zero
            if (((IFIDInstruction >> 15) & 0x1) == 0)
            {
                nOffset = IFIDInstruction & 0x0000ffff;
            }
            // IDEXImmediateValue is minus. sign extended.
            else if (((IFIDInstruction >> 15) & 0x1) == 1)
            {
                nOffset = IFIDInstruction | 0xffff0000;
            }
            else
            {
                printf("Why IDEXImmediateValue sign value is not binary?\n");
            }
            int nTarget = 0;
            // nTarget is plus or zero
            if (((IFIDInstruction >> 25) & 0x1) == 0)
            {
                nTarget = IFIDInstruction & 0x03ffffff;
            }
            // nTarget is minus. sign extended.
            else if (((IFIDInstruction >> 25) & 0x1) == 1)
            {
                nTarget = IFIDInstruction | 0xff000000;
            }
            else
            {
                printf("Why nTarget sign value is not binary?\n");
            }
            // ID/EX Pipeline State Register
            IDEXRs = nRs;
            IDEXRt = nRt;
            IDEXRd = (IFIDInstruction >> 11) & 0x1f;
            IDEXShamt = (IFIDInstruction >> 6) & 0x1f;
            IDEXImmediateValue = nOffset;
            IDEXReadData1 = Registerfile[nRs];
            IDEXReadData2 = Registerfile[nRt];
            IDEXNPC = IFIDNPC;
            // R type instruction
            // R type control signal is blessed
            if (nOpcode == 0)
            {
                IDEXRegDst = 1;
                IDEXRegWrite = 1;
                IDEXALUSrc = 0;
                IDEXMemtoReg = 0;
                IDEXMemRead = 0;
                IDEXMemWrite = 0;
                IDEXBranch = 0;
                IDEXALUControlLines = 3;
                IDEXFunctionField = nFunctionField;
                // JR
                if (nFunctionField == 0x8)
                {
                    ProgramCounter = Registerfile[nRs];
                    IDEXRegDst;
                    IDEXRegWrite = 0;
                    IDEXALUSrc;
                    IDEXMemtoReg;
                    IDEXMemRead = 0;
                    IDEXMemWrite = 0;
                    IDEXBranch = 0;
                    IDEXALUControlLines;
                    // One Flush
                    isFlush = 1;
                    nIFPC = 0;
                    IFIDInstruction = 0;
                    IFIDNPC = 0;
                    IFIDRs = 0;
                    IFIDRt = 0;
                }
                if (nIDInstruction == 0)
                {
                    IDEXRegDst = 0;
                    IDEXRegWrite = 0;
                    IDEXALUSrc = 0;
                    IDEXMemtoReg = 0;
                    IDEXMemRead = 0;
                    IDEXMemWrite = 0;
                    IDEXBranch = 0;
                    IDEXALUControlLines = 3;
                    IDEXFunctionField = nFunctionField;
                }
            }
            // J type instruction
             // J
            else if (nOpcode == 0x2)
            {
                IDEXRegDst;
                IDEXALUControlLines;
                IDEXALUSrc;
                IDEXBranch = 0;
                IDEXMemRead = 0;
                IDEXMemWrite = 0;
                IDEXRegWrite = 0;
                IDEXMemtoReg;
                ProgramCounter = nTarget * 4 + (ProgramCounter & 0xf0000000);
                // One Flush
                nIFPC = 0;
                isFlush = 1;
                IFIDInstruction = 0;
                IFIDNPC = 0;
                IFIDRs = 0;
                IFIDRt = 0;
            }
            // JAL
            else if (nOpcode == 0x3)
            {
                IDEXRegDst;
                IDEXRegWrite = 0;
                IDEXALUSrc;
                IDEXMemRead = 0;
                IDEXMemWrite = 0;
                IDEXBranch = 0;
                IDEXMemtoReg;
                IDEXALUControlLines;
                Registerfile[31] = ProgramCounter;
                ProgramCounter = nTarget * 4 + (ProgramCounter & 0xf0000000);
                // One Flush
                nIFPC = 0;
                isFlush = 1;
                IFIDInstruction = 0;
                IFIDNPC = 0;
                IFIDRs = 0;
                IFIDRt = 0;
            }
            // I type instruction
             // ADDIU
            else if (nOpcode == 0x9)
            {
                IDEXRegDst = 0;
                IDEXRegWrite = 1;
                IDEXALUSrc = 1;
                IDEXMemRead = 0;
                IDEXMemWrite = 0;
                IDEXBranch = 0;
                IDEXMemtoReg = 0;
                IDEXALUControlLines = 2;
            }
            // ANDI
            else if (nOpcode == 0xc)
            {
                IDEXRegDst = 0;
                IDEXRegWrite = 1;
                IDEXALUSrc = 1;
                IDEXMemRead = 0;
                IDEXMemWrite = 0;
                IDEXBranch = 0;
                IDEXMemtoReg = 0;
                IDEXALUControlLines = 0;
            }
            // BEQ
            else if (nOpcode == 0x4)
            {
                IDEXRegDst;
                IDEXRegWrite = 0;
                IDEXALUSrc = 0;
                IDEXMemRead;
                IDEXMemWrite;
                IDEXBranch = 2;
                IDEXMemtoReg;
                IDEXALUControlLines = 6;
                if (nAlwaysTaken == 1)
                {
                    ProgramCounter = IFIDNPC + (nOffset << 2);
                    // One Flush
                    nIFPC = 0;
                    isFlush = 1;
                    IFIDInstruction = 0;
                    IFIDNPC = 0;
                    IFIDRs = 0;
                    IFIDRt = 0;
                }
            }
            // BNE
            else if (nOpcode == 0x5)
            {
                IDEXRegDst;
                IDEXRegWrite = 0;
                IDEXALUSrc = 0;
                IDEXMemRead;
                IDEXMemWrite;
                IDEXBranch = 1;
                IDEXMemtoReg;
                IDEXALUControlLines = 6;
                if (nAlwaysTaken == 1)
                {
                    ProgramCounter = IFIDNPC + (nOffset << 2);
                    // One Flush
                    nIFPC = 0;
                    isFlush = 1;
                    IFIDInstruction = 0;
                    IFIDNPC = 0;
                    IFIDRs = 0;
                    IFIDRt = 0;
                }
            }
            // LUI
            else if (nOpcode == 0xf)
            {
                IDEXRegDst = 0;
                IDEXRegWrite = 1;
                IDEXALUSrc = 1;
                IDEXMemRead = 0;
                IDEXMemWrite = 0;
                IDEXBranch = 0;
                IDEXMemtoReg = 0;
                IDEXALUControlLines = 5;
            }
            // LW
            else if (nOpcode == 0x23)
            {
                IDEXRegDst = 0;
                IDEXRegWrite = 1;
                IDEXALUSrc = 1;
                IDEXMemRead = 1;
                IDEXMemWrite = 0;
                IDEXBranch = 0;
                IDEXMemtoReg = 1;
                IDEXALUControlLines = 2;
            }
            // LB
            else if (nOpcode == 0x20)
            {
                IDEXRegDst = 0;
                IDEXRegWrite = 3;
                IDEXALUSrc = 1;
                IDEXMemRead = 2;
                IDEXMemWrite = 0;
                IDEXBranch = 0;
                IDEXMemtoReg = 1;
                IDEXALUControlLines = 2;
            }
            // ORI
            else if (nOpcode == 0xd)
            {
                IDEXRegDst = 0;
                IDEXRegWrite = 1;
                IDEXALUSrc = 3;
                IDEXMemRead = 0;
                IDEXMemWrite = 0;
                IDEXBranch = 0;
                IDEXMemtoReg = 0;
                IDEXALUControlLines = 1;
            }
            // SLTIU
            else if (nOpcode == 0xb)
            {
                IDEXRegDst = 0;
                IDEXRegWrite = 1;
                IDEXALUSrc = 1;
                IDEXMemRead = 0;
                IDEXMemWrite = 0;
                IDEXBranch = 0;
                IDEXMemtoReg = 0;
                IDEXALUControlLines = 7;
            }
            // SW
            else if (nOpcode == 0x2b)
            {
                IDEXRegDst;
                IDEXRegWrite = 0;
                IDEXALUSrc = 1;
                IDEXMemRead = 0;
                IDEXMemWrite = 1;
                IDEXBranch = 0;
                IDEXMemtoReg;
                IDEXALUControlLines = 2;
            }
            // SB
            else if (nOpcode == 0x28)
            {
                IDEXRegDst;
                IDEXRegWrite = 0;
                IDEXALUSrc = 1;
                IDEXMemRead = 0;
                IDEXMemWrite = 2;
                IDEXBranch = 0;
                IDEXMemtoReg;
                IDEXALUControlLines = 2;
            }
            else
            {
                printf("is this in assignment 1?\n");
            }

            // Instruction Fetch
            nIFPC = ProgramCounter;
            if (ProgramCounter >= (0x400000 + TextSectionSize))
            {
                nIFInstruction = 0;
                nIFPC = 0;
            }
            else if (isFlush == 1)
            {
                nIFInstruction = 0;
                nIFPC = 0;
                isFlush = 0;
            }
            else if (ProgramCounter >= (0x400000) && (ProgramCounter < (0x400000 + TextSectionSize)))
            {
                for (int i = 0; i < 4; i++)
                {
                    nIFInstruction = (nIFInstruction << 8) + (int)(TextMemory[ProgramCounter - 0x400000 + 3 - i] & 0xff);
                }
                if ((IDEXBranch == 0) || (nAlwaysTaken == 0))
                {
                    ProgramCounter = ProgramCounter + 4;
                }
            }
            else
            {
                printf("Undefined ProgramCounter : %d\n", ProgramCounter);
            }
            // IF/ID Pipeline State Register
            IFIDInstruction = nIFInstruction;
            IFIDNPC = ProgramCounter;
            IFIDRs = (nIFInstruction >> 21) & 0x1f;
            IFIDRt = (nIFInstruction >> 16) & 0x1f;
        }
        isStall = 0;

        // there's no left instruction
        if ((nIFPC == 0) &
            (nIDPC == 0) &
            (nEXPC == 0) &
            (nMEMPC == 0))
        {
            nWBPC = 0;
            break;
        }

        // print Cycle
        printf("===== Cycle %d =====\n\n", nClockCycleNumber);

        // option P mode print
        if (isPMode == 1)
        {
            printf("Current pipeline PC state:\n");
            printf("{");
            if (nIFPC != 0)
            {
                printf("0x%x", nIFPC);
            }
            printf("|");
            if (nIDPC != 0)
            {
                printf("0x%x", nIDPC);
            }
            printf("|");
            if (nEXPC != 0)
            {
                printf("0x%x", nEXPC);
            }
            printf("|");
            if (nMEMPC != 0)
            {
                printf("0x%x", nMEMPC);
            }
            printf("|");
            if (nWBPC != 0)
            {
                printf("0x%x", nWBPC);
            }
            printf("}\n\n");
        }

        // option D mode print
        if (isDMode == 1)
        {
            printf("Current register values:\n");
            printf("PC: 0x%x\n", ProgramCounter);
            printf("Registers:\n");
            for (int j = 0; j < 32; j++)
            {
                printf("R%d: 0x%x\n", j, Registerfile[j]);
            }
        }
    }
    // print Cycle
    printf("===== Completion cycle: %d =====\n\n", nClockCycleNumber);

    printf("Current pipeline PC state:\n");
    printf("{");
    if (nIFPC != 0)
    {
        printf("0x%x", nIFPC);
    }
    printf("|");
    if (nIDPC != 0)
    {
        printf("0x%x", nIDPC);
    }
    printf("|");
    if (nEXPC != 0)
    {
        printf("0x%x", nEXPC);
    }
    printf("|");
    if (nMEMPC != 0)
    {
        printf("0x%x", nMEMPC);
    }
    printf("|");
    if (nWBPC != 0)
    {
        printf("0x%x", nWBPC);
    }
    printf("}\n\n");

    printf("Current register values:\n");
    printf("PC: 0x%x\n", ProgramCounter);
    printf("Registers:\n");
    for (int j = 0; j < 32; j++)
    {
        printf("R(%d): 0x%x\n", j, Registerfile[j]);
    }
    // option M memory print
    if (isMMode == 1)
    {
        printf("\n");
        printf("Memory content [%x..%x]:\n", nAddr1, nAddr2);
        printf("------------------------------------\n");
        for (int j = nAddr1; j <= nAddr2; j = j + 4)
        {
            if (j >= 0x400000 && j <= (0x400000 + TextSectionSize))
            {
                int nMemoryValue = 0;
                for (int i = 0; i < 4; i++)
                {
                    nMemoryValue = (nMemoryValue << 8) + ((int)TextMemory[j - 0x400000 + 3 - i] & 0xff);
                }
                printf("0x%x: 0x%x\n", j, nMemoryValue);
            }
            else if (j >= 0x10000000 && j <= (0x10000000 + DataSectionSize))
            {
                int nMemoryValue = 0;
                for (int i = 0; i < 4; i++)
                {
                    nMemoryValue = (nMemoryValue << 8) + ((int)DataMemory[j - 0x10000000 + 3 - i] & 0xff);
                }
                printf("0x%x: 0x%x\n", j, nMemoryValue);
            }
            // out of range : 0
            else
            {
                printf("0x%x: 0x0\n", j);
            }
        }
    }

    if (fp != NULL)
    {
        fclose(fp);
    }
    return 0;
}
