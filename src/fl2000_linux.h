// fl2000_linux.h
//
// (c)Copyright 20017, Fresco Logic, Incorporated.
//
// Purpose:
//

#ifndef _FL2000_LINUX_H_
#define _FL2000_LINUX_H_

/////////////////////////////////////////////////////////////////////////////////
// Porting Constants Definitions
/////////////////////////////////////////////////////////////////////////////////
//

#define NTSTATUS                                int
#define STATUS_SUCCESS                          (0)
#define STATUS_UNSUCCESSFUL                     (-EIO)
#define STATUS_INVALID_PARAMETER                (-EINVAL)
#define STATUS_NO_SUCH_DEVICE                   (-ENODEV)

#define NT_SUCCESS( status )                    (status == STATUS_SUCCESS)

/////////////////////////////////////////////////////////////////////////////////
// Porting Functions
/////////////////////////////////////////////////////////////////////////////////
//

#define DELAY_MS(msecs)				msleep(msecs)

#define ASSERT(cond)							\
do {									\
    if ( !( cond ) ) 							\
    { 									\
        printk( "ERROR %s %d ASSERT fail.", __FUNCTION__, __LINE__ ); 	\
        panic( #cond ); 						\
    }									\
} while( 0 );

#define ERROR_SOFTWARE()                        ASSERT(false)
#define UNREFERENCED_PARAMETER(P)             	((void) P)

#define RtlCopyMemory(dst, src, length)		memcpy(dst, src, length)
#define RtlZeroMemory(dst, length)		memset(dst, 0, length)
#define RtlFillMemory(dst, length, pattern)	memset(dst, pattern, length)

#endif // _FL2000_LINUX_H_

// eof: fl2000_linux.h
//
