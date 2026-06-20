/***************************************************************************
 *
 * Copyright 2015-2022 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifndef __BT_COMMON_DEFINE_H__
#define __BT_COMMON_DEFINE_H__
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "cmsis.h"
#include "hal_aud.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "bt_sys_config.h"

#include "debug_cfg.h"
#include "besbt_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Baisc Type
 */

typedef unsigned char  byte;
typedef unsigned char  uint8;
typedef unsigned char  uint8_t;
typedef signed   char  int8;
typedef unsigned short uint16;
typedef signed   short int16;
typedef unsigned int   uint32;
typedef signed   int   int32;

#ifndef BOOL_DEFINED
typedef unsigned int BOOL;
#endif

#ifndef __U32_TYPE
#define __U32_TYPE
typedef unsigned int U32;
#endif

#ifndef __U16_TYPE
#define __U16_TYPE
typedef unsigned short U16;
#endif

#ifndef __U8_TYPE
#define __U8_TYPE
typedef unsigned char U8;
#endif

typedef int S32;
typedef short S16;
typedef char S8;

#ifndef U32_PTR_DEFINED
typedef U32 U32_PTR;
#define U32_PTR_DEFINED
#endif

#ifndef __I32_TYPE
#define __I32_TYPE
typedef unsigned long I32;
#endif

#if XA_INTEGER_SIZE == 4
#ifndef __I16_TYPE
#define __I16_TYPE
typedef unsigned long I16;
#endif
#ifndef __I8_TYPE
#define __I8_TYPE
typedef unsigned long I8;
#endif

#elif XA_INTEGER_SIZE == 2
typedef unsigned short I16;
typedef unsigned short I8;
#elif XA_INTEGER_SIZE == 1
typedef unsigned short I16;
typedef unsigned char I8;
#else
#error No XA_INTEGER_SIZE specified!
#endif

typedef uint8_t UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TimeT
typedef U32 TimeT;
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAILURE
#define FAILURE 1
#endif

#ifndef INPROGRESS
#define INPROGRESS 2
#endif

#define OUTOFMEMORY 3

#define BUSYSTATUS 4

#define SENDFAILED 5

#define EINVAHNDLE 6

#define EINVACHNNL 7

#define EINVASTATE 8

#define CO_UINT8_VALUE(a) ((uint8_t)((a)&0xFF))

#define CO_SPLIT_128_UUID_LE(uuid_be) \
    CO_UINT8_VALUE(((uint8_t [16](uuid_be)))[15]), \
    CO_UINT8_VALUE(((uint8_t [16](uuid_be)))[14]), \
    CO_UINT8_VALUE(((uint8_t [16](uuid_be)))[13]), \
    CO_UINT8_VALUE(((uint8_t [16](uuid_be)))[12]), \
    CO_UINT8_VALUE(((uint8_t [16](uuid_be)))[11]), \
    CO_UINT8_VALUE(((uint8_t [16](uuid_be)))[10]), \
    CO_UINT8_VALUE(((uint8_t [16](uuid_be)))[9]), \
    CO_UINT8_VALUE(((uint8_t [16](uuid_be)))[8]), \
    CO_UINT8_VALUE(((uint8_t [16](uuid_be)))[7]), \
    CO_UINT8_VALUE(((uint8_t [16](uuid_be)))[6]), \
    CO_UINT8_VALUE(((uint8_t [16](uuid_be)))[5]), \
    CO_UINT8_VALUE(((uint8_t [16](uuid_be)))[4]), \
    CO_UINT8_VALUE(((uint8_t [16](uuid_be)))[3]), \
    CO_UINT8_VALUE(((uint8_t [16](uuid_be)))[2]), \
    CO_UINT8_VALUE(((uint8_t [16](uuid_be)))[1]), \
    CO_UINT8_VALUE(((uint8_t [16](uuid_be)))[0])

#define CO_SPLIT_UINT16_LE(a) \
    CO_UINT8_VALUE(a), CO_UINT8_VALUE((a)>>8)

#define CO_SPLIT_UINT32_LE(a) \
    CO_UINT8_VALUE(a), CO_UINT8_VALUE((a)>>8), \
    CO_UINT8_VALUE((a)>>16), CO_UINT8_VALUE((a)>>24)

#define CO_SPLIT_UINT24_LE(a) \
    CO_UINT8_VALUE(a), CO_UINT8_VALUE((a)>>8), \
    CO_UINT8_VALUE((a)>>16)

#define CO_COMBINE_UINT16_LE(p) \
    (CO_UINT8_VALUE(((uint8_t *)(p))[0]) | (((uint16_t)CO_UINT8_VALUE(((uint8_t *)(p))[1]))<<8))

#define CO_COMBINE_UINT24_LE(p) \
    (CO_UINT8_VALUE(((uint8_t *)(p))[0]) | (((uint16_t)CO_UINT8_VALUE(((uint8_t *)(p))[1]))<<8) | \
    (((uint16_t)CO_UINT8_VALUE(((uint8_t *)(p))[2]))<<16))

#define CO_COMBINE_UINT32_LE(p) \
    (CO_UINT8_VALUE(((uint8_t *)(p))[0]) | (((uint16_t)CO_UINT8_VALUE(((uint8_t *)(p))[1]))<<8) | \
    (((uint16_t)CO_UINT8_VALUE(((uint8_t *)(p))[2]))<<16) | (((uint16_t)CO_UINT8_VALUE(((uint8_t *)(p))[3]))<<24))

#define CO_SPLIT_UINT16_BE(a) \
    CO_UINT8_VALUE((a)>>8), CO_UINT8_VALUE(a)

#define CO_SPLIT_UINT24_BE(a) \
    CO_UINT8_VALUE((a)>>16), CO_UINT8_VALUE((a)>>8), \
    CO_UINT8_VALUE(a)

#define CO_SPLIT_UINT32_BE(a) \
    CO_UINT8_VALUE((a)>>24), CO_UINT8_VALUE((a)>>16), \
    CO_UINT8_VALUE((a)>>8), CO_UINT8_VALUE(a)

#define CO_COMBINE_UINT16_BE(p) \
    ((((uint16_t)CO_UINT8_VALUE(((uint8_t *)(p))[0]))<<8) | CO_UINT8_VALUE(((uint8_t *)(p))[1]))

#define CO_COMBINE_UINT24_BE(p) \
    ((((uint16_t)CO_UINT8_VALUE(((uint8_t *)(p))[0]))<<16) | (((uint16_t)CO_UINT8_VALUE(((uint8_t *)(p))[1]))<<8) |  \
    (((uint16_t)CO_UINT8_VALUE(((uint8_t *)(p))[2]))))

#define CO_COMBINE_UINT32_BE(p) \
    ((((uint16_t)CO_UINT8_VALUE(((uint8_t *)(p))[0]))<<24) | (((uint16_t)CO_UINT8_VALUE(((uint8_t *)(p))[1]))<<16) |  \
    (((uint16_t)CO_UINT8_VALUE(((uint8_t *)(p))[2]))<<8) | CO_UINT8_VALUE(((uint8_t *)(p))[3]))

static inline uint32_t co_round_size(uint32_t a)
{
    return ((a + 3) / 4) * 4;
}

static inline uint32_t co_range_value_map(uint32_t from_val, uint32_t from_min,
                                          uint32_t from_max, uint32_t to_min, uint32_t to_max)
{
    if (from_val <= from_min)
    {
        return to_min;
    }

    if (from_val >= from_max)
    {
        return to_max;
    }

    return ((from_val - from_min) * (to_max - to_min) + (from_max - from_min) / 2) / (from_max - from_min) + to_min;
}

#define CO_BIT_MASK(bit_index) (1 << bit_index)

#define CO_MIN_VALUE(a, b) ((a)<=(b) ? (a) : (b))

#define CO_MAX_VALUE(a, b) ((a)>(b) ? (a) : (b))

#define CO_UINT_TO_PTR(a) ((void *)(uintptr_t)(a))

/**
 * byte order
 */

#define HOST_OS_LITTLE_ENDIAN 1

#if HOST_OS_LITTLE_ENDIAN
static inline uint16_t co_host_to_uint16_le(uint16_t n) { return n; }
static inline uint16_t co_uint16_le_to_host(uint16_t n) { return n; }
static inline uint32_t co_host_to_uint32_le(uint32_t n) { return n; }
static inline uint32_t co_uint32_le_to_host(uint32_t n) { return n; }
static inline uint32_t co_host_to_uint24_le(uint32_t n) { return n; }
static inline uint32_t co_uint24_le_to_host(uint32_t n) { return n; }
#else
static inline uint16_t co_host_to_uint16_le(uint16_t n) {
    return ((n & 0xff) << 8) | (n >> 8);
}
static inline uint32_t co_host_to_uint32_le(uint32_t n) {
    return ((n & 0xff) << 24) | ((n & 0xff00) << 8) | ((n & 0xff0000) >> 8) | ((n & 0xff000000) >> 24);
}
static inline uint32_t co_host_to_uint24_le(uint32_t n) {
    return ((n << 16) & 0xFF0000) | (n &0xFF00) | ((n >> 16) & 0xFF);
}
static inline uint16_t co_uint16_le_to_host(uint16_t n) {
    return co_host_to_uint16_le(n);
}
static inline uint32_t co_uint32_le_to_host(uint32_t n) {
    return co_host_to_uint32_le(n);
}
static inline uint32_t co_uint24_le_to_host(uint32_t n) {
    return co_host_to_uint24_le(n);
}
#endif

#if HOST_OS_LITTLE_ENDIAN
static inline uint16_t co_host_to_uint16_be(uint16_t n) {
    return ((n & 0xff) << 8) | (n >> 8);
}
static inline uint32_t co_host_to_uint32_be(uint32_t n) {
    return ((n & 0xff) << 24) | ((n & 0xff00) << 8) | ((n & 0xff0000) >> 8) | ((n & 0xff000000) >> 24);
}
static inline uint32_t co_host_to_uint24_be(uint32_t n) {
    return ((n << 16) & 0xFF0000) | (n &0xFF00) | ((n >> 16) & 0xFF);
}
static inline uint16_t co_uint16_be_to_host(uint16_t n) {
    return co_host_to_uint16_be(n);
}
static inline uint32_t co_uint32_be_to_host(uint32_t n) {
    return co_host_to_uint32_be(n);
}
static inline uint32_t co_uint24_be_to_host(uint32_t n) {
    return co_host_to_uint24_be(n);
}
#else
static inline uint16_t co_host_to_uint16_be(uint16_t n) { return n; }
static inline uint16_t co_uint16_be_to_host(uint16_t n) { return n; }
static inline uint32_t co_host_to_uint32_be(uint32_t n) { return n; }
static inline uint32_t co_uint32_be_to_host(uint32_t n) { return n; }
static inline uint32_t co_host_to_uint24_be(uint32_t n) { return n; }
static inline uint32_t co_uint24_be_to_host(uint32_t n) { return n; }
#endif

