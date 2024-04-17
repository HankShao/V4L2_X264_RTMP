#include <deque>

class MyFramedSource:public FramedSource
{
public:
    MyFramedSource();
    MyFramedSource(UsageEnvironment& env,
                                   unsigned preferredFrameSize, 
                                   unsigned playTimePerFrame);

    ~MyFramedSource();
    MyFramedSource* createNew(UsageEnvironment& env, unsigned preferredFrameSize, unsigned playTimePerFrame);
    void doGetNextFrame();
    void deliverFrame0(void* clientData);
private:
    void deliverFrame();
    std::deque<int> m_queue;
    EventTriggerId eventTriggerId;
    unsigned FrameSize;
    unsigned referenceCount;
    unsigned fPreferredFrameSize;
    unsigned fPlayTimePerFrame;
    unsigned fLastPlayTime;
    unsigned fCurIndex;
}