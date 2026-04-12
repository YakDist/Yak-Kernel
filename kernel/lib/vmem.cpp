#define VMEM_NAME_MAX 32

#define STATIC_BT_COUNT 128
#define BT_MINRESERVED  4

#define HASH_BUCKET_COUNT 16
#define FREELIST_COUNT    (sizeof(void *) * CHAR_BIT)

#define VMEM_QCACHE_MAX 16

struct VMem {
};
