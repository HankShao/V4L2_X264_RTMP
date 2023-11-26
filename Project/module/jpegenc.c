#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

const unsigned char Lum_Quantization_Tab[64]=
{
    16, 11, 10, 16, 24, 40, 51, 61,
    12, 12, 14, 19, 26, 58, 60, 55,
    14, 13, 16, 24, 40, 57, 69, 56,
    14, 17, 22, 29, 51, 87, 80, 62,
    18, 22, 37, 56, 68,109,103, 77,
    24, 35, 55, 64, 81,104,113, 92,
    49, 64, 78, 87,103,121,120,101,
    72, 92, 95, 98,112,100,103, 99
};

const unsigned char Chroma_Quantization_Tab[64]=
{
    17, 18, 24, 47, 99, 99, 99, 99,
    18, 21, 26, 66, 99, 99, 99, 99,
    24, 26, 56, 99, 99, 99, 99, 99,
    47, 66, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99
};

const unsigned char ZigZag[64] =
{
    0, 1, 5, 6, 14, 15, 27, 28,
    2, 4, 7,13, 16, 26, 29, 42,
    3, 8,12,17, 25, 30, 41, 43,
    9,11,18,24, 31, 40, 44, 53,
   10,19,23,32, 39, 45, 52, 54,
   20,22,33,38, 46, 51, 55, 60,
   21,34,37,47, 50, 56, 59, 61,
   35,36,48,49, 57, 58, 62, 63
};

