/******************************************************************************
* NORTi Network TELNET Daemon                                                 *
*                                                                             *
*  Copyright (c) 1999-2005, MiSPO Co., Ltd.                                   *
*  All rights reserved.                                                       *
*                                                                             *
* 23/Feb/1999                                                                 *
* 22/Mar/1999 signal is added.                                                *
* 25/Apr/1999 All functions are modified.                                     *
* 01/May/1999 Timeout is modified.                                            *
* 05/May/1999 kbhit and getch are added.                                      *
* 08/May/1999 Option negotiation is added.                                    *
* 29/May/1999 telnet_tmout 10 -> 5 min.                                       *
* 22/Sep/1999 resource ID auto assign added.                                  *
* 16/Nov/2000 Changed from telnet_tmout to TELNET_TMOUT.                      *
* 23/Feb/2002 vsta_tsk is called for 16-bit INT CPU.                       HM *
* 14/Jul/2004 Added IPv6 support                                           PK *
* 20/Apr/2005 Extra typecasting removed                                    PK *
* 13/May/2005 Added uninitialization code for Telnet server.               AK *
* 13/May/2005 Modified V3 -> V4                                            AK *
* 03/Aug/2009 Modified time-out processing for input data reception        FY *
* 03/Aug/2009 Added initialization of 'tmo' member of T_CONSOLE structure  FY *
* 05/Aug/2009 Added handling of DEL key code                               FY *
******************************************************************************/

#include <stdio.h>
#include <string.h>
#ifdef GAIO
#include <memory.h>
#endif
#include "kernel.h"
#include "nosio.h"
#include "nonet.h"
#include "nonteln.h"
#include "noncons.h"

#define CTRL_C      0x03
#define TELNET_TMOUT  5 * 60 * (1000/MSEC)

BOOL terminal_sendbin(T_TERMINAL *t, const B *s, INT len);
BOOL telnetd_sendbin(T_TELNETD *t, const B *s, INT len);
TASK telnetd_tsk(T_TELNETD *t);
const T_CTSK telnetd_ctsk = { TA_HLNG, NULL, telnetd_tsk, TELNETD_PRI, TELNETD_SSZ+COMMAND_LINE_LEN, NULL, "telnetd_tsk" };
const T_CMBF telnetd_cmbf = { TA_TFIFO, 2, 16, NULL, "telnetd_mbf" };
extern T_TERMINAL *current_terminal;
extern SYSTIME SNEAR cdecl SYSCK;
extern UH telnet_portno;				/* OYO F.Saeki */
static BOOL s_bTmoutBlock;				/* OYO F.Saeki */

/************************************************/
/* Put String to TELNET or Console              */
/************************************************/

BOOL terminal_print(T_TERMINAL *t, const char *s)
{
    if (t == NULL)
    {   t = current_terminal;
        if (t == NULL)
            return FALSE;
    }

    s_bTmoutBlock = TRUE;               /* OYO F.Saeki */

//    if (t->cepid == 0)
//        return console_print((T_CONSOLE *)t, s);
//    else
        return telnetd_print((T_TELNETD *)t, s);
}

/************************************************/
/* Put binary to TELNET or Console(By F.Saeki)  */
/************************************************/

BOOL terminal_sendbin(T_TERMINAL *t, const B *s, INT len)
{
    if (t == NULL)
    {   t = current_terminal;
        if (t == NULL)
            return FALSE;
    }

    s_bTmoutBlock = TRUE;               /* OYO F.Saeki */

//    if (t->cepid == 0)
//        return console_print((T_CONSOLE *)t, s);
//    else
        return telnetd_sendbin((T_TELNETD *)t, s, len);
}

/************************************************/
/* Put String and CR+LF to TELNET or Console    */
/************************************************/

BOOL terminal_puts(T_TERMINAL *t, const char *s)
{
    if (!terminal_print(t, s))
        return FALSE;
    return terminal_print(t, "\r\n");
}

/************************************************/
/* Put CR+LF and String to TELNET or Console    */
/************************************************/

BOOL terminal_rputs(T_TERMINAL *t, const char *s)
{
    if (!terminal_print(t, "\r\n"))
        return FALSE;
    return terminal_print(t, s);
}

