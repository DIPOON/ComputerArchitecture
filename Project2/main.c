#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    // option
    FILE* fp = NULL;
    int isMMode = 0;
    int isDMode = 0;
    int nAddr1 = 0;
    int nAddr2 = 0;
    int num_instruction = 0x7fffffff;
    for (int i = 1; i < argc; i++)
    {
        // option m
        if (!strcmp(argv[i],"-m"))
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
    if (fp == NULL)
    {
        printf("file open fail\n");
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
    int nInstructionNumber = 0;
    // instruction
    int instruction = 0;
    int nOpcode = 0;
    int nRs = 0;
    int nRt = 0;
    int nRd = 0;
    int nShamt = 0;
    int nFunctionField = 0;
    int nOffset = 0;
    int nTarget = 0;
    int nJumpAddress = 0;
    // Control
    int nRegDst = 0;
    int nRegWrite = 0;
    int nALUSrc = 0;
    int nPCSrc = 0;
    int nMemRead = 0;
    int nMemWrite = 0;
    int nBranch = 0;
    int nMemtoReg = 0;
    int nALUControlLines = 0;
    // Circuit data
    int nReadData1 = 0;
    int nReadData2 = 0;
    int nALUResult = 0;
    int nReadData = 0;
    while (nInstructionNumber < num_instruction)
    {
        // Instruction Fetch
        // program end instruction case need
        if (ProgramCounter >= (0x400000 + TextSectionSize))
        {
            break;
        }
        for (int i = 0; i < 4; i++)
        {
            instruction = (instruction << 8) + (int)(TextMemory[ProgramCounter - 0x400000 + 3 - i] & 0xff);
        }
        ProgramCounter = ProgramCounter + 4;

        // Instruction Decode
        nOpcode = (instruction >> 26) & 0x3f;
        nRs = (instruction >> 21) & 0x1f;
        nRt = (instruction >> 16) & 0x1f;
        nRd = (instruction >> 11) & 0x1f;
        nShamt = (instruction >> 6) & 0x1f;
        nFunctionField = (instruction) & 0x3f;
         // nOffset is plus or zero
        if (((instruction >> 15) & 0x1) == 0)
        {
            nOffset = instruction & 0x0000ffff;
        }
         // nOffset is minus. sign extended.
        else if (((instruction >> 15) & 0x1) == 1)
        {
            nOffset = instruction | 0xffff0000;
        }
        else
        {
            printf("why nOffset sign value is not binary?\n");
        }
         // nTarget is plus or zero
        if (((instruction >> 25) & 0x1) == 0)
        {
            nTarget = instruction & 0x03ffffff;
        }
         // nOffset is minus. sign extended.
        else if (((instruction >> 25) & 0x1) == 1)
        {
            nTarget = instruction | 0xff000000;
        }
        else
        {
            printf("why nTarget sign value is not binary?\n");
        }
        nTarget = ((instruction << 6) & 0xffffffc0) >> 6;
        nReadData1 = Registerfile[nRs];
        nReadData2 = Registerfile[nRt];
        // R type instruction
        // R type control signal is blessed
        if (nOpcode == 0)
        {
            nRegDst = 1;
            nRegWrite = 1;
            nALUSrc = 0;
            nPCSrc = 0; // JR will deal directly
            nMemtoReg = 0;
            nMemRead = 0;
            nMemWrite = 0;
            nBranch = 0;
            nALUControlLines = 3;
            // JR
            if (nFunctionField == 0x8)
            {
                nJumpAddress = Registerfile[nRs];
                ProgramCounter = nJumpAddress;
                // option D mode print
                 // 범위 바깥은 0 출력
                if (isDMode == 1)
                {
                    printf("Current register values:\n");
                    printf("------------------------------------\n");
                    printf("PC: 0x%x\n", ProgramCounter);
                    printf("Registers:\n");
                    for (int j = 0; j < 32; j++)
                    {
                        printf("R(%d): 0x%x\n", j, Registerfile[j]);
                    }
                }
                continue;
            }
        }
        // J type instruction
         // J
        else if (nOpcode == 0x2)
        {
            nRegDst;
            nRegWrite = 0;
            nALUSrc;
            nPCSrc = 2;
            nMemRead = 0;
            nMemWrite = 0;
            nBranch;
            nMemtoReg;
            nALUControlLines;
            nJumpAddress = nTarget * 4 + (ProgramCounter & 0xf0000000);
            ProgramCounter = nJumpAddress;
            // option D mode print
             // 범위 바깥은 0 출력
            if (isDMode == 1)
            {
                printf("Current register values:\n");
                printf("------------------------------------\n");
                printf("PC: 0x%x\n", ProgramCounter);
                printf("Registers:\n");
                for (int j = 0; j < 32; j++)
                {
                    printf("R(%d): 0x%x\n", j, Registerfile[j]);
                }
            }
            continue;
        }
         // JAL
        else if (nOpcode == 0x3)
        {
            nRegDst;
            nRegWrite = 0;
            nALUSrc;
            nPCSrc = 2;
            nMemRead = 0;
            nMemWrite = 0;
            nBranch;
            nMemtoReg;
            nALUControlLines;
            nJumpAddress = nTarget * 4 + (ProgramCounter & 0xf0000000);
            Registerfile[31] = ProgramCounter;
            ProgramCounter = nJumpAddress;
            // option D mode print
             // 범위 바깥은 0 출력
            if (isDMode == 1)
            {
                printf("Current register values:\n");
                printf("------------------------------------\n");
                printf("PC: 0x%x\n", ProgramCounter);
                printf("Registers:\n");
                for (int j = 0; j < 32; j++)
                {
                    printf("R(%d): 0x%x\n", j, Registerfile[j]);
                }
            }
            continue;
        }
        // I type instruction
         // ADDIU
        else if (nOpcode == 0x9)
        {
            nRegDst = 0;
            nRegWrite = 1;
            nALUSrc = 1;
            nPCSrc = 0;
            nMemRead = 0;
            nMemWrite = 0;
            nBranch = 0;
            nMemtoReg = 0;
            nALUControlLines = 2;
        }
         // ANDI
        else if (nOpcode == 0xc)
        {
            nRegDst = 0;
            nRegWrite = 1;
            nALUSrc = 1;
            nPCSrc = 0;
            nMemRead = 0;
            nMemWrite = 0;
            nBranch = 0;
            nMemtoReg = 0;
            nALUControlLines = 0;
        }
         // BEQ
        else if (nOpcode == 0x4)
        {
            nRegDst;
            nRegWrite = 0;
            nALUSrc = 0;
            nPCSrc; // ALU result
            nMemRead; 
            nMemWrite;
            nBranch = 2;
            nMemtoReg;
            nALUControlLines = 6;
        }
         // BNE
        else if (nOpcode == 0x5)
        {
            nRegDst;
            nRegWrite = 0;
            nALUSrc = 0;
            nPCSrc; // ALU result
            nMemRead;
            nMemWrite;
            nBranch = 1;
            nMemtoReg;
            nALUControlLines = 6;
        }
         // LUI
        else if (nOpcode == 0xf)
        {
            nRegDst = 0;
            nRegWrite = 2;
            nALUSrc;
            nPCSrc = 0;
            nMemRead = 0;
            nMemWrite = 0;
            nBranch = 0;
            nMemtoReg = 2; // immediate field direct
            nALUControlLines;
        }
         // LW
        else if (nOpcode == 0x23)
        {
            nRegDst = 0;
            nRegWrite = 1;
            nALUSrc = 1;
            nPCSrc = 0;
            nMemRead = 1;
            nMemWrite = 0;
            nBranch = 0;
            nMemtoReg = 1;
            nALUControlLines = 2;
        }
         // LB
        else if (nOpcode == 0x20)
        {
        nRegDst = 0;
        nRegWrite = 1;
        nALUSrc = 1;
        nPCSrc = 0;
        nMemRead = 2;
        nMemWrite = 0;
        nBranch = 0;
        nMemtoReg = 1;
        nALUControlLines = 2;
        }
         // ORI
        else if (nOpcode == 0xd)
        {
        nRegDst = 0;
        nRegWrite = 1;
        nALUSrc = 3;
        nPCSrc = 0;
        nMemRead = 0;
        nMemWrite = 0;
        nBranch = 0;
        nMemtoReg = 0;
        nALUControlLines = 1;
        }
         // SLTIU
        else if (nOpcode == 0xb)
        {
        nRegDst = 0;
        nRegWrite = 1;
        nALUSrc = 1;
        nPCSrc = 0;
        nMemRead = 0;
        nMemWrite = 0;
        nBranch = 0;
        nMemtoReg = 0;
        nALUControlLines = 7;
        }
         // SW
        else if (nOpcode == 0x2b)
        {
        nRegDst;
        nRegWrite = 0;
        nALUSrc = 1;
        nPCSrc = 0;
        nMemRead = 0;
        nMemWrite = 1;
        nBranch = 0;
        nMemtoReg;
        nALUControlLines = 2;
        }
         // SB
        else if (nOpcode == 0x28)
        {
        nRegDst;
        nRegWrite = 0;
        nALUSrc = 1;
        nPCSrc = 0;
        nMemRead = 0;
        nMemWrite = 2;
        nBranch = 0;
        nMemtoReg;
        nALUControlLines = 2;
        }
        else
        {
        printf("is this in assignment 1?\n");
        }
        
        // Instruction Execution
         // ALU
          // R type function field check
         if (nALUControlLines == 3)
         {
             // SLL
             if (nFunctionField == 0x0)
             {
                 nALUControlLines = 4;
                 nALUResult = Registerfile[nRt] << nShamt;
             }
             // SRL
             else if (nFunctionField == 0x2)
             {
                 nALUControlLines = 4;
                 nALUResult = Registerfile[nRt] >> nShamt;
             }
             // JR
             else if (nFunctionField == 0x8)
             {
                 printf("why not jumping in decode?\n");
             }
             // ADDU
             else if (nFunctionField == 0x21)
             {
                 nALUControlLines = 2;
             }
             // SUBU
             else if (nFunctionField == 0x23)
             {
                 nALUControlLines = 6;
             }
             // AND
             else if (nFunctionField == 0x24)
             {
                 nALUControlLines = 0;
             }
             // OR
             else if (nFunctionField == 0x25)
             {
                 nALUControlLines = 1;
             }
             // NOR
             else if (nFunctionField == 0x27)
             {
                 nALUControlLines = 12;
             }
             else if (nFunctionField == 0x2b)
             {
                 nALUControlLines = 7;
             }
             else
             {
                 printf("Not R type but why see function field?\n");
             }
         }
          // AND
         if (nALUControlLines == 0)
         {
             nALUResult = nReadData1 & (nALUSrc ? nOffset : nReadData2);
         }
          // OR
         else if (nALUControlLines == 1)
         {
             nALUResult = nReadData1 | (nALUSrc ? nOffset : nReadData2);
         }
          // add
         else if (nALUControlLines == 2)
         {
             nALUResult = nReadData1 + (nALUSrc ? nOffset : nReadData2);
         }
         // See Function Field
         else if (nALUControlLines == 3)
         {
             printf("why not re-ALUControlLines in a second?\n");
         }
         // Like Shift, need pass
         else if (nALUControlLines == 4)
         {
         }
          // subtract
         else if (nALUControlLines == 6)
         {
             nALUResult = nReadData1 - (nALUSrc ? nOffset : nReadData2);
         }
          // set less than
         else if (nALUControlLines == 7)
         {
             nALUResult = nReadData1 < (nALUSrc ? nOffset : nReadData2);

         }
          // NOR
         else if (nALUControlLines == 12)
         {
             for (int j = 31; j >= 0; j--)
             {
                 if (((nReadData1 >> j) & 0x1) == 0 && (((nALUSrc ? nOffset : nReadData2) >> j) & 0x1) == 0)
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
             printf("Undefined nALUControlLines : %d\n", nALUControlLines);
         }
         // PCSrc
         // BNE and not equal
         if (nALUResult != 0 && nBranch == 1)
         {
             nPCSrc = 1;
         }
         // BEQ and equal
         else if (nALUResult == 0 && nBranch == 2)
         {
             nPCSrc = 1;
         }
         else
         {
             nPCSrc = 0;
         }
         if (nPCSrc == 0)
         {
         }
         else if (nPCSrc == 1)
         {
             ProgramCounter = ProgramCounter + (nOffset << 2);
         }
         else if (nPCSrc == 2)
         {
             printf("why not jumping in decode?\n");
         }
         else
         {
             printf("Undefined nPCSrc : %d\n", nPCSrc);
         }

        // Memory
         // LW
        if (nMemRead == 1)
        {
            for (int i = 0; i < 4; i++)
            {
                nReadData = (nReadData << 8) + ((int)DataMemory[nALUResult - 0x10000000 + 3 - i] & 0xff);
            }
        }
         // LB need one byte sign-extended
        else if (nMemRead == 2)
        {
            nReadData = ((int)DataMemory[nALUResult - 0x10000000]);
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
         // SW
        if (nMemWrite == 1)
        {
            for (int j = 0; j < 4; j++)
            {
                DataMemory[nALUResult - 0x10000000 + j] = (char)(nReadData2 >> (8 * (j % 4))) & 0xff;
            }
        }
         // SB needs cutting
        if (nMemWrite == 2)
        {
            DataMemory[nALUResult - 0x10000000] = (char)(nReadData2 & 0xff);
        }

        // WriteBack
        if (nRegWrite == 0)
        {
            Registerfile;
        }
        else if (nRegWrite == 1)
        {
            if (nMemtoReg == 0)
            {
                Registerfile[(nRegDst ? nRd : nRt)] = nALUResult;
            }
            else if (nMemtoReg == 1)
            {
                Registerfile[nRegDst ? nRd : nRt] = nReadData;
            }
            // is this route needed?
            else if (nMemtoReg == 2)
            {
                Registerfile[nRegDst ? nRd : nRt] = (nOffset << 16) >> 16;
            }
        }
         // LUI need << 16
        else if (nRegWrite == 2)
        {
            if (nMemtoReg == 2)
            {
                Registerfile[nRegDst ? nRd : nRt] = (nOffset << 16);
            }
            else
            {
                printf("why nRegWrtie 2 but nMemtoReg is not 2?\n");
            }
        }
        else
        {
            printf("Undefined nRegWrtie = %d\n", nRegWrite);
        }
        
        // option D mode print
        // 범위 바깥은 0 출력
        if (isDMode == 1)
        {
            printf("Current register values:\n");
            printf("------------------------------------\n");
            printf("PC: 0x%x\n", ProgramCounter);
            printf("Registers:\n");
            for (int j = 0; j < 32; j++)
            {
                printf("R(%d): 0x%x\n", j, Registerfile[j]);
            }
        }
        
         nInstructionNumber++;
    }
    // D mode none register print
    if (isDMode == 0)
    {
        printf("Current register values:\n");
        printf("------------------------------------\n");
        printf("PC: 0x%x\n", ProgramCounter);
        printf("Registers:\n");
        for (int j = 0; j < 32; j++)
        {
            printf("R(%d): 0x%x\n", j, Registerfile[j]);
        }
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