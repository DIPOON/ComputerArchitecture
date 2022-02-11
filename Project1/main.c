#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int getToken(FILE* fp, char* token)
{
    char* pointerWriteHere = token;
    char charArrayBuffer[1];
    while (!feof(fp))
    {
        fread(charArrayBuffer, sizeof(char), 1, fp);
        if (*charArrayBuffer == 0x20 || 
            *charArrayBuffer == 0x0a || 
            *charArrayBuffer == 0xcc || 
            *charArrayBuffer == '('  || 
            *charArrayBuffer == ')'  || 
            *charArrayBuffer == ',')
        {
            if (pointerWriteHere != token)
            {
                *(pointerWriteHere) = '\0';
                return 1;
            }
            else
            {
                continue;
            }
        }
        else
        {
            *pointerWriteHere = *charArrayBuffer;
            pointerWriteHere += 1;
        }
    }
    return 0;
}

int islabeldirective(char* token, int length)
{
    for (int i = 0; i < length; i++)
    {
        if (token[i] == ':')
        {
            return 1;
        }
    }
    return 0;
}

typedef struct Label
{
    char name[11];
    int address;
} Label;

int isinstruction(char* token)
{
    if (!strcmp(token, "addiu") || !strcmp(token, "addu") || !strcmp(token, "and") || !strcmp(token, "andi") ||
        !strcmp(token, "beq") || !strcmp(token, "bne") || !strcmp(token, "j") || !strcmp(token, "jal") ||
        !strcmp(token, "jr") || !strcmp(token, "lui") || !strcmp(token, "lw") || !strcmp(token, "la") ||
        !strcmp(token, "nor") || !strcmp(token, "or") || !strcmp(token, "ori") || !strcmp(token, "sltiu") ||
        !strcmp(token, "sltu") || !strcmp(token, "sll") || !strcmp(token, "srl") || !strcmp(token, "sw") ||
        !strcmp(token, "subu") || !strcmp(token, "lb") || !strcmp(token, "sb"))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int islabel(char* token, Label* LabelTable, int LabelLength)
{
    for (int i = 0; i < LabelLength; i++)
    {
        if (!strcmp(token, LabelTable[i].name))
        {
            return 1;
        }
    }
    return 0;
}

enum InstructionType
{
    Nothing = 0,
    R = 1,
    I = 2,
    J = 3
};

enum WriteMode
{
    NoMode = 0,
    DataMode = 1,
    TextMode = 2
};

void storeDecInBin(char* aBinary, int nBinarySize, int nValue)
{
    int i = 0;
    int aTemp[32];
    if (nValue >= 0)
    {
        for (i = 0; i < 32; i++)
        {
            aTemp[i] = (nValue % 2);
            nValue = nValue / 2;
        }
    }
    else if (nValue < 0)
    {
        nValue = -nValue;
        for (i = 0; i < 32; i++)
        {
            aTemp[i] = (int)nValue % 2;
            nValue = nValue / 2;
        }
        for (i = 0; i < 32; i++)
        {
            (aTemp[i] == 0) ? (aTemp[i] = 1) : (aTemp[i] = 0);
        }
        aTemp[0] += 1;
        for (i = 0; i < 32 - 1; i++)
        {
            if (aTemp[i] == 2)
            {
                aTemp[i] = 0;
                aTemp[i + 1] += 1;
            }
            else
            {
                break;
            }
        }
    }
    else
    {
        printf("how int value is not (less than and same and grater than 0)?");
        exit(6);
    }
    for (i = 0; i < nBinarySize; i++)
    {
        aBinary[i] = (char)(aTemp[nBinarySize - 1 - i] & 0x01);
    }
}

int getIntTokenValue(char* pointerCharToken)
{
    int intValue = 0;
    int intI = 0;
    while (pointerCharToken[intI] != '\0')
    {
        // - 48 because ASCII code for 0 is 48. 
        intValue = (int)pointerCharToken[intI] - 48 + intValue * 10;
        intI++;
    }
    return intValue;
}

int getHexTokenValue(char* pointerCharToken)
{
    int intValue = 0;
    int intI = 0;
    while (pointerCharToken[intI] != '\0')
    {
        if (isdigit(pointerCharToken[intI]))
        {
            // - 48 because ASCII code for 0 is 48.
            intValue = (int)pointerCharToken[intI] - 48 + intValue * 16;
        }
        else if ((pointerCharToken[intI] >= 0x41) && (pointerCharToken[intI] <= 0x46))
        {
            // a is 97
            intValue = (int)pointerCharToken[intI] - 55 + intValue * 16;
        }
        else if ((pointerCharToken[intI] >= 0x61) && (pointerCharToken[intI] <= 0x66))
        {
            // a is 97
            intValue = (int)pointerCharToken[intI] - 87 + intValue * 16;
        }
        intI++;
    }
    return intValue;
}

char changeDecToHexChar(int value)
{
    if (value == 0)
    {
        return '0';
    }
    else if (value == 1)
    {
        return '1';
    }
    else if (value == 2)
    {
        return '2';
    }
    else if (value == 3)
    {
        return '3';
    }
    else if (value == 4)
    {
        return '4';
    }
    else if (value == 5)
    {
        return '5';
    }
    else if (value == 6)
    {
        return '6';
    }
    else if (value == 7)
    {
        return '7';
    }
    else if (value == 8)
    {
        return '8';
    }
    else if (value == 9)
    {
        return '9';
    }
    else if (value == 10)
    {
        return 'a';
    }
    else if (value == 11)
    {
        return 'b';
    }
    else if (value == 12)
    {
        return 'c';
    }
    else if (value == 13)
    {
        return 'd';
    }
    else if (value == 14)
    {
        return 'e';
    }
    else if (value == 15)
    {
        return 'f';
    }
    else
    {
        printf("%d isn't 0~15", value);
        return 'k';
    }
}

void storeData(char* MemoryBlock, char* Buffer, int* RelativeSegmentAddress, int* pointerAmount, int* SegmentAddress, int* WorkingInstruction)
{
    int intFlag = 1;
    int intTemp = 0;
    int intSurplus0 = 1;
    int i = 0;
    MemoryBlock[*RelativeSegmentAddress] = 0x30;
    (*RelativeSegmentAddress)++;
    MemoryBlock[*RelativeSegmentAddress] = 'x';
    (*RelativeSegmentAddress)++;
    for (i = 0; i < 32; i++)
    {
        intTemp = intTemp * 2 + (int)Buffer[i];
        if (intFlag == 4)
        {
            if (intTemp != 0)
            {
                intSurplus0 = 0;
            }
            if (intSurplus0 == 0)
            {
                MemoryBlock[*RelativeSegmentAddress] = changeDecToHexChar(intTemp);
                (*RelativeSegmentAddress)++;
            }
            intTemp = 0;
            intFlag = 0;
        }
        intFlag++;
        Buffer[i] = -1;
    }
    if (MemoryBlock[*RelativeSegmentAddress - 1] == 'x')
    {
        MemoryBlock[*RelativeSegmentAddress] = '0';
        (*RelativeSegmentAddress)++;
    }
    MemoryBlock[*RelativeSegmentAddress] = 0xA;
    (*RelativeSegmentAddress)++;
    *SegmentAddress += 4;
    *pointerAmount += 1;
    for (i = 0; i < 32; i++)
    {
        Buffer[i] = -1;
    }
    Buffer[32] = '\0';
    *WorkingInstruction = Nothing;
}

void storeOPCode(char* chararrayBuffer, int intHexValue)
{
    char chararrayTemp[33];
    storeDecInBin(chararrayTemp, 32, intHexValue);
    for (int i = 0; i < 6; i++)
    {
        chararrayBuffer[i] = chararrayTemp[26 + i];
    }
}

void storeFunctionField(char* chararrayBuffer, int intHexValue)
{
    char chararrayTemp[33];
    storeDecInBin(chararrayTemp, 32, intHexValue);
    for (int i = 0; i < 6; i++)
    {
        chararrayBuffer[26 + i] = chararrayTemp[26 + i];
    }
}

int isRegisterFilled(char* Register)
{
    for (int i = 0; i < 5; i++)
    {
        if (Register[i] != 0 && Register[i] != 1)
        {
            return 0;
        }
    }
    return 1;
}

int isFilled(char* Buffer)
{
    int i = 0;
    while (Buffer[i] != '\0')
    {
        if (Buffer[i] == -1)
            return 0;
        i++;
    }
    return 1;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("Please do ./runfile sample.s");
        exit(1);
    }
    FILE* fp1, * fp2;
    fp1 = fopen(argv[1], "rb");
    fp2 = fopen("sample.o", "wb");
    if (fp1 == NULL)
    {
        printf("file open fail\n");
        exit(2);
    }
    printf("file open suceed\n");

    int mode = NoMode;
    int max_length_of_token = 10;
    char token[33];
    int result = 1;
    int* MemoryPointer = 0;
    int DataSegmentAddress = 0x10000000;
    int RelativeDataSegmentAddress = 0;
    int DataSegment = 0;
    char DataValue[1024] = { 0, };
    int TextSegmentAddress = 0x400000;
    int RelativeTextSegmentAddress = 0;
    int TextSegment = 0;
    char TextValue[1024] = { 0, };
    int DataSize = 0;
    int LabelMaxAmount = 10;
    Label LabelTable[10];
    int LabelCurrentNumber = 0;
    int WorkingInstruction = Nothing;
    char Buffer[33] = { -1, };
    int intIsShift = 0;
    int intIsLa = 0;
    int intLaLower16bit = 0;

    int intIsBEQBNE = 0;

    Buffer[32] = '\0';
    if (LabelTable == NULL)
    {
        printf("pLabelTable malloc error");
        exit(3);
    }
    // label registration process
    while (result)
    {
        result = getToken(fp1, token);
        if (islabeldirective(token, max_length_of_token))
        {
            for (int i = 0; i < max_length_of_token; i++)
            {
                if (token[i] != ':')
                {
                    LabelTable[LabelCurrentNumber].name[i] = token[i];
                }
                else if (token[i] == ':')
                {
                    LabelTable[LabelCurrentNumber].name[i] = '\0';
                    if (MemoryPointer != NULL)
                    {
                        LabelTable[LabelCurrentNumber].address = *MemoryPointer;
                        printf("%s is in %08x\n", LabelTable[LabelCurrentNumber].name, LabelTable[LabelCurrentNumber].address);
                        LabelCurrentNumber += 1;
                    }
                    break;
                }
                else
                {
                    printf("how can element not : but :?");
                    exit(5);
                }
            }
            if (LabelMaxAmount <= LabelCurrentNumber)
            {
                // MaxAmount realloc in twice
            }

        }
        else if (!strcmp(token, ".data"))
        {
            MemoryPointer = &DataSegmentAddress;
        }
        else if (!strcmp(token, ".text"))
        {
            MemoryPointer = &TextSegmentAddress;
        }
        else if (!strcmp(token, ".word"))
        {
            if (MemoryPointer != NULL)
            {
                *MemoryPointer = *MemoryPointer + 4;
            }
        }
        else if (!strcmp(token, "addiu"))
        {
            TextSegmentAddress += 4;
        }
        else if (!strcmp(token, "addu"))
        {
            TextSegmentAddress += 4;
        }
        else if (!strcmp(token, "and"))
        {
            TextSegmentAddress += 4;
        }
        else if (!strcmp(token, "andi"))
        {
            TextSegmentAddress += 4;
        }
        else if (!strcmp(token, "beq"))
        {
            TextSegmentAddress += 4;
        }
        else if (!strcmp(token, "bne"))
        {
            TextSegmentAddress += 4;
        }
        else if (!strcmp(token, "j"))
        {
            TextSegmentAddress += 4;
        }
        else if (!strcmp(token, "jal"))
        {
            TextSegmentAddress += 4;
        }
        else if (!strcmp(token, "jr"))
        {
            TextSegmentAddress += 4;
        }
        else if (!strcmp(token, "lui"))
        {
            TextSegmentAddress += 4;
        }
        else if (!strcmp(token, "lw"))
        {
            TextSegmentAddress += 4;
        }
        else if (!strcmp(token, "la"))
        {
            // special case
            result = getToken(fp1, token);
            result = getToken(fp1, token);
            int intTokenValue = -1;
            for (int i = 0; i < LabelCurrentNumber + 1; i++)
            {
                if (!strcmp(LabelTable[i].name, token))
                {
                    intTokenValue = LabelTable[i].address;
                    break;
                }
            }
            if ((intTokenValue & 0xffff) == 0x0000)
            {
                TextSegmentAddress += 4;
            }
            else if ((intTokenValue & 0xffff) != 0x0000)
            {
                TextSegmentAddress += 4;
                TextSegmentAddress += 4;
            }
        }
        else if (!strcmp(token, "nor"))
        {
            TextSegmentAddress += 4;
        }
        else if (!strcmp(token, "or"))
        {
            TextSegmentAddress += 4;
        }
        else if (!strcmp(token, "ori"))
        {
            TextSegmentAddress += 4;
        }
        else if (!strcmp(token, "sltiu"))
        {
            TextSegmentAddress += 4;
        }
        else if (!strcmp(token, "sltu"))
        {
            TextSegmentAddress += 4;
        }
        else if (!strcmp(token, "sll"))
        {
            TextSegmentAddress += 4;
        }
        else if (!strcmp(token, "srl"))
        {
            TextSegmentAddress += 4;
        }
        else if (!strcmp(token, "sw"))
        {
            TextSegmentAddress += 4;
        }
        else if (!strcmp(token, "subu"))
        {
            TextSegmentAddress += 4;
        }
        else if (!strcmp(token, "lb"))
        {
            TextSegmentAddress += 4;
        }
        else if (!strcmp(token, "sb"))
        {
            TextSegmentAddress += 4;
        }
        else
        {
            continue;
        }
        
        for (int i = 0; i < max_length_of_token; i++)
        {
            token[i] = -1;
        }
    }

    rewind(fp1);
    DataSegmentAddress = 0x10000000;
    TextSegmentAddress = 0x400000;
    result = 1;

    while (result)
    {
        // get token
        result = getToken(fp1, token);
        // directives
        if (!strcmp(token, ".data"))
        {
            // data mode
            mode = DataMode;
            MemoryPointer = &DataSegmentAddress;
        }
        else if (!strcmp(token, ".text"))
        {
            // instruction mode
            mode = TextMode;
            MemoryPointer = &TextSegmentAddress;
        }
        // is data size directive?
        else if (!strcmp(token, ".word"))
        {
            printf("%s program3 work\n", token);
            DataSize = 4;
        }
        // is label directive like array:?
        else if (islabeldirective(token, max_length_of_token))
        {
        }
        // instruction
        else if (!strcmp(token, "addiu"))
        {
            WorkingInstruction = I;
            storeOPCode(Buffer, 0x09);
        }
        else if (!strcmp(token, "addu"))
        {
            WorkingInstruction = R;
            storeOPCode(Buffer, 0x00);
            storeDecInBin(Buffer + 21, 5, 0);
            storeDecInBin(Buffer + 26, 6, 0x21);
        }
        else if (!strcmp(token, "and"))
        {
            WorkingInstruction = R;
            storeOPCode(Buffer, 0x00);
            storeDecInBin(Buffer + 21, 5, 0);
            storeDecInBin(Buffer + 26, 6, 0x24);
        }
        else if (!strcmp(token, "andi"))
        {
            WorkingInstruction = I;
            storeOPCode(Buffer, 0xc);
        }
        else if (!strcmp(token, "beq"))
        {
            // special case that rs comes first
            WorkingInstruction = I;
            intIsBEQBNE = 1;
            storeOPCode(Buffer, 0x4);
        }
        else if (!strcmp(token, "bne"))
        {
            // special case that rs comes first
            WorkingInstruction = I;
            intIsBEQBNE = 1;
            storeOPCode(Buffer, 0x5);
        }
        else if (!strcmp(token, "j"))
        {
            WorkingInstruction = J;
            storeOPCode(Buffer, 0x2);
        }
        else if (!strcmp(token, "jal"))
        {
            WorkingInstruction = J;
            storeOPCode(Buffer, 0x3);
        }
        else if (!strcmp(token, "jr"))
        {
            // special case
            WorkingInstruction = R;
            storeOPCode(Buffer, 0x00);
            storeDecInBin(Buffer + 11, 15, 0);
            storeDecInBin(Buffer + 26, 6, 0x8);
        }
        else if (!strcmp(token, "lui"))
        {
            WorkingInstruction = I;
            storeOPCode(Buffer, 0xf);
            storeDecInBin(Buffer + 6, 5, 0);
        }
        else if (!strcmp(token, "lw"))
        {
            WorkingInstruction = I;
            storeOPCode(Buffer, 0x23);
        }
        else if (!strcmp(token, "la"))
        {
            // special case
            WorkingInstruction = I;
            intIsLa = 1;
        }
        else if (!strcmp(token, "nor"))
        {
            WorkingInstruction = R;
            storeOPCode(Buffer, 0x00);
            storeDecInBin(Buffer + 21, 5, 0);
            storeDecInBin(Buffer + 26, 6, 0x27);
        }
        else if (!strcmp(token, "or"))
        {
            WorkingInstruction = R;
            storeOPCode(Buffer, 0x00);
            storeDecInBin(Buffer + 21, 5, 0);
            storeDecInBin(Buffer + 26, 6, 0x25);
        }
        else if (!strcmp(token, "ori"))
        {
            WorkingInstruction = I;
            storeOPCode(Buffer, 0xd);
        }
        else if (!strcmp(token, "sltiu"))
        {
            WorkingInstruction = I;
            storeOPCode(Buffer, 0xb);
        }
        else if (!strcmp(token, "sltu"))
        {
            WorkingInstruction = R;
            storeOPCode(Buffer, 0x00);
            storeDecInBin(Buffer + 21, 5, 0);
            storeDecInBin(Buffer + 26, 6, 0x2b);
        }
        else if (!strcmp(token, "sll"))
        {
            // special case
            WorkingInstruction = R;
            storeOPCode(Buffer, 0x00);
            storeDecInBin(Buffer + 6, 5, 0);
            storeDecInBin(Buffer + 26, 6, 0x0);
            intIsShift = 1;
        }
        else if (!strcmp(token, "srl"))
        {
            // special case
            WorkingInstruction = R;
            storeOPCode(Buffer, 0x00);
            storeDecInBin(Buffer + 6, 5, 0);
            storeDecInBin(Buffer + 26, 6, 0x2);
            intIsShift = 1;
        }
        else if (!strcmp(token, "sw"))
        {
            WorkingInstruction = I;
            storeOPCode(Buffer, 0x2b);
        }
        else if (!strcmp(token, "subu"))
        {
            WorkingInstruction = R;
            storeOPCode(Buffer, 0x00);
            storeDecInBin(Buffer + 21, 5, 0);
            storeDecInBin(Buffer + 26, 6, 0x23);
        }
        else if (!strcmp(token, "lb"))
        {
            WorkingInstruction = I;
            storeOPCode(Buffer, 0x20);
        }
        else if (!strcmp(token, "sb"))
        {
            WorkingInstruction = I;
            storeOPCode(Buffer, 0x28);
        }
        // label?
        else if (islabel(token, LabelTable, LabelCurrentNumber + 1))
        {
            int intTokenValue = -1;
            for (int i = 0; i < LabelCurrentNumber + 1; i++)
            {
                if (!strcmp(LabelTable[i].name, token))
                {
                    intTokenValue = LabelTable[i].address;
                    break;
                }
            }
            if (intTokenValue == -1)
            {
                printf("There is no %s label in LabelTable\n", token);
                exit(68);
            }
            if (mode == TextMode)
            {
                if (intIsLa != 0)
                {
                    if ((intTokenValue & 0xffff) == 0x0000)
                    {
                        WorkingInstruction = I;
                        storeOPCode(Buffer, 0xf);
                        storeDecInBin(Buffer + 16, 16, (intTokenValue >> 16));
                        storeDecInBin(Buffer + 6, 5, 0);
                        intIsLa = 0;
                    }
                    else if ((intTokenValue & 0xffff) != 0x0000)
                    {
                        WorkingInstruction = I;
                        storeOPCode(Buffer, 0xf);
                        storeDecInBin(Buffer + 6, 5, 0);
                        storeDecInBin(Buffer + 16, 16, (intTokenValue >> 16));
                        intLaLower16bit = intTokenValue & 0xffff;
                    }
                }
                else if (WorkingInstruction == I)
                {
                    if (intIsBEQBNE == 1)
                    {
                        storeDecInBin(Buffer + 16, 16, (intTokenValue - TextSegmentAddress - 4) / 4);
                        intIsBEQBNE = 0;
                    }
                    else
                    {
                        storeDecInBin(Buffer + 16, 16, intTokenValue / 4);
                    }
                }
                else if (WorkingInstruction == J)
                {
                    storeDecInBin(Buffer + 6, 26, intTokenValue / 4);
                }
            }
            else
            {
                printf("No Text Mode but label call");
                exit(654);
            }
        }
        // just dex number
        else if (token[0] == '0' && token[1] == 'x')
        {
            // data
            // dec to bin + how long
            int intTokenValue = getHexTokenValue(token);
            if (mode == DataMode)
            {
                storeDecInBin(Buffer, 32, intTokenValue);
                storeData(DataValue, Buffer, &RelativeDataSegmentAddress, &DataSegment, &DataSegmentAddress, &WorkingInstruction);
            }

            // instruction
            else if (mode == TextMode)
            {
                if (WorkingInstruction == R)
                {
                    if (intIsShift == 1)
                    {
                        storeDecInBin(Buffer + 21, 5, intTokenValue);
                    }
                    else
                    {
                        printf("R Instruction don't need just value");
                        exit(123);
                    }
                }
                else if (WorkingInstruction == I)
                {
                    storeDecInBin(Buffer + 16, 16, intTokenValue);
                }
                else if (WorkingInstruction == J)
                {
                    storeDecInBin(Buffer + 6, 26, intTokenValue / 4);
                }
            }
            else
            {
                printf("No DataMode and TextMode But suddenly number came\n");
                exit(11);
            }
        }
        else if ((token[0] == 0x30) ||
                 (token[0] == 0x31) ||
                 (token[0] == 0x32) ||
                 (token[0] == 0x33) ||
                 (token[0] == 0x34) ||
                 (token[0] == 0x35) ||
                 (token[0] == 0x36) ||
                 (token[0] == 0x37) ||
                 (token[0] == 0x38) ||
                 (token[0] == 0x39) ||
                 (token[0] == '-'))
        {
             int intTokenValue;
             if (token[0] == '-')
             {
                 intTokenValue = -getIntTokenValue(token + 1);
             }
             else
             {
                 intTokenValue = getIntTokenValue(token);
             }
             if (mode == DataMode)
             {
                 storeDecInBin(Buffer, 32, intTokenValue);
                 storeData(DataValue, Buffer, &RelativeDataSegmentAddress, &DataSegment, &DataSegmentAddress, &WorkingInstruction);
             }
             else if (mode == TextMode)
             {
                 if (WorkingInstruction == R)
                 {
                     if (intIsShift == 1)
                     {
                         storeDecInBin(Buffer + 21, 5, intTokenValue);
                     }
                     else
                     {
                         printf("R Instruction don't need just value");
                         exit(124);
                     }
                 }
                 else if (WorkingInstruction == I)
                 {
                     storeDecInBin(Buffer + 16, 16, intTokenValue);
                 }
                 else if (WorkingInstruction == J)
                 {
                     storeDecInBin(Buffer + 6, 26, intTokenValue / 4);
                 }
             }
             else
             {
                 printf("Mode isn't DataMode and TextMode but there exists just number");
             }
             
        }


        // register
        else if (token[0] == '$')
        {
            int intTokenValue = getIntTokenValue(token + 1);
            if (mode == DataMode)
            {
                storeDecInBin(Buffer, 32, intTokenValue);
                storeData(DataValue, Buffer, &RelativeDataSegmentAddress, &DataSegment, &DataSegmentAddress, &WorkingInstruction);
            }

            // instruction
            else if (mode == TextMode)
            {
                if (WorkingInstruction == R)
                {
                    if (!isRegisterFilled(Buffer + 16))
                    {
                        storeDecInBin(Buffer + 16, 5, intTokenValue);
                    }
                    else if (!isRegisterFilled(Buffer + 6))
                    {
                        storeDecInBin(Buffer + 6, 5, intTokenValue);
                    }
                    else if (!isRegisterFilled(Buffer + 11))
                    {
                        storeDecInBin(Buffer + 11, 5, intTokenValue);
                    }
                    else
                    {
                        printf("R Instruction don't need anymore registers");
                        exit(256);
                    }
                }
                else if (WorkingInstruction == I)
                {
                    if (intIsBEQBNE == 1)
                    {
                        if (!isRegisterFilled(Buffer + 6))
                        {
                            storeDecInBin(Buffer + 6, 5, intTokenValue);
                        }
                        else if (!isRegisterFilled(Buffer + 11))
                        {
                            storeDecInBin(Buffer + 11, 5, intTokenValue);
                        }
                        else
                        {
                            printf("I Instruction don't need anymore registers");
                            exit(256);
                        }
                    }
                    else if (!isRegisterFilled(Buffer + 11))
                    {
                        storeDecInBin(Buffer + 11, 5, intTokenValue);
                    }
                    else if (!isRegisterFilled(Buffer + 6))
                    {
                        storeDecInBin(Buffer + 6, 5, intTokenValue);
                    }
                    else
                    {
                        printf("I Instruction don't need anymore registers");
                        exit(256);
                    }
                }
                else if (WorkingInstruction == J)
                {
                    if (!isRegisterFilled(Buffer + 6))
                    {
                        printf("J Type Instruction don't need anymore registers");
                        exit(312);
                    }
                }
            }
        }

        else
        {
            break;
        }

        // Instruction Write
        if (mode == TextMode)
        {
            if (WorkingInstruction == R && isRegisterFilled(Buffer + 6) && isRegisterFilled(Buffer + 11) && isRegisterFilled(Buffer + 16) && isRegisterFilled(Buffer + 21))
            {
                storeData(TextValue, Buffer, &RelativeTextSegmentAddress, &TextSegment, &TextSegmentAddress, &WorkingInstruction);
                printf("R complete. Text amount : %d\n", TextSegment);
                intIsShift = 0;
            }
            else if (WorkingInstruction == I && isRegisterFilled(Buffer + 6) && isRegisterFilled(Buffer + 11) && isFilled(Buffer + 16))
            {
                if (intIsLa == 1)
                {
                    // backup register for ori
                    for (int i = 0; i < 5; i++)
                    {
                        token[i] = Buffer[11 + i];
                    }
                    // input lui
                    storeData(TextValue, Buffer, &RelativeTextSegmentAddress, &TextSegment, &TextSegmentAddress, &WorkingInstruction);
                    printf("I complete. Text amount : %d\n", TextSegment);
                    // store ori
                    for (int i = 0; i < 5; i++)
                    {
                        Buffer[6 + i] = token[i];
                        Buffer[11 + i] = token[i];
                    }
                    storeOPCode(Buffer, 0xd);
                    storeDecInBin(Buffer + 16, 16, intLaLower16bit);
                    intLaLower16bit = 0;
                    intIsLa--;
                }
                storeData(TextValue, Buffer, &RelativeTextSegmentAddress, &TextSegment, &TextSegmentAddress, &WorkingInstruction);
                printf("I complete. Text amount : %d\n", TextSegment);
            }
            else if (WorkingInstruction == J && isFilled(Buffer + 6))
            {
                storeData(TextValue, Buffer, &RelativeTextSegmentAddress, &TextSegment, &TextSegmentAddress, &WorkingInstruction);
                printf("J complete. Text amount : %d\n", TextSegment);
            }
            for (int i = 0; i < max_length_of_token; i++)
            {
                token[i] = -1;
            }
        }
    }

    rewind(fp2);
    char test[4] = "yes";
    char arrayTemp[33] = { 0, };
    int intTemp;
    int intTempAddress = 0;
    char arraySeparate[2] = "\n";
    // InstructionSizeWriteInFile
    storeDecInBin(Buffer, 32, TextSegment * 4);
    storeData(arrayTemp, Buffer, &intTempAddress, &intTemp, &intTemp, &intTemp);
    intTempAddress = 0;
    intTemp = 0;
    while (arrayTemp[intTemp] != '\n')
    {
        fwrite(arrayTemp + intTemp, sizeof(char), sizeof(char), fp2);
        intTemp++;
    }
    fwrite(arraySeparate, sizeof(char), sizeof(char), fp2);
    // DataSizeWriteInFile
    storeDecInBin(Buffer, 32, DataSegment * 4);
    storeData(arrayTemp, Buffer, &intTempAddress, &intTemp, &intTemp, &intTemp);
    intTempAddress = 0;
    intTemp = 0;
    while (arrayTemp[intTemp] != '\n')
    {
        fwrite(arrayTemp + intTemp, sizeof(char), sizeof(char), fp2);
        intTemp++;
    }
    fwrite(arraySeparate, sizeof(char), sizeof(char), fp2);
    // TextSegmentValueWriteInFile
    intTemp = 0;
    for (int i = 0; i < TextSegment; i++)
    {
        while (TextValue[intTemp] != '\n')
        {
            fwrite(TextValue + intTemp, sizeof(char), sizeof(char), fp2);
            intTemp++;
        }
        intTemp++;
        fwrite(arraySeparate, sizeof(char), sizeof(char), fp2);
    }
    // DataSegmentValueWriteInFile
    intTemp = 0;
    for (int i = 0; i < DataSegment; i++)
    {
        while (DataValue[intTemp] != '\n')
        {
            fwrite(DataValue + intTemp, sizeof(char), sizeof(char), fp2);
            intTemp++;
        }
        intTemp++;
        fwrite(arraySeparate, sizeof(char), sizeof(char), fp2);
    }
        fclose(fp1);
        fclose(fp2);

        return 0;
}