/************************************************/
/* Put Charactor to TELNET or Console           */
/************************************************/

BOOL terminal_putch(T_TERMINAL *t, INT c)
{
    char s[2];

    s[0] = (char)c;
    s[1] = '\0';
    return terminal_print(t, s);
}

/************************************************/
/* Get String from TELNET or Console            */
/************************************************/

INT terminal_input(T_TERMINAL *t, char *buf, INT size, TMO tmo)
{
    ER ercd;
    INT i, skip;
    H c;

    if (t == NULL)
    {   t = current_terminal;
        if (t == NULL)
        {   dly_tsk(100/MSEC);
            return 0;
        }
    }

    for (i = skip = c = 0;;)
    {
        ercd = trcv_mbf(t->rx_mbfid, &c, tmo);
        if (ercd <= 0 || ercd != 2)
        {
            if (ercd == E_TMOUT && s_bTmoutBlock == TRUE) {    /* OYO F.Saeki */
                s_bTmoutBlock = FALSE;
                continue;
            }
            t->ercd = ercd;
            return ercd;
        }

        if (c <= 0)
            return c;
        if (i == 0 && c == '\n')    /* Ignore Last LF */
            continue;
        if (skip)
        {   skip--;
            continue;
        }
        if (c == '\r' || c == CTRL_C)   /* CR or CTRL-C */
        {   buf[i] = '\0';
            return c;
        }
        if (c == '\b' || c == 0x7f)      /* BS or DEL */
        {   if (i > 0)
            {   i--;
                if (t->echo_on)
                {   if (!terminal_print(t, "\b \b"))
                        break;
                }
            }
            continue;
        }
        if (c == 0x1b)      /* ESC x x */
        {   skip = 2;
            continue;
        }
        if ((UB)c < 0x20 || (UB)c > 0x7e)
            continue;
        if (i >= size - 1)
            continue;
        buf[i++] = (char)c;
        if (t->echo_on)
        {   if (t->echo_on == '*')
                c = '*';
            if (!terminal_putch(t, c))
                break;
        }
    }
    buf[i] = '\0';
    if (c & 0x80)
        c |= 0xff00;
    return c;       /* Last Charactor */
}

char *terminal_gets(T_TERMINAL *t, char *s, INT size)
{
    INT c;

    for (;;)
    {   c = terminal_input(t, s, size, TMO_FEVR);
        if (c == '\r')
            break;
        dly_tsk(1000/MSEC);
    }
    terminal_print(t, "\r\n");
    return s;
}

/************************************************/
/* Test a Character be Input or not             */
/************************************************/

BOOL terminal_kbhit(T_TERMINAL *t)
{
    T_RMBF rmbf;
    ER ercd;

    if (t == NULL)
    {   t = current_terminal;
        if (t == NULL)
            return FALSE;
    }

    ercd = ref_mbf(t->rx_mbfid, &rmbf);
    if (ercd == E_OK && rmbf.smsgcnt != 0)
        return TRUE;
    else
        return FALSE;
}

/************************************************/
/* Get a Character                              */
/************************************************/

INT terminal_getch(T_TERMINAL *t)
{
    ER ercd;
    H c;

    if (t == NULL)
    {   t = current_terminal;
        if (t == NULL)
        {   dly_tsk(100/MSEC);
            return 0;
        }
    }

    ercd = rcv_mbf(t->rx_mbfid, &c);
    if ((ercd <= 0) || (ercd != 2))
    {   t->ercd = ercd;
        return 0;
    }

    return c;
}

/************************************************/
/* Define Signal Handler                         */
/************************************************/

SIGNAL_FUNC terminal_signal(T_TERMINAL *t, int sig, SIGNAL_FUNC func)
{
    SIGNAL_FUNC old;

    if (t == NULL)
    {   t = current_terminal;
        if (t == NULL)
            return FALSE;
    }

    if (sig != SIGINT)
        return SIG_ERR;
    old = (SIGNAL_FUNC)t->sigint;
    t->sigint = (FP)func;
    return old;
}

/************************************************/
/* TELNET Low Level Put String                  */
/************************************************/

