#pragma once
class IMvds {
    public:
        virtual int SendRTPPacket(int channel, const void *data, size_t len) = 0;
        virtual int SendRTCPPacket(int channel, const void *data, size_t len) = 0;
};