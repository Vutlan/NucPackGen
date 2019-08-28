#ifndef NUCPACK_H
#define NUCPACK_H

#include <stdint.h>

typedef enum {
    WRITE_ACTION=0,
    MODIFY_ACTION=1,
    ERASE_ACTION=2,
    VERIFY_ACTION=3,
    READ_ACTION=4,
    PACK_ACTION=5,
    FORMAT_ACTION=6
} ACTION_FLAG;

typedef struct __attribute__((__packed__)) _PACK_CHILD_HEAD
{
    uint32_t filelen;
    uint32_t startaddr;
    uint32_t imagetype;
    uint32_t eserve[1];
} PACK_CHILD_HEAD,*PPACK_CHILD_HEAD;

typedef struct __attribute__((__packed__)) _PACK_HEAD
{
    uint32_t actionFlag;
    uint32_t fileLength;
    uint32_t num;
    uint32_t reserve[1];
} PACK_HEAD,*PPACK_HEAD;

typedef enum
{
    DATA=0,
    ENV=1,
    UBOOT=2,
    PACK=3,
    IMAGE=4,

    PMTP=15
} PACK_ITEM_TYPE;

typedef struct __attribute__((__packed__)) _PACK_ITEM
{
    PACK_ITEM_TYPE type;
    char* name;
    uint32_t start;  // start address
    uint32_t exec;   // exec or start address
} PACK_ITEM,*PPACK_ITEM;


int nucpack_create(char* items_dir, PPACK_ITEM item, size_t item_sz, char* ddrinifile, char* outputfile);
int nucpack_repack(char* repack_file, char* ddr_ini_file, char* output_file);

#endif