BOOL telnetd_print(T_TELNETD *t, const char *s)
{
    ER ercd;
    INT len;

    len = strlen((char *)s);
    if (len == 0)
        return TRUE;

    for (;;)

#if defined(_USE_UDP_)
	{	ercd = udp_snd_dat(t->cepid, &(t->dstaddr), s, len, TMO_FEVR);
#else
    {   ercd = tcp_snd_dat(t->cepid, s, len, TMO_FEVR);
#endif
 		if (ercd <= 0)
        {   t->ercd = ercd;
            return FALSE;
        }
        if ((len -= ercd) == 0)
            return TRUE;
        s += ercd;
    }
}

/************************************************/
/* TELNET Low Level Put Binary Data(By F.Saeki) */
/************************************************/

BOOL telnetd_sendbin(T_TELNETD *t, const B *s, INT len)
{
    ER ercd;

    if (len == 0)
        return TRUE;

    for (;;)
#if defined(_USE_UDP_)
	{	ercd = udp_snd_dat(t->cepid, &(t->dstaddr), s, len, TMO_FEVR);
#else
    {   ercd = tcp_snd_dat(t->cepid, s, len, TMO_FEVR);
#endif
        if (ercd <= 0)
        {   t->ercd = ercd;
            return FALSE;
        }
        if ((len -= ercd) == 0)
            return TRUE;
        s += ercd;
    }
}

/************************************************/
/* TELNET Low Level Get Charactors              */
/************************************************/

INT telnetd_getch(T_TELNETD *t)
{
    ER len;
    UB c;

    for (;;)
    {
#if defined(_USE_UDP_)
    {	len = udp_rcv_dat(t->cepid, &(t->dstaddr), str, sizeof str, TMO_FEVR);
#else
        len = tcp_rcv_dat(t->cepid, &c, 1, TMO_FEVR);
#endif
        if (len <= 0)
            return len;     /* Error Code */
        if (c != 0)
            return c;
    }
}

/************************************************/
/* UDP Based Echo Server Callback               */
/************************************************/

ER udpcmd_cbk(ID cepid, FN fncd, VP parblk)
{
    return E_OK;
}

/************************************************/
/* TELNET Option Negotiation                    */
/************************************************/

INT telnetd_opt_nego(T_TELNETD *t)
{
    INT c;

    telnetd_print(t, "\xff""\xfb""\x03"     /* <IAC><WILL><suppress GA> */
                          "\xff""\xfb""\x01"     /* <IAC><WILL><echo> */
                          "\xff""\xfe""\x01");   /* <IAC><DONT><echo> */
    for (;;)
    {
        c = telnetd_getch(t);
        if (c <= 0)
            return c;
        c = telnetd_getch(t);
        if (c <= 0)
            return c;
        c = telnetd_getch(t);
        if (c != 0xff)
            return c;
    }
}

/************************************************/
/* TELNET Daemon Task                           */
/************************************************/

TASK telnetd_tsk(T_TELNETD *t)
{
    H c;
    ER ercd;
    BOOL sio;
    SIGNAL_FUNC func;

    sio = (t->cepid == 0);

    for (;;)
    {
        t->logged_in = FALSE;

        /* Accept TCP Connection */

#if defined(_USE_UDP_)
#else        if (!sio)
        {   while (1) {
                ercd = tcp_acp_cep(t->cepid, t->repid, &t->dstaddr, TMO_FEVR);
                if (ercd == E_OK)
                    break;

                if ((ercd == E_TMOUT) || (ercd == E_CLS)) {
                    dly_tsk(1000/MSEC);
                }
                else {

                    /* Panic Error! Can't continue process */
                    tcp_can_cep(t->cepid, TFN_TCP_ALL);
                    tcp_del_cep(t->cepid);
                    tcp_del_rep(t->repid);
                    del_mbf(t->rx_mbfid);
                    t->cepid = 0;
                    /* Check whether telnetd_tsk is used by Shell */
                    if (t->shell_tskid == 0)
                        exd_tsk();
                }
            }

            telnetd_opt_nego(t); /* 06APR2005 - Send default options */
        }
#endif

        /* Start Command Shell Task */

        if (t->shell_tskid != FALSE)
        {   while (vsta_tsk(t->shell_tskid, (VP)t) != E_OK)
            {   ter_tsk(t->shell_tskid);
//                dly_tsk(1000/MSEC);
                dly_tsk(100/MSEC);
            }
        }

        /* Queue Received Charactors */

        for (;;)
        {
            {   c = (H)telnetd_getch(t);
                if (c == 0xff) /* IAC */
                {   c = telnetd_opt_nego(t);
                    if (c <= 0)
                        break;
                }
            }

            if (t->logged_in == (B)-1)
                break;

            ercd = snd_mbf(t->rx_mbfid, &c, 2);
            if (ercd != E_OK)
                break;
            if (c == CTRL_C)
            {   if (t->echo_on)
                    terminal_print(t, "^C");
            }
            if (c == CTRL_C || c <= 0)
            {   func = (SIGNAL_FUNC)t->sigint;
                if (func != SIG_DFL)
                    func(SIGINT);
            }
            if (c == 0)
//            {   dly_tsk(1000/MSEC);
            {
                break;
            }
        }

        /* Close TCP */

        tcp_cls_cep(t->cepid, TMO_FEVR);
    }
}

/************************************************/
/* Delete TELNET Server resources               */
/************************************************/

void telnetd_ext(T_TELNETD *t)
{
    if (t->cepid != 0) {
        tcp_del_rep(t->repid);
        tcp_can_cep(t->cepid, TFN_TCP_ALL);
    }
    else
        del_tsk(t->telnetd_tskid);
}

/************************************************/
/* TELNET Daemon Intialize                      */
/************************************************/

ER telnetd_ini(T_TELNETD *t, ID tskid, ID mbfid, ID cepid, ID repid)
{
#if defined(_USE_UDP_)
	static const T_UDP_CCEP c_udpcmd_cep = { 0, { IPV4_ADDRANY, UDP_PORT_CMD }, (FP)udpcmd_cbk };

#else
//    static const T_TCP_CREP crep = { 0, { IPV4_ADDRANY, TCP_PORT_TELNET } };
    static T_TCP_CREP crep;
    T_TCP_CCEP ccep;
#endif
    ER ercd;

#ifndef _USE_UDP_                        /* OYO F.Saeki */
    crep.repatr = 0;
    crep.myaddr.ipaddr = IPV4_ADDRANY;
    crep.myaddr.portno = telnet_portno;
#endif

    /* Control Block Initialize */

    memset(t, 0, sizeof (T_TELNETD));
    t->telnetd_tskid = tskid;
    t->rx_mbfid = mbfid;
    t->cepid = cepid;
    t->repid = repid;
    t->tmo = TELNET_TMOUT;
    t->echo_on = 0;                      /* OYO F.Saeki */
    s_bTmoutBlock = FALSE;               /* OYO F.Saeki */

    /* Create MessageBuffer */

    if(t->rx_mbfid == 0){/* ID auto (Add by Y.Y) */
        ercd = acre_mbf(&telnetd_cmbf);
        if (ercd < 0)
            return ercd;
        t->rx_mbfid = ercd;
    }
    else{
        ercd = cre_mbf(t->rx_mbfid, &telnetd_cmbf);
        if (ercd != E_OK)
            return ercd;
    }

#if defined(_USE_UDP_)
#else
    /* Create TCP Commnunication End Point */

    memset(&ccep, 0, sizeof (T_TCP_CCEP));
    ccep.sbuf = t->sbuf;
    ccep.sbufsz = sizeof t->sbuf;
    ccep.rbuf = t->rbuf;
    ccep.rbufsz = sizeof t->rbuf;

    if(t->cepid == 0)
    {   ercd = t->cepid = tcp_vcre_cep(&ccep);
        if(t->cepid <= 0)
            goto ERR;
    }
    else
    {   ercd = tcp_cre_cep(t->cepid, &ccep);
        if (ercd != E_OK)
            goto ERR;
    }

    /* Create TCP Reception End Point */

    if(t->repid == 0)
    {   ercd = t->repid = tcp_vcre_rep(&crep);
        if(t->repid <= 0 && ercd != E_OBJ)
            goto ERR;
    }
    else
    {   ercd = tcp_cre_rep(t->repid, &crep);
        if (ercd != E_OK && ercd != E_OBJ)
            goto ERR;
    }
#endif

    /* Create TELNET Daemon Task */

    if(t->telnetd_tskid == 0){/* ID auto (Add by Y.Y) */
        ercd = acre_tsk(&telnetd_ctsk);
        if (ercd < 0)
            goto ERR;
        t->telnetd_tskid = ercd;
    }
    else{
        ercd = cre_tsk(t->telnetd_tskid, &telnetd_ctsk);
        if (ercd != E_OK)
            goto ERR;
    }
#if defined(_USE_UDP_)
    /* Start Echo Server Task */
    /* Create UDP Commnunication End Point */

    if(cepid == 0)
    {   ercd = cepid = udp_vcre_cep(&c_udpcmd_cep);
        if(cepid <= 0)
            goto ERR;
    }
    else
    {   ercd = udp_cre_cep(cepid, &c_udpcmd_cep);
        if (ercd != E_OK)
            goto ERR;
    }
#endif
    return E_OK;

ERR:
#if defined(_USE_UDP_)
	udp_del_cep(cepid);
#else
    tcp_del_rep(t->repid);
    tcp_can_cep(t->cepid, TFN_TCP_ALL);
    tcp_del_cep(t->cepid);
#endif
    del_mbf(t->rx_mbfid);
    del_tsk(t->telnetd_tskid);

    return ercd;
}

#ifdef DUAL_STK

ER telnetd6_ini(T_TELNETD *t, ID tskid, ID mbfid, ID cepid, ID repid)
{
    static const T_TCP_CREP crep = { 0, { IPV4_ADDRANY, TCP_PORT_TELNET, IPV6_ADDRANY, IPV6_ADDR } };
    T_TCP_CCEP ccep;
    ER ercd;

    /* Control Block Initialize */

    memset(t, 0, sizeof (T_TELNETD));
    t->telnetd_tskid = tskid;
    t->rx_mbfid = mbfid;
    t->cepid = cepid;
    t->repid = repid;
    t->tmo = TELNET_TMOUT;
    t->echo_on = 1;

    /* Create MessageBuffer */

    if(t->rx_mbfid == 0){/* ID auto (Add by Y.Y) */
        ercd = acre_mbf(&telnetd_cmbf);
        if (ercd < 0)
            return ercd;
        t->rx_mbfid = ercd;
    }
    else{
        ercd = cre_mbf(t->rx_mbfid, &telnetd_cmbf);
        if (ercd != E_OK)
            return ercd;
    }

    /* Create TCP Commnunication End Point */

    memset(&ccep, 0, sizeof (T_TCP_CCEP));
    ccep.sbuf = t->sbuf;
    ccep.sbufsz = sizeof t->sbuf;
    ccep.rbuf = t->rbuf;
    ccep.rbufsz = sizeof t->rbuf;

    if(t->cepid == 0)
    {   ercd = t->cepid = tcp_vcre_cep(&ccep);
        if(t->cepid <= 0)
            goto ERR;
    }
    else
    {   ercd = tcp_cre_cep(t->cepid, &ccep);
        if (ercd != E_OK)
            goto ERR;
    }

    /* Create TCP Reception End Point */

    if(t->repid == 0)
    {   ercd = t->repid = tcp_vcre_rep(&crep);
        if(t->repid <= 0 && ercd != E_OBJ)
            goto ERR;
    }
    else
    {   ercd = tcp_cre_rep(t->repid, &crep);
        if (ercd != E_OK && ercd != E_OBJ)
            goto ERR;
    }

    /* Create TELNET Daemon Task */

    if(t->telnetd_tskid == 0){/* ID auto (Add by Y.Y) */
        ercd = acre_tsk(&telnetd_ctsk);
        if (ercd < 0)
            goto ERR;
        t->telnetd_tskid = ercd;
    }
    else{
        ercd = cre_tsk(t->telnetd_tskid, &telnetd_ctsk);
        if (ercd != E_OK)
            goto ERR;
    }

    return E_OK;

ERR:

    tcp_del_rep(t->repid);
    tcp_can_cep(t->cepid, TFN_TCP_ALL);
    tcp_del_cep(t->cepid);
    del_mbf(t->rx_mbfid);
    del_tsk(t->telnetd_tskid);

    return ercd;
}

#endif
/* end */
