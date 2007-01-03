/*                                                                           
 * Dibbler - a portable DHCPv6                                               
 *                                                                           
 * author: Krzysztof Wnuk <keczi@poczta.onet.pl>
 * 
 * released under GNU GPL v2 or later licence
 *                                                                           
 * $Id: SrvCfgPD.cpp,v 1.4 2007-01-02 22:24:52 thomson Exp $
 *
 */

#include "SrvCfgPD.h"
#include "SmartPtr.h"
#include "SrvParsGlobalOpt.h"
#include "DHCPConst.h"
#include "Logger.h"

/*
 * static field initialization
 */
unsigned long TSrvCfgPD::staticID=0;
 
TSrvCfgPD::TSrvCfgPD() {
    this->PD_T1Beg    = 0;
    this->PD_T1End    = DHCPV6_INFINITY;
    this->PD_T2Beg    = 0;
    this->PD_T2End    = DHCPV6_INFINITY;
    this->ID = staticID++;
    this->PD_Assigned = 0;
    this->PD_Count = 0;
    this->PD_Length = 0;
}

TSrvCfgPD::~TSrvCfgPD() {
}

unsigned long TSrvCfgPD::chooseTime(unsigned long beg, unsigned long end, unsigned long clntTime)
{
    if (clntTime < beg)
	return beg;
    if (clntTime > end)
	return end;
    return clntTime;
}

unsigned long TSrvCfgPD::getT1(unsigned long hintT1) {
    return chooseTime(PD_T1Beg, PD_T1End, hintT1);
}

unsigned long TSrvCfgPD::getT2(unsigned long hintT2) {
    return chooseTime(PD_T2Beg, PD_T2End, hintT2);
}

unsigned long TSrvCfgPD::getPrefered(unsigned long hintPref) {
    return chooseTime(PD_PrefBeg, PD_PrefEnd, hintPref);
}

unsigned long TSrvCfgPD::getValid(unsigned long hintValid) {
    return chooseTime(PD_ValidBeg, PD_ValidEnd, hintValid);
}

unsigned long TSrvCfgPD::getPD_Length() {
    return this->PD_Length;
}

bool TSrvCfgPD::setOptions(SmartPtr<TSrvParsGlobalOpt> opt, int prefixLength)
{
    int poolLength;
    int cnt = 0;
    Log(Debug) << "PD: Client will receive /" << prefixLength << " prefixes (T1=" << opt->getT1Beg() 
	       << ".." << opt->getT1End() << ", T2=" << opt->getT2Beg() << ".." << opt->getT2End()
	       << ")." <<LogEnd; 
    this->PD_T1Beg    = opt->getT1Beg();
    this->PD_T2Beg    = opt->getT2Beg();
    this->PD_T1End    = opt->getT1End();
    this->PD_T2End    = opt->getT2End();
    this->PD_PrefBeg  = opt->getPrefBeg();
    this->PD_PrefEnd  = opt->getPrefEnd();
    this->PD_ValidBeg = opt->getValidBeg();
    this->PD_ValidEnd = opt->getValidEnd();
    
    this->PD_Length   = prefixLength;
    PD_MaxLease = opt->getClassMaxLease();
    
    SmartPtr<TStationRange> PD_Range;
    // FIXME: Implement white-list, black-list support
    /* For a while white and black list will be not used
	opt->firstRejedClnt();
    while(PD_Range=opt->getRejedClnt())
        this->RejedClnt.append(PD_Range);
    
    opt->firstAcceptClnt();
    while(PD_Range=opt->getAcceptClnt())
        this->AcceptClnt.append(PD_Range);
	*/

    opt->firstPool();
    SPtr<TStationRange> pool = 0;
    if (!(pool=opt->getPool())) {
	Log(Error) << "Unable to find any prefix pools. Please define at least one using 'pd-pool' keyword." << LogEnd;
	return false;
    }
    this->PD_Count = prefixLength - pool->getPrefixLength();
    if (this->PD_Count > 31)
	this->PD_Count = DHCPV6_INFINITY;
    this->PD_Count = 2 << (this->PD_Count-1);

    opt->firstPool();
    while ( pool = opt->getPool() ) {
	poolLength = pool->getPrefixLength();
	PoolLst.append(pool);
	Log(Debug) << "PD: Pool " << pool->getAddrL()->getPlain() << " - " 
		   << pool->getAddrR()->getPlain() << ", pool length: " 
		   << pool->getPrefixLength() << "." << LogEnd;
	if (this->PD_Count > pool->rangeCount())
	    this->PD_Count = pool->rangeCount();
	cnt++;
    }

    // calculate common section
    PoolLst.first();
    pool = PoolLst.get();
    if (!pool) {
	Log(Crit) << "Unable to find first prefix pool. Something is wrong, very wrong." << LogEnd;
	return false;
    }
    CommonPool = new TStationRange( new TIPv6Addr(*pool->getAddrL()), new TIPv6Addr(*pool->getAddrR()));
    CommonPool->truncate(pool->getPrefixLength(), prefixLength);
    CommonPool->setPrefixLength(poolLength);

    /* Log(Debug) << "PD: Common part is " << CommonPool->getAddrL()->getPlain() << " - " 
       << CommonPool->getAddrR()->getPlain() << ", pool length: " 
       << CommonPool->getPrefixLength() << "." << LogEnd; */

    // set up prefix counter counts
    this->PD_Assigned = 0;
    if (this->PD_MaxLease > this->PD_Count)
	this->PD_MaxLease = this->PD_Count;
    Log(Debug) << "PD: Up to " << this->PD_Count << " prefixes may be assigned." << LogEnd;
    return true;
}

