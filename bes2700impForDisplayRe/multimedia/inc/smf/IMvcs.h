#pragma once
class IMvcs {
    public:
        enum {
            SMF_VOIP_ERR_RTP_TIMEOUT,
            SMF_VOIP_ERR_RTCP_TIMEOUT
        };
    public:
        virtual int SendPacket(int channel, const void *data, size_t len) = 0;
        virtual int SendRTCPPacket(int iStrmid, const void *data, size_t len) = 0;
        virtual void CallbackOnError(int channel, int errCode) = 0;
};