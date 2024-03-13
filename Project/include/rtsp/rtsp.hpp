#ifndef _RTSP_H_
#define _RTSP_H_

#include <iostream>
#include <deque>
#include <memory>

typedef struct{
    char *pdata;
    uint32_t len;
    bool     key;
}nalu_data;

class rtsp_server{
    public:
        rtsp_server();
        ~rtsp_server();
        int rtsp_server_start();
        int rtsp_server_stop();
        int rtsp_put_frame();

        std::deque< std::shared_ptr<nalu_data> > m_framedq;
        uint32_t m_dqlimit;
    private:
        int m_start;
};

#endif
