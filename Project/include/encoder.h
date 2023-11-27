#ifndef _ENCODER_H_
#define _ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int framenum;
    char *packetdata[8];
    int packetlen[8];
}enc_h264_out;

/*
* 将YUYV格式编码成H264码流
*/
int Enc_yuvToh264(char *pdata, int width, int height, int pts, int eof, enc_h264_out *out);

/*
* 将YUYV格式编码成JPEG文件保存
*/
int Enc_yuyv2jpg(char *pYuyv, int width, int height, int Qp, char *pJpgFile);

#ifdef __cplusplus
}
#endif
#endif