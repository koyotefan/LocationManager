#include <iostream>
#include <memory>
#include <poll.h>

#include "NDFServiceLog.hpp"

#include "CNDFConv.hpp"
#include "CNDFATOMAdaptor.hpp"
#include "CPDBAlarmTrap.hpp"


#include "LmMain.hpp"

#include "LmHistory.hpp"
#include "LmDataLoader.hpp"
#include "LmDataNotifier.hpp"

#include "CThreadMonitor.hpp"
#include "Verbose.hpp"

std::string             gSysName;
std::string             gProcName;
CNDFIniConfigContainer * gCfg = nullptr;
LmHistory               gHist;

auto strVERSION = std::string(PACKAGE_VERSION); 

LmMain::LmMain(int _argc,char **_argv,int _initMode)
    :  CNDFApplication(_argc, _argv, _initMode),
       bRunf_(true) {

    pcm_ = std::make_shared<PDB::ConnectionManager>();
    lmData_ = std::make_shared<LmData>();
}

LmMain::~LmMain(void) {

}

int LmMain::AppInitial(int _initResult)
{
    switch(_initResult)
    {
        case NDF_APP_RUN_OK :
            break ;
        case NDF_APP_RUN_DUPLICATED :
        case NDF_APP_RUN_DAEMONIZE_FAILED :
        case NDF_APP_RUN_VNF_INIT_FAILED :
        case NDF_APP_RUN_GETENV_FAILED :
        case NDF_APP_RUN_CONFIG_FAILED :
        case NDF_APP_RUN_LOGGER_FAILED :
        case NDF_APP_RUN_LOGGER_PATH_NOT_FOUND :
        case NDF_APP_RUN_LOGGER_APPL_INDEX_INVALID :
        case NDF_APP_RUN_ATOM_FAILED : // environment variable($NDF_ATOM_IGNORE) undefined or defined with 'no'
        default :
            std::cout << "LmMain::AppInitial fail :" << _initResult << std::endl;
            return 0;
    }

    gSysName  = GetPkgNodeID();
    gProcName = GetProcName();

    gCfg = GetConfigContainer();

    // PDB Init ////////////////////////////////
    char    buf[256];
    if ( gCfg->Get((char *)"PCF_PDB",(char *)"CONFIG",buf) == false )
    {
        E_LOG("configuration (PCF_PDB:CONFIG) not found!") ;
        return 0;
    }

    char    fname[256];
    CNDFConv::EnvReplace(fname,buf) ;

    if(pcm_->Init(gSysName, gProcName, fname) == false) {
        E_LOG("ConnectionManager Init() fail");
        return 0;
    }

    // PDB Hangup Time Setting ///////////////////
    if ( gCfg->Get((char *)"COMMON",(char *)"APP_HANGUP_SEC",buf) == false )
    {
        W_LOG("configuration (COMMON:APP_HANGUP_SEC) not found!") ;
    } else {
        nApp_HANGUP_SEC_ = -1;

        try {  nApp_HANGUP_SEC_ = std::stoi(std::string(buf)); } catch (...) {}
        if(nApp_HANGUP_SEC_ > 0) {
            pcm_->SetHangupValidTime(nApp_HANGUP_SEC_);
            I_LOG("Configuration (COMMON:APP_HANGUP_SEC) [%d]", nApp_HANGUP_SEC_) ;
        }
    }

    // 이력 저장 Init ////////////////////////////
    gHist.Setup(gSysName, gProcName);

    if(netconfPDBActive_.Init() == false) {
        E_LOG("Netconf for PDB Init() fail");
        return 0;
    }

    if(netconfPDBActive_.LoadConfig() == false) {
        E_LOG("Netconf for PDB LoadConfig() fail");
        return 0;
    }

    I_LOG("LoadData [%s]", netconfPDBActive_.GetData());
    pcm_->SwitchActiveDB(netconfPDBActive_.GetData());


    if(G_ATOM_ADAPTOR.SendVersion(strVERSION.c_str()) ==  false)
    {
        W_LOG("SendVersion Fail %s", strVERSION.c_str());
    }
    

    I_LOG("AppInitial ok") ;
    return 1;
}


