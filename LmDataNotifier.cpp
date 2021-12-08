
#include <string>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <poll.h>

#include "NDFServiceLog.hpp"
#include "LmDataNotifier.hpp"

LmDataNotifier::LmDataNotifier()
    :bRunf_(true),
     pRefCounter_(std::make_shared<bool>()) {

    nListenPort_ = 10100;
    bIpv6_ = false;

    nPingTimeout_ = 90;
    nMaxReadQCnt_   = 10;
    nMaxReadAcceptEventCnt_ = 100;
    nNotifierSleepMilliSec_ = 50;

    className_ = typeid(*this).name();
}

LmDataNotifier::~LmDataNotifier() {

    bRunf_.store(false);

    if(thr_.joinable())
        thr_.join();
}

bool LmDataNotifier::Init(std::shared_ptr<PDB::ConnectionManager>  _mgr,
                        std::shared_ptr<LmData>     _lmData,
                        CNDFIniConfigContainer  * _cfg) {
    lmData_ = _lmData;

    if(readConfig(_cfg) == false) {
        E_LOG("readConfig() fail");
        return false;
    }

    pdb_.Assign(_mgr);

    if(pdb_.TurnOn(PDB::eDefDBType::PCF) == false) {
        E_LOG("PDB TurnOn() fail");
        return false;
    }

    if(server_.Init(nListenPort_, bIpv6_) == false) {
        E_LOG("Server Init() fail");
        return false;
    }

    I_LOG("LmDataNotifier Init() success");

    return true;
}

bool LmDataNotifier::readConfig(CNDFIniConfigContainer  * _cfg) {

    char    buf[256];
    if (_cfg->Get((char *)"PCF_LM",(char *)"NOTIFIER_SLEEP_MSEC",buf) == false )
    {
        W_LOG("Config Read fail [PCF_LM:NOTIFIER_SLEEP_MSEC]");
    } else {
        nNotifierSleepMilliSec_ = std::atoi(buf);
    }
    I_LOG("Config [PCF_LM:NOTIFIER_SLEEP_MSEC] = [%d]", nNotifierSleepMilliSec_);

    if (_cfg->Get((char *)"PCF_LM",(char *)"LISTEN_PORT",buf) == false )
    {
        W_LOG("Config Read fail [PCF_LM:LISTEN_PORT]");
    } else {
        nListenPort_ = std::atoi(buf);
    }
    I_LOG("Config [PCF_LM:LISTEN_PORT] = [%d]", nListenPort_);

    if (_cfg->Get((char *)"PCF_LM",(char *)"PING_TIMEOUT",buf) == false )
    {
        W_LOG("Config Read fail [PCF_LM:PING_TIMEOUT]");
    } else {
        nPingTimeout_ = std::atoi(buf);
    }
    I_LOG("Config [PCF_LM:PING_TIMEOUT] = [%d]", nPingTimeout_);

    if (_cfg->Get((char *)"PCF_LM",(char *)"USED_IP_VERSION6",buf) == false )
    {
        W_LOG("Config Read fail [PCF_LM:USED_IP_VERSION6]");
    } else {
        bIpv6_ = (std::atoi(buf) == 1)?true:false;
    }
    I_LOG("Config [PCF_LM:USED_IP_VERSION6] = [%d]", bIpv6_);

    if (_cfg->Get((char *)"PCF_LM",(char *)"MAX_READ_Q_CNT",buf) == false )
    {
        W_LOG("Config Read fail [PCF_LM:MAX_READ_Q_CNT]");
    } else {
        nMaxReadQCnt_ = std::atoi(buf);
    }
    I_LOG("Config [PCF_LM:MAX_READ_Q_CNT] = [%d]", nMaxReadQCnt_);

    if (_cfg->Get((char *)"PCF_LM",(char *)"MAX_READ_ACCEPT_EVENT_CNT",buf) == false )
    {
        W_LOG("Config Read fail [PCF_LM:MAX_READ_ACCEPT_EVENT_CNT]");
    } else {
        nMaxReadAcceptEventCnt_ = std::atoi(buf);
    }
    I_LOG("Config [PCF_LM:MAX_READ_ACCEPT_EVENT_CNT] = [%d]", nMaxReadAcceptEventCnt_);

    return true;
}

void LmDataNotifier::Run() {
    thr_ = std::thread(&LmDataNotifier::run, this, pRefCounter_);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void LmDataNotifier::run(std::shared_ptr<bool>   _pRefCounter) {

    (void)_pRefCounter;

    I_LOG("LmDataNotifier in service [%s]", className_.c_str());
    pThreadMonitor_->Register(className_.c_str());

    while(bRunf_) {

        pThreadMonitor_->Refresh(className_.c_str());

        for(int nLoop=0; nMaxReadQCnt_ > nLoop; ++nLoop) {

            if(lmData_->GetQSize() <= 0)
                break;

            auto sp = lmData_->PopQ();

            size_t clientCnt = server_.Notify(sp);

            // 보낸 Client 가 있을 때에만 찍어요.
            if(clientCnt > 0) {

                for(auto & citer : sp->vecAdditions)
                    D_LOG("zcode:[%s] add location:[%s]",
                        sp->zoneCode.c_str(), citer.c_str());

                for(auto & citer : sp->vecDeletions)
                    D_LOG("zcode:[%s] del location:[%s]",
                        sp->zoneCode.c_str(), citer.c_str());
            }

            server_.MakePushMessage(lmData_.get());
        }

        for(int nLoop=0; nMaxReadAcceptEventCnt_ > nLoop; ++nLoop) {
            char    clientIp[64];
            memset(clientIp, 0, sizeof(clientIp));

            int sock = server_.DoAccept(clientIp);

            if(sock <= 0)
                break;

            auto lmConnection = std::make_shared<LmConnection>(sock, clientIp);

            if(server_.ConnectProcess(lmConnection.get()) == false)
                continue;

            if(server_.Push(lmConnection.get()) == true)
                server_.AddConnection(lmConnection);
        }

        server_.PingEvent();

        server_.GarbageConnection(nPingTimeout_);

        poll(nullptr, 0, nNotifierSleepMilliSec_);
    }

    W_LOG("LmDataNotifier out of service");
}

