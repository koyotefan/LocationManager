
#include <unordered_map>
#include <vector>

#include "CNDFATOMAdaptor.hpp"
#include "LmHistory.hpp"

LmHistory::LmHistory() {

}

LmHistory::~LmHistory() {

}

void LmHistory::Setup(std::string _nodeName, std::string _procName) {

    nodeName_ = _nodeName;
    procName_ = _procName;
}

// 통계처럼 쏘는건줄 알고 ㅠ.ㅜ 괜히 만들었어요..
void LmHistory::append(std::string & _destSysName,
                       std::string & _destProcName,
                       std::string & _zoneCode,
                       bool _bResult,
                       size_t _additionCnt,
                       size_t _deletionCnt) {

    std::lock_guard<std::mutex>   guard(mutex_);

    TupDest     tDest = std::make_tuple(_destSysName, _destProcName, _zoneCode);

    std::map<TupDest, stNotiLocationCnt> & m = (_bResult)?mapS_:mapF_;

    auto iter = m.find(tDest);

    if(iter == m.end()) {
        stNotiLocationCnt   st(_additionCnt, _deletionCnt);
        m.emplace(tDest, st);

        return ;
    }

    stNotiLocationCnt & stCnt = iter->second;
    stCnt.add += _additionCnt;
    stCnt.del += _deletionCnt;

    return ;

}

void LmHistory::clear() {
    for(auto & citer : mapS_) {
        stNotiLocationCnt & st = citer.second;

        st.add = 0;
        st.del = 0;
    }

    for(auto & citer : mapF_) {
        stNotiLocationCnt & st = citer.second;

        st.add = 0;
        st.del = 0;
    }
}

void LmHistory::Send(std::string & _destSysName,
                       std::string & _destProcName,
                       bool _bResult,
                       stDeltaLocationIds * _sp) {

    send(_destSysName,
           _destProcName,
           _sp->zoneCode,
           _bResult,
           _sp->vecAdditions.size(),
           _sp->vecDeletions.size());

}

void LmHistory::Send(std::string & _destSysName,
                       std::string & _destProcName,
                       bool _bResult,
                       LmTotalMessage & _msg) {

    std::unordered_map<std::string, size_t>  um;
    _msg.GetCntByZoneCode(um);

    for(auto & citer : um) {

        std::string zoneCode = citer.first;

        send(_destSysName,
               _destProcName,
               zoneCode,
               _bResult,
               citer.second,
               0);
    }
}


void LmHistory::batchTransfer(std::map<TupDest, stNotiLocationCnt> & _m) {

    std::vector<std::string>    vecStr;
    std::vector<int>            vecInt;

    std::lock_guard<std::mutex>   guard(mutex_);

    for(auto & citer : _m) {
        vecStr.clear();
        vecInt.clear();

        vecStr.push_back(procName_);
        vecStr.push_back(std::get<0>(citer.first));
        vecStr.push_back(std::get<1>(citer.first));
        vecStr.push_back(std::get<2>(citer.first));
        vecStr.push_back("Y");

        vecInt.push_back(citer.second.add);
        vecInt.push_back(citer.second.del);

        citer.second.add = 0;
        citer.second.del = 0;

        ATOM_SEND_HIST("T_PCF_LM_HIST", vecStr, vecInt);
    }
}

void LmHistory::send(std::string & _destSysName,
                     std::string & _destProcName,
                     std::string & _zoneCode,
                     bool _bResult,
                     size_t _additionCnt,
                     size_t _deletionCnt) {

    std::vector<std::string>    vecStr;
    std::vector<int>            vecInt;

    vecStr.push_back(procName_);
    vecStr.push_back(_destSysName);
    vecStr.push_back(_destProcName);
    vecStr.push_back(_zoneCode);

    if(_bResult)
        vecStr.push_back("Y");
    else
        vecStr.push_back("N");

    vecInt.push_back(_additionCnt);
    vecInt.push_back(_deletionCnt);

    ATOM_SEND_HIST("T_PCF_LM_HIST", vecStr, vecInt);
}

void LmHistory::BatchTransfer() {
    batchTransfer(mapS_);
    batchTransfer(mapF_);
}