const char Standard_DC_Lum_NRCodes[] = {0, 0, 7, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0};
const unsigned char Standard_DC_Lum_Values[] = {4, 5, 3, 2, 6, 1, 0, 7, 8, 9, 10, 11};
const char Standard_DC_Chroma_NRCodes[] = {0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0};
const unsigned char Standard_DC_Chroma_Values[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
const char Standard_AC_Lum_NRCodes[] = { 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d };
const unsigned char Standard_AC_Lum_Values[] =
{
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
    0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
    0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
    0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
    0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
    0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
    0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
    0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
    0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
    0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
    0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
    0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa
};

//-------------------------------------------------------------------------------
const char Standard_AC_Chroma_NRCodes[] = { 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77 };
const unsigned char Standard_AC_Chroma_Values[] =
{
    0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
    0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
    0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
    0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
    0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
    0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
    0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
    0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
    0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
    0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
    0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
    0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
    0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
    0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa
};

unsigned short picwidth, picheight;
unsigned char MB_YTab[64];
unsigned char MB_CbCrTab[64];

typedef struct
{
    int length;
    int value;
}BitString;
BitString Y_DC_Huffman_Tab[12];
BitString Y_AC_Huffman_Tab[256];
BitString CbCr_DC_Huffman_Tab[12];
BitString CbCr_AC_Huffman_Tab[256];

//bmp file structure
#pragma pack(push, 2)
typedef struct{
    unsigned short bfType;  //"BM"
    unsigned int   bfSize;  //文件大小
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int   bfOffset; //图像数据开始偏移
}BITMAPFILEHEADER;

typedef struct{
    unsigned int biSize;  //数据头大小
    int biWidth;  //图片宽度
    int biHeight; //图片高度
    unsigned short biPlanes;   //色彩分量平面数
    unsigned short biBitCount; //像素点所占bit数
    unsigned int biCompression;//压缩方式
    unsigned int biSizeImage;  //原始位图数据大小
    int biXPelsPerMeter; //纵向分辨率
    int biYPelsPerMeter; //横向分辨率
    unsigned int biClrUsed;
    unsigned int biClrImportant;
}BITMAPINFOHEADER;
#pragma pack(pop)

int readbmp(char *path, char **ppRgb, int *pw, int *ph)
{
    FILE *fp = fopen(path, "r");
    long bmpsize;
    char *prgb = NULL;
    BITMAPFILEHEADER bmpFhead;
    BITMAPINFOHEADER bmpIhead;
    fread(&bmpFhead, 1, sizeof(bmpFhead), fp); //读取BMP图片头信息
    if (bmpFhead.bfType != 0x4D42)
        return -1;
    fread(&bmpIhead, 1, sizeof(bmpIhead), fp); //读取BMP图片详细头信息
    if (bmpIhead.biBitCount != 24 || bmpIhead.biCompression != 0)
        return -1;
    if ((bmpIhead.biWidth&7)!=0 || (bmpIhead.biHeight&7)!=0) //sample简化宏块处理，只支持宽高宏块对齐尺寸
    {
        printf("bmp %dx%d not support!\n", bmpIhead.biWidth, bmpIhead.biHeight);
        return -1;
    }
    bmpsize = bmpIhead.biWidth * bmpIhead.biHeight * 3;
    prgb = malloc(bmpsize);
    for(int i=0; i < bmpIhead.biHeight; i++)
    {//数据存储方式是从左到右，从下到上，小端序
        if (bmpIhead.biWidth != fread(prgb + (bmpIhead.biHeight-1-i)*bmpIhead.biWidth*3, 3, bmpIhead.biWidth, fp))
        {
            free(prgb);
            return -1;
        }
    }

    fclose(fp);
    *ppRgb = prgb;
    *pw    = bmpIhead.biWidth;
    *ph    = bmpIhead.biHeight;

    return 0;
}

int _init_QualityTables(int Qp)
{
    Qp = (Qp >= 100)?99:(Qp<=0?1:Qp);
    for (int i=0, tmp=0; i<64; i++)
    {
        tmp = ((int)Lum_Quantization_Tab[i] * Qp + 50) / 100;
        tmp = (tmp > 0xFF)?0xFF:(tmp<=0?1:tmp);
        MB_YTab[ZigZag[i]] = (unsigned char)tmp;   //按照ZigZag扫描的顺序存储Qp调整后的量化值

        tmp = ((int)Chroma_Quantization_Tab[i] * Qp + 50) / 100;
        tmp = (tmp > 0xFF)?0xFF:(tmp<=0?1:tmp);
        MB_CbCrTab[ZigZag[i]] = (unsigned char)tmp;//同上
    }

    return 0;
}

void _computeHuffmanTable(const char* nr_codes, const unsigned char* std_table, BitString* huffman_table)
{
    unsigned char pos_in_table = 0;
    unsigned short code_value = 0;

    for(int k = 1; k <= 16; k++)
    {
        for(int j = 1; j <= nr_codes[k-1]; j++)
        {
            huffman_table[std_table[pos_in_table]].value = code_value;
            huffman_table[std_table[pos_in_table]].length = k;
            pos_in_table++;
            code_value++;
        }
        code_value <<= 1;
    }
}
void _initHuffmanTables(void)
{
    memset(&Y_DC_Huffman_Tab, 0, sizeof(Y_DC_Huffman_Tab));
    _computeHuffmanTable(Standard_DC_Lum_NRCodes, Standard_DC_Lum_Values, Y_DC_Huffman_Tab);

    memset(&Y_AC_Huffman_Tab, 0, sizeof(Y_AC_Huffman_Tab));
    _computeHuffmanTable(Standard_AC_Lum_NRCodes, Standard_AC_Lum_Values, Y_AC_Huffman_Tab);

    memset(&CbCr_DC_Huffman_Tab, 0, sizeof(CbCr_DC_Huffman_Tab));
    _computeHuffmanTable(Standard_DC_Chroma_NRCodes, Standard_DC_Chroma_Values, CbCr_DC_Huffman_Tab);

    memset(&CbCr_AC_Huffman_Tab, 0, sizeof(CbCr_AC_Huffman_Tab));
    _computeHuffmanTable(Standard_AC_Chroma_NRCodes, Standard_AC_Chroma_Values, CbCr_AC_Huffman_Tab);
}

BitString _getBitCode(int value)
{
    BitString ret;
    int v = (value>0) ? value : -value;

    //bit 的长度
    int length = 0;
    for(length=0; v; v>>=1) length++;

    ret.value = value>0 ? value : ((1<<length)+value-1);
    ret.length = length;

    return ret;
}

void _write_(const void* p, int byteSize, FILE* fp)
{
    fwrite(p, 1, byteSize, fp);
}
void _write_word_(unsigned short value, FILE *fp)
{
    unsigned short _value = ((value>>8)&0xFF)|((value&0xFF)<<8); //value是小端模式，对字节顺序调整
    fwrite(&_value, 1, 2, fp);
    return;
}

void _write_byte_(unsigned char value, FILE *fp)
{
    fwrite(&value, 1, 1, fp);
    return;
}

void _write_bitstring_(const BitString* bs, int counts, int* pnewByte, int *pnewBytePos, FILE* fp)
{
    unsigned short mask[] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768};
    int newByte = *pnewByte;
    int newBytePos = *pnewBytePos;

    for(int i=0; i<counts; i++)
    {
        int value = bs[i].value;
        int posval = bs[i].length - 1;

        while (posval >= 0)
        {
            if ((value & mask[posval]) != 0)
            {
                newByte = newByte  | mask[newBytePos];
            }
            posval--;
            newBytePos--;
            if (newBytePos < 0)
            {
                // Write to stream
                _write_byte_((unsigned char)(newByte), fp);
                if (newByte == 0xFF)
                {
                    // Handle special case
                    _write_byte_((unsigned char)(0x00), fp);
                }

                // Reinitialize
                newBytePos = 7;
                newByte = 0;
            }
        }
    }

    *pnewByte = newByte;
    *pnewBytePos = newBytePos;
}
void _write_jpeg_header(FILE* fp)
{
    //SOI
    _write_word_(0xFFD8, fp);        // marker = 0xFFD8

    //APPO
    _write_word_(0xFFE0,fp);        // marker = 0xFFE0
    _write_word_(16, fp);            // length = 16 for usual JPEG, no thumbnail
    _write_("JFIF", 5, fp);            // 'JFIF\0'
    _write_byte_(1, fp);            // version_hi
    _write_byte_(1, fp);            // version_low
    _write_byte_(0, fp);            // xyunits = 0 no units, normal density
    _write_word_(1, fp);            // xdensity
    _write_word_(1, fp);            // ydensity
    _write_byte_(0, fp);            // thumbWidth
    _write_byte_(0, fp);            // thumbHeight

    //DQT
    _write_word_(0xFFDB, fp);        //marker = 0xFFDB
    _write_word_(132, fp);            //size=132
    _write_byte_(0, fp);            //QTYinfo== 0:  bit 0..3: number of QT = 0 (table for Y)
                                    //                bit 4..7: precision of QT
                                    //                bit 8    : 0
    _write_(MB_YTab, 64, fp);        //YTable
    _write_byte_(1, fp);            //QTCbinfo = 1 (quantization table for Cb,Cr)
    _write_(MB_CbCrTab, 64, fp);    //CbCrTable

    //SOFO
    _write_word_(0xFFC0, fp);            //marker = 0xFFC0
    _write_word_(17, fp);                //length = 17 for a truecolor YCbCr JPG
    _write_byte_(8, fp);                //precision = 8: 8 bits/sample
    _write_word_(picheight&0xFFFF, fp);    //height
    _write_word_(picwidth&0xFFFF, fp);    //width
    _write_byte_(3, fp);                //nrofcomponents = 3: We encode a truecolor JPG

    _write_byte_(1, fp);                //IdY = 1
    _write_byte_(0x11, fp);                //HVY sampling factors for Y (bit 0-3 vert., 4-7 hor.)(SubSamp 1x1)
    _write_byte_(0, fp);                //QTY  Quantization Table number for Y = 0

    _write_byte_(2, fp);                //IdCb = 2
    _write_byte_(0x11, fp);                //HVCb = 0x11(SubSamp 1x1)
    _write_byte_(1, fp);                //QTCb = 1

    _write_byte_(3, fp);                //IdCr = 3
    _write_byte_(0x11, fp);                //HVCr = 0x11 (SubSamp 1x1)
    _write_byte_(1, fp);                //QTCr Normally equal to QTCb = 1

    //DHT
    _write_word_(0xFFC4, fp);        //marker = 0xFFC4
    _write_word_(0x01A2, fp);        //length = 0x01A2
    _write_byte_(0, fp);            //HTYDCinfo bit 0..3    : number of HT (0..3), for Y =0
                                    //            bit 4        : type of HT, 0 = DC table,1 = AC table
                                    //            bit 5..7    : not used, must be 0
    _write_(Standard_DC_Lum_NRCodes, sizeof(Standard_DC_Lum_NRCodes), fp);    //DC_L_NRC
    _write_(Standard_DC_Lum_Values, sizeof(Standard_DC_Lum_Values), fp);        //DC_L_VALUE
    _write_byte_(0x10, fp);            //HTYACinfo
    _write_(Standard_AC_Lum_NRCodes, sizeof(Standard_AC_Lum_NRCodes), fp);
    _write_(Standard_AC_Lum_Values, sizeof(Standard_AC_Lum_Values), fp); //we'll use the standard Huffman tables
    _write_byte_(0x01, fp);            //HTCbDCinfo
    _write_(Standard_DC_Chroma_NRCodes, sizeof(Standard_DC_Chroma_NRCodes), fp);
    _write_(Standard_DC_Chroma_Values, sizeof(Standard_DC_Chroma_Values), fp);
    _write_byte_(0x11, fp);            //HTCbACinfo
    _write_(Standard_AC_Chroma_NRCodes, sizeof(Standard_AC_Chroma_NRCodes), fp);
    _write_(Standard_AC_Chroma_Values, sizeof(Standard_AC_Chroma_Values), fp);

    //SOS
    _write_word_(0xFFDA, fp);        //marker = 0xFFC4
    _write_word_(12, fp);            //length = 12
    _write_byte_(3, fp);            //nrofcomponents, Should be 3: truecolor JPG

    _write_byte_(1, fp);            //Idy=1
    _write_byte_(0, fp);            //HTY    bits 0..3: AC table (0..3)
                                    //        bits 4..7: DC table (0..3)
    _write_byte_(2, fp);            //IdCb
    _write_byte_(0x11, fp);            //HTCb

    _write_byte_(3, fp);            //IdCr
    _write_byte_(0x11, fp);            //HTCr

    _write_byte_(0, fp);            //Ss not interesting, they should be 0,63,0
    _write_byte_(0x3F, fp);            //Se
    _write_byte_(0, fp);            //Bf
}

