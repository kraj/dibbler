/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 *
 * released under GNU GPL v2 or later licence
 *
 * $Id: OptIA_NA.h,v 1.4 2004-09-07 22:02:33 thomson Exp $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2004/06/17 23:53:54  thomson
 * Server Address Assignment rewritten.
 */

class TOptIA_NA;
#ifndef OPTIA_NA_H
#define OPTIA_NA_H

#include "Opt.h"

class TOptIA_NA : public TOpt
{
  public:
    TOptIA_NA( long IAID, long t1,  long t2, TMsg* parent);
    TOptIA_NA( char * &buf, int &bufsize, TMsg* parent);
    int getSize();
    int getStatusCode();
    
    unsigned long getIAID();
    unsigned long getT1();
    unsigned long getT2();
    unsigned long getMaxValid();
    int countAddrs();
    
    char * storeSelf( char* buf);
    bool isValid();
 protected:
    bool Valid;
    unsigned long IAID;
    unsigned long T1;
    unsigned long T2;
};


#endif /*  */
