#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "x264/x264.h"
#include "jpegenc.h"
#include "encoder.h"

#ifdef __cplusplus
extern "C" {
#endif

int YUYV2NV12(char *pin, int w, int h, char *yData, char *uvData)
{
    for (int y=0; y<h; y++)
    {
        char* py = pin + (y)*w*2;
        char* pu = pin + (y)*w*2 + 1;
        char* pv = pin + (y)*w*2 + 3;
        for (int x=0; x<w; x++)
        {
            yData[y*w+x] = *(py + 2*x);
            if (y%2==0 && x%2==0)
            {
                uvData[y*w/2+x] = *(pu + (x/2)*4); 
                uvData[y*w/2+x+1] = *(pv + (x/2)*4); 
            }
        }
    }

    return 0;
}


//------------------X264编码定义区-------------------------
static x264_t *handle;
static x264_picture_t pic;
void *Enc_OpenX264(int width, int height, int fps)
{
    x264_param_t param;

    /* 获取默认参数 */
    if( x264_param_default_preset( &param, "medium", NULL ) < 0 )
        return NULL;

    /* 配置非默认的参数 */
    //param.i_log_level = X264_LOG_DEBUG;
    param.i_bitdepth = 8;
    param.i_csp = X264_CSP_NV12;
    param.i_width  = width;
    param.i_height = height;
    param.b_vfr_input = 0;
    param.b_repeat_headers = 1;  //每个I帧带SPS/PPS
    param.b_annexb = 1;
    param.i_bframe = 0;      //不编码B帧
    param.i_keyint_min = fps; //设置I帧最小间隔25，避免I帧太多
    param.i_keyint_max = fps*2;
    param.i_fps_num    = fps;
    param.i_fps_den    = 1;
    param.i_timebase_num = param.i_fps_num;
    param.i_timebase_den = param.i_fps_den;

    /* 设置profile限制 */
    if( x264_param_apply_profile( &param, "high" ) < 0 )
        return NULL;

    if( x264_picture_alloc( &pic, param.i_csp, param.i_width, param.i_height ) < 0 )
        return NULL;

    /* 申请句柄 */
    handle = x264_encoder_open(&param);

    return handle;
}

int Enc_CloseX264(void *xhandle)
{
    x264_encoder_close( xhandle );
    x264_picture_clean( &pic ); 
    return 0;
}

int Enc_yuvToh264(char *pdata, int width, int height, int pts, int eof, enc_h264_out *out)
{
    x264_picture_t pic_out;
    int i_frame_size;
    x264_nal_t *nal;
    int i_nal;

    x264_picture_init(&pic_out);
    out->framenum = 0;
    if (!handle)
       return -1;

    YUYV2NV12(pdata, width, height, (char *)pic.img.plane[0], (char *)pic.img.plane[1]);
    pic.i_pts++;

    i_frame_size = x264_encoder_encode(handle, &nal, &i_nal, &pic, &pic_out );
    if( i_frame_size < 0 )
        return -1;
    else if( i_frame_size )
    {
        out->packetdata[out->framenum] = (char *)nal->p_payload;
        out->packetlen[out->framenum]  = i_frame_size;
        out->key = pic_out.b_keyframe;
        out->framenum++;
    }

    if(eof)
    {
        /* Flush delayed frames */
        while( x264_encoder_delayed_frames( handle ) )
        {
            i_frame_size = x264_encoder_encode( handle, &nal, &i_nal, NULL, &pic_out);
            if( i_frame_size < 0 )
               return -1;
            else if( i_frame_size )
            {
                out->packetdata[out->framenum] = (char *)nal->p_payload;
                out->packetlen[out->framenum]  = i_frame_size;
                out->key = pic_out.b_keyframe;
                out->framenum++;
            }
        }
    }

    return 0;    
}
int Enc_yuyv2jpg(char *pYuyv, int width, int height, int Qp, char *pJpgFile)
{
    return encode_yuyvjpg(pYuyv, width, height, Qp, pJpgFile);
}

#ifdef __cplusplus
}
#endif