static inline void co_host_uint16_to_ptr_le(uint16_t n, uint8_t *p) {
    p[0] = (uint8_t)(n & 0xFF);
    p[1] = (uint8_t)((n >> 8) & 0xFF);
}
static inline void co_host_uint32_to_ptr_le(uint32_t n, uint8_t *p) {
    p[0] = (uint8_t)(n & 0xFF);
    p[1] = (uint8_t)((n >> 8) & 0xFF);
    p[2] = (uint8_t)((n >> 16) & 0xFF);
    p[3] = (uint8_t)((n >> 24) & 0xFF);
}
static inline void co_host_uint16_to_ptr_be(uint16_t n, uint8_t *p) {
    p[1] = (uint8_t)(n & 0xFF);
    p[0] = (uint8_t)((n >> 8) & 0xFF);
}
static inline void co_host_uint32_to_ptr_be(uint32_t n, uint8_t *p) {
    p[3] = (uint8_t)(n & 0xFF);
    p[2] = (uint8_t)((n >> 8) & 0xFF);
    p[1] = (uint8_t)((n >> 16) & 0xFF);
    p[0] = (uint8_t)((n >> 24) & 0xFF);
}

/**
 * list entry
 */

typedef struct _list_entr {
    struct _list_entr *Flink;
    struct _list_entr *Blink;
    unsigned int resv;
} list_entry_t;

#define initialize_list_head(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead) )

#define initialize_list_entry(Entry) (\
    (Entry)->Flink = (Entry)->Blink = 0 )

#define is_entry_available(Entry) (\
    ((Entry)->Flink == 0))

#define is_list_empty(ListHead) (\
    ((ListHead)->Flink == (ListHead)))

#define get_head_list(ListHead) (ListHead)->Flink

#define get_tail_list(ListHead) (ListHead)->Blink

#define get_next_node(Node)     (Node)->Flink

#define get_prior_node(Node)    (Node)->Blink

#define is_node_connected(n) (((n)->Blink->Flink == (n)) && ((n)->Flink->Blink == (n)))
BOOL is_list_circular(list_entry_t * list);
BOOL btif_is_list_circular(list_entry_t * list);
void btif_insert_tail_list(list_entry_t * head, list_entry_t * entry);

#define list_assert(exp) ({ASSERT(exp, "%s %s, %d\n", #exp, __func__, __LINE__);})

void _insert_tail_list(list_entry_t * head, list_entry_t * entry);

#define insert_tail_list(a, b) (list_assert(btif_is_list_circular(a)), \
                            btif_insert_tail_list(a, b), \
                            list_assert(btif_is_list_circular(a)))

void insert_head_list(list_entry_t * head, list_entry_t * entry);
void _insert_head_list(list_entry_t * head, list_entry_t * entry);

#define insert_head_list(a, b) (list_assert(btif_is_list_circular(a)), \
                            _insert_head_list(a, b), \
                            list_assert(btif_is_list_circular(a)))

list_entry_t *remove_head_list(list_entry_t * head);
list_entry_t *_remove_head_list(list_entry_t * head);

#define remove_head_list(a) (list_assert(btif_is_list_circular(a)), \
                            _remove_head_list(a))

void remove_entry_list(list_entry_t * entry);
BOOL is_node_on_list(list_entry_t * head, list_entry_t * node);
U8 get_list_number(list_entry_t * head);
BOOL is_list_circular(list_entry_t * list);
void move_list(list_entry_t * dest, list_entry_t * src);

#define iterate_list_safe(head, cur, next, type) \
    for ( (cur) = (type) get_head_list(head) ; \
          (next) = (type) get_next_node(&(cur)->node), \
            (cur) != (type) (head); \
          (cur) = (next))

#define iterate_list(head, cur, type) \
    for ( (cur) = (type) get_head_list(&(head)) ; \
          (cur) != (type) &(head); \
          (cur) = (type) get_next_node(&(cur)->node) )

#ifndef OFFSETOF
#define OFFSETOF(type, member) ((unsigned int) &((type *)0)->member)
#endif

#ifndef CONTAINER_OF
#define CONTAINER_OF(ptr, type, member) ((type *)( (char *)ptr - OFFSETOF(type,member) ))
#endif

U32 be_to_host32(const U8* ptr);

#define STR_BE32(buff,num) ( (((U8*)buff)[0] = (U8) ((num)>>24)),  \
                              (((U8*)buff)[1] = (U8) ((num)>>16)),  \
                              (((U8*)buff)[2] = (U8) ((num)>>8)),   \
                              (((U8*)buff)[3] = (U8) (num)) )

#define STR_BE16(buff,num) ( (((U8*)buff)[0] = (U8) ((num)>>8)),    \
                              (((U8*)buff)[1] = (U8) (num)) )

#define BEtoHost16(ptr)  (U16)( ((U16) *((U8*)(ptr)) << 8) | \
                                ((U16) *((U8*)(ptr)+1)) )

#define BEtoHost32(ptr)  (U32)( ((U32) *((U8*)(ptr)) << 24)   | \
                                ((U32) *((U8*)(ptr)+1) << 16) | \
                                ((U32) *((U8*)(ptr)+2) << 8)  | \
                                ((U32) *((U8*)(ptr)+3)) )

/* Store value into a buffer in Little Endian format */
#define StoreLE16(buff,num) ( ((buff)[1] = (U8) ((num)>>8)),    \
                              ((buff)[0] = (U8) (num)) )

#define StoreLE32(buff,num) ( ((buff)[3] = (U8) ((num)>>24)),  \
                              ((buff)[2] = (U8) ((num)>>16)),  \
                              ((buff)[1] = (U8) ((num)>>8)),   \
                              ((buff)[0] = (U8) (num)) )

/* Store value into a buffer in Big Endian format */
#define StoreBE16(buff,num) ( ((buff)[0] = (U8) ((num)>>8)),    \
                              ((buff)[1] = (U8) (num)) )

#define StoreBE32(buff,num) ( ((buff)[0] = (U8) ((num)>>24)),  \
                              ((buff)[1] = (U8) ((num)>>16)),  \
                              ((buff)[2] = (U8) ((num)>>8)),   \
                              ((buff)[3] = (U8) (num)) )

#define LEtoHost16(ptr)  (U16)(((U16) *((U8*)(ptr)+1) << 8) | \
        (U16) *((U8*)(ptr)))

#define LETOHOST16(ptr)  (uint16_t)( ((uint16_t) *((uint8_t*)(ptr)+1) << 8) | \
                                ((uint16_t) *((uint8_t*)(ptr))) )

#define LETOHOST32(ptr)  (uint32_t)( ((uint32_t) *((uint8_t*)(ptr)+3) << 24) | \
                                ((uint32_t) *((uint8_t*)(ptr)+2) << 16) | \
                                ((uint32_t) *((uint8_t*)(ptr)+1) << 8)  | \
                                ((uint32_t) *((uint8_t*)(ptr))) )

/**
 * list node
 */

struct list_node {
    struct list_node *next;
    struct list_node *prev;
};

#define DEF_LIST_HEAD(head) \
    struct list_node head = { &(head), &(head) }

#define INIT_LIST_HEAD(head) do { \
    (head)->next = (head); (head)->prev = (head); \
} while (0)

void colist_addto_head(struct list_node *n, struct list_node *head);
void colist_addto_tail(struct list_node *n, struct list_node *head);
void colist_insert_after(struct list_node *n, struct list_node *head);
void colist_delete(struct list_node *entry);
int colist_is_node_on_list(struct list_node *list, struct list_node *node);
int colist_item_count(struct list_node *list);
struct list_node *colist_get_head(struct list_node *head);
int colist_is_list_empty(struct list_node *head);

#define colist_structure(ptr, type, member) \
    ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

#define colist_iterate(pos, head) \
    for (int __list__node__iter__cnt = ((pos = (head)->next), 0); \
         pos != (head); \
         (pos = pos->next), ITER_CHK(__list__node__iter__cnt))

#define colist_iterate_prev(pos, head) \
    for (int __list__node__iter__cnt = ((pos = (head)->prev), 0); \
         pos != (head); \
         (pos = pos->prev), ITER_CHK(__list__node__iter__cnt))

#define colist_iterate_safe(pos, n, head) \
    for (int __list__node__iter__cnt = ((pos = (head)->next), (n = pos->next), 0); \
         pos != (head); \
         (pos = n), (n = pos->next), ITER_CHK(__list__node__iter__cnt))

#define colist_iterate_entry(pos, type, head, member) \
    for (int __list__node__iter__cnt = ((pos = colist_structure((head)->next, type, member)), 0); \
         &pos->member != (head); \
         (pos = colist_structure(pos->member.next, type, member)), ITER_CHK(__list__node__iter__cnt))

#define colist_iterate_entry_safe(pos, n, type, head, member) \
    for (int __list__node__iter__cnt = ((pos = colist_structure((head)->next, type, member)), \
         (n = colist_structure(pos->member.next, type, member)), 0); \
         &pos->member != (head); \
         (pos = n), (n = colist_structure(n->member.next, type, member)), ITER_CHK(__list__node__iter__cnt))

static inline void ITERM_ASSERT(uintptr_t count)
{
    ASSERT(0, "list too much loop %d", count);
}

#define COLIST_MAX_NODES_ON_LIST 1500
#define ITER_CHK(count) (count>COLIST_MAX_NODES_ON_LIST?ITERM_ASSERT(count):(void)(count++))

struct single_link_node_t {
    struct single_link_node_t *next;
};

struct single_link_head_t {
    struct single_link_node_t head;
    struct single_link_node_t *tail;
    uint32_t size;
};

#define DEF_SINGLE_LINK_HEAD(head) \
    struct single_link_head_t head = {{(struct single_link_node_t *)&(head)}, (struct single_link_node_t *)&(head), 0}

#define INIT_SINGLE_LINK_HEAD(list) do { \
    (list)->tail = (list)->head.next = &(list)->head; (list)->size = 0; \
} while (0)

#define colist_single_link_iterate(pos, list) \
    for (uintptr_t __link__node_next_item = ((pos = (list)->head.next), (uintptr_t)(pos->next)), __list__node__iter__cnt = 0; \
         pos != &((list)->head); \
         (pos = (struct single_link_node_t *)__link__node_next_item), \
         (__link__node_next_item = (uintptr_t)(pos->next)), ITER_CHK(__list__node__iter__cnt))

#define colist_single_link_iterate_trace(tag, pos, list) \
    for (uintptr_t __link__node_next_item = ((pos = (list)->head.next), (uintptr_t)(pos->next)), __list__node__iter__cnt = 0; \
         (pos != &((list)->head)) ? (TRACE(0, tag " slink_list: %d %d %p ca=%x %d", (list)->size, __list__node__iter__cnt+1, pos, CO_LR_ADDRESS, __LINE__), true) : (false); \
         (pos = (struct single_link_node_t *)__link__node_next_item), \
         (__link__node_next_item = (uintptr_t)(pos->next)), ITER_CHK(__list__node__iter__cnt))

