#ifndef __LM_HISTORY_HPP__
#define __LM_HISTORY_HPP__

#include <tuple>
#include <string>
#include <map>
#include <mutex>

#include "LmDefine.hpp"
#include "LmMessage.hpp"

struct stNotiLocationCnt {
    size_t      add;
    size_t      del;

    stNotiLocationCnt() {
        add = 0;
        del = 0;
    }

    stNotiLocationCnt(size_t _add, size_t _del) {
        add = _add;
        del = _del;
    }

    stNotiLocationCnt & operator=(const stNotiLocationCnt & _st) {
        if(&_st != this) {
            add = _st.add;
            del = _st.del;
        }

        return *this;
    }
};

// Dest Node, Dest Proc, Zone Code
using TupDest = std::tuple<std::string, std::string, std::string>;

class LmHistory {
public:
    explicit LmHistory();
    ~LmHistory();

    void Setup(std::string _nodeName, std::string _procName);

    void Send(std::string & _destSysName,
                std::string & _destProcName,
                bool    _bResult,
                stDeltaLocationIds * _st);

    void Send(std::string & _destSysName,
                std::string & _destProcName,
                bool    _bResult,
                LmTotalMessage & _msg);

    void BatchTransfer();

private:
    void clear();

    void append(std::string & _destSysName,
                std::string & _destProcName,
                std::string & _zoneCode,
                bool _bResult,
                size_t _additionCnt,
                size_t _deletionCnt);
    void send(std::string & _destSysName,
             std::string & _destProcName,
             std::string & _zoneCode,
             bool _bResult,
             size_t _additionCnt,
             size_t _deletionCnt);
    void batchTransfer(std::map<TupDest, stNotiLocationCnt> & _m);

private:
    std::mutex      mutex_;

    std::string     nodeName_;
    std::string     procName_;

    std::map<TupDest, stNotiLocationCnt> mapS_;
    std::map<TupDest, stNotiLocationCnt> mapF_;
};

#endif
