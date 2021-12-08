
#include "NDFServiceLog.hpp"

#include "LmHistory.hpp"
#include "LmServer.hpp"

extern LmHistory    gHist;

LmServer::LmServer()
    : server_(nullptr) {

}

LmServer::~LmServer() {

    if(server_) {
        delete server_;
        server_ = nullptr;
    }
}

bool LmServer::Init(int _nListenPort, bool _bIpv6) {

    server_ = new (std::nothrow)CNDFTCPServer();

    if(server_ == nullptr) {
        E_LOG("LmServer can't create instance");
        return false;
    }

    if(server_->Create(_nListenPort, (_bIpv6)?AF_INET6:AF_INET) == false) {
        E_LOG("LmServer Create() fail [%d:%d]", _nListenPort, _bIpv6);
        return false;
    }

    return true;
}

int LmServer::DoAccept(char * _clientIp) {

    int sock = server_->Wait(_clientIp, false);

    if(sock < 0)
        E_LOG("LmServer DoAccept fail [%d]", sock);

    if(sock > 0)
        I_LOG("LmServer DoAccept [%d] [%s]", sock, _clientIp);

    return sock;
}

bool LmServer::ConnectProcess(LmConnection * _conn) {

    stHeader    h;
    if(_conn->Recv((char *)&h, sizeof(h), 100) <= 0) {
        E_LOG("ConnectProcess Recv() fail [%s]", _conn->GetIp());
        return false;
    }

    if(h.nType != 1) {
        E_LOG("ConnectProcess Recv() unexpected type [%d] [%s]",
            h.nType, _conn->GetIp());
       return false;
    }

    _conn->SetSysName(h.sysName);
    _conn->SetProcName(h.procName);

    LmConnectMessage    msg;
    if(_conn->Send(msg.GetData(), msg.GetDataLength()) == false) {
        E_LOG("ConnectProcess Send() fail [%s]", _conn->GetIp());
        return false;
    }

    I_LOG("ConnectProcess OK [t:%d len:%d sys:%s proc:%s]",
        h.nType, h.nBodyLength, h.sysName, h.procName);

    return true;
}

void LmServer::AddConnection(std::shared_ptr<LmConnection>  _conn) {

    int sock = _conn->GetSock();

    // 설마 이런일은 없겠죠..
    auto iter = mapConnections_.find(sock);
    if(iter != mapConnections_.end()) {
        LmConnection * p = iter->second.get();
        W_LOG("new socket is already exist [%d] [%d]", sock, p->GetSock());
    }

    mapConnections_[sock] = _conn;
}

void LmServer::PingEvent() {

    for(auto & citer : mapConnections_)
        citer.second->RecvPing();
}

void LmServer::GarbageConnection(int _nTimeout) {

    for(auto iter = mapConnections_.begin(); iter != mapConnections_.end(); ) {

        LmConnection * conn = (iter->second).get();

        if(conn->IsConnected() == false) {
            D_LOG("GarbageConnection [%d:%s] is disconnected",
                conn->GetSock(), conn->GetIp());
            iter = mapConnections_.erase(iter);
            continue;
        }

        if(conn->IsNoPingEvent(_nTimeout) == true) {
            I_LOG("GarbageConnection [%d:%s] is no ping until timeout [%d]",
                conn->GetSock(), conn->GetIp(), _nTimeout);
            iter = mapConnections_.erase(iter);
            continue;
        }

        ++iter;
    }
}

size_t LmServer::Notify(std::shared_ptr<stDeltaLocationIds> _sp) {

    size_t  connCnt = 0;

    LmDeltaMessage   msg;
    msg.MakeData(_sp.get());

    for(auto & citer : mapConnections_) {
        LmConnection * conn = citer.second.get();
        if(conn->Send(msg.GetData(), msg.GetDataLength()) == false) {
            W_LOG("connection [%d] Send() fail [zone Code:%s] [Cnt:%zu]",
                conn->GetSock(), _sp->zoneCode.c_str(), msg.GetLocationCnt());
            conn->Disconnect();

            gHist.Send(conn->GetSysName(),
                         conn->GetProcName(),
                         false,
                         _sp.get());
            continue;
        }

        I_LOG("Notify success [%d:%s] [zone Code:%s] [Cnt:%zu]",
            citer.second->GetSock(), citer.second->GetIp(),
            _sp->zoneCode.c_str(), msg.GetLocationCnt());

        gHist.Send(conn->GetSysName(),
                     conn->GetProcName(),
                     true,
                     _sp.get());
        connCnt++;
    }

    return connCnt;
}

void LmServer::MakePushMessage(LmData * _lmData) {

    totalMessage_.Clear();

    std::vector<std::string>    vecZoneCodes;
    _lmData->GetZoneCodes(vecZoneCodes);

    stLocationIds   locationIds;

    for(auto & citer : vecZoneCodes) {
        size_t cnt = _lmData->GetLocationIds(citer, locationIds);

        if(cnt > 0)
            totalMessage_.Append(locationIds);
    }

    totalMessage_.MakeData();
}

bool LmServer::Push(LmConnection * _conn) {

    if(_conn->Send(totalMessage_.GetData(), totalMessage_.GetDataLength()) == false) {
        W_LOG("conn [%d] Send() fail [total Cnt:%zu]",
            _conn->GetSock(), totalMessage_.GetLocationCnt());

	    gHist.Send(_conn->GetSysName(),
				     _conn->GetProcName(),
				     false,
                     totalMessage_);

        return false;
    }

    I_LOG("conn [%d:%s] Total data Send success [total Cnt:%zu] [size:%zu]",
            _conn->GetSock(), _conn->GetIp(), totalMessage_.GetLocationCnt(), totalMessage_.GetDataLength());

    gHist.Send(_conn->GetSysName(),
			     _conn->GetProcName(),
			     true,
			     totalMessage_);

    return true;
}

void LmServer::delConnection(int _sock) {
    auto iter = mapConnections_.find(_sock);

    if(iter == mapConnections_.end())
        return ;

    mapConnections_.erase(iter);
}