void colist_single_link_push_head(struct single_link_node_t *new_node, struct single_link_head_t *list);
void colist_single_link_push_tail(struct single_link_node_t *new_node, struct single_link_head_t *list);
void colist_single_link_push_after(struct single_link_node_t *new_node, struct single_link_node_t *exist_node, struct single_link_head_t *list);
void colist_single_link_delete_entry(struct single_link_head_t *list, struct single_link_node_t *entry);
struct single_link_node_t *colist_single_link_delete_entry_after(struct single_link_head_t *list, struct single_link_node_t *entry);
struct single_link_node_t *colist_single_link_delete_head(struct single_link_head_t *list);
int colist_single_link_is_node_on_list(struct single_link_head_t *list, struct single_link_node_t *node);
uint32_t colist_single_link_size(struct single_link_head_t *list);
struct single_link_node_t *colist_single_link_get_head(struct single_link_head_t *list);
struct single_link_node_t *colist_single_link_pop_head(struct single_link_head_t *list);
int colist_single_link_is_list_empty(struct single_link_head_t *list);

/**
 * bt packet
 */

#define BTIF_BTP_FLAG_NONE       0x0000  /* No current flags */
#define BTIF_BTP_FLAG_INUSE      0x0001  /* Used only by packet owner */
#define BTIF_BTP_FLAG_LSYSTEM    0x0002  /* Used only by L2CAP */
#define BTIF_BTP_FLAG_TAIL       0x0004  /* Used only by L2CAP Applications */
#define BTIF_BTP_FLAG_RDEV       0x0008  /* Used only by L2CAP */
#define BTIF_BTP_FLAG_FCS        0x0010  /* FCS field is valid, set only by L2CAP */
#define BTIF_BTP_FLAG_NON_FLUSH  0x0020  /* Used by L2CAP, HCI or packet owner */
#define BTIF_BTP_FLAG_ENHANCED   0x0040  /* Used only by L2CAP */
#define BTIF_BTP_FLAG_SEGMENTED  0x0080  /* Used only by L2CAP */
#define BTIF_BTP_FLAG_TXDONE     0x0100  /* Used only by L2CAP */
#define BTIF_BTP_FLAG_USER       0x0200  /* Used only by L2CAP */
#define BTIF_BTP_FLAG_IMMEDIATE  0x0400  /* Used only by L2CAP */

#define BTIF_BT_PACKET_HEADER_LEN 25

typedef struct btif_bt_packet_t {
    list_entry_t node;
    uint8_t *data;              /* Points to a buffer of user data.  */
    uint16_t dataLen;           /* Indicates the length of "data" in bytes. */
    uint16_t flags;             /* Must be initialized to BTIF_BTP_FLAG_NONE by
                                 * applications running on top of L2CAP.
                                 */

    uint8_t l2cap_priority;

    /* Group: The following fields are for internal use only by the stack. */
    void *ulpContext;
    uint8_t *tail;
    uint16_t tailLen;

    uint16_t llpContext;
    uint16_t remoteCid;

    uint8_t hciPackets;
    uint8_t headerLen;
    uint8_t header[BTIF_BT_PACKET_HEADER_LEN];
} btif_bt_packet_t;

/**
 * AUD_ID_ENUM
 */

typedef enum {
    AUD_ID_INVALID  = -1,
    AUD_ID_POWER_ON = 0x0,
    AUD_ID_POWER_OFF = 0x01,
    AUD_ID_LANGUAGE_SWITCH = 0x02,

    AUD_ID_NUM_0 = 0x03,
    AUD_ID_NUM_1 = 0x04,
    AUD_ID_NUM_2 = 0x05,
    AUD_ID_NUM_3 = 0x06,
    AUD_ID_NUM_4 = 0x07,
    AUD_ID_NUM_5 = 0x08,
    AUD_ID_NUM_6 = 0x09,
    AUD_ID_NUM_7 = 0x0A,
    AUD_ID_NUM_8 = 0x0B,
    AUD_ID_NUM_9 = 0x0C,

    AUD_ID_BT_PAIR_ENABLE = 0x0D,
    AUD_ID_BT_PAIRING = 0x0E,
    AUD_ID_BT_PAIRING_SUC = 0x0F,
    AUD_ID_BT_PAIRING_FAIL = 0x10,
    AUD_ID_BT_CALL_REFUSE = 0x11,
    AUD_ID_BT_CALL_OVER = 0x12,
    AUD_ID_BT_CALL_ANSWER = 0x13,
    AUD_ID_BT_CALL_HUNG_UP = 0x14,
    AUD_ID_BT_CALL_INCOMING_CALL = 0x15,
    AUD_ID_BT_CALL_INCOMING_NUMBER = 0x16,
    AUD_ID_BT_CHARGE_PLEASE = 0x17,
    AUD_ID_BT_CHARGE_FINISH = 0x18,
    AUD_ID_BT_CLEAR_SUCCESS = 0x19,
    AUD_ID_BT_CLEAR_FAIL = 0x1A,
    AUD_ID_BT_CONNECTED = 0x1B,
    AUD_ID_BT_DIS_CONNECT = 0x1C,
    AUD_ID_BT_WARNING = 0x1D,
    AUDIO_ID_BT_ALEXA_START = 0x1E,
    AUDIO_ID_FIND_MY_BUDS = 0x1F,
    AUDIO_ID_FIND_TILE = 0x20,
    AUDIO_ID_BT_ALEXA_STOP = 0x21,
    AUDIO_ID_BT_GSOUND_MIC_OPEN = 0x22,
    AUDIO_ID_BT_GSOUND_MIC_CLOSE = 0x23,
    AUDIO_ID_BT_GSOUND_NC = 0x24,
    AUDIO_ID_BT_MUTE = 0x25,
    AUD_ID_RING_WARNING = 0x26,
#ifdef __INTERACTION__
    AUD_ID_BT_FINDME = 0x27,
#endif
    AUD_ID_ANC_PROMPT = 0x28,
    AUD_ID_ANC_MUTE = 0x29,
    AUD_ID_ANC_MODE0 = 0x2A,
    AUD_ID_ANC_MODE1 = 0x2B,
#if BLE_AUDIO_ENABLED
    AUD_ID_LE_AUD_INCOMING_CALL = 0x2C,
#endif
#ifdef SLT_AUTO_TEST
    AUD_ID_TONE_1K = 0x2D,
#endif
    AUD_ID_CUSTOM_LEAK_DETECT = 0x2E,
    MAX_RECORD_NUM = 0x2F,
    AUD_ID_WT_FIND_DIVICE = 0x32,//walkie build process find divce
    AUD_ID_WT_ERROR = 0x3f,//walkie build process error
	AUD_ID_OPENEAR_PROMPT = 0x40,

    AUD_ID_ENUM_BOTTOM = 0x10000000,    // AUD_ID_ENUM is uint32_t
} AUD_ID_ENUM;

const char *aud_id2str(uint16_t aud_id);

/**
 * BLE Debug
 */

const char *DebugMask2Prefix(uint8_t mask);

#define DEBUG_PREFIX_FORMAT         "%s<%s> "
#define DEBUG_SUFFIX_FORMAT         "\n"

#define LOG_HCI_TAG  "[BLE HCI]: "
#define LOG_L2C_TAG  "[BLE L2C]: "
#define LOG_ATT_TAG  "[BLE ATT]: "
#define LOG_GATT_TAG "[BLE GATT]: "
#define LOG_SMP_TAG  "[BLE SMP]: "

#define LOG_GAP_TAG "[BLE GAP]: "
#define LOG_APP_TAG "[BLE APP]: "
#define LOG_BLE_TAG "[BLE]: "
#define LOG_HTP_TAG "[BLE HTP]: "

#define GAP_ERROR   1
#define GAP_OUT     2
#define GATT_ERROR  3
#define GATT_OUT    4
#define ATT_ERROR   5
#define ATT_OUT     6
#define L2C_ERROR   7
#define L2C_OUT     8
#define HCI_ERROR   9
#define HCI_OUT     10
#define SMP_ERROR   11
#define SMP_OUT     12
#define APP_ERROR   13
#define APP_OUT     14
#define PRF_HT_ERROR    15
#define PRF_HT_OUT      16
#define BLE_ERROR       30
#define BLE_OUT         31

