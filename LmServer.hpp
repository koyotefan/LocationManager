#ifndef LM_SERVER_HPP
#define LM_SERVER_HPP

#include <memory>

#include "LmData.hpp"
#include "LmMessage.hpp"
#include "LmConnection.hpp"
#include "CNDFTCPServer.hpp"

class LmServer {
public:
    explicit LmServer();
    ~LmServer();

    bool Init(int _nListenPort, bool _bIpv6);
    size_t Notify(std::shared_ptr<stDeltaLocationIds> _sp);
    bool Push(LmConnection *  _conn);
    int DoAccept(char * _clientIp);

    bool ConnectProcess(LmConnection * _conn);
    void MakePushMessage(LmData * _lmData);

    void AddConnection(std::shared_ptr<LmConnection>  _conn);
    void PingEvent();
    void GarbageConnection(int _nTimeout);

private:
    void delConnection(int _sock);

private:

    CNDFTCPServer * server_;
    std::unordered_map<int, std::shared_ptr<LmConnection>>  mapConnections_;

private:
    LmTotalMessage  totalMessage_;

};

#endif // LM_SERVER_HPP
