//////////////////////////////////////////////////////////////////////////
// Database.h
//
//////////////////////////////////////////////////////////////////////////
#pragma once
#include "Manager.h"
#include "Transactions.h"

//////////////////////////////////////////////////////////////////////////
// SQL database class
//////////////////////////////////////////////////////////////////////////
class Database
{
private:
    // connection details
    std::string         mHost;
    std::string         mPort;
    std::string         mUser;
    std::string         mPass;
    std::string         mSrvc;
    std::string         mConn;
    // transactions lock
    std::mutex          mSync;
    // database session
    soci::session       mSQL;
    // database procedures
    soci::procedure    *mProcPriceUpdate;
    soci::procedure    *mProcUserUpdate;
    soci::procedure    *mProcTradeUpdate;
    soci::procedure    *mProcSymbolUpdate;
    soci::procedure    *mProcGroupUpdate;
    soci::procedure    *mProcSymbolGroupUpdate;
    soci::procedure    *mProcMarginUpdate;
    // database transactions
    TransQuote          mRowQuote;
    TransTrade          mRowTrade;
    TransUser           mRowUser;
    TransSymbol         mRowSymbol;
    TransGroup          mRowGroup;
    TransSymbolGroup    mRowSymbolGroup;
    TransMargin         mRowMargin;

public:
    // ctor/dtor
    Database();
    virtual ~Database();
    // init/shutdown
    bool            init(char *host,char *port,char *user,char *pass,char *schema);
    void            shutdown();
    // connect
    bool            connect();
    bool            connected();
    // database id
    const std::string id() const { return(mSrvc); }
    // commit transactions
    bool            commitQuote(const TransQuote *trans);
    bool            commitTrade(const TransTrade *trans);
    bool            commitUser(const TransUser *trans);
    bool            commitSymbol(const TransSymbol *trans);
    bool            commitGroup(const TransGroup *trans);
    bool            commitSymbolGroup(const TransSymbolGroup *trans);
    bool            commitMargin(const TransMargin *trans);

private:
    // stored procedures
    void            release();
    bool            prepare();
    // disconnect
    void            disconnect();
};
