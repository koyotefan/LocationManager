
#include "NDFServiceLog.hpp"
#include "LmMessage.hpp"

extern std::string      gSysName;
extern std::string      gProcName;

LmDeltaMessage::LmDeltaMessage()
    : ptr_(nullptr),
      len_(0) {

    h_.nType = 7;
    //h_.nBodyLength = 0;
    //h_.tTime = 0;
    memset(h_.sysName, 0, sizeof(h_.sysName));
    strncpy(h_.sysName, gSysName.c_str(), sizeof(h_.sysName)-1);

    memset(h_.procName, 0, sizeof(h_.procName));
    strncpy(h_.procName, gProcName.c_str(), sizeof(h_.procName)-1);

    dataCnt_ = 0;
}

LmDeltaMessage::~LmDeltaMessage() {
    clear();
}

void LmDeltaMessage::clear() {
    if(ptr_ != nullptr) {
        delete [] ptr_;
        ptr_ = nullptr;

        len_ = 0;
    }

    h_.nBodyLength = 0;
    h_.tTime = 0;

    dataCnt_ = 0;
}

bool LmDeltaMessage::MakeData(stDeltaLocationIds * _sp) {

    clear();

    dataCnt_ = _sp->vecAdditions.size() + _sp->vecDeletions.size();
    h_.nBodyLength =  sizeof(stDataBody) * dataCnt_;
    h_.tTime = time(nullptr);

    ptr_ = new (std::nothrow) char [sizeof(stHeader) + h_.nBodyLength];

    if(ptr_ == nullptr) {
        E_LOG("MakeData() fail [new operation] [%p]", ptr_);
        return false;
    }

    len_ =  sizeof(stHeader) + h_.nBodyLength;

    memcpy(ptr_, (char *)&h_, sizeof(stHeader));
    stDataBody * body = (stDataBody *)(ptr_ + sizeof(stHeader));

    for(auto & citer : _sp->vecAdditions) {

        if(citer.size() >= sizeof(body->locationId)) {
            E_LOG("MakeData() add locationId too long [%s]", citer.c_str());
            continue;
        }
        memset(body->locationId, 0, sizeof(body->locationId));
        strncpy(body->locationId, citer.c_str(), sizeof(body->locationId)-1);

        memset(body->zoneCode, 0, sizeof(body->zoneCode));
        strncpy(body->zoneCode, _sp->zoneCode.c_str(), sizeof(body->zoneCode)-1);

        body->locationType[0] = '\0';

        memset(body->status, 0, sizeof(body->status));
        strncpy(body->status, "I", sizeof(body->status)-1);
        body++;
    }

    for(auto & citer : _sp->vecDeletions) {

        if(citer.size() >= sizeof(body->locationId)) {
            E_LOG("MakeData() del locationId too long [%s]", citer.c_str());
            continue;
        }

        memset(body->locationId, 0, sizeof(body->locationId));
        strncpy(body->locationId, citer.c_str(), sizeof(body->locationId)-1);

        memset(body->zoneCode, 0, sizeof(body->zoneCode));
        strncpy(body->zoneCode, _sp->zoneCode.c_str(), sizeof(body->zoneCode)-1);

        body->locationType[0] = '\0';


        memset(body->status, 0, sizeof(body->status));
        strncpy(body->status, "D", sizeof(body->status)-1);
        body++;
    }

    return true;
}

LmTotalMessage::LmTotalMessage()
    : ptr_(nullptr),
      len_(0) {

    h_.nType = 5;

    memset(h_.sysName, 0, sizeof(h_.sysName));
    strncpy(h_.sysName, gSysName.c_str(), sizeof(h_.sysName)-1);

    memset(h_.procName, 0, sizeof(h_.procName));
    strncpy(h_.procName, gProcName.c_str(), sizeof(h_.procName)-1);
}

LmTotalMessage::~LmTotalMessage() {
    Clear();
}

