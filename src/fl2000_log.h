// fl2000_log.h
//
// (c)Copyright 2017, Fresco Logic, Incorporated.
//
// Purpose: Companion File
//

#ifndef _FL2000_LOG_H_
#define _FL2000_LOG_H_

#define TRACE_LEVEL_NONE	0
#define TRACE_LEVEL_CRITICAL	1
#define TRACE_LEVEL_FATAL	1
#define TRACE_LEVEL_ERROR	2
#define TRACE_LEVEL_WARNING	3
#define TRACE_LEVEL_INFO	4
#define TRACE_LEVEL_VERBOSE	5
#define TRACE_LEVEL_RESERVED6	6
#define TRACE_LEVEL_RESERVED7	7
#define TRACE_LEVEL_RESERVED8	8
#define TRACE_LEVEL_RESERVED9	9

// Define Debug Flags
//
#define DBG_INIT                                (1 << 0)
#define DBG_PNP                                 (1 << 1)
#define DBG_POWER                               (1 << 2)
#define DBG_IOCTL                               (1 << 3)
#define DBG_RENDER                              (1 << 4)
#define DBG_OTHER                               (1 << 5)
#define DBG_FRAMEUPDATE                         (1 << 6)
#define DBG_HW                                  (1 << 7)
#define DBG_COMPRESSION                         (1 << 8)

#define	DEFAULT_DBG_FLAGS	(DBG_INIT | DBG_PNP | DBG_POWER | DBG_IOCTL | DBG_COMPRESSION)

#define dbg_msg(level, flags, msg, ...)		\
	do {                                            \
		if ((level <= currentTraceLevel) &&	\
		    (flags & currentTraceFlags)) {	\
			printk("%s:" msg "\n", __func__, ##__VA_ARGS__);   \
		}					\
	} while (0)

extern uint32_t currentTraceLevel;
extern uint32_t currentTraceFlags;

#endif // _FL2000_LOG_H_

// eof: fl2000_log.h
//