int LmMain::AppRun(void) {

    I_LOG("LM.Run.start");

    pThreadMonitor_ = ThdMonitorSTPtr;

    if(nApp_HANGUP_SEC_ > 0) {
        pThreadMonitor_->SetValidTime(nApp_HANGUP_SEC_);
    }

    LmDataLoader    loader;
    loader.SetHangupThreadMonitor(pThreadMonitor_);

    if(loader.Init(pcm_, lmData_, gCfg) == false) {
        E_LOG("LmDataLoader Init() fail");
        return 0;
    }

    LmDataNotifier  notifier;
    notifier.SetHangupThreadMonitor(pThreadMonitor_);

    if(notifier.Init(pcm_, lmData_, gCfg) == false) {
        E_LOG("LmDataNotifier Init() fail");
        return 0;
    }

    loader.Run();
    notifier.Run();

    ATOM_SET_MANUAL_HANGUP_CONTROL();
    ATOM_NOTIFY_READY();
    while(bRunf_) {
        if(doCommand() == false)
            break;

        if(loader.IsRun() == false) {
            E_LOG("LmDataLoader Down");
            break;
        }

        if(notifier.IsRun() == false) {
            E_LOG("LmDataLoader Down");
            break;
        }

        if(netconfPDBActive_.IsChanged()) {
            if(netconfPDBActive_.LoadConfig()) {
                pcm_->SwitchActiveDB(netconfPDBActive_.GetData());
            }
        }

        CPDBAlarmTrap::Send(pcm_.get());

        // poll(nullptr, 0, 100);
    }

    W_LOG("LM.Run.end") ;

    notifier.Stop();
    loader.Stop();

    // bRunf_ 가 True 임에도 빠져 나왔다는 것은, 문제가 있다는 뜻 입니다.
    return (bRunf_ == false)?1:0;
}

bool LmMain::doCommand() {
    int              cmd ;
    char             args[4096] ;
    bool             bShouldResponse = false ;
    int              ret ;

    if(IsExit())
    {
        I_LOG("LmMain.doCommand.IsExit");
        bRunf_ = false;
        return true;
    }

    ret = GetCommand(&cmd,args) ;
    if ( ret == false )
    {
        W_LOG("LmMain.doCommand.GetCommand fail");
        return false ;
    }

    if ( cmd == NDF_CMD_EMPTY ) {
        poll(nullptr, 0, 100);
        return true ;
    }

    D_LOG("LmMain.doCommand.GetCommand [%d]", cmd) ;

    switch(cmd)
    {
        case NDF_CMD_APP_INIT  :
            bShouldResponse = true ;
            break ;
        case NDF_CMD_APP_STOP :
            I_LOG("LmMain.doCommand.APP_STOP");
            bRunf_ = false;
            break ;
        case NDF_CMD_APP_SUSPEND :
        case NDF_CMD_APP_RESUME :
            // what to do?
            break;
        case NDF_CMD_APP_LOGLEVEL :
           break;
        case NDF_CMD_APP_STATISTICS :
            // gHist.BatchTransfer();
            break;
        case    NDF_CMD_UNKNOWN :
            // ATOM Returns Unknown command
            break;
        case    NDF_CMD_EMPTY :
            // no command
            break;
        case 110001 : // DM Signal Command
            bShouldResponse = true ;
            /* TO-DO cli command */
            break;
        case NDF_CMD_APP_PING:
            if(pThreadMonitor_->IsHangup()) {
                break;
            }

            if(pcm_->IsHangup()) {
                E_LOG ("[DETECTED THREAD HANG-UP] PDB Thread");
                break;
            }

            ATOM_SEND_HANGUP_RESPONSE();
            break;
        case 12 :
        case 13 :
        case 14 :
            break;
        case 18:
            if(netconfPDBActive_.Init() == false) {
                E_LOG("Netconf for PDB Init() fail");
            }

            ATOM_SET_MANUAL_HANGUP_CONTROL();
            ATOM_NOTIFY_READY();
            break;
        default :
            E_LOG("LmMain.doCommand.GetCommand.unknown [%d]", cmd);
            break ;
    }

    if ( bShouldResponse == true )
    {
        //PutResponse(true,0) ;
    }

    return true ;

}

int LmMain::AppFinal(int _isNormal) {

    W_LOG("LmMain.AppFinal End [%d]", _isNormal);
    return (!_isNormal)?NDF_APP_STOPPED_BY_ERROR:NDF_APP_STOPPED_BY_USER;
}

int main(int argc, char *argv[]) {
    LmMain  svc(argc, argv, NDF_APPL_INIT_PKG|NDF_APPL_INIT_ATOM);
    svc.Run();

    return 0;
}

