/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef tcp_H_
#define tcp_H_

extern long resolv(char [], unsigned long *);

extern long connectto(char [], char []);

extern long waitconnect(char [], unsigned long);

extern long acceptconnect(long, char [], long *);

extern void ipnum2str(char [], char [], unsigned long);

extern long sendsock(long, char [], long);

extern long sendmore(long, char [], long);

extern long readsock(long, char [], long);

extern long getsockipnum(long, char [], long *);

extern long getpeeripnum(long, char [], long *);

extern void ipnumport2str(char [], unsigned long, char [], unsigned long,
                char [], unsigned long);

extern long getunack(long);

extern void stoptxrx(long, long);


#endif /* tcp_H_ */
