#ifndef LM_DATA_NOTIFIER_HPP
#define LM_DATA_NOTIFIER_HPP

#include <thread>
#include <memory>
#include <atomic>

#include "CNDFIniConfigContainer.hpp"
#include "PDBConnectionManager.hpp"
#include "PDBWorker.hpp"

#include "LmData.hpp"
#include "LmServer.hpp"

#include "CThreadMonitor.hpp"

class LmDataNotifier {
public:
    explicit LmDataNotifier();
    ~LmDataNotifier();

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
    bool readConfig(CNDFIniConfigContainer  * _cfg);

private:
    std::atomic_bool        bRunf_;

    std::shared_ptr<bool>   pRefCounter_;
    std::thread             thr_;

    std::shared_ptr<LmData> lmData_;
    PDB::Worker             pdb_;

    LmServer                server_;

private: // Config

    int     nListenPort_;
    bool    bIpv6_;

    int     nPingTimeout_;
    int     nMaxReadQCnt_;
    int     nMaxReadAcceptEventCnt_;

    int     nNotifierSleepMilliSec_;

    std::string      className_;
    CThreadMonitor * pThreadMonitor_ = nullptr;
};


#endif // LM_DATA_NOTIFIER_HPP
