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

int Enc_yuvToh264(char *pdata, int width, int height, int pts, int eof, enc_h264_out *out)
{
    x264_param_t param;
    static x264_picture_t pic;
    x264_picture_t pic_out;
    static x264_t *handle;

    out->framenum = 0;
    if (!handle)
    {
        /* 获取默认参数 */
        if( x264_param_default_preset( &param, "medium", NULL ) < 0 )
            goto fail;

        /* 配置非默认的参数 */
        param.i_bitdepth = 8;
        param.i_csp = X264_CSP_NV12;
        param.i_width  = width;
        param.i_height = height;
        param.b_vfr_input = 0;
        param.b_repeat_headers = 1;
        param.b_annexb = 1;   

        /* 设置profile限制 */
        if( x264_param_apply_profile( &param, "high" ) < 0 )
            goto fail;

        if( x264_picture_alloc( &pic, param.i_csp, param.i_width, param.i_height ) < 0 )
            goto fail;

    #undef fail
    #define fail fail2
        /* 申请句柄 */
        handle = x264_encoder_open( &param );
        if (!handle)
            goto fail;
    }
#undef fail
#define fail fail3
    int i_frame_size;
    x264_nal_t *nal;
    int i_nal;

    YUYV2NV12(pdata, width, height, (char *)pic.img.plane[0], (char *)pic.img.plane[1]);

    pic.i_pts = pts;
    i_frame_size = x264_encoder_encode(handle, &nal, &i_nal, &pic, &pic_out );
    if( i_frame_size < 0 )
        goto fail;
    else if( i_frame_size )
    {
        out->packetdata[out->framenum] = (char *)nal->p_payload;
        out->packetlen[out->framenum]  = i_frame_size;
        out->framenum++;
    }

    if(eof)
    {
        /* Flush delayed frames */
        while( x264_encoder_delayed_frames( handle ) )
        {
            i_frame_size = x264_encoder_encode( handle, &nal, &i_nal, NULL, &pic_out );
            if( i_frame_size < 0 )
                goto fail;
            else if( i_frame_size )
            {
                out->packetdata[out->framenum] = (char *)nal->p_payload;
                out->packetlen[out->framenum]  = i_frame_size;
                out->framenum++;
            }
        }

        x264_encoder_close( handle );
        x264_picture_clean( &pic );
        handle = NULL;
    }

    return 0;

#undef fail
fail3:
    x264_encoder_close( handle );

fail2:
     x264_picture_clean( &pic );    
fail:
    return -1;
    
}
int Enc_yuyv2jpg(char *pYuyv, int width, int height, int Qp, char *pJpgFile)
{
    return encode_yuyvjpg(pYuyv, width, height, Qp, pJpgFile);
}

#ifdef __cplusplus
}
#endif