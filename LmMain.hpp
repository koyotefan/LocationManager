#ifndef LM_MAIN_HPP
#define LM_MAIN_HPP

#include <string>
#include <string>
#include <memory>

#include "CNDFApplication.hpp"
#include "CNetconfPDBActive.hpp"
#include "PDBConnectionManager.hpp"
#include "LmData.hpp"

#include "CThreadMonitor.hpp"

class LmMain : public CNDFApplication
{
public:
    explicit LmMain(int _argc, char **_argsv, int _initMode);
    ~LmMain();

    bool doCommand();


private:
    int     AppInitial  (int  _initResult) ;
    int     AppRun      (void) ;
    int     AppFinal    (int  _error) ;

private:
    bool    bRunf_;

    CNDFIniConfigContainer      lmCfg_;
    CNetconfPDBActive           netconfPDBActive_;

    std::shared_ptr<PDB::ConnectionManager>     pcm_;
    std::shared_ptr<LmData>     lmData_;

    int                         nApp_HANGUP_SEC_ = -1;
    CThreadMonitor *            pThreadMonitor_ = nullptr;

};

#endif // LM_MAIN_HPP
