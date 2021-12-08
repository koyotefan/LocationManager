
#include <string>
#include <vector>
#include <chrono>
#include <poll.h>
#include <cstdlib>

#include "NDFServiceLog.hpp"
#include "LmDataLoader.hpp"

LmDataLoader::LmDataLoader()
    :bRunf_(true),
     pRefCounter_(std::make_shared<bool>()) {

    className_ = typeid(*this).name();

}

LmDataLoader::~LmDataLoader() {
    bRunf_.store(false);

    if(thr_.joinable())
        thr_.join();
}

bool LmDataLoader::Init(std::shared_ptr<PDB::ConnectionManager>  _mgr,
                        std::shared_ptr<LmData>     _lmData,
                        CNDFIniConfigContainer *    _cfg
                        ) {
    lmData_ = _lmData;

    readConfig(_cfg);

    pdb_.Assign(_mgr);

    if(pdb_.TurnOn(PDB::eDefDBType::Subscriber) == false) {
        E_LOG("PDB TurnOn() fail");
        return false;
    }

    if(zcSQL_.Bind() == false) {
        E_LOG("T_ZONE_CELL_NOTI Bind() fail");
        return false;
    }

    if(loadLocationIds(lmData_.get()) == false) {
        E_LOG("LmDataLoader Init() loadCells fail");
    }

    I_LOG("LmDataLoader Init() success");
    return true;
}

void LmDataLoader::readConfig(CNDFIniConfigContainer  * _cfg) {


    char    buf[256];
    if ( _cfg->Get((char *)"PCF_LM",(char *)"LOADER_SLEEP_MSEC",buf) == false )
    {
        W_LOG("Config Read fail [PCF_LM:LOADER_SLEEP_MSEC]");
        nLoaderSleepMilliSec_ = 3000;
    } else {
        nLoaderSleepMilliSec_ = std::atoi(buf);
    }

    I_LOG("Config [PCF_LM:LOADER_SLEEP_MSEC] = [%d]", nLoaderSleepMilliSec_);

}

bool LmDataLoader::loadLocationIds(LmData * _p) {

    if(pdb_.Execute(zcSQL_) == false) {
        W_LOG("T_ZONE_CELL_NOTI Select Execute() fail");
        return false;
    }

    const std::vector<stZoneCellNotiRecord> & vecZCN = zcSQL_.GetRecords();

    if(_p->IsUpdated(vecZCN) == false) {
        // 확인했는데 바뀐게 없네요.
        return true;
    }

    for(auto & citer : vecZCN) {

        if(_p->IsUpdated(citer.zoneCode, citer.uTime) == false)
            continue;

        if(zSQL_.SetQuery(citer.tblName.c_str(), citer.zoneCode.c_str()) == false) {
            W_LOG("T_ZONE_CELL SetQuery() fail");
            return false;
        }

        if(pdb_.Execute(zSQL_) == false) {
            W_LOG("T_ZONE_CELL Execute() fail");
            return false;
        }

        _p->Set(citer, zSQL_.GetRecords());
        _p->Update(citer.zoneCode, citer.uTime);

        pThreadMonitor_->Refresh(className_.c_str());
    }

    return true;
}

void LmDataLoader::Run() {
    thr_ = std::thread(&LmDataLoader::run, this, pRefCounter_);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void LmDataLoader::run(std::shared_ptr<bool>   _pRefCounter) {

    (void)_pRefCounter;

    I_LOG("LmDataLoader in service [%s]", className_.c_str());
    pThreadMonitor_->Register(className_.c_str());

    int unitMilliSec  = 50;
    int checkMillisec = unitMilliSec;

    while(bRunf_) {
        pThreadMonitor_->Refresh(className_.c_str());

        poll(nullptr, 0, unitMilliSec);

        if(nLoaderSleepMilliSec_ > checkMillisec) {
            checkMillisec += unitMilliSec;
            continue;
        } else {
            checkMillisec = unitMilliSec;
        }

        if(loadLocationIds(lmData_.get()) == false) {
            E_LOG("LmDataLoader Init() loadCells fail");
        }
    }

    W_LOG("LmDataLoader out of service");
}
