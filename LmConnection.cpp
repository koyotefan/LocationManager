
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>

#include "NDFServiceLog.hpp"

#include "LmMessage.hpp"
#include "LmConnection.hpp"


LmConnection::LmConnection(int _sock, char * _clientIp)
    : sock_(_sock),
      clientIp_(_clientIp),
      tLastRecvTime_(time(nullptr)) {

    struct timeval to;
    to.tv_sec  = 3;
    to.tv_usec = 0;

    setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));
    setsockopt(sock_, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof(to));
}

LmConnection::~LmConnection() {
    Disconnect();
}

void LmConnection::Disconnect() {
    if(sock_ >= 0) {
        close(sock_);
        sock_ = -1;
    }
}


int LmConnection::Recv(char * _ptr, size_t _size, int _milliSeconds) {
    if(sock_ < 0)
        return -1;

    struct pollfd fds;
    fds.fd      = sock_;
    fds.events  = POLLIN;

    int ret = poll(&fds, 1, _milliSeconds);
    if(ret <= 0) {
        if(ret < 0) {
            char buf[128];

            E_LOG("Recv poll() fail [%d] [%d:%s] [client:%s]",
                ret, errno, strerror_r(errno, buf, sizeof(buf)), clientIp_.c_str());
        }
        return ret;
    }

    bool    bRetry = true;
    size_t  readt = 0;
    while(_size > readt) {
        ssize_t readn = recv(sock_, _ptr + readt, _size - readt, 0);

        if(readn <= 0) {
            char buf[128];

            E_LOG("Recv recv() fail [%zu] [%zd] [%d:%s] [client:%s]",
                readt, readn, errno, strerror_r(errno, buf, sizeof(buf)), clientIp_.c_str());

            if(readn < 0 && errno == 11 && bRetry) {
                bRetry = false;
                poll(nullptr, 0, 5);
                continue;
            }

            return -1;
        }

        readt += readn;
    }

    tLastRecvTime_ = time(nullptr);
    return (int)readt;
}

bool LmConnection::Send(char * _ptr, size_t _size) {

    if(sock_ < 0)
        return false;

    bool    bRetry = true;
    size_t sendt = 0;

    while(_size > sendt) {
        ssize_t sendn = send(sock_, _ptr + sendt, _size - sendt, 0);

        if(sendn <= 0) {
            char buf[128];

            E_LOG("Send send() fail [%zd] [%d:%s] [%s]",
                sendn, errno, strerror_r(errno, buf, sizeof(buf)), clientIp_.c_str());

            if(sendn < 0 && errno == 11 && bRetry) {
                bRetry = false;
                poll(nullptr, 0, 5);
                continue;
            }

            return false;
        }

        sendt += sendn;
    }

    return true;
}


void LmConnection::RecvPing() {

    stHeader     h;

    int nRet = 0;
    if((nRet = Recv((char *)&h, sizeof(h))) < 0) {
        E_LOG("RecvPing fail");
        Disconnect();

        return ;
    }

    if(nRet == 0)
        return ;

    // D_LOG("RecvPing...... Recv [%d] [%zu]", h.nType, tLastRecvTime_);

    if(h.nType != 3) {
        E_LOG("RecvPing fail [unexpected type:%d] [client:%s]",
            h.nType, clientIp_.c_str());
        Disconnect();

        return ;
    }


}

bool LmConnection::IsNoPingEvent(int _nTimeout) {
    return (tLastRecvTime_ + _nTimeout < time(nullptr));
}