bool TSrvCfgPD::prefixInPool(SmartPtr<TIPv6Addr> prefix)
{
    SPtr<TStationRange> pool = 0;
    PoolLst.first();
    while ( pool=PoolLst.get() ) {
	if (pool->in(prefix))
	    return true;
    }
    return false;
}

/** 
 * returns random prefix from a first pool
 * 
 * @return 
 */
SmartPtr<TIPv6Addr> TSrvCfgPD::getRandomPrefix()
{
    SPtr<TStationRange> pool;
    PoolLst.first();
    pool = PoolLst.get();
    if (pool)
	return pool->getRandomPrefix();
    return 0;
}

/** 
 * gets random prefix from the common part (b) and 
 * returns a list of prefixes generated by contatenation
 * of the common part and pool-specific prefix
 * 
 * @param TIPv6Addr 
 * 
 * @return 
 */
List(TIPv6Addr) TSrvCfgPD::getRandomList() {
    SPtr<TIPv6Addr> commonPart,tmp;
    SPtr<TStationRange> range;

    List(TIPv6Addr) lst;
    lst.clear();

    commonPart = this->CommonPool->getRandomPrefix();
    commonPart->truncate(0, this->getPD_Length());
    PoolLst.first();
    while (range = PoolLst.get()) {
	tmp = range->getAddrL();
        //Log(Debug) << " #### prefix=" << tmp->getPlain() << ", host=" << commonPart->getPlain() << ", prefixLength=" 
        //           << this->CommonPool->getPrefixLength() << LogEnd;
	lst.append( new TIPv6Addr(tmp->getAddr(), commonPart->getAddr(), this->CommonPool->getPrefixLength()) );
	
    }
    return lst;
}

unsigned long TSrvCfgPD::getPD_MaxLease() {
    return PD_MaxLease;
}

unsigned long TSrvCfgPD::getID()
{
    return this->ID;
}

long TSrvCfgPD::incrAssigned(int count) {
    this->PD_Assigned += count;
    return this->PD_Assigned;
}

long TSrvCfgPD::decrAssigned(int count) {
    this->PD_Assigned -= count;
    return this->PD_Assigned;
}

unsigned long TSrvCfgPD::getAssignedCount() {
    return this->PD_Assigned;
}

ostream& operator<<(ostream& out,TSrvCfgPD& prefix)
{
    out << "    <PD id=\"" << prefix.ID << "\">" << std::endl;
    out << "      <!-- total prefixes in class: " << prefix.PD_Count 
	<< ", prefixes assigned: " << prefix.PD_Assigned << " -->" << endl;
    out << "      <T1 min=\"" << prefix.PD_T1Beg << "\" max=\"" << prefix.PD_T1End  << "\" />" << endl;
    out << "      <T2 min=\"" << prefix.PD_T2Beg << "\" max=\"" << prefix.PD_T2End  << "\" />" << endl;
    out << "      <prefered-lifetime min=\"" << prefix.PD_PrefBeg << "\" max=\"" << prefix.PD_PrefEnd  << "\" />" << endl;
    out << "      <valid-lifetime min=\"" << prefix.PD_ValidBeg << "\" max=\"" << prefix.PD_ValidEnd  << "\" />" << endl;
    out << "      <PDMaxLease>" << prefix.PD_MaxLease << "</PDMaxLease>" << endl;
        
    SmartPtr<TStationRange> statRange;
    out << "      <!-- prefix range -->" << endl;
    SPtr<TStationRange> pool;
    prefix.PoolLst.first();
    while (pool = prefix.PoolLst.get()) {
	out << *pool;
    }
    
    /*out << "      <!-- reject-clients ranges:" << addrClass.RejedClnt.count() << " -->" << endl;
    addrClass.RejedClnt.first();
    while(statRange=addrClass.RejedClnt.get())
	out << *statRange;	
	
    out << "      <!-- accept-only ranges:" << addrClass.AcceptClnt.count() << " -->" << endl;
    addrClass.AcceptClnt.first();
    while(statRange=addrClass.AcceptClnt.get())
	out << *statRange;*/
    out << "    </PD>" << std::endl;
    return out;
}