#define DebugOut(mask, str, ...)                             \
    do                                                       \
    {                                                        \
        const char *prefix = NULL;                           \
        prefix             = DebugMask2Prefix(mask);         \
        TRACE(1, DEBUG_PREFIX_FORMAT, prefix, __FUNCTION__); \
        TRACE(1, str, ##__VA_ARGS__);                        \
    } while (0)

#if DEBUG_BLE_HCI_DBG
#define BLE_HCI_DBG(str,...)     DebugOut(HCI_OUT, str, ##__VA_ARGS__)
#define BLE_HCI_ERR(str,...)     DebugOut(HCI_ERROR, str, ##__VA_ARGS__)
#define BLE_HCI_FUNC_ENTER()     TRACE(LOG_BLE_TAG"%s line: %d +++\n", __FUNCTION__, __LINE__)
#define BLE_HCI_FUNC_LEAVE()     TRACE(LOG_BLE_TAG"%s line: %d ---\n", __FUNCTION__, __LINE__)
#else
#define BLE_HCI_DBG(str,...)
#define BLE_HCI_ERR(str,...)     DebugOut(HCI_ERROR, str, ##__VA_ARGS__)
#define BLE_HCI_FUNC_ENTER()
#define BLE_HCI_FUNC_LEAVE()
#endif

#if DEBUG_BLE_GAP_DBG
#define BLE_GAP_DBG(str,...)     DebugOut(GAP_OUT, str, ##__VA_ARGS__)
#define BLE_GAP_ERR(str,...)     DebugOut(GAP_ERROR, str, ##__VA_ARGS__)
#define BLE_GAP_FUNC_ENTER()     TRACE(LOG_GAP_TAG"%s line: %d +++\n", __FUNCTION__, __LINE__)
#define BLE_GAP_FUNC_LEAVE()     TRACE(LOG_GAP_TAG"%s line: %d ---\n", __FUNCTION__, __LINE__)
#else
#define BLE_GAP_DBG(str,...)
#define BLE_GAP_ERR(str,...)     DebugOut(GAP_ERROR, str, ##__VA_ARGS__)
#define BLE_GAP_FUNC_ENTER()
#define BLE_GAP_FUNC_LEAVE()
#endif

#if DEBUG_BLE_GATT_DBG
#define BLE_GATT_DBG(str,...)    DebugOut(GATT_OUT, str, ##__VA_ARGS__)
#define BLE_GATT_ERR(str,...)    DebugOut(GATT_ERROR, str, ##__VA_ARGS__)
#define BLE_GATT_FUNC_ENTER()    TRACE(LOG_GATT_TAG"%s line: %d +++\n", __FUNCTION__, __LINE__)
#define BLE_GATT_FUNC_LEAVE()    TRACE(LOG_GATT_TAG"%s line: %d ---\n", __FUNCTION__, __LINE__)
#else
#define BLE_GATT_DBG(str,...)
#define BLE_GATT_ERR(str,...)    DebugOut(GATT_ERROR, str, ##__VA_ARGS__)
#define BLE_GATT_FUNC_ENTER()
#define BLE_GATT_FUNC_LEAVE()
#endif

#if DEBUG_BLE_ATT_DBG
#define BLE_ATT_DBG(str,...)     DebugOut(ATT_OUT, str, ##__VA_ARGS__)
#define BLE_ATT_ERR(str,...)     DebugOut(ATT_ERROR, str, ##__VA_ARGS__)
#define BLE_ATT_FUNC_ENTER()     TRACE(LOG_ATT_TAG"%s line: %d +++\n", __FUNCTION__, __LINE__)
#define BLE_ATT_FUNC_LEAVE()     TRACE(LOG_ATT_TAG"%s line: %d ---\n", __FUNCTION__, __LINE__)
#else
#define BLE_ATT_DBG(str,...)
#define BLE_ATT_ERR(str,...)     DebugOut(ATT_ERROR, str, ##__VA_ARGS__)
#define BLE_ATT_FUNC_ENTER()
#define BLE_ATT_FUNC_LEAVE()
#endif

#if DEBUG_BLE_L2C_DBG
#define BLE_L2C_DBG(str,...)     DebugOut(L2C_OUT, str, ##__VA_ARGS__)
#define BLE_L2C_ERR(str,...)     DebugOut(L2C_ERROR, str, ##__VA_ARGS__)
#define BLE_L2C_FUNC_ENTER()     TRACE(LOG_L2C_TAG"%s line: %d +++\n", __FUNCTION__, __LINE__)
#define BLE_L2C_FUNC_LEAVE()     TRACE(LOG_L2C_TAG"%s line: %d ---\n", __FUNCTION__, __LINE__)
#else
#define BLE_L2C_DBG(str,...)
#define BLE_L2C_ERR(str,...)     DebugOut(L2C_ERROR, str, ##__VA_ARGS__)
#define BLE_L2C_FUNC_ENTER()
#define BLE_L2C_FUNC_LEAVE()
#endif

#if DEBUG_BLE_SMP_DBG
#define BLE_SMP_DBG(str,...)     DebugOut(SMP_OUT, str, ##__VA_ARGS__)
#define BLE_SMP_ERR(str,...)     DebugOut(SMP_ERROR, str, ##__VA_ARGS__)
#define BLE_SMP_FUNC_ENTER()     TRACE(LOG_SMP_TAG"%s line: %d +++\n", __FUNCTION__, __LINE__)
#define BLE_SMP_FUNC_LEAVE()     TRACE(LOG_SMP_TAG"%s line: %d ---\n", __FUNCTION__, __LINE__)
#else
#define BLE_SMP_DBG(str,...) 
#define BLE_SMP_ERR(str,...)     DebugOut(SMP_ERROR, str, ##__VA_ARGS__)
#define BLE_SMP_FUNC_ENTER()
#define BLE_SMP_FUNC_LEAVE()
#endif

#if DEBUG_BLE_APP_DBG
#define BLE_APP_DBG(str,...)     DebugOut(APP_OUT, str, ##__VA_ARGS__)
#define BLE_APP_ERR(str,...)     DebugOut(APP_ERROR, str, ##__VA_ARGS__)
#define BLE_APP_FUNC_ENTER()     TRACE(LOG_APP_TAG"%s line: %d +++\n", __FUNCTION__, __LINE__)
#define BLE_APP_FUNC_LEAVE()     TRACE(LOG_APP_TAG"%s line: %d ---\n", __FUNCTION__, __LINE__)
#else
#define BLE_APP_DBG(str,...)
#define BLE_APP_ERR(str,...)     DebugOut(APP_ERROR, str, ##__VA_ARGS__)
#define BLE_APP_FUNC_ENTER()
#define BLE_APP_FUNC_LEAVE()
#endif

#if DEBUG_BLE_PRF_DBG
#define BLE_PRF_HP_DBG(str,...)  DebugOut(PRF_HT_OUT, str, ##__VA_ARGS__)
#define BLE_PRF_HP_ERR(str,...)  DebugOut(PRF_HT_ERROR, str, ##__VA_ARGS__)
#define BLE_PRF_HP_FUNC_ENTER()  TRACE(LOG_HTP_TAG"%s line: %d +++\n", __FUNCTION__, __LINE__)
#define BLE_PRF_HP_FUNC_LEAVE()  TRACE(LOG_HTP_TAG"%s line: %d ---\n", __FUNCTION__, __LINE__)
#else
#define BLE_PRF_HP_DBG(str,...)
#define BLE_PRF_HP_ERR(str,...)  DebugOut(PRF_HT_ERROR, str, ##__VA_ARGS__)
#define BLE_PRF_HP_FUNC_ENTER()
#define BLE_PRF_HP_FUNC_LEAVE()
#endif

#if DEBUG_BLE_BLE_DBG
#define BLE_DBG(str,...)         DebugOut(BLE_OUT, str, ##__VA_ARGS__)
#define BLE_ERR(str,...)         DebugOut(BLE_ERROR, str, ##__VA_ARGS__)
#define BLE_FUNC_ENTER()         TRACE(LOG_BLE_TAG"%s line: %d +++\n", __FUNCTION__, __LINE__)
#define BLE_FUNC_LEAVE()         TRACE(LOG_BLE_TAG"%s line: %d ---\n", __FUNCTION__, __LINE__)
#define BLE_DUMP8(x,y,z)         DUMP8(x,y,z)
#else
#define BLE_DBG(str,...)
#define BLE_ERR(str,...)         DebugOut(BLE_ERROR, str, ##__VA_ARGS__)
#define BLE_FUNC_ENTER() 
#define BLE_FUNC_LEAVE()
#define BLE_DUMP8(x,y,z)
#endif

/**
 * error code
 */

typedef uint8_t btif_error_code_t;

#define BTIF_BEC_NO_ERROR             0x00
#define BTIF_BEC_UNKNOWN_HCI_CMD      0x01
#define BTIF_BEC_NO_CONNECTION        0x02
#define BTIF_BEC_HARDWARE_FAILURE     0x03
#define BTIF_BEC_PAGE_TIMEOUT         0x04
#define BTIF_BEC_AUTHENTICATE_FAILURE 0x05
#define BTIF_BEC_MISSING_KEY          0x06
#define BTIF_BEC_MEMORY_FULL          0x07
#define BTIF_BEC_CONNECTION_TIMEOUT   0x08
#define BTIF_BEC_MAX_CONNECTIONS      0x09
#define BTIF_BEC_MAX_SCO_CONNECTIONS  0x0a
#define BTIF_BEC_ACL_ALREADY_EXISTS   0x0b
#define BTIF_BEC_COMMAND_DISALLOWED   0x0c
#define BTIF_BEC_LIMITED_RESOURCE     0x0d
#define BTIF_BEC_SECURITY_ERROR       0x0e
#define BTIF_BEC_PERSONAL_DEVICE      0x0f
#define BTIF_BEC_HOST_TIMEOUT         0x10
#define BTIF_BEC_UNSUPPORTED_FEATURE  0x11
#define BTIF_BEC_INVALID_HCI_PARM     0x12
#define BTIF_BEC_USER_TERMINATED      0x13
#define BTIF_BEC_LOW_RESOURCES        0x14
#define BTIF_BEC_POWER_OFF            0x15
#define BTIF_BEC_LOCAL_TERMINATED     0x16
#define BTIF_BEC_REPEATED_ATTEMPTS    0x17
#define BTIF_BEC_PAIRING_NOT_ALLOWED  0x18
#define BTIF_BEC_UNKNOWN_LMP_PDU      0x19
#define BTIF_BEC_UNSUPPORTED_REMOTE   0x1a
#define BTIF_BEC_SCO_OFFSET_REJECT    0x1b
#define BTIF_BEC_SCO_INTERVAL_REJECT  0x1c
#define BTIF_BEC_SCO_AIR_MODE_REJECT  0x1d
#define BTIF_BEC_INVALID_LMP_PARM     0x1e
#define BTIF_BEC_UNSPECIFIED_ERR      0x1f
#define BTIF_BEC_UNSUPPORTED_LMP_PARM 0x20
#define BTIF_BEC_ROLE_CHG_NOT_ALLOWED 0x21
#define BTIF_BEC_LMP_RESPONSE_TIMEOUT 0x22
#define BTIF_BEC_LMP_TRANS_COLLISION  0x23
#define BTIF_BEC_LMP_PDU_NOT_ALLOWED  0x24
#define BTIF_BEC_ENCRYP_MODE_NOT_ACC  0x25
#define BTIF_BEC_UNIT_KEY_USED        0x26
#define BTIF_BEC_QOS_NOT_SUPPORTED    0x27
#define BTIF_BEC_INSTANT_PASSED       0x28
#define BTIF_BEC_PAIR_UNITKEY_NO_SUPP 0x29
#define BTIF_BEC_NOT_FOUND            0xf1
#define BTIF_BEC_REQUEST_CANCELLED    0xf2
#define BTIF_BEC_INVALID_SDP_PDU      0xd1
#define BTIF_BEC_SDP_DISCONNECT       0xd2
#define BTIF_BEC_SDP_NO_RESOURCES     0xd3
#define BTIF_BEC_SDP_INTERNAL_ERR     0xd4
#define BTIF_BEC_STORE_LINK_KEY_ERR   0xe0
#define BTIF_BEC_BT_LINK_REAL_DISCONNECTED 0xb8
#define BTIF_BEC_BT_CANCEL_PAGE       0xb9

//BES vendor error code
#define BT_ECODE_DISCONNECT_ITSELF      0xba
#define BT_ECODE_IBRT_SLAVE_CLEANUP     0xbb
#define BT_ECODE_SDP_OPEN_TIMEOUT       0xbc
#define BT_ECODE_SDP_ClIENT_TX_TIMEOUT  0xbd

#define BT_CONN_HANDLE_MASK         0x0FFF

/**
 * bt address
 */

#define BTIF_BD_ADDR_SIZE   6
#define BTIF_LINK_KEY_SIZE  16

typedef struct {
    uint8_t addr[BTIF_BD_ADDR_SIZE];
} __attribute__((packed)) bd_addr_t;

struct bdaddr_t {
    uint8_t address[BTIF_BD_ADDR_SIZE];
}__attribute__ ((packed));

typedef struct bdaddr_t bt_bdaddr_t;

typedef enum {
    BT_ADDR_TYPE_PUBLIC = 0x00,
    BT_ADDR_TYPE_RANDOM = 0x01,
    BT_ADDR_TYPE_PUB_IA = 0x02,
    BT_ADDR_TYPE_RND_IA = 0x03,
} bt_addr_type_t;

typedef struct ble_bdaddr_t
{
    uint8_t addr[BTIF_BD_ADDR_SIZE];
    uint8_t addr_type;
} __attribute__ ((packed)) ble_bdaddr_t;

typedef enum {
    HCI_CONN_TYPE_INVALID = 0,
    HCI_CONN_TYPE_BT_ACL,
    HCI_CONN_TYPE_BT_SCO,
    HCI_CONN_TYPE_LE_ACL,
    HCI_CONN_TYPE_LE_ISO,
} hci_conn_type_t;

/**
 * bt device
 */

#ifndef BT_DEVICE_NUM
#if defined(IBRT_UI)
#define BT_DEVICE_NUM 2
#elif defined(__BT_ONE_BRING_TWO__)
#define BT_DEVICE_NUM 2
#else
#define BT_DEVICE_NUM 1
#endif
#endif

enum BT_DEVICE_ID_T {
    BT_DEVICE_ID_1 = 0,
    BT_DEVICE_ID_2 = 1,
    BT_DEVICE_ID_3 = 2,
    BT_DEVICE_ID_N = BT_DEVICE_NUM,
    BT_DEVICE_TWS_ID = 0x0f,
    BT_DEVICE_AUTO_CHOICE_ID = 0xee,
    BT_DEVICE_SEND_AVRCP_PLAY = 0xf0,
    BT_DEVICE_INVALID_ID = 0xff,
};

#define BLE_DEVICE_ID_BASE 0x30
#define BLE_DEVICE_ID_1 (BLE_DEVICE_ID_BASE + 0)
#define BLE_DEVICE_ID_2 (BLE_DEVICE_ID_BASE + 1)
#define BLE_DEVICE_ID_3 (BLE_DEVICE_ID_BASE + 2)

#ifdef BT_SOURCE
#if defined(BT_MULTI_SOURCE)
#define BT_SOURCE_DEVICE_NUM 2
#else
#define BT_SOURCE_DEVICE_NUM 1
#endif
#endif

#ifndef BT_SOURCE_DEVICE_NUM
#define BT_SOURCE_DEVICE_NUM 0
#endif

#define BT_SOURCE_DEVICE_ID_BASE 0x10

#define BT_INVALID_CONN_HANDLE 0xFFFF

enum BT_SOURCE_DEVICE_ID_T {
    BT_SOURCE_DEVICE_ID_1 = BT_SOURCE_DEVICE_ID_BASE,
    BT_SOURCE_DEVICE_ID_2 = BT_SOURCE_DEVICE_ID_BASE + 1,
    BT_SOURCE_DEVICE_ID_N = BT_SOURCE_DEVICE_ID_BASE + BT_SOURCE_DEVICE_NUM,
    BT_SOURCE_DEVICE_INVALID_ID = 0xff,
};

#define GAP_LE_CONN_ID_PREFIX 0x8000
#define GAP_LE_CONN_ID_1 (GAP_LE_CONN_ID_PREFIX|BLE_DEVICE_ID_1)
#define GAP_LE_CONN_ID_2 (GAP_LE_CONN_ID_PREFIX|BLE_DEVICE_ID_2)
#define GAP_LE_CONN_ID_3 (GAP_LE_CONN_ID_PREFIX|BLE_DEVICE_ID_3)

#define GAP_BT_CONN_ID_PREFIX 0x4000
#define GAP_BT_CONN_ID_1 (GAP_BT_CONN_ID_PREFIX|BT_DEVICE_ID_1)
#define GAP_BT_CONN_ID_2 (GAP_BT_CONN_ID_PREFIX|BT_DEVICE_ID_2)
#define GAP_BT_CONN_ID_3 (GAP_BT_CONN_ID_PREFIX|BT_DEVICE_ID_3)

#ifndef BTM_MAX_LINK_NUMS
#define BTM_MAX_LINK_NUMS      3
#endif

#ifndef BT_ACL_MAX_LINK_NUMS
#if defined(IBRT_UI) || defined(BT_MULTI_SOURCE)
#define BT_ACL_MAX_LINK_NUMS   (BTM_MAX_LINK_NUMS)
#else
#define BT_ACL_MAX_LINK_NUMS   (BTM_MAX_LINK_NUMS - 1)
#endif
#endif

#define Plt_Assert ASSERT
#define Plt_TICKS_TO_MS(ticks) TICKS_TO_MS(ticks)
#define Plt_DUMP8 DUMP8

#ifdef MOUDLE
#define CO_LOG_MODULE BT_DBG_CONCAT(MOUDLE, _MODULE)
#else
#define CO_LOG_MODULE NULL_MODULE
#endif

typedef union {
    uint32_t data;
    struct {
    uint32_t module_id  : 6;    // max 1 ~ 63
    uint32_t line       : 14;   // max 16383
    uint32_t status     : 9;    // status code
    uint32_t log_level  : 3;    // max 7
    } s;
} bt_log_mod_t;

#define CO_GEN_LOG_MODULE(log_level, status) ((uint32_t)(\
        (((CO_LOG_MODULE) & 0x3F) << 0) | \
        (((__LINE__) & 0x3FFF) << 6) | \
        (((status) & 0x1FF) << 20) | \
        (((log_level) & 0x7) << 29) \
    ))

