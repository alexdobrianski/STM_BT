// hex2download.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "windows.h"

int FirstEntry;
DWORD LastEntry;
DWORD SkipCheck;
int iBytesInLine;
void OutputOffset(FILE *OutputF, DWORD dwOff)
{
    if (FirstEntry ==0)
    {
        FirstEntry = 1;
        fprintf(OutputF, "%08x:  ", dwOff);
        LastEntry = dwOff;
        iBytesInLine = 0;
    }
    else
    {
        if (dwOff != SkipCheck)
        {
            int iCount = dwOff - SkipCheck;
            if (iCount <0)
            {
                iCount = 16 - iBytesInLine;
            }

            if (iCount)
            {
                //if (iCount > 16)
                //    iCount =((dwOff - SkipCheck + 16)&0x0000000f);
                for (int i = 0 ; i < iCount; i++)
                {
                    //if (iCount >=8)
                    if (iBytesInLine == 8)
                        fprintf(OutputF, "! ");
                    fprintf(OutputF, "FF ");
                    iBytesInLine++;
                    if (iBytesInLine >= 16) // no more than 16 bytes in one line
                    {
                        //fprintf(OutputF, "\nNewLine");
                        iBytesInLine=0;
                        break;
                    }
                }
            }
        }
        if ((dwOff >= (LastEntry +16)) || ((int)(dwOff - SkipCheck) < 0))
        {
            fprintf(OutputF, "\n%08x:  ", dwOff);
            LastEntry = dwOff;
            iBytesInLine = 0;
        }
        else if (iBytesInLine == 8) //if (dwOff == (LastEntry +8))
        {
            fprintf(OutputF, "| ");
        }
    }
    SkipCheck = dwOff+1;
}
DWORD HexVal(char *pHex)
{
    DWORD dwHex = 0;
    int i;
    for ( i = 0; i < strlen(pHex); i++)
    {
        dwHex *= 16;
        if ((pHex[i] >= '0') && (pHex[i] <= '9'))
            dwHex += pHex[i] - '0';
        else if ((pHex[i] >= 'A') && (pHex[i] <= 'F'))
            dwHex += pHex[i] - 'A' + 10;
        else if ((pHex[i] >= 'a') && (pHex[i] <= 'f'))
            dwHex += pHex[i] - 'a' + 10;
        else 
            break;
    }
    return dwHex;
}


