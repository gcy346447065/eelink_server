/*
 * version.h
 *
 *  Created on: Apr 19, 2015
 *      Author: jk
 */

#ifndef SRC_VERSION_H_
#define SRC_VERSION_H_

/*
 * Changelog
 * 1.0: support communication to TK115, using libevent
 * 1.2: support communication to GIZWITS, using moquitto
 * 1.3: support communication to Leancloud, using libcurl
 * 1.4: remove support for GIZWITS
 * 1.5: support yeelink, using libcurl
 * 1.5.1: add SIGINT and SIGTERM handler
 * 1.5.2: change log interface
 * 1.5.3: add timeout for tk115 connection
 */

#define VERSION_MAJOR   1
#define VERSION_MINOR   5
#define VERSION_MICRO   3

#define VERSION_INT(a, b, c)    (a << 16 | b << 8 | c)
#define VERSION_DOT(a, b, c)    a##.##b##.##c
#define VERSION(a, b, c)        VERSION_DOT(a, b, c)

#define STRINGIFY(s)         TOSTRING(s)
#define TOSTRING(s) #s

#define VERSION_STR STRINGIFY(VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO))



#endif /* SRC_VERSION_H_ */