typedef struct {
    char str[4];
} bt_log_tag_t;

#ifdef BT_LOG_SIMPLIFY
void co_func_log_trace_simplify(uint32_t mod);
#define co_func_log_trace_0(mod) co_func_log_trace_simplify(mod)
#define co_func_log_trace_1(mod, a) co_func_log_trace_simplify(mod)
#define co_func_log_trace_2(mod, a, b) co_func_log_trace_simplify(mod)
#define co_func_log_trace_3(mod, a, b, c) co_func_log_trace_simplify(mod)
#define co_func_log_trace_4(mod, a, b, c, d) co_func_log_trace_simplify(mod)
#else
void co_func_log_trace_0(uint32_t mod);
void co_func_log_trace_1(uint32_t mod, uintptr_t a);
void co_func_log_trace_2(uint32_t mod, uintptr_t a, uintptr_t b);
void co_func_log_trace_3(uint32_t mod, uintptr_t a, uintptr_t b, uintptr_t c);
void co_func_log_trace_4(uint32_t mod, uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d);
#endif
void co_func_log_str_0(uint32_t mod, const char *s);
void co_func_log_str_1(uint32_t mod, const char *s, uintptr_t a);
void co_func_log_str_2(uint32_t mod, const char *s, uintptr_t a, uintptr_t b);
void co_func_log_str_3(uint32_t mod, const char *s, uintptr_t a, uintptr_t b, uintptr_t c);
void co_func_log_trace_with_tag_0(uint32_t mod, bt_log_tag_t tag);
void co_func_log_trace_with_tag_1(uint32_t mod, bt_log_tag_t tag, uintptr_t a);
void co_func_log_trace_with_tag_2(uint32_t mod, bt_log_tag_t tag, uintptr_t a, uintptr_t b);
void co_func_log_trace_with_tag_3(uint32_t mod, bt_log_tag_t tag, uintptr_t a, uintptr_t b, uintptr_t c);
const char *co_log_module_str(void);

#define CO_LR_ADDRESS (uintptr_t)__builtin_return_address(0)

#define CO_ASSERT_0(e) { if (!(e)) { \
        co_func_log_trace_1(CO_GEN_LOG_MODULE(DBG_ASSERT_LEVEL, 0), CO_LR_ADDRESS); \
        Plt_Assert(0, co_log_module_str(), CO_LOG_MODULE, __LINE__, CO_LR_ADDRESS); \
    }}

#define CO_ASSERT_1(e, a) { if (!(e)) { \
        co_func_log_trace_2(CO_GEN_LOG_MODULE(DBG_ASSERT_LEVEL, 0), (uintptr_t)(a), CO_LR_ADDRESS); \
        Plt_Assert(0, co_log_module_str(), CO_LOG_MODULE, __LINE__, CO_LR_ADDRESS); \
    }}

#define CO_ASSERT_2(e, a, b) { if (!(e)) { \
        co_func_log_trace_3(CO_GEN_LOG_MODULE(DBG_ASSERT_LEVEL, 0), (uintptr_t)(a), (uintptr_t)(b), CO_LR_ADDRESS); \
        Plt_Assert(0, co_log_module_str(), CO_LOG_MODULE, __LINE__, CO_LR_ADDRESS); \
    }}

#define CO_ASSERT_3(e, a, b, c) { if (!(e)) { \
        co_func_log_trace_4(CO_GEN_LOG_MODULE(DBG_ASSERT_LEVEL, 0), (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c), CO_LR_ADDRESS); \
        Plt_Assert(0, co_log_module_str(), CO_LOG_MODULE, __LINE__, CO_LR_ADDRESS); \
    }}

#define CO_LOG_MAIN_0(status) \
    co_func_log_trace_0(CO_GEN_LOG_MODULE(DBG_MAIN_FLOW_LEVEL, (status)))
#define CO_LOG_MAIN_1(status, a) \
    co_func_log_trace_1(CO_GEN_LOG_MODULE(DBG_MAIN_FLOW_LEVEL, (status)), (uintptr_t)(a))
#define CO_LOG_MAIN_2(status, a, b) \
    co_func_log_trace_2(CO_GEN_LOG_MODULE(DBG_MAIN_FLOW_LEVEL, (status)), (uintptr_t)(a), (uintptr_t)(b))
#define CO_LOG_MAIN_3(status, a, b, c) \
    co_func_log_trace_3(CO_GEN_LOG_MODULE(DBG_MAIN_FLOW_LEVEL, (status)), (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c))
#define CO_LOG_MAIN_4(status, a, b, c, d) \
    co_func_log_trace_4(CO_GEN_LOG_MODULE(DBG_MAIN_FLOW_LEVEL, (status)), (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c), (uintptr_t)(d))
