#ifndef LM_CONNECTION_HPP_HPP
#define LM_CONNECTION_HPP_HPP

class LmConnection {
public:
    explicit LmConnection(int _sock, char * _clientIp);
    ~LmConnection();

    void    SetSysName(const char * _name) { sysName_ = _name; }
    void    SetProcName(const char  * _name) { procName_ = _name; }
    std::string & GetSysName() { return sysName_; }
    std::string & GetProcName() { return procName_; }

    int     Recv(char * _ptr, size_t _length, int _milliSeconds=0);
    bool    Send(char * _ptr, size_t _length);
    void    Disconnect();

    int     GetSock() { return sock_; }
    const char *  GetIp() { return clientIp_.c_str(); }
    void    RecvPing();
    bool    IsConnected() { return sock_ > 0; }
    bool    IsNoPingEvent(int _nTimeout);

private:
    int         sock_;
    std::string clientIp_;
    time_t      tLastRecvTime_;

    std::string sysName_;
    std::string procName_;
};


#endif // LM_CONNECTION_HPP_HPP
