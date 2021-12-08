
#ifndef LM_DATA_LOADER_HPP
#define LM_DATA_LOADER_HPP

#include <thread>
#include <memory>
#include <atomic>

#include "CNDFIniConfigContainer.hpp"
#include "PDBConnectionManager.hpp"
#include "PDBWorker.hpp"
#include "Select_ZoneCellNotiSQL.hpp"
#include "Select_ZoneCellSQL.hpp"


#include "LmData.hpp"

#include "CThreadMonitor.hpp"

class LmDataLoader {
public:
    explicit LmDataLoader();
    ~LmDataLoader();

    bool Init(std::shared_ptr<PDB::ConnectionManager>  _mgr,
              std::shared_ptr<LmData> _lmData,
              CNDFIniConfigContainer  * _cfg);
    void SetHangupThreadMonitor(CThreadMonitor * _pThreadMonitor) { 
        pThreadMonitor_ = _pThreadMonitor;
    }

    bool IsRun() { return pRefCounter_.use_count() > 1; }

    void Run();
    void Stop() { bRunf_.store(false); }


private:
    void run(std::shared_ptr<bool> _pRefCounter);
    bool loadLocationIds(LmData * _p);
    void readConfig(CNDFIniConfigContainer * _cfg);

private:
    std::atomic_bool        bRunf_;

    std::shared_ptr<bool>   pRefCounter_;
    std::thread             thr_;

    std::shared_ptr<LmData> lmData_;
    PDB::Worker             pdb_;

    Select_ZoneCellNotiSQL  zcSQL_;
    Select_ZoneCellSQL      zSQL_;

    std::string             className_;
    CThreadMonitor *        pThreadMonitor_ = nullptr;

private:

    int     nLoaderSleepMilliSec_;

};


#endif // LM_DATA_LOADER_HPP