#define CO_LOG_MAIN_S_0(status, tag) \
    co_func_log_trace_with_tag_0(CO_GEN_LOG_MODULE(DBG_MAIN_FLOW_LEVEL, (status)), (bt_log_tag_t){#tag})
#define CO_LOG_MAIN_S_1(status, tag, a) \
    co_func_log_trace_with_tag_1(CO_GEN_LOG_MODULE(DBG_MAIN_FLOW_LEVEL, (status)), (bt_log_tag_t){#tag}, (uintptr_t)(a))
#define CO_LOG_MAIN_S_2(status, tag, a, b) \
    co_func_log_trace_with_tag_2(CO_GEN_LOG_MODULE(DBG_MAIN_FLOW_LEVEL, (status)), (bt_log_tag_t){#tag}, (uintptr_t)(a), (uintptr_t)(b))
#define CO_LOG_MAIN_S_3(status, tag, a, b, c) \
    co_func_log_trace_with_tag_3(CO_GEN_LOG_MODULE(DBG_MAIN_FLOW_LEVEL, (status)), (bt_log_tag_t){#tag}, (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c))
#define CO_LOG_MAIN_WITH_STR_0(status, s) \
    co_func_log_str_0(CO_GEN_LOG_MODULE(DBG_MAIN_FLOW_LEVEL, (status)), (s))
#define CO_LOG_MAIN_WITH_STR_1(status, s, a) \
    co_func_log_str_1(CO_GEN_LOG_MODULE(DBG_MAIN_FLOW_LEVEL, (status)), (s), (uintptr_t)(a))
#define CO_LOG_MAIN_WITH_STR_2(status, s, a, b) \
    co_func_log_str_2(CO_GEN_LOG_MODULE(DBG_MAIN_FLOW_LEVEL, (status)), (s), (uintptr_t)(a), (uintptr_t)(b))
#define CO_LOG_MAIN_WITH_STR_3(status, s, a, b, c) \
    co_func_log_str_3(CO_GEN_LOG_MODULE(DBG_MAIN_FLOW_LEVEL, (status)), (s), (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c))

/**
 * Usage:
 *
 * 0. make sure globle cfg control DBG_DEBUG_PRINT_ENABLE is 1
 *
 * 1. set your  module in your source code file at the top(MUST).
 *      #undef MOUDLE
 *      #define MOUDLE BB
 *
 * 2. define the module debug level in debug_cfg.h,
 *      NOTICE,if no define, then no print
 *      #define BB_LEVEL INFO_LEVEL
 *
 * 3. use DEBUG_INFO,DEBUG_WARNING,and DEBUG_ERROR in your module's codes
 *
 */

#if DBG_DEBUG_PRINT_ENABLE == 1

#define DEBUGPRINT2(num,...) TRACE(num,__VA_ARGS__)
#define CO_DEBUG_PRINTF(num,...) do { DEBUGPRINT2(num,__VA_ARGS__); } while(0);

#if (BT_DBG_CONCAT(MOUDLE, _LEVEL) >= DBG_ERROR_LEVEL)
#define DEBUG_ERROR(num,...) CO_DEBUG_PRINTF(num, __VA_ARGS__)
#define CO_LOG_ERR_0(status) \
    co_func_log_trace_0(CO_GEN_LOG_MODULE(DBG_ERROR_LEVEL, (status)))
#define CO_LOG_ERR_1(status, a) \
    co_func_log_trace_1(CO_GEN_LOG_MODULE(DBG_ERROR_LEVEL, (status)), (uintptr_t)(a))
#define CO_LOG_ERR_2(status, a, b) \
    co_func_log_trace_2(CO_GEN_LOG_MODULE(DBG_ERROR_LEVEL, (status)), (uintptr_t)(a), (uintptr_t)(b))
#define CO_LOG_ERR_3(status, a, b, c) \
    co_func_log_trace_3(CO_GEN_LOG_MODULE(DBG_ERROR_LEVEL, (status)), (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c))
#define CO_LOG_ERR_4(status, a, b, c, d) \
    co_func_log_trace_4(CO_GEN_LOG_MODULE(DBG_ERROR_LEVEL, (status)), (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c), (uintptr_t)(d))
#define CO_LOG_ERR_S_0(status, tag) \
    co_func_log_trace_with_tag_0(CO_GEN_LOG_MODULE(DBG_ERROR_LEVEL, (status)), (bt_log_tag_t){#tag})
#define CO_LOG_ERR_S_1(status, tag, a) \
    co_func_log_trace_with_tag_1(CO_GEN_LOG_MODULE(DBG_ERROR_LEVEL, (status)), (bt_log_tag_t){#tag}, (uintptr_t)(a))
#define CO_LOG_ERR_S_2(status, tag, a, b) \
    co_func_log_trace_with_tag_2(CO_GEN_LOG_MODULE(DBG_ERROR_LEVEL, (status)), (bt_log_tag_t){#tag}, (uintptr_t)(a), (uintptr_t)(b))
#define CO_LOG_ERR_S_3(status, tag, a, b, c) \
    co_func_log_trace_with_tag_3(CO_GEN_LOG_MODULE(DBG_ERROR_LEVEL, (status)), (bt_log_tag_t){#tag}, (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c))
#else
#define DEBUG_ERROR(...)
#define CO_LOG_ERR_0(status)
#define CO_LOG_ERR_1(status, a)
#define CO_LOG_ERR_2(status, a, b)
#define CO_LOG_ERR_3(status, a, b, c)
#define CO_LOG_ERR_4(status, a, b, c, d)
#define CO_LOG_ERR_S_0(status, tag)
#define CO_LOG_ERR_S_1(status, tag, a)
#define CO_LOG_ERR_S_2(status, tag, a, b)
#define CO_LOG_ERR_S_3(status, tag, a, b, c)
#endif

#if (BT_DBG_CONCAT(MOUDLE, _LEVEL) >= DBG_WARNING_LEVEL)
#define DEBUG_WARNING(num,...) CO_DEBUG_PRINTF(num, __VA_ARGS__)
#define CO_LOG_WAR_0(status) \
    co_func_log_trace_0(CO_GEN_LOG_MODULE(DBG_WARNING_LEVEL, (status)))
#define CO_LOG_WAR_1(status, a) \
    co_func_log_trace_1(CO_GEN_LOG_MODULE(DBG_WARNING_LEVEL, (status)), (uintptr_t)(a))
#define CO_LOG_WAR_2(status, a, b) \
    co_func_log_trace_2(CO_GEN_LOG_MODULE(DBG_WARNING_LEVEL, (status)), (uintptr_t)(a), (uintptr_t)(b))
#define CO_LOG_WAR_3(status, a, b, c) \
    co_func_log_trace_3(CO_GEN_LOG_MODULE(DBG_WARNING_LEVEL, (status)), (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c))
#define CO_LOG_WAR_4(status, a, b, c, d) \
    co_func_log_trace_4(CO_GEN_LOG_MODULE(DBG_WARNING_LEVEL, (status)), (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c), (uintptr_t)(d))
#define CO_LOG_WAR_S_0(status, tag) \
    co_func_log_trace_with_tag_0(CO_GEN_LOG_MODULE(DBG_WARNING_LEVEL, (status)), (bt_log_tag_t){#tag})
#define CO_LOG_WAR_S_1(status, tag, a) \
    co_func_log_trace_with_tag_1(CO_GEN_LOG_MODULE(DBG_WARNING_LEVEL, (status)), (bt_log_tag_t){#tag}, (uintptr_t)(a))
#define CO_LOG_WAR_S_2(status, tag, a, b) \
    co_func_log_trace_with_tag_2(CO_GEN_LOG_MODULE(DBG_WARNING_LEVEL, (status)), (bt_log_tag_t){#tag}, (uintptr_t)(a), (uintptr_t)(b))
#define CO_LOG_WAR_S_3(status, tag, a, b, c) \
    co_func_log_trace_with_tag_3(CO_GEN_LOG_MODULE(DBG_WARNING_LEVEL, (status)), (bt_log_tag_t){#tag}, (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c))
#else
#define DEBUG_WARNING(...)
#define CO_LOG_WAR_0(status)
#define CO_LOG_WAR_1(status, a)
#define CO_LOG_WAR_2(status, a, b)
#define CO_LOG_WAR_3(status, a, b, c)
#define CO_LOG_WAR_4(status, a, b, c, d)
#define CO_LOG_WAR_S_0(status, tag)
#define CO_LOG_WAR_S_1(status, tag, a)
#define CO_LOG_WAR_S_2(status, tag, a, b)
#define CO_LOG_WAR_S_3(status, tag, a, b, c)
#endif

#if (BT_DBG_CONCAT(MOUDLE, _LEVEL) >= DBG_INFO_LEVEL)
#define DEBUG_INFO(num,...) CO_DEBUG_PRINTF(num, __VA_ARGS__)
#define CO_LOG_INFO_0(status) \
    co_func_log_trace_0(CO_GEN_LOG_MODULE(DBG_INFO_LEVEL, (status)))
#define CO_LOG_INFO_1(status, a) \
    co_func_log_trace_1(CO_GEN_LOG_MODULE(DBG_INFO_LEVEL, (status)), (uintptr_t)(a))
#define CO_LOG_INFO_2(status, a, b) \
    co_func_log_trace_2(CO_GEN_LOG_MODULE(DBG_INFO_LEVEL, (status)), (uintptr_t)(a), (uintptr_t)(b))
#define CO_LOG_INFO_3(status, a, b, c) \
    co_func_log_trace_3(CO_GEN_LOG_MODULE(DBG_INFO_LEVEL, (status)), (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c))
#define CO_LOG_INFO_4(status, a, b, c, d) \
    co_func_log_trace_4(CO_GEN_LOG_MODULE(DBG_INFO_LEVEL, (status)), (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c), (uintptr_t)(d))
#define CO_LOG_INFO_S_0(status, tag) \
    co_func_log_trace_with_tag_0(CO_GEN_LOG_MODULE(DBG_INFO_LEVEL, (status)), (bt_log_tag_t){#tag})
#define CO_LOG_INFO_S_1(status, tag, a) \
    co_func_log_trace_with_tag_1(CO_GEN_LOG_MODULE(DBG_INFO_LEVEL, (status)), (bt_log_tag_t){#tag}, (uintptr_t)(a))
#define CO_LOG_INFO_S_2(status, tag, a, b) \
    co_func_log_trace_with_tag_2(CO_GEN_LOG_MODULE(DBG_INFO_LEVEL, (status)), (bt_log_tag_t){#tag}, (uintptr_t)(a), (uintptr_t)(b))
#define CO_LOG_INFO_S_3(status, tag, a, b, c) \
    co_func_log_trace_with_tag_3(CO_GEN_LOG_MODULE(DBG_INFO_LEVEL, (status)), (bt_log_tag_t){#tag}, (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c))
#define CO_LOG_INFO_WITH_STR_0(status, s) \
    co_func_log_str_0(CO_GEN_LOG_MODULE(DBG_INFO_LEVEL, (status)), (s))
#define CO_LOG_INFO_WITH_STR_1(status, s, a) \
    co_func_log_str_1(CO_GEN_LOG_MODULE(DBG_INFO_LEVEL, (status)), (s), (uintptr_t)(a))
#define CO_LOG_INFO_WITH_STR_2(status, s, a, b) \
    co_func_log_str_2(CO_GEN_LOG_MODULE(DBG_INFO_LEVEL, (status)), (s), (uintptr_t)(a), (uintptr_t)(b))
#define CO_LOG_INFO_WITH_STR_3(status, s, a, b, c) \
    co_func_log_str_3(CO_GEN_LOG_MODULE(DBG_INFO_LEVEL, (status)), (s), (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c))
#else
#define DEBUG_INFO(...)
#define CO_LOG_INFO_0(status)
#define CO_LOG_INFO_1(status, a)
#define CO_LOG_INFO_2(status, a, b)
#define CO_LOG_INFO_3(status, a, b, c)
#define CO_LOG_INFO_4(status, a, b, c, d)
#define CO_LOG_INFO_S_0(status, tag)
#define CO_LOG_INFO_S_1(status, tag, a)
#define CO_LOG_INFO_S_2(status, tag, a, b)
#define CO_LOG_INFO_S_3(status, tag, a, b, c)
#define CO_LOG_INFO_WITH_STR_0(status, s)
#define CO_LOG_INFO_WITH_STR_1(status, s, a)
#define CO_LOG_INFO_WITH_STR_2(status, s, a, b)
#define CO_LOG_INFO_WITH_STR_3(status, s, a, b, c)
#endif

#if (BT_DBG_CONCAT(MOUDLE, _LEVEL) >= DBG_VERBOSE_LEVEL)
#define DEBUG_VERBOSE(num,...) CO_DEBUG_PRINTF(num, __VA_ARGS__)
#define CO_LOG_VBS_0(status) \
    co_func_log_trace_0(CO_GEN_LOG_MODULE(DBG_VERBOSE_LEVEL, (status)))
#define CO_LOG_VBS_1(status, a) \
    co_func_log_trace_1(CO_GEN_LOG_MODULE(DBG_VERBOSE_LEVEL, (status)), (uintptr_t)(a))
#define CO_LOG_VBS_2(status, a, b) \
    co_func_log_trace_2(CO_GEN_LOG_MODULE(DBG_VERBOSE_LEVEL, (status)), (uintptr_t)(a), (uintptr_t)(b))
#define CO_LOG_VBS_3(status, a, b, c) \
    co_func_log_trace_3(CO_GEN_LOG_MODULE(DBG_VERBOSE_LEVEL, (status)), (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c))
#define CO_LOG_VBS_4(status, a, b, c, d) \
    co_func_log_trace_4(CO_GEN_LOG_MODULE(DBG_VERBOSE_LEVEL, (status)), (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c), (uintptr_t)(d))
#define CO_LOG_VBS_S_0(status, tag) \
    co_func_log_trace_with_tag_0(CO_GEN_LOG_MODULE(DBG_VERBOSE_LEVEL, (status)), (bt_log_tag_t){#tag})
#define CO_LOG_VBS_S_1(status, tag, a) \
    co_func_log_trace_with_tag_1(CO_GEN_LOG_MODULE(DBG_VERBOSE_LEVEL, (status)), (bt_log_tag_t){#tag}, (uintptr_t)(a))
#define CO_LOG_VBS_S_2(status, tag, a, b) \
    co_func_log_trace_with_tag_2(CO_GEN_LOG_MODULE(DBG_VERBOSE_LEVEL, (status)), (bt_log_tag_t){#tag}, (uintptr_t)(a), (uintptr_t)(b))
#define CO_LOG_VBS_S_3(status, tag, a, b, c) \
    co_func_log_trace_with_tag_3(CO_GEN_LOG_MODULE(DBG_VERBOSE_LEVEL, (status)), (bt_log_tag_t){#tag}, (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c))
#else
#define DEBUG_VERBOSE(...)
#define CO_LOG_VBS_0(status)
#define CO_LOG_VBS_1(status, a)
#define CO_LOG_VBS_2(status, a, b)
#define CO_LOG_VBS_3(status, a, b, c)
#define CO_LOG_VBS_4(status, a, b, c, d)
#define CO_LOG_VBS_S_0(status, tag)
#define CO_LOG_VBS_S_1(status, tag, a)
#define CO_LOG_VBS_S_2(status, tag, a, b)
#define CO_LOG_VBS_S_3(status, tag, a, b, c)
#endif

#if (BT_DBG_CONCAT(MOUDLE, _LEVEL) >= INFO_LEVEL)
void _debug_print_dump_data(char *mem, int mem_size);
#define DEBUG_DUMP_DATA(memaddr, memlen) do { \
        DEBUGPRINT2(2,"\r\n"  "module %s #%d: ", BT_DBG_GET_NAME(MOUDLE), __LINE__);\
        DEBUGPRINT2(0,"\r\n");\
        Plt_DUMP8("%02x ", (char *)memaddr,memlen);\
    } while(0);
#else
#define DEBUG_DUMP_DATA(...)
#endif

#define DEBUG_RAW(...) DEBUG_PRINT(__VA_ARGS__)
#define DEBUG_IMM(...) TRACE_IMM(__VA_ARGS__)

#else

#define DEBUG_ERROR(...)
#define CO_LOG_ERR_0(status)
#define CO_LOG_ERR_1(status, a)
#define CO_LOG_ERR_2(status, a, b)
#define CO_LOG_ERR_3(status, a, b, c)
#define CO_LOG_ERR_4(status, a, b, c, d)
#define CO_LOG_ERR_S_0(status, tag)
#define CO_LOG_ERR_S_1(status, tag, a)
#define CO_LOG_ERR_S_2(status, tag, a, b)
#define CO_LOG_ERR_S_3(status, tag, a, b, c)

#define DEBUG_WARNING(...)
#define CO_LOG_WAR_0(status)
#define CO_LOG_WAR_1(status, a)
#define CO_LOG_WAR_2(status, a, b)
#define CO_LOG_WAR_3(status, a, b, c)
#define CO_LOG_WAR_4(status, a, b, c, d)
#define CO_LOG_WAR_S_0(status, tag)
#define CO_LOG_WAR_S_1(status, tag, a)
#define CO_LOG_WAR_S_2(status, tag, a, b)
#define CO_LOG_WAR_S_3(status, tag, a, b, c)

#define DEBUG_INFO(...)
#define CO_LOG_INFO_0(status)
#define CO_LOG_INFO_1(status, a)
#define CO_LOG_INFO_2(status, a, b)
#define CO_LOG_INFO_3(status, a, b, c)
#define CO_LOG_INFO_4(status, a, b, c, d)
#define CO_LOG_INFO_S_0(status, tag)
#define CO_LOG_INFO_S_1(status, tag, a)
#define CO_LOG_INFO_S_2(status, tag, a, b)
#define CO_LOG_INFO_S_3(status, tag, a, b, c)

#define DEBUG_VERBOSE(...)
#define CO_LOG_VBS_0(status)
#define CO_LOG_VBS_1(status, a)
#define CO_LOG_VBS_2(status, a, b)
#define CO_LOG_VBS_3(status, a, b, c)
#define CO_LOG_VBS_4(status, a, b, c, d)
#define CO_LOG_VBS_S_0(status, tag)
#define CO_LOG_VBS_S_1(status, tag, a)
#define CO_LOG_VBS_S_2(status, tag, a, b)
#define CO_LOG_VBS_S_3(status, tag, a, b, c)

#define DEBUG_ASSERT(expr)
#define DEBUG_DUMP_DATA(...)
#define DEBUG_RAW(...)
#define DEBUG_IMM(...)
#endif /* DBG_DEBUG_PRINT_ENABLE */

bool co_uuid_is_equal_le(const uint8_t *uuid_a_le, uint16_t a, const uint8_t *uuid_b_le, uint16_t b);
bool co_uuid_16_equal_le(uint16_t uuid_16_bit, const uint8_t *uuid_b_le, uint16_t b);

typedef struct btif_event_t btif_event_t;

enum BESBT_HOOK_USER_T {
    BESBT_HOOK_USER_0 = 0,
    BESBT_HOOK_USER_1,
    BESBT_HOOK_USER_2,
    BESBT_HOOK_USER_3,
    BESBT_HOOK_USER_QTY
};

typedef void (*BESBT_HOOK_HANDLER)(void);

typedef struct APP_KEY_STATUS APP_KEY_STATUS;

typedef struct
{
    const char* string;
    void (*cmd_function)(const char* param, uint32_t param_len);
} app_bt_cmd_handle_t;

enum APP_BT_REQ_T {
    APP_BT_REQ_ACCESS_MODE_SET,
    APP_BT_REQ_AUTO_TEST,
    APP_BT_DO_FUNCTION,
    APP_BT_REQ_NUM
};

enum BT_ACL_CONN_TYPE
{
    BT_ACL_CONNECT_REQ  = 0,
    BT_ACL_CONNECT      = 1,
    BT_ACL_DISCONNECT   = 2
};

typedef struct app_bt_search_result {
    bt_bdaddr_t *addr;
    const char *name;
    int name_len;
} app_bt_search_result_t;

#define BTM_RX_FILTER_HCI_TYPE   (0x01)
#define BTM_RX_FILTER_L2C_TYPE   (0x02)
#define BT_EIR_GLOBAL_SRV_UUIDS_GATHER_CALLBACK_COUNT 3

typedef enum {
    HCI_CMD_COMPLETE_USER_IBRT_CMD        = 0,
    HCI_CMD_COMPLETE_USER_BLE             = 1,
    HCI_CMD_COMPLETE_USER_RESEVER_1       = 2,
    HCI_CMD_COMPLETE_USER_RESEVER_2       = 3,
    HCI_CMD_COMPLETE_USER_RESEVER_3       = 4,
    HCI_CMD_COMPLETE_USER_RESEVER_4       = 5,
    HCI_CMD_COMPLETE_USER_NUM,
} HCI_CMD_COMPLETE_USER_E;

#define BTM_BONDING_NOT_REQUIRED 0x00
#define BTM_DEDICATED_BONDING    0x02
#define BTM_GENERAL_BONDING      0x04
#define BTM_BONDING_NOT_ALLOWED  0x10

#define BTM_AUTH_MITM_PROTECT_NOT_REQUIRED  0x00
#define BTM_AUTH_MITM_PROTECT_REQUIRED      0x01

#define BTM_IO_DISPLAY_ONLY   0
#define BTM_IO_DISPLAY_YESNO  1
#define BTM_IO_KEYBOARD_ONLY  2
#define BTM_IO_NO_IO          3

struct bt_alloc_param_t {
    uint32_t param0;
    uint32_t param1;
    uint32_t param2;
    uint32_t param3;
};

typedef struct {
    uint8_t bonding_mode;
    uint8_t mitm_protection_required;
    uint8_t io_cap;
    uint8_t oob_present;
} bt_iocap_requirement_t;

struct BT_DEVICE_T;
struct BT_SOURCE_DEVICE_T;
struct coheap_global_t;

typedef struct _evm_timer evm_timer_t;
typedef void (*evm_timer_notify) (evm_timer_t *);
typedef struct { /* empty */ } btif_remote_device_t;
typedef bool (*ibrt_rx_data_filter_func)(const bt_bdaddr_t *bdaddr,uint8_t rx_filter_type,void*para);
typedef void (*ibrt_cmd_complete_callback)(const uint8_t *para);
typedef void (*l2cap_sdp_disconnect_callback)(const bt_bdaddr_t *bdaddr);
typedef uint8 (*bt_get_ibrt_role_callback)(const bt_bdaddr_t *para);
typedef uint8 (*bt_get_ui_role_callback)(void);
typedef uint8_t (*bt_get_tss_state_callback)(const bt_bdaddr_t *para);
typedef bool (*bt_user_allow_accept_sco_callback)(uint8_t device_id, void *addr);
typedef bool (*extra_acl_conn_req_callback)(uint8_t *remote, uint8_t *cod);
typedef void (*bt_cmgr_sniff_timeout_ext_handler)(evm_timer_t * timer, unsigned int* skipInternalHandler);
typedef bool (*bt_remote_is_mobile_callback_t)(const bt_bdaddr_t *remote);
typedef void (*bt_eir_fill_manufacture_data)(uint8_t *buff, uint32_t* offset);
typedef void (*bt_register_sdp_disc_callback_t)(const bt_bdaddr_t* addr);
typedef void (*l2cap_process_bredr_smp_req_callback_func)(uint8 device_id, uint16 conn_handle, uint16 len, uint8 *data);
typedef void (*bt_hci_cmd_status_callback_t)(const void *para);
typedef void (*btif_event_callback_t) (const btif_event_t *);
typedef uint16_t (*bt_get_ibrt_handle_callback_t)(const bt_bdaddr_t* remote);
typedef uint8_t (*bt_get_ibrt_role_callback_t)(const bt_bdaddr_t* addr);
typedef uint8_t (*bt_get_ibrt_ui_role_callback_t)(void);
typedef int (*bt_a2dp_stream_command_pack_callback_t)(void* remote, uint8_t transaction, uint8_t signal_id);
typedef bool (*bt_ibrt_rx_data_filter_callback_t)(const bt_bdaddr_t* addr, uint8_t rx_filter_type, void* para);
typedef void (*bt_sco_codec_info_sync_callback_t)(const bt_bdaddr_t* remote, uint8_t codec);
typedef void (*bt_report_ibrt_slave_spp_closed_t)(const bt_bdaddr_t *remote, uint8_t dlci);
typedef bool (*bt_avrcp_register_notify_callback_t)(uint8_t event);
typedef void (*bt_avrcp_register_notify_response_callback_t)(uint8_t event);
typedef void (*bt_hci_tx_tss_process_callback_t)(void);
typedef void (*bt_stack_create_acl_failed_callback_t)(const bt_bdaddr_t *);
typedef void (*bt_acl_conn_state_change_cb_t)(uint8_t type,bt_bdaddr_t* addr, uint32_t para);
typedef void (*bt_pair_state_change_cb_t)(bt_bdaddr_t* addr, uint32_t para);
typedef void (*btif_hci_vendor_event_callback_func)(uint8_t* pbuf, uint32_t length);
typedef uint8_t (*btif_hci_sync_airmode_check_ind_func)(uint8_t status, bt_bdaddr_t *bdaddr);
typedef bt_iocap_requirement_t *(*bt_iocap_request_callback_func)(uint8_t device_id, uint16_t connhdl, const bt_bdaddr_t *remote, const bt_iocap_requirement_t *remote_initiate);

struct bt_defer_param_t {
    bool is_fixed_param;
    uint16_t alloc_size;
    uint32_t param;
};

#define BT_DONT_CALL_FUNC_IN_BT_THREAD 0x01
#define BT_CALL_FUNC_BY_MAIL_IN_BT_THREAD 0x02
#define BT_LOCK_CALL_FUNC_IN_OTHER_THREAD 0x04
#define BT_DELAY_CALL_FUNC_IN_BT_THREAD 0x08

#define BT_DELAY_CALL_TIME_U24_MAX 0x00FFFFFF

struct bt_defer_param_t bt_fixed_param_impl(uint32_t param);
struct bt_defer_param_t bt_alloc_param_size(const void *data_ptr, uint16_t alloc_size);

#define bt_fixed_param(param) bt_fixed_param_impl((uint32_t)(uintptr_t)(param))
#define bt_alloc_param(value_ptr) bt_alloc_param_size((value_ptr), sizeof(*(value_ptr)))

uint32_t bt_defer_curr_func_0_impl(uintptr_t func, uint32_t flags);
uint32_t bt_defer_curr_func_1_impl(uintptr_t func, struct bt_defer_param_t param0, uint32_t flags);
uint32_t bt_defer_curr_func_2_impl(uintptr_t func, struct bt_defer_param_t param0, struct bt_defer_param_t param1, uint32_t flags);
uint32_t bt_defer_curr_func_3_impl(uintptr_t func, struct bt_defer_param_t param0, struct bt_defer_param_t param1, struct bt_defer_param_t param2, uint32_t flags);
uint32_t bt_defer_curr_func_4_impl(uintptr_t func, struct bt_defer_param_t param0, struct bt_defer_param_t param1, struct bt_defer_param_t param2, struct bt_defer_param_t param3, uint32_t flags);

#define bt_defer_curr_func_0(func)          bt_defer_curr_func_0_impl((uintptr_t)(func), BT_DONT_CALL_FUNC_IN_BT_THREAD)
#define bt_defer_curr_func_1(func,a)        bt_defer_curr_func_1_impl((uintptr_t)(func),(a), BT_DONT_CALL_FUNC_IN_BT_THREAD)
#define bt_defer_curr_func_2(func,a,b)      bt_defer_curr_func_2_impl((uintptr_t)(func),(a),(b), BT_DONT_CALL_FUNC_IN_BT_THREAD)
#define bt_defer_curr_func_3(func,a,b,c)    bt_defer_curr_func_3_impl((uintptr_t)(func),(a),(b),(c), BT_DONT_CALL_FUNC_IN_BT_THREAD)
#define bt_defer_curr_func_4(func,a,b,c,d)  bt_defer_curr_func_4_impl((uintptr_t)(func),(a),(b),(c),(d), BT_DONT_CALL_FUNC_IN_BT_THREAD)

#define bt_lock_call_func_0(func)          bt_defer_curr_func_0_impl((uintptr_t)(func), BT_LOCK_CALL_FUNC_IN_OTHER_THREAD)
#define bt_lock_call_func_1(func,a)        bt_defer_curr_func_1_impl((uintptr_t)(func),(a), BT_LOCK_CALL_FUNC_IN_OTHER_THREAD)
#define bt_lock_call_func_2(func,a,b)      bt_defer_curr_func_2_impl((uintptr_t)(func),(a),(b), BT_LOCK_CALL_FUNC_IN_OTHER_THREAD)
#define bt_lock_call_func_3(func,a,b,c)    bt_defer_curr_func_3_impl((uintptr_t)(func),(a),(b),(c), BT_LOCK_CALL_FUNC_IN_OTHER_THREAD)
#define bt_lock_call_func_4(func,a,b,c,d)  bt_defer_curr_func_4_impl((uintptr_t)(func),(a),(b),(c),(d), BT_LOCK_CALL_FUNC_IN_OTHER_THREAD)

#define bt_defer_call_func_0(func)          do { (void)bt_defer_curr_func_0_impl((uintptr_t)(func), 0x00); } while (0)
#define bt_defer_call_func_1(func,a)        do { (void)bt_defer_curr_func_1_impl((uintptr_t)(func),(a), 0x00); } while (0)
#define bt_defer_call_func_2(func,a,b)      do { (void)bt_defer_curr_func_2_impl((uintptr_t)(func),(a),(b), 0x00); } while (0)
#define bt_defer_call_func_3(func,a,b,c)    do { (void)bt_defer_curr_func_3_impl((uintptr_t)(func),(a),(b),(c), 0x00); } while (0)
#define bt_defer_call_func_4(func,a,b,c,d)  do { (void)bt_defer_curr_func_4_impl((uintptr_t)(func),(a),(b),(c),(d), 0x00); } while (0)

#define bt_thread_call_func_0(func)          do { (void)bt_defer_curr_func_0_impl((uintptr_t)(func), BT_CALL_FUNC_BY_MAIL_IN_BT_THREAD); } while (0)
#define bt_thread_call_func_1(func,a)        do { (void)bt_defer_curr_func_1_impl((uintptr_t)(func),(a), BT_CALL_FUNC_BY_MAIL_IN_BT_THREAD); } while (0)
#define bt_thread_call_func_2(func,a,b)      do { (void)bt_defer_curr_func_2_impl((uintptr_t)(func),(a),(b), BT_CALL_FUNC_BY_MAIL_IN_BT_THREAD); } while (0)
#define bt_thread_call_func_3(func,a,b,c)    do { (void)bt_defer_curr_func_3_impl((uintptr_t)(func),(a),(b),(c), BT_CALL_FUNC_BY_MAIL_IN_BT_THREAD); } while (0)
#define bt_thread_call_func_4(func,a,b,c,d)  do { (void)bt_defer_curr_func_4_impl((uintptr_t)(func),(a),(b),(c),(d), BT_CALL_FUNC_BY_MAIL_IN_BT_THREAD); } while (0)

#define bt_delay_call_func_0(func,delay_ms_u24)          do { (void)bt_defer_curr_func_0_impl((uintptr_t)(func), \
                                                                (CO_MIN_VALUE(delay_ms_u24, BT_DELAY_CALL_TIME_U24_MAX) << 8) | BT_DELAY_CALL_FUNC_IN_BT_THREAD); } while (0)
#define bt_delay_call_func_1(func,delay_ms_u24,a)        do { (void)bt_defer_curr_func_1_impl((uintptr_t)(func),(a), \
                                                                (CO_MIN_VALUE(delay_ms_u24, BT_DELAY_CALL_TIME_U24_MAX) << 8) | BT_DELAY_CALL_FUNC_IN_BT_THREAD); } while (0)
#define bt_delay_call_func_2(func,delay_ms_u24,a,b)      do { (void)bt_defer_curr_func_2_impl((uintptr_t)(func),(a),(b), \
                                                                (CO_MIN_VALUE(delay_ms_u24, BT_DELAY_CALL_TIME_U24_MAX) << 8) | BT_DELAY_CALL_FUNC_IN_BT_THREAD); } while (0)
#define bt_delay_call_func_3(func,delay_ms_u24,a,b,c)    do { (void)bt_defer_curr_func_3_impl((uintptr_t)(func),(a),(b),(c), \
                                                                (CO_MIN_VALUE(delay_ms_u24, BT_DELAY_CALL_TIME_U24_MAX) << 8) | BT_DELAY_CALL_FUNC_IN_BT_THREAD); } while (0)
#define bt_delay_call_func_4(func,delay_ms_u24,a,b,c,d)  do { (void)bt_defer_curr_func_4_impl((uintptr_t)(func),(a),(b),(c),(d), \
                                                                (CO_MIN_VALUE(delay_ms_u24, BT_DELAY_CALL_TIME_U24_MAX) << 8) | BT_DELAY_CALL_FUNC_IN_BT_THREAD); } while (0)

#ifdef __cplusplus
}
#endif
#endif /* __BT_COMMON_DEFINE_H__ */
