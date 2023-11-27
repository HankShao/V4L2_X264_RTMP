#ifndef _JPEG_ENC_H_
#define _JPEG_ENC_H_

#ifdef __cplusplus
extern "C" {
#endif

int encode_yuyvjpg(char *pYuyv, int width, int height, int Qp, char *pJpgFile);
int encode_yuv444Tojpg(char *pYuv, int width, int height, int Qp, char *pJpgFile);
int encode_rgb2jpg(char *pRgb, int width, int height, int Qp, char *pJpgFile);

#ifdef __cplusplus
}
#endif
#endif