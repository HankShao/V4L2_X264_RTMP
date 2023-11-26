#ifndef _JPEG_ENC_H_
#define _JPEG_ENC_H_

#ifdef cplusplus
extern "c"{
#endif

int encode_yuyvjpg(char *pYuyv, int width, int height, int Qp, char *pJpgFile);
int encode_yuv444Tojpg(char *pYuv, int width, int height, int Qp, char *pJpgFile);
int encode_rgb2jpg(char *pRgb, int width, int height, int Qp, char *pJpgFile);

#ifdef cplusplus
}
#endif
#endif