void _convertMBColorRGB2YUV(char *pRgb, int xPos, int yPos, int patch, char* yData, char* cbData, char* crData)
{
    for (int y=0; y<8; y++)
    {
        char* p = pRgb + (y+yPos)*patch*3 + xPos*3;
        for (int x=0; x<8; x++)
        {
            unsigned char B = *p++;
            unsigned char G = *p++;
            unsigned char R = *p++;

            yData[y*8+x] = (char)(0.299f * R + 0.587f * G + 0.114f * B - 128); //公式根据JPEG FILE Interchange Format标准
            cbData[y*8+x] = (char)(-0.1687f * R - 0.3313f * G + 0.5f * B );
            crData[y*8+x] = (char)(0.5f * R - 0.4187f * G - 0.0813f * B);
        }
    }
}

void _convertMBColorYUYV2YUV(char *pIn, int xPos, int yPos, int patch, char* yData, char* cbData, char* crData)
{
    for (int y=0; y<8; y++)
    {
        char* py = pIn + (y+yPos)*patch*2 + xPos*2;
        char* pu = pIn + (y+yPos)*patch*2 + (xPos/2)*4 + 1;
        char* pv = pIn + (y+yPos)*patch*2 + (xPos/2)*4 + 3;
        for (int x=0; x<8; x++)
        {
            yData[y*8+x] = *(py + 2*x)-128;        //这里比标准公式要多减128
            cbData[y*8+x] = *(pu + (x/2)*4)-128;   //这里比标准公式要多减128
            crData[y*8+x] = *(pv + (x/2)*4)-128;   //这里比标准公式要多减128
        }
    }
}

void _convertMBColorYUY444ToYUV(char *pIn, int xPos, int yPos, int patch, char* yData, char* cbData, char* crData)
{
    for (int y=0; y<8; y++)
    {
        char* py = pIn + (y+yPos)*patch + xPos;
        char* pu = pIn + (y+yPos)*patch + xPos + 640*480;
        char* pv = pIn + (y+yPos)*patch + xPos + 640*480*2;
        for (int x=0; x<8; x++)
        {
            yData[y*8+x] = *(py + x)-128;   //这里比标准公式要多减128
            cbData[y*8+x] = *(pu + x)-128;  //这里比标准公式要多减128
            crData[y*8+x] = *(pv + x)-128;  //这里比标准公式要多减128          
        }
    }
}

void _forword_FDC(const char* channel_data, short* fdc_data, unsigned char QTab[])
{
    const float PI = 3.1415926f;
    for(int v=0; v<8; v++)
    {
        for(int u=0; u<8; u++)
        {
            //使用二维DCT对8x8宏块进行变换，得到DC和AC系数
            float alpha_u = (u==0) ? 1/sqrt(8.0f) : 0.5f;
            float alpha_v = (v==0) ? 1/sqrt(8.0f) : 0.5f;

            float temp = 0.f;
            for(int x=0; x<8; x++)
            {
                for(int y=0; y<8; y++)
                {
                    float data = channel_data[y*8+x];

                    data *= cos((2*x+1)*u*PI/16.0f);
                    data *= cos((2*y+1)*v*PI/16.0f);

                    temp += data;
                }
            }

            //使用ZigZag方式存储的量化表对变换系数量化
            temp *= alpha_u*alpha_v/QTab[ZigZag[v*8+u]];

            //对量化后的值四舍五入处理，放入ZigZag排列的输出缓存区
            fdc_data[ZigZag[v*8+u]] = (short) ((short)(temp + 16384.5) - 16384);
        }
    }
}

void _doHuffmanEncoding(const short* DU, short *prevDC, const BitString* HTDC, const BitString* HTAC,
    BitString* outputBitString, int *bitStringCounts)
{
    BitString EOB = HTAC[0x00];
    BitString SIXTEEN_ZEROS = HTAC[0xF0];

    int index=0;

    // encode DC
    int dcDiff = (int)(DU[0] - *prevDC);
    *prevDC = DU[0];

    if (dcDiff == 0)
        outputBitString[index++] = HTDC[0];
    else
    {
        BitString bs = _getBitCode(dcDiff);

        outputBitString[index++] = HTDC[bs.length];
        outputBitString[index++] = bs;
    }

    // encode ACs
    int endPos=63; //end0pos = first element in reverse order != 0
    while((endPos > 0) && (DU[endPos] == 0)) endPos--;

    for(int i=1; i<=endPos; )
    {
        int startPos = i;
        while((DU[i] == 0) && (i <= endPos)) i++;

        int zeroCounts = i - startPos;
        if (zeroCounts >= 16)
        {
            for (int j=1; j<=zeroCounts/16; j++)
                outputBitString[index++] = SIXTEEN_ZEROS;
            zeroCounts = zeroCounts%16;
        }

        BitString bs = _getBitCode(DU[i]);

        outputBitString[index++] = HTAC[(zeroCounts << 4) | bs.length];
        outputBitString[index++] = bs;
        i++;
    }

    if (endPos != 63)
        outputBitString[index++] = EOB;

    *bitStringCounts = index;
}

