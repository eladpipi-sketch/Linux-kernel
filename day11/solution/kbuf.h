/* kbuf.h: the ioctl commands, shared by the driver and user programs */
#ifndef KBUF_H
#define KBUF_H

#include <linux/ioctl.h>

#define KBUF_IOC_CLEAR	_IO('k', 1)		/* drop everything in the buffer */
#define KBUF_IOC_USED	_IOR('k', 2, int)	/* how many bytes are waiting */

#endif
