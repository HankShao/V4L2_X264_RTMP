#include <liveMedia.hh>

#include <BasicUsageEnvironment.hh>
//#include "announceURL.hh"
#include <GroupsockHelper.hh>
#include "rtsp.hpp"

using namespace std;

UsageEnvironment* env;
char const* inputFileName = "test.264";
H264VideoStreamFramer* videoSource;
RTPSink* videoSink;

void play(); // forward

void announceURL(RTSPServer* rtspServer, ServerMediaSession* sms) {
  if (rtspServer == NULL || sms == NULL) return; // sanity check

  UsageEnvironment& env = rtspServer->envir();

  env << "Play this stream using the URL ";
  if (weHaveAnIPv4Address(env)) {
    char* url = rtspServer->ipv4rtspURL(sms);
    env << "\"" << url << "\"";
    delete[] url;
    if (weHaveAnIPv6Address(env)) env << " or ";
  }
  if (weHaveAnIPv6Address(env)) {
    char* url = rtspServer->ipv6rtspURL(sms);
    env << "\"" << url << "\"";
    delete[] url;
  }
  env << "\n";
}

int rtsp_task(void *param) 
{
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  // Create 'groupsocks' for RTP and RTCP:
  struct sockaddr_storage destinationAddress;
  destinationAddress.ss_family = AF_INET;
  ((struct sockaddr_in&)destinationAddress).sin_addr.s_addr = chooseRandomIPv4SSMAddress(*env);
  // Note: This is a multicast address.  If you wish instead to stream
  // using unicast, then you should use the "testOnDemandRTSPServer"
  // test program - not this test program - as a model.

  const unsigned short rtpPortNum = 18888;
  const unsigned short rtcpPortNum = rtpPortNum+1;
  const unsigned char ttl = 255;

  const Port rtpPort(rtpPortNum);
  const Port rtcpPort(rtcpPortNum);

  Groupsock rtpGroupsock(*env, destinationAddress, rtpPort, ttl);
  rtpGroupsock.multicastSendOnly(); // we're a SSM source
  Groupsock rtcpGroupsock(*env, destinationAddress, rtcpPort, ttl);
  rtcpGroupsock.multicastSendOnly(); // we're a SSM source

  // Create a 'H264 Video RTP' sink from the RTP 'groupsock':
  OutPacketBuffer::maxSize = 100000;
  videoSink = H264VideoRTPSink::createNew(*env, &rtpGroupsock, 96);

  // Create (and start) a 'RTCP instance' for this RTP sink:
  const unsigned estimatedSessionBandwidth = 500; // in kbps; for RTCP b/w share
  const unsigned maxCNAMElen = 100;
  unsigned char CNAME[maxCNAMElen+1];
  gethostname((char*)CNAME, maxCNAMElen);
  CNAME[maxCNAMElen] = '\0'; // just in case
  RTCPInstance* rtcp
  = RTCPInstance::createNew(*env, &rtcpGroupsock,
			    estimatedSessionBandwidth, CNAME,
			    videoSink, NULL /* we're a server */,
			    True /* we're a SSM source */);
  // Note: This starts RTCP running automatically

  RTSPServer* rtspServer = RTSPServer::createNew(*env, 8554);
  if (rtspServer == NULL) {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }
  ServerMediaSession* sms
    = ServerMediaSession::createNew(*env, "testStream", inputFileName,
		   "Session streamed by \"testH264VideoStreamer\"",
					   True /*SSM*/);
  sms->addSubsession(PassiveServerMediaSubsession::createNew(*videoSink, rtcp));
  rtspServer->addServerMediaSession(sms);
  announceURL(rtspServer, sms);

  // Start the streaming:
  *env << "Beginning streaming...\n";
  play();

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}

void afterPlaying(void* /*clientData*/) {
  *env << "...done reading from file\n";
  videoSink->stopPlaying();
  Medium::close(videoSource);
  // Note that this also closes the input file that this source read from.

  // Start playing once again:
  play();
}

void play() {
  // Open the input file as a 'byte-stream file source':
  ByteStreamFileSource* fileSource
    = ByteStreamFileSource::createNew(*env, inputFileName);
  if (fileSource == NULL) {
    *env << "Unable to open file \"" << inputFileName
         << "\" as a byte-stream file source\n";
    exit(1);
  }

  FramedSource* videoES = fileSource;

  // Create a framer for the Video Elementary Stream:
  videoSource = H264VideoStreamFramer::createNew(*env, videoES);

  // Finally, start playing:
  *env << "Beginning to read from file...\n";
  videoSink->startPlaying(*videoSource, afterPlaying, videoSink);
}

void *rtsp_task_run(void *param)
{
    rtsp_task(param);
    return NULL;
}

rtsp_server::rtsp_server():m_dqlimit(10)
{
  m_framedq.resize(m_dqlimit);
}

rtsp_server::~rtsp_server()
{
  m_framedq.clear();
}

int rtsp_server::rtsp_server_start(void *param)
{
    int *pexit = static_cast<int*>(param);
    while(*pexit == 0)
    {
        usleep(40000);
        if(m_framedq.empty())
            continue;
        shared_ptr<nalu_data> nalu = m_framedq.back();
        m_framedq.pop_back();
    }

    return 0;
}

int rtsp_server::rtsp_server_stop()
{
    
    return 0;
}
