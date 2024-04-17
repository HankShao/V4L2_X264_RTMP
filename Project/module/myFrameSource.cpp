#include <liveMedia.hh>

#include <BasicUsageEnvironment.hh>
//#include "announceURL.hh"
#include <GroupsockHelper.hh>
#include <myFrameSource.hpp>

MyFramedSource* MyFramedSource::createNew(UsageEnvironment& env,
                                              unsigned preferredFrameSize, 
                                              unsigned playTimePerFrame) 
{
        return new MyFramedSource(env, preferredFrameSize, playTimePerFrame);
}

MyFramedSource::MyFramedSource(UsageEnvironment& env,
                                   unsigned preferredFrameSize, 
                                   unsigned playTimePerFrame)
    : FramedSource(env),
    fPreferredFrameSize(fMaxSize),
    fPlayTimePerFrame(playTimePerFrame),
    fLastPlayTime(0),
    fCurIndex(0)
{
        if (referenceCount == 0) 
        {

        }
        ++referenceCount;

        if (eventTriggerId == 0) 
        {
            eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
        }
}

MyFramedSource::~MyFramedSource() 
{
    --referenceCount;
    if (referenceCount == 0) 
    {
        // Reclaim our 'event trigger'
        envir().taskScheduler().deleteEventTrigger(eventTriggerId);
        eventTriggerId = 0;
    }
}

void MyFramedSource::deliverFrame0(void* clientData)
{
    ((MyFramedSource*)clientData)->deliverFrame();
}

void MyFramedSource::doGetNextFrame()
{
    deliverFrame();
}

void MyFramedSource::deliverFrame() 
{
    if (fPlayTimePerFrame > 0 && fPreferredFrameSize > 0) {
        if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) {
            // This is the first frame, so use the current time:
            gettimeofday(&fPresentationTime, NULL);
        } else {
            // Increment by the play time of the previous data:
            unsigned uSeconds   = fPresentationTime.tv_usec + fLastPlayTime;
            fPresentationTime.tv_sec += uSeconds/1000000;
            fPresentationTime.tv_usec = uSeconds%1000000;
        }

        // Remember the play time of this data:
        fLastPlayTime = (fPlayTimePerFrame*fFrameSize)/fPreferredFrameSize;
        fDurationInMicroseconds = fLastPlayTime;
    } else {
        // We don't know a specific play time duration for this data,
        // so just record the current time as being the 'presentation time':
        gettimeofday(&fPresentationTime, NULL);
    }

    if(!m_queue.empty())
    {
        //m_queue.pop(nalToDeliver);

        //memcpy(fTo, nalToDeliver.p_payload, nalToDeliver.i_payload);

        FramedSource::afterGetting(this);
    }
}