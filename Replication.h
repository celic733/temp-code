//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
#pragma once
#include "Config.h"
#include "Manager.h"
#include "Database.h"
#include "Workpool.h"

//////////////////////////////////////////////////////////////////////////
// type definitions
//////////////////////////////////////////////////////////////////////////
typedef std::vector<Manager*>            ManagerArray;
typedef std::vector<Database*>           DatabaseArray;
typedef Workpool<TransGeneric*>          TransQueue;
typedef std::map<std::string,TransQuote> QuotesMap;
typedef std::map<int,TransMargin>        TransMarginMap;

//////////////////////////////////////////////////////////////////////////
// replication
//////////////////////////////////////////////////////////////////////////
class Replication
{
private:
    // servers config
    Config          mConfig;
    // MT5 managers
    ManagerArray    mManagers;
    // SQL databases
    DatabaseArray   mDatabases;
    // working dir
    std::string     mWorkPath;
    // transactions queue
    TransQueue      mQueue;
    // quotes
    QuotesMap       mQuotes;
    TransMarginMap  mMargins;
    // thread pool
    std::vector<std::thread*> mThreads;

public:
    // constructor/destructor
    Replication();
    virtual ~Replication();
    // initialization/shutdown
    bool            init();
    void            shutdown();
    // received data from mt4
    void            onReceive(TransGeneric *trans);

private:
    // process data
    bool            consume(TransGeneric *trans);
    // thread functions
    static void     funcWrapConsume(void *param);
    static void     funcWrapProcess(void *param);
    bool            runConsume();
    bool            runProcess();
};
