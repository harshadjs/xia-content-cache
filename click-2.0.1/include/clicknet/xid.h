#ifndef __XID_H__
#define __XID_H__
#include <stdint.h>

#define CLICK_XIA_XID_ID_LEN        20

struct click_xia_xid {
	uint32_t type;
	uint8_t id[CLICK_XIA_XID_ID_LEN];
} __attribute__((packed));

#endif