int _tmain(int argc, _TCHAR* argv[])
{
    DWORD AddresConfig;
    DWORD OfsetConfig;
    FirstEntry=0;
    LastEntry = 0;
    SkipCheck = 0;
    if ((argc != 6) && (argc != 5))
    {
        printf ("\n usage:\n      hex2download <input> <output> 0x0ffsetInFlash 0xAddresConfig [0xOfsetConfig] ");
        printf ("\n  PIC18F25  0xAddresConfig=0x300000 0xOfsetConfig=0x10000");
        printf ("\n  PIC24Hb   0xAddresConfig=0x1f00000 0xOfsetConfig=0x20000");
        return 1;
    }
    FILE * InputF = fopen(argv[1],"r");
    if (InputF)
    {
        FILE *OutputF = fopen(argv[2],"w");
        if (OutputF)
        {
            if ((argc ==6) || (argc ==5))
            {
                DWORD Offset = HexVal(&argv[3][2]);
                AddresConfig = HexVal(&argv[4][2]);
                if (argc ==6)
                    OfsetConfig  = HexVal(&argv[5][2]);
                else
                    OfsetConfig = 0;
                DWORD OffsetAddon= 0;;
                char szString[256];
                int iget;
                memset(szString, 0, sizeof(szString));
                while(fgets(szString, sizeof(szString)-1, InputF))
                {
                    //  Each line in the file has this format:
                    // :BBAAAATT[DDDDDDDD]CC
                    //                             where
                    // :                           is start of line marker
                    //  BB                         is number of data bytes on line
                    //    AAAA                     is address in bytes
                    //        TT                   is type. 00 means data, 01 means EOF and 04 means extended address
                    //             DD              is data bytes, number depends on BB value
                    //                    CC       is checksum (2s-complement of number of bytes+address+data)
                    //    Code: This is at the top of the file and may be proceeded by an extended address 
                    //     line – :020000040000FA, where 04 is the type for extended address. 
                    //     Standard Intel Hex files can only address 64KB of data, 
                    //     and extended addressing gets over this limitation by adding extended address lines for every 64KB, 
                    //          for example
                    //    :020000040001F9 – 64KB marker
                    //    :020000040002F9 – 128KB marker
                    //:02 0000 04 0000 FA
                    //:04 0000 00 C7EF02F0 54
                    //:10 0008 00 D8CF60F0E0CF61F00001626FE9CF63F0 14
                    //:10 0018 00 EACF64F0CECF15F4CFCF16F4B2CF19F4 EF
                    //:10 0028 00 B3CF1AF41DC417F41EC418F41FC41BF4 6C
                    //:10 0038 00 20C41CF49EA056D09E90040100A104D0 B8
                    //:10 0048 00 0B2B000E0C234ED0040100A34BD0132B 16
                    //:10 0058 00 000E142313511411D8A444D0CE500D27 E8
                    //:10 0068 00 CF500E230E51CF6E0D51CE6E11C413F4 26
                    //:10 0078 00 12C414F40FC40DF410C40EF41D2B000E 9A
                    //:10 0088 00 1E2300A703D08A82898000970401052B CC
                    //:10 0098 00 020E05651FD0056BE9C006F40001D2AD 5C
                    //:10 00A8 00 0FD0E3A70DD00401032F0AD00001D289 95
                    //:10 00B8 00 040E0401036F0001D2BF02D0D74FD29D B6
                    //012 3456 78 9a....
                    // .....
                    //:10 2C88 00 F86A0800F55012007E000D0A434F4E4E B8
                    //:06 2C98 00 4543540D0A00 43
                    //:02 0000 04 0030 CA
                    //:0E 0000 00 FF061100FF8880FF0FC00FE00F40 C9
                    //:00 0000 01 FF

                    if (szString[0] == ':')
                    {
                        char szlen[8];
                        int ilen;
                        char szOffset[8];
                        DWORD dwOffset;
                        char szType[8];
                        char szEachByte[8];
                        int iType;
                        memset(szlen, 0, sizeof(szlen));
                        memset(szOffset, 0, sizeof(szOffset));
                        memset(szType, 0, sizeof(szOffset));
                        memcpy(szlen,&szString[1],2);
                        memcpy(szOffset,&szString[3],4);
                        memcpy(szType,&szString[7],2);
                        ilen = HexVal(szlen);
                        dwOffset =HexVal(szOffset);
                        iType = HexVal(szType);
                        if (iType == 0)
                        {
                            if ((argc ==6) || ((argc ==5) && (OffsetAddon != OfsetConfig-Offset)))
                            {
                                //iBytesInLine = 0;
                                for (int i =0; i < ilen; i++)
                                {
                                    OutputOffset(OutputF, Offset + OffsetAddon + dwOffset + i);
                                    memset(szEachByte, 0, sizeof(szEachByte));
                                    memcpy(szEachByte,&szString[9+2*i],2);
                                    fprintf(OutputF, "%s ", szEachByte);
                                    iBytesInLine++;
                                }
                            }
                        }
                        else if (iType == 4)
                        {
                            memset(szOffset, 0, sizeof(szOffset));
                            memcpy(szOffset,&szString[9],4);
                            OffsetAddon = HexVal(szOffset);
                            
                            OffsetAddon <<=16;
                            if (OffsetAddon == AddresConfig)
                                OffsetAddon = OfsetConfig-Offset; 
                            //FirstEntry=0;
                            //LastEntry = 0;
                            //SkipCheck = 0;
                        }
                        else if (iType == 1)
                        {
                            break;
                        }
                    }
                }
                int iRest = SkipCheck&0x0000000f;
                for (;iRest < 16; iRest++)
                {
                    fprintf(OutputF, "FF ");
                }
            }
            fclose(OutputF);
        }
        fclose(InputF);
    }
    else
    {
        printf ("\n usage:\n      hex2download <input> <output> 0x0ffsetInFlash 0xAddresConfig [0xOfsetConfig] ");
        printf ("\n  PIC18F25  0xAddresConfig=0x300000 0xOfsetConfig=0x10000");
        printf ("\n  PIC24Hb   0xAddresConfig=0x1f00000 0xOfsetConfig=0x20000");
    }
	return 0;
}


