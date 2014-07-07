//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "Workpool.h"
//////////////////////////////////////////////////////////////////////////
// forward declarations
//////////////////////////////////////////////////////////////////////////
class Replication;

//////////////////////////////////////////////////////////////////////////
// manager class which gets data from MT4
//////////////////////////////////////////////////////////////////////////
class Manager
{
private:
    // parent
    Replication    &mParent;
    // server identifier
    int             mSID;
    int             mFlags;
    std::string     mIP;
    int             mLogin;
    std::string     mPass;
    // MT manager
    CManagerFactory mFactory;
    CManagerInterface *mManager;
    // synchronizer
    std::mutex      mSync;
    // last ping timestamp
    __int64         mPingTime;
    // transactions queue
    Workpool<TransGeneric*> &mQueue;

public:
    // ctor/dtor
    Manager(Replication &parent,Workpool<TransGeneric*> &queue);
    ~Manager();
    // init/shutdown
    bool            init(int id,int flags,std::string ip,int login,std::string password);
    void            shutdown();
    // server props
    int             sid()       const { return(mSID); }
    bool            connect();
    bool            connected();
    bool            disconnect();
    // sync margin levels
    bool            syncMargins();

private:
    // 
    bool            onReceive(TransGeneric *trans);
    // sync
    bool            sync();
    // events
    void            onPumpingStart();
    void            onPumpingStop();
    void            onPing();
    void            onQuote();
    void            onGroup(int type,const ConGroup *data);
    void            onUser(int type,const UserRecord *data);
    void            onTrade(int type,const TradeRecord *data);
    void            onSymbol(int type,const ConSymbol *data);
    void            onSymbolGroup(int type,int id,const ConSymbolGroup *data);
    void            onMarginLevel(const MarginLevel *data);
    // callback
    static void __stdcall onEvent(int code,int type,void *data,void *param);
};
