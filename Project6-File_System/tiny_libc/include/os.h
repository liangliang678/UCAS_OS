#ifndef OS_H
#define OS_H

#include <stdint.h>

typedef enum {
    ENTER_ZOMBIE_ON_EXIT,
    AUTO_CLEANUP_ON_EXIT,
} spawn_mode_t;

typedef enum {
    KERNEL_PROCESS,
    KERNEL_THREAD,
    USER_PROCESS,
    USER_THREAD,
} task_type_t;

/* task information, used to init PCB */
typedef struct task_info
{
    uintptr_t entry_point;
    task_type_t type;
} task_info_t;

typedef int32_t pid_t;

/* The next few constants help upper layers determine the size of memory
 * pools used for Ethernet buffers and descriptor lists.
 */
#define XEMACPS_MAC_ADDR_SIZE   6U	/* size of Ethernet header */

#define XEMACPS_MTU             1500U	/* max MTU size of Ethernet frame */
#define XEMACPS_MTU_JUMBO       10240U	/* max MTU size of jumbo frame */
#define XEMACPS_HDR_SIZE        14U	/* size of Ethernet header */
#define XEMACPS_HDR_VLAN_SIZE   18U	/* size of Ethernet header with VLAN */
#define XEMACPS_TRL_SIZE        4U	/* size of Ethernet trailer (FCS) */
#define XEMACPS_MAX_FRAME_SIZE       (XEMACPS_MTU + XEMACPS_HDR_SIZE + XEMACPS_TRL_SIZE)
#define XEMACPS_MAX_VLAN_FRAME_SIZE  (XEMACPS_MTU + XEMACPS_HDR_SIZE +  XEMACPS_HDR_VLAN_SIZE + XEMACPS_TRL_SIZE)
#define XEMACPS_MAX_VLAN_FRAME_SIZE_JUMBO  (XEMACPS_MTU_JUMBO + XEMACPS_HDR_SIZE +  XEMACPS_HDR_VLAN_SIZE + XEMACPS_TRL_SIZE)

typedef char EthernetFrame[XEMACPS_MAX_FRAME_SIZE]
	__attribute__ ((aligned(64)));

/* define for test_fs.c */
#define O_RDONLY 1 /* read only open */
#define O_WRONLY 2 /* write only open */
#define O_RDWR 3 /* read/write open */

#endif // OS_H