int encode_rgb2jpg(char *pRgb, int width, int height, int Qp, char *pJpgFile)
{
    FILE *fp = fopen(pJpgFile, "w");
    short prev_DC_Y = 0, prev_DC_Cb = 0, prev_DC_Cr = 0;
    int newByte = 0, newBytePos = 7;
    BitString MbBitString[128];
    int bitStringCnt;
    char yData[64], cbData[64], crData[64];
    short yQuant[64], cbQuant[64], crQuant[64];

    picwidth  = width;
    picheight = height;
    //根据质量因子调整量化表
    _init_QualityTables(Qp);

    //初始化huffman熵编码表
    _initHuffmanTables();

    /* 1、写入JPG文件各个字段信息 */
    _write_jpeg_header(fp);

    /* 2、图片数据压缩 */
    for(int y = 0; y < height; y += 8)
    {//按照8x8宏块进行编码
        for(int x = 0; x < width; x += 8)
        {
            //当前宏块色彩RGB转为YCbCr，JPG只对YUV进行编码
            _convertMBColorRGB2YUV(pRgb, x, y, width, yData, cbData, crData);

            //Y通道数据进行DCT变换 + 量化 + 哈夫曼编码 + 写入文件
            _forword_FDC(yData, yQuant, MB_YTab);
            _doHuffmanEncoding(yQuant, &prev_DC_Y, Y_DC_Huffman_Tab, Y_AC_Huffman_Tab, MbBitString, &bitStringCnt);
            _write_bitstring_(MbBitString, bitStringCnt, &newByte, &newBytePos, fp);

            //Cb通道数据进行DCT变换 + 量化 + 哈夫曼编码 + 写入文件
            _forword_FDC(cbData, cbQuant, MB_CbCrTab);
            _doHuffmanEncoding(cbQuant, &prev_DC_Cb, CbCr_DC_Huffman_Tab, CbCr_AC_Huffman_Tab, MbBitString, &bitStringCnt);
            _write_bitstring_(MbBitString, bitStringCnt, &newByte, &newBytePos, fp);

            //Cr通道数据进行DCT变换 + 量化 + 哈夫曼编码 + 写入文件
            _forword_FDC(crData, crQuant, MB_CbCrTab);
            _doHuffmanEncoding(crQuant, &prev_DC_Cr, CbCr_DC_Huffman_Tab, CbCr_AC_Huffman_Tab, MbBitString, &bitStringCnt);
            _write_bitstring_(MbBitString, bitStringCnt, &newByte, &newBytePos, fp);
        }
    }

    /* 3、写入JPG文件尾 */
    _write_word_(0xFFD9, fp);

    fclose(fp);

    return 0;
}

int encode_yuyvjpg(char *pYuyv, int width, int height, int Qp, char *pJpgFile)
{
    FILE *fp = fopen(pJpgFile, "w");
    short prev_DC_Y = 0, prev_DC_Cb = 0, prev_DC_Cr = 0;
    int newByte = 0, newBytePos = 7;
    BitString MbBitString[128];
    int bitStringCnt;
    char yData[64], cbData[64], crData[64];
    short yQuant[64], cbQuant[64], crQuant[64];

    picwidth  = width;
    picheight = height;
    //根据质量因子调整量化表
    _init_QualityTables(Qp);

    //初始化huffman熵编码表
    _initHuffmanTables();

    /* 1、写入JPG文件各个字段信息 */
    _write_jpeg_header(fp);

    /* 2、图片数据压缩 */
    for(int y = 0; y < height; y += 8)
    {//按照8x8宏块进行编码
        for(int x = 0; x < width; x += 8)
        {
            //当前宏块色彩RGB转为YCbCr，JPG只对YUV进行编码
            _convertMBColorYUYV2YUV(pYuyv, x, y, width, yData, cbData, crData);

            //Y通道数据进行DCT变换 + 量化 + 哈夫曼编码 + 写入文件
            _forword_FDC(yData, yQuant, MB_YTab);
            _doHuffmanEncoding(yQuant, &prev_DC_Y, Y_DC_Huffman_Tab, Y_AC_Huffman_Tab, MbBitString, &bitStringCnt);
            _write_bitstring_(MbBitString, bitStringCnt, &newByte, &newBytePos, fp);

            //Cb通道数据进行DCT变换 + 量化 + 哈夫曼编码 + 写入文件
            _forword_FDC(cbData, cbQuant, MB_CbCrTab);
            _doHuffmanEncoding(cbQuant, &prev_DC_Cb, CbCr_DC_Huffman_Tab, CbCr_AC_Huffman_Tab, MbBitString, &bitStringCnt);
            _write_bitstring_(MbBitString, bitStringCnt, &newByte, &newBytePos, fp);

            //Cr通道数据进行DCT变换 + 量化 + 哈夫曼编码 + 写入文件
            _forword_FDC(crData, crQuant, MB_CbCrTab);
            _doHuffmanEncoding(crQuant, &prev_DC_Cr, CbCr_DC_Huffman_Tab, CbCr_AC_Huffman_Tab, MbBitString, &bitStringCnt);
            _write_bitstring_(MbBitString, bitStringCnt, &newByte, &newBytePos, fp);
        }
    }

    /* 3、写入JPG文件尾 */
    _write_word_(0xFFD9, fp);

    fclose(fp);

    printf("%s jpeg enc success!\n", pJpgFile);

    return 0;
}

