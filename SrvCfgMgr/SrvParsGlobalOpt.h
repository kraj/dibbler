/*                                                                           
 * Dibbler - a portable DHCPv6                                               
 *                                                                           
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>                           
 *          Marek Senderski <msend@o2.pl>                                    
 *                                                                           
 * released under GNU GPL v2 or later licence                                
 *                                                                           
 * $Id: SrvParsGlobalOpt.h,v 1.10 2007-09-20 23:40:59 thomson Exp $
 *
 */

#ifndef TSRVPARSGLOBALOPT_H_
#define TSRVPARSGLOBALOPT_H_

#include "SrvParsIfaceOpt.h"
#include "DHCPConst.h"

typedef enum {
    SRV_IFACE_ID_ORDER_BEFORE,
    SRV_IFACE_ID_ORDER_AFTER,
    SRV_IFACE_ID_ORDER_NONE
} ESrvIfaceIdOrder;

class TSrvParsGlobalOpt : public TSrvParsIfaceOpt
{
public:
    TSrvParsGlobalOpt(void);
    ~TSrvParsGlobalOpt(void);

    string getWorkDir();
    void setWorkDir(string dir);
    void setStateless(bool stateless);
    bool getStateless();
    void setCacheSize(int bytes);
    int  getCacheSize();
    void addDigest(DigestTypes x);
    List(DigestTypes) getDigest();
    bool getExperimental(); // is experimental stuff allowed?
    void setExperimental(bool exper);
    void setInterfaceIDOrder(ESrvIfaceIdOrder order);
    ESrvIfaceIdOrder getInterfaceIDOrder();
    void setInactiveMode(bool flex);
    bool getInactiveMode();

    void setAuthLifetime(unsigned int lifetime);
    unsigned int getAuthLifetime();
    void setAuthKeyLen(unsigned int len);
    unsigned int getAuthKeyLen();
    
private:
    bool   Experimental;
    string WorkDir;
    bool   Stateless;
    bool   InactiveMode;
    int    CacheSize;
    unsigned int AuthLifetime;
    unsigned int AuthKeyLen;
    List(DigestTypes) DigestLst;
    ESrvIfaceIdOrder InterfaceIDOrder;
};
#endif
