//////////////////////////////////////////////////////////////////////////
// Database.cpp
//
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Database.h"

//////////////////////////////////////////////////////////////////////////
// ctor
//////////////////////////////////////////////////////////////////////////
Database::Database()
    : mProcPriceUpdate(nullptr),
      mProcUserUpdate(nullptr),
      mProcTradeUpdate(nullptr),
      mProcSymbolUpdate(nullptr),
      mProcGroupUpdate(nullptr),
      mProcSymbolGroupUpdate(nullptr),
      mProcMarginUpdate(nullptr)
{
}
//////////////////////////////////////////////////////////////////////////
// dtor
//////////////////////////////////////////////////////////////////////////
Database::~Database()
{
    // finalize work
    shutdown();
}
//////////////////////////////////////////////////////////////////////////
// initialization
//////////////////////////////////////////////////////////////////////////
bool Database::init(char *host,char *port,char *user,char *pass,char *schema)
{
    // checks
    if(host==nullptr || port==nullptr || user==nullptr || pass==nullptr || schema==nullptr)
    {
        Logger::get().log("failed to initialize '%s' database [invalid connection details]",schema);
        return(false);
    }
    // lock
    mSync.lock();
    // copy params
    mHost=host;
    mPort=port;
    mUser=user;
    mPass=pass;
    mSrvc=schema;
    // format connection string
    std::ostringstream conn;
    conn << "mysql://host=" << mHost.c_str() << " port=" << mPort << " dbname=" << mSrvc << " user=" << mUser << " password='" << mPass << "'";
    mConn=conn.str();
    // unlock
    mSync.unlock();
    // connect immediately
    connect();
    // success
    return(true);
}
//////////////////////////////////////////////////////////////////////////
// connect to db
//////////////////////////////////////////////////////////////////////////
bool Database::connect()
{
    // lock
    mSync.lock();
    // disconnect first
    disconnect();
    // create session
    try
    {
        // open db connection
        mSQL.open(mConn);
        // prepare stored proc-s
        prepare();
    }
    catch(soci::mysql_soci_error &e)
    {
        Logger::get().log("'%s': failed to create mysql session [%s]",mSrvc.c_str(),e.what());
        mSync.unlock();
        return(false);
    }
    catch(soci::soci_error &e)
    {
        Logger::get().log("'%s': failed to create sql session [%s]",mSrvc.c_str(),e.what());
        mSync.unlock();
        return(false);
    }
    // unlock
    mSync.unlock();
    // log info
    Logger::get().log("'%s': connected to '%s@%s' database",mUser.c_str(),mSrvc.c_str(),mHost.c_str());
    // success
    return(true);
}
//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
bool Database::connected()
{
    int res=0;
    // lock
    mSync.lock();
    // get mysql back-end
    soci::mysql_session_backend *session=static_cast<soci::mysql_session_backend*>(mSQL.get_backend());
    if(session==nullptr)
    {
        mSync.unlock();
        return(false);
    }
    // ping mysql
    res=mysql_ping(session->conn_);
    // unlock
    mSync.unlock();
    // check mysql state
    if(res)
    {
        Logger::get().log("'%s': connection to mysql server has been lost, reconnecting.. [%d]",mSrvc.c_str(),res);
        return(false);
    }
    // connected
    return(true);
}
//////////////////////////////////////////////////////////////////////////
// shutdown
//////////////////////////////////////////////////////////////////////////
void Database::shutdown()
{
    // disconnect
    if(connected())
        disconnect();
    // logout
    Logger::get().log("'%s': database '%s@%s' shutdown",mUser.c_str(),mSrvc.c_str(),mHost.c_str());
}
//////////////////////////////////////////////////////////////////////////
// prices commit
//////////////////////////////////////////////////////////////////////////
bool Database::commitQuote(const TransQuote *trans)
{
    bool res=false;
    // checks
    if(trans==nullptr)
        return(false);
    // lock
    mSync.lock();
    // check
    if(mProcPriceUpdate==nullptr)
    {
        mSync.unlock();
        return(false);
    }
    // copy data
    memcpy(&mRowQuote,trans,sizeof(mRowQuote));
    // execute procedure
    try
    {
        res=mProcPriceUpdate->execute(true);
    }
    catch(soci::mysql_soci_error &e)
    {
        Logger::get().log("'%s': failed to commit '%s' quote [%s]",mSrvc.c_str(),trans->data.symbol,e.what());
        mSync.unlock();
        return(false);
    }
    catch(soci::soci_error &e)
    {
        Logger::get().log("'%s': failed to commit '%s' quote [%s]",mSrvc.c_str(),trans->data.symbol,e.what());
        mSync.unlock();
        return(false);
    }
    // unlock
    mSync.unlock();
    // result
    return(res);
}
//////////////////////////////////////////////////////////////////////////
// user commit
//////////////////////////////////////////////////////////////////////////
bool Database::commitUser(const TransUser *trans)
{
    bool res=false;
    // checks
    if(trans==nullptr)
        return(false);
    // lock
    mSync.lock();
    // checks
    if(mProcUserUpdate==nullptr)
    {
        mSync.unlock();
        return(false);
    }
    // copy data
    memcpy(&mRowUser,trans,sizeof(mRowUser));
    // execute procedure
    try
    {
        res=mProcUserUpdate->execute(true);
    }
    catch(soci::mysql_soci_error &e)
    {
        Logger::get().log("'%s': failed to commit user '#%d' [%s]",mSrvc.c_str(),trans->data.login,e.what());
        mSync.unlock();
        return(false);
    }
    catch(soci::soci_error &e)
    {
        Logger::get().log("'%s': failed to commit user '#%d' [%s]",mSrvc.c_str(),trans->data.login,e.what());
        mSync.unlock();
        return(false);
    }
    // unlock
    mSync.unlock();
    // log info
    Logger::get().log("'%s': committed user '#%d'",mSrvc.c_str(),trans->data.login);
    // result
    return(res);
}
//////////////////////////////////////////////////////////////////////////
// trade commit
//////////////////////////////////////////////////////////////////////////
bool Database::commitTrade(const TransTrade *trans)
{
    bool res=false;
    // checks
    if(trans==nullptr)
        return(false);
    // lock
    mSync.lock();
    // check
    if(mProcTradeUpdate==nullptr)
    {
        mSync.unlock();
        return(false);
    }
    // copy data
    memcpy(&mRowTrade,trans,sizeof(mRowTrade));
    // execute procedure
    try
    {
        res=mProcTradeUpdate->execute(true);
    }
    catch(soci::mysql_soci_error &e)
    {
        Logger::get().log("'%s': failed to commit trade '%d' [%s]",mSrvc.c_str(),trans->data.order,e.what());
        mSync.unlock();
        return(false);
    }
    catch(soci::soci_error &e)
    {
        Logger::get().log("'%s': failed to commit trade '%d' [%s]",mSrvc.c_str(),trans->data.order,e.what());
        mSync.unlock();
        return(false);
    }
    // unlock
    mSync.unlock();
    // log info
    Logger::get().log("'%s': committed trade '%d'",mSrvc.c_str(),trans->data.order);
    // result
    return(res);
}
//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
bool Database::commitSymbol(const TransSymbol *trans)
{
	bool res=false;
    // checks
    if(trans==nullptr)
        return(false);
    // lock
    mSync.lock();
    // checks
    if(mProcSymbolUpdate==nullptr)
    {
        mSync.unlock();
        return(false);
    }
	// copy data
	memcpy(&mRowSymbol,trans,sizeof(mRowSymbol));
    // execute procedure
    try
    {
        res=mProcSymbolUpdate->execute(true);
    }
    catch(soci::mysql_soci_error &e)
    {
        Logger::get().log("'%s': failed to commit '%s' symbol [%s]",mSrvc.c_str(),trans->data.symbol,e.what());
        mSync.unlock();
        return(false);
    }
    catch(soci::soci_error &e)
    {
        Logger::get().log("'%s': failed to commit '%s' symbol [%s]",mSrvc.c_str(),trans->data.symbol,e.what());
        mSync.unlock();
        return(false);
    }
    // unlock
    mSync.unlock();
    // log info
    Logger::get().log("'%s': committed symbol '%s'",mSrvc.c_str(),trans->data.symbol);
	// result
	return(res);
}
//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
bool Database::commitGroup(const TransGroup *trans)
{
    bool res=false;
    // checks
    if(trans==nullptr)
        return(false);
    // lock
    mSync.lock();
    // checks
    if(mProcGroupUpdate==nullptr)
    {
        mSync.unlock();
        return(false);
    }
    // copy data
    memcpy(&mRowGroup,trans,sizeof(mRowGroup));
    // execute procedure
    try
    {
        res=mProcGroupUpdate->execute(true);
    }
    catch(soci::mysql_soci_error &e)
    {
        Logger::get().log("'%s': failed to commit '%s' group [%s]",mSrvc.c_str(),trans->data.group,e.what());
        mSync.unlock();
        return(false);
    }
    catch(soci::soci_error &e)
    {
        Logger::get().log("'%s': failed to commit '%s' group [%s]",mSrvc.c_str(),trans->data.group,e.what());
        mSync.unlock();
        return(false);
    }
    // unlock
    mSync.unlock();
    // log info
    Logger::get().log("'%s': committed group '%s'",mSrvc.c_str(),trans->data.group);
    // result
    return(res);
}
//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
bool Database::commitSymbolGroup(const TransSymbolGroup *trans)
{
    bool res=false;
    // checks
    if(trans==nullptr)
        return(false);
    // lock
    mSync.lock();
    // checks
    if(mProcSymbolGroupUpdate==nullptr)
    {
        mSync.unlock();
        return(false);
    }
    // copy data
    memcpy(&mRowSymbolGroup,trans,sizeof(mRowSymbolGroup));
    // execute procedure
    try
    {
        res=mProcSymbolGroupUpdate->execute(true);
    }
    catch(soci::mysql_soci_error &e)
    {
        Logger::get().log("'%s': failed to commit '%s' symbol group [%s]",mSrvc.c_str(),trans->data.name,e.what());
        mSync.unlock();
        return(false);
    }
    catch(soci::soci_error &e)
    {
        Logger::get().log("'%s': failed to commit '%s' symbol group [%s]",mSrvc.c_str(),trans->data.name,e.what());
        mSync.unlock();
        return(false);
    }
    // unlock
    mSync.unlock();
    // log info
    Logger::get().log("'%s': committed symbol group '%s'",mSrvc.c_str(),trans->data.name);
    // result
    return(res);
}
//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
bool Database::commitMargin(const TransMargin *trans)
{
    bool res=false;
    // checks
    if(trans==nullptr)
        return(false);
    // lock
    mSync.lock();
    // checks
    if(mProcMarginUpdate==nullptr)
    {
        mSync.unlock();
        return(false);
    }
    // copy data
    memcpy(&mRowMargin,trans,sizeof(mRowMargin));
    // execute procedure
    try
    {
        res=mProcMarginUpdate->execute(true);
    }
    catch(soci::mysql_soci_error &e)
    {
        Logger::get().log("'%s': failed to commit user '#%d' margin level [%s]",mSrvc.c_str(),trans->data.login,e.what());
        mSync.unlock();
        return(false);
    }
    catch(soci::soci_error &e)
    {
        Logger::get().log("'%s': failed to commit user '#%d' margin level [%s]",mSrvc.c_str(),trans->data.login,e.what());
        mSync.unlock();
        return(false);
    }
    // unlock
    mSync.unlock();
    // log info
    //Logger::get().log("'%s': committed margin level for user [#%d]",mSrvc.c_str(),trans->data.login);
    // result
    return(res);
}
////////////////////////////////////////////////////////////////////////
// prepare procedures
////////////////////////////////////////////////////////////////////////
bool Database::prepare()
{
    // logout
    Logger::get().log("'%s': preparing stored functions",mSrvc.c_str());
    try
    {
        // create procedures
        mProcPriceUpdate      =new soci::procedure((mSQL.prepare << PROC_UPDATE_PRICE,      soci::use(mRowQuote)));
        mProcUserUpdate       =new soci::procedure((mSQL.prepare << PROC_UPDATE_USER,       soci::use(mRowUser)));
        mProcTradeUpdate      =new soci::procedure((mSQL.prepare << PROC_UPDATE_TRADE,      soci::use(mRowTrade)));
        mProcSymbolUpdate     =new soci::procedure((mSQL.prepare << PROC_UPDATE_SYMBOL,     soci::use(mRowSymbol)));
        mProcGroupUpdate      =new soci::procedure((mSQL.prepare << PROC_UPDATE_GROUP,      soci::use(mRowGroup)));
        mProcSymbolGroupUpdate=new soci::procedure((mSQL.prepare << PROC_UPDATE_SYMBOLGROUP,soci::use(mRowSymbolGroup)));
        mProcMarginUpdate     =new soci::procedure((mSQL.prepare << PROC_UPDATE_MARGIN,     soci::use(mRowMargin)));
    }
    catch(soci::mysql_soci_error &e)
    {
        Logger::get().log("'%s': failed to prepare mysql stored functions [%s]",mSrvc.c_str(),e.what());
        return(false);
    }
    catch(soci::soci_error &e)
    {
        Logger::get().log("'%s': failed to prepare database objects [%s]",mSrvc.c_str(),e.what());
        return(false);
    }
    // success
    return(true);
}
////////////////////////////////////////////////////////////////////////
// release CLOBs and procedures
////////////////////////////////////////////////////////////////////////
void Database::release()
{
    // logout
    Logger::get().log("'%s': releasing stored functions",mSrvc.c_str());
    // release func's
    if(mProcPriceUpdate)        { delete(mProcPriceUpdate);       mProcPriceUpdate      =nullptr; }
    if(mProcUserUpdate)         { delete(mProcUserUpdate);        mProcUserUpdate       =nullptr; }
    if(mProcTradeUpdate)        { delete(mProcTradeUpdate);       mProcTradeUpdate      =nullptr; }
    if(mProcSymbolUpdate)       { delete(mProcSymbolUpdate);      mProcSymbolUpdate     =nullptr; }
    if(mProcGroupUpdate)        { delete(mProcGroupUpdate);       mProcSymbolUpdate     =nullptr; }
    if(mProcSymbolGroupUpdate)  { delete(mProcSymbolGroupUpdate); mProcSymbolGroupUpdate=nullptr; }
    if(mProcMarginUpdate)       { delete(mProcMarginUpdate);      mProcMarginUpdate     =nullptr; }
}
//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
void Database::disconnect()
{
    // release procedures and blobs
    release();
    // close connections
    mSQL.close();
    // logout
    Logger::get().log("'%s': disconnected from '%s@%s' database",mUser.c_str(),mSrvc.c_str(),mHost.c_str());
}
