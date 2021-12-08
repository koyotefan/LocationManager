#ifndef LM_MESSAGE_HPP
#define LM_MESSAGE_HPP

#include <vector>

#include "LmDefine.hpp"
#include "CNDFATOMAdaptor.hpp"

struct stHeader {
    int     nType = 0;
    int     nBodyLength = 0;
    time_t  tTime = 0;
    char    sysName[64] = {};
    char    procName[64] = {};

};

struct stConnectionBody {
    char    result[4] = {};
    char    dummy[4] = {};
};

struct stDataBody {
    char    locationId[14] = {};
    char    zoneCode[20] = {};
    char    locationType[8] = {};
    char    status[2] = {};
    char    dummy[6] = {};
};



class LmDeltaMessage {
public:
    explicit LmDeltaMessage();
    ~LmDeltaMessage();

    bool MakeData(stDeltaLocationIds * _p);
    char * GetData() { return ptr_; }
    size_t GetDataLength() { return len_; }
    size_t GetLocationCnt() { return dataCnt_; }

private :
    void clear();

private :
    stHeader    h_;
    size_t      dataCnt_;

    char * ptr_;
    size_t  len_;
};


class LmTotalMessage {
public:
    explicit LmTotalMessage();
    ~LmTotalMessage();

    void Clear();
    void Append(stLocationIds & _locationIds);
    bool MakeData();

    char * GetData() { return ptr_; }
    size_t GetDataLength() { return len_; }
    size_t GetLocationCnt() { return vecLocationIds_.size(); }
    void GetCntByZoneCode(std::unordered_map<std::string, size_t> & _um);

private:
    void addCount(std::string _zoneCode, size_t _cnt);

private:
    stHeader    h_;

    char * ptr_;
    size_t          len_;

    std::vector<stDataBody>  vecLocationIds_;
    
    // for history
    std::unordered_map<std::string, size_t>  mapCntByZone_;

};

class LmConnectMessage {
public:
    explicit LmConnectMessage();
    ~LmConnectMessage();

    char * GetData() { return ptr_; }
    size_t GetDataLength() { return len_; }

private:
    bool makeData();
    void clear();

private:
    stHeader    h_;
    char *      ptr_;
    size_t      len_;

};

#endif // LM_MESSAGE_HPP