void LmTotalMessage::Clear() {
    if(ptr_ != nullptr) {
        delete [] ptr_;
        ptr_ = nullptr;

        len_ = 0;
    }

    h_.nBodyLength = 0;
    h_.tTime = 0;
    vecLocationIds_.clear();
    mapCntByZone_.clear();
}

void LmTotalMessage::Append(stLocationIds & _locationIds) {

    stDataBody  body;
    memset(&body, 0, sizeof(body));

    memset(body.zoneCode, 0, sizeof(body.zoneCode));
    strncpy(body.zoneCode, _locationIds.record.zoneCode.c_str(), sizeof(body.zoneCode)-1);

    memset(body.status, 0, sizeof(body.status));
    strncpy(body.status, "I", sizeof(body.status)-1);

    for(auto & citer : _locationIds.setLocationIds) {
        if(citer.size() >= sizeof(body.locationId)) {
            E_LOG("Append() add locationId too long [%s]", citer.c_str());
            continue;
        }

        memset(body.locationId, 0, sizeof(body.locationId));
        strncpy(body.locationId, citer.c_str(), sizeof(body.locationId)-1);
        vecLocationIds_.emplace_back(body);
    }

    mapCntByZone_[body.zoneCode] = _locationIds.setLocationIds.size();
}

void LmTotalMessage::GetCntByZoneCode(std::unordered_map<std::string, size_t> & _um) {
    _um = mapCntByZone_;
}

bool LmTotalMessage::MakeData() {

    size_t dataCnt = vecLocationIds_.size();
    h_.nBodyLength = sizeof(stDataBody) * dataCnt;
    h_.tTime = time(nullptr);

    D_LOG("Total Message BodyLength:[%d] dataCnt:[%zu] body:[%zu]",
		h_.nBodyLength, dataCnt, sizeof(stDataBody));

    ptr_ = new (std::nothrow) char [sizeof(stHeader) + h_.nBodyLength];

    if(ptr_ == nullptr) {
        E_LOG("MakeData() fail [new operation] [%p]", ptr_);
        return false;
    }

    len_ =  sizeof(stHeader) + h_.nBodyLength;

    memcpy(ptr_, (char *)&h_, sizeof(stHeader));
    stDataBody * body = (stDataBody *)(ptr_ + sizeof(stHeader));

    for(auto & citer : vecLocationIds_) {
        memcpy(body, (stDataBody *)&citer, sizeof(stDataBody));
        body++;
    }

    return true;
}


LmConnectMessage::LmConnectMessage()
    : ptr_(nullptr),
      len_(0) {

    h_.nType = 2;
    h_.nBodyLength = sizeof(stConnectionBody);
    h_.tTime = time(nullptr);

    memset(h_.sysName, 0, sizeof(h_.sysName));
    strncpy(h_.sysName, gSysName.c_str(), sizeof(h_.sysName)-1);

    memset(h_.procName, 0, sizeof(h_.procName));
    strncpy(h_.procName,gProcName.c_str(), sizeof(h_.procName)-1);

    makeData();
}

LmConnectMessage::~LmConnectMessage() {
    clear();
}

bool LmConnectMessage::makeData() {

    ptr_ = new (std::nothrow) char [sizeof(stHeader) + h_.nBodyLength];

    if(ptr_ == nullptr) {
        E_LOG("makeData() fail [new operation] [%p]", ptr_);
        return false;
    }

    len_ = sizeof(stHeader) + h_.nBodyLength;

    memcpy(ptr_, (char *)&h_, sizeof(stHeader));
    stConnectionBody * body = (stConnectionBody *)(ptr_ + sizeof(stHeader));

    memset(body->result, 0, sizeof(body->result));
    strncpy(body->result, "0", sizeof(body->result)-1);

    return true;
}

void LmConnectMessage::clear() {

    if(ptr_ != nullptr) {
        delete [] ptr_;
        len_ = 0;
    }
}