int encode_yuv444Tojpg(char *pYuv, int width, int height, int Qp, char *pJpgFile)
{
    FILE *fp = fopen(pJpgFile, "w");
    short prev_DC_Y = 0, prev_DC_Cb = 0, prev_DC_Cr = 0;
    int newByte = 0, newBytePos = 7;
    BitString MbBitString[128];
    int bitStringCnt;
    char yData[64], cbData[64], crData[64];
    short yQuant[64], cbQuant[64], crQuant[64];

    picwidth  = width;
    picheight = height;
    //根据质量因子调整量化表
    _init_QualityTables(Qp);

    //初始化huffman熵编码表
    _initHuffmanTables();

    /* 1、写入JPG文件各个字段信息 */
    _write_jpeg_header(fp);

    /* 2、图片数据压缩 */
    for(int y = 0; y < height; y += 8)
    {//按照8x8宏块进行编码
        for(int x = 0; x < width; x += 8)
        {
            //当前宏块色彩RGB转为YCbCr，JPG只对YUV进行编码
            _convertMBColorYUY444ToYUV(pYuv, x, y, width, yData, cbData, crData);

            //Y通道数据进行DCT变换 + 量化 + 哈夫曼编码 + 写入文件
            _forword_FDC(yData, yQuant, MB_YTab);
            _doHuffmanEncoding(yQuant, &prev_DC_Y, Y_DC_Huffman_Tab, Y_AC_Huffman_Tab, MbBitString, &bitStringCnt);
            _write_bitstring_(MbBitString, bitStringCnt, &newByte, &newBytePos, fp);

            //Cb通道数据进行DCT变换 + 量化 + 哈夫曼编码 + 写入文件
            _forword_FDC(cbData, cbQuant, MB_CbCrTab);
            _doHuffmanEncoding(cbQuant, &prev_DC_Cb, CbCr_DC_Huffman_Tab, CbCr_AC_Huffman_Tab, MbBitString, &bitStringCnt);
            _write_bitstring_(MbBitString, bitStringCnt, &newByte, &newBytePos, fp);

            //Cr通道数据进行DCT变换 + 量化 + 哈夫曼编码 + 写入文件
            _forword_FDC(crData, crQuant, MB_CbCrTab);
            _doHuffmanEncoding(crQuant, &prev_DC_Cr, CbCr_DC_Huffman_Tab, CbCr_AC_Huffman_Tab, MbBitString, &bitStringCnt);
            _write_bitstring_(MbBitString, bitStringCnt, &newByte, &newBytePos, fp);
        }
    }

    /* 3、写入JPG文件尾 */
    _write_word_(0xFFD9, fp);

    fclose(fp);

    printf("%s jpeg enc success!\n", pJpgFile);

    return 0;
}