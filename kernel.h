#pragma once
#include "common.h"

#define PROCS_MAX 8

#define PROC_UNUSED   0
#define PROC_RUNNABLE 1
#define PROC_EXITED   2

#define SCAUSE_ECALL 8

#define USER_BASE 0x1000000

#define SSTATUS_SPIE (1 << 5)

#define SATP_SV32 (1u << 31)

// PTE flags:
#define PAGE_V (1 << 0) // Valid bit
#define PAGE_R (1 << 1) // Readable
#define PAGE_W (1 << 2) // Writeable
#define PAGE_X (1 << 3) // Executable
#define PAGE_U (1 << 4) // User (accessible in user mode)
#define SSTATUS_SUM  (1 << 18) // Supervisor User Memory

// DISK I/O ------------------------------------

#define SECTOR_SIZE       512
#define VIRTQ_ENTRY_NUM   16
#define VIRTIO_DEVICE_BLK 2
#define VIRTIO_BLK_PADDR  0x10001000
#define VIRTIO_REG_MAGIC         0x00
#define VIRTIO_REG_VERSION       0x04
#define VIRTIO_REG_DEVICE_ID     0x08
#define VIRTIO_REG_QUEUE_SEL     0x30
#define VIRTIO_REG_QUEUE_NUM_MAX 0x34
#define VIRTIO_REG_QUEUE_NUM     0x38
#define VIRTIO_REG_QUEUE_ALIGN   0x3c
#define VIRTIO_REG_QUEUE_PFN     0x40
#define VIRTIO_REG_QUEUE_READY   0x44
#define VIRTIO_REG_QUEUE_NOTIFY  0x50
#define VIRTIO_REG_DEVICE_STATUS 0x70
#define VIRTIO_REG_DEVICE_CONFIG 0x100
#define VIRTIO_STATUS_ACK       1
#define VIRTIO_STATUS_DRIVER    2
#define VIRTIO_STATUS_DRIVER_OK 4
#define VIRTIO_STATUS_FEAT_OK   8

// Flags 
#define VIRTQ_DESC_F_NEXT          1
#define VIRTQ_DESC_F_WRITE         2

#define VIRTQ_AVAIL_F_NO_INTERRUPT 1

// Types
#define VIRTIO_BLK_T_IN  0 // Read from disk (device)
#define VIRTIO_BLK_T_OUT 1 // Write to the disk (device)

#define FILES_MAX      4
#define DISK_MAX_SIZE  align_up(sizeof(struct file) * FILES_MAX, SECTOR_SIZE)

// Virtqueue Descriptor area entry.
struct virtq_desc {
    uint64_t addr; // Physical address of the buffer
    uint32_t len; // Length of the buffer
    uint16_t flags; // Flags: NEXT (chained), WRITE (device writes data)
    uint16_t next; // Index of the next descriptor (if chaining)
} __attribute__((packed));

// Virtqueue Available Ring.
struct virtq_avail {
    uint16_t flags;
    uint16_t index;
    uint16_t ring[VIRTQ_ENTRY_NUM]; // Indices of available descriptors
} __attribute__((packed));

// Virtqueue Used Ring entry.
struct virtq_used_elem {
    uint32_t id; // Descriptor index
    uint32_t len; // Number of bytes written by device
} __attribute__((packed));

// Virtqueue Used Ring.
struct virtq_used {
    uint16_t flags;
    uint16_t index;
    struct virtq_used_elem ring[VIRTQ_ENTRY_NUM];
} __attribute__((packed));

// Virtqueue (All components)
struct virtio_virtq {
    struct virtq_desc descs[VIRTQ_ENTRY_NUM]; // Descriptor table
    struct virtq_avail avail; // Available ring
    struct virtq_used used __attribute__((aligned(PAGE_SIZE))); // Used ring
    int queue_index;
    volatile uint16_t *used_index; // Tracks last used descriptor
    uint16_t last_used_index; // Tracks last completed request
} __attribute__((packed));

// Virtio-blk request.
struct virtio_blk_req {
    // First descriptor: read-only from the device
    // Header:
    uint32_t type; // Type of request (READ/WRITE)
    uint32_t reserved;
    uint64_t sector; // Sector number on disk

    // Second descriptor: writable by the device if it's a read operation (VIRTQ_DESC_F_WRITE)
    // R/W data: 
    uint8_t data[SECTOR_SIZE]; // Data buffer

    // Third descriptor: writable by the device (VIRTQ_DESC_F_WRITE)
    // Status:
    uint8_t status; // Request status
} __attribute__((packed));

// DISK I/O END ------------------------------------


// FILE SYSTEM -------------------------------------

struct tar_header {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char type;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char padding[12];
    char data[];      // Pointer to the file itself

} __attribute__((packed));

struct file {
    bool in_use;
    char name[100];
    char data[1024];
    size_t size;
};

// FILE SYSTEM END ---------------------------------

struct process {
    int pid;
    int state; // PROC_UNUSED or PROC_RUNNABLE
    vaddr_t sp;
    uint32_t* page_table;
    uint8_t stack[8192]; // kernel stack
};

struct trap_frame {
    uint32_t ra;
    uint32_t gp;
    uint32_t tp;
    uint32_t t0;
    uint32_t t1;
    uint32_t t2;
    uint32_t t3;
    uint32_t t4;
    uint32_t t5;
    uint32_t t6;
    uint32_t a0;
    uint32_t a1;
    uint32_t a2;
    uint32_t a3;
    uint32_t a4;
    uint32_t a5;
    uint32_t a6;
    uint32_t a7;
    uint32_t s0;
    uint32_t s1;
    uint32_t s2;
    uint32_t s3;
    uint32_t s4;
    uint32_t s5;
    uint32_t s6;
    uint32_t s7;
    uint32_t s8;
    uint32_t s9;
    uint32_t s10;
    uint32_t s11;
    uint32_t sp;
} __attribute__((packed));

#define READ_CSR(reg)                                                          \
    ({                                                                         \
        unsigned long __tmp;                                                   \
        __asm__ __volatile__("csrr %0, " #reg : "=r"(__tmp));                  \
        __tmp;                                                                 \
    })

#define WRITE_CSR(reg, value)                                                  \
    do {                                                                       \
        uint32_t __tmp = (value);                                              \
        __asm__ __volatile__("csrw " #reg ", %0" ::"r"(__tmp));                \
    } while (0)

#define PANIC(fmt, ...)                                                        \
    do {                                                                       \
        printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);  \
        while (1) {}                                                           \
    } while (0)

struct sbiret {
    long error;
    long value;
};