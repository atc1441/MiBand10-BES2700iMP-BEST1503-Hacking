/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define RM_FILESYS_CLIENT

#ifdef RM_FILESYS_CLIENT

#include "cmsis.h"
#include "sfs.h"
#include "hal_trace.h"

#define printf(...)


static uint32_t RPC_MEM[1024];
void *rm_cmd_malloc(size_t __size)
{
    if (__size > 1024){
      // ASSERT(0);
    }
    return (void *)RPC_MEM;
}
void rm_cmd_free(void * ptr)
{
}

/*
  remote command header format

  header[5]<<40 | header[4]<<32 | header[3]<<24 | header[2]<<16 | header[1]<<8 | header[0] : "RMCMD\0"
  header[7]<<8 |header[6]: req_or anwser|command index
  header[11]<<24 | header[10]<<16 | header[9]<<8 | header[8] : crc
  header[15]<<24 | header[14]<<16 | header[13]<<8 | header[12] : data length
  header[17]<<8 | header[16] : reserved
*/

#define RM_CMD_HEADER_LENGTH 18

#define RM_CMD_TYPE_REQ      0
#define RM_CMD_TYPE_ANSWER   1



typedef enum {
  RM_CMD_FOPEN,
  RM_CMD_FCLOSE,
  RM_CMD_FREAD,
  RM_CMD_FWRITE,
  RM_CMD_FTELL,
  RM_CMD_FSEEK,
}rm_cmd_idx_t;



typedef struct  __attribute__((packed))  {
   char header[RM_CMD_HEADER_LENGTH];
   char data[1];
}rm_cmd_t;


typedef struct  __attribute__((packed))  {
    uint32_t buf_sz;
    char buff[1];
} buff_ptr_t;


static int rpc_trans(rm_cmd_t * cmd, rm_cmd_t ** result_cmd );

static int set_cmd_header_magic_string(rm_cmd_t *cmd)
{
  int ret = 0;
  char * ptr = cmd->header;
  memcpy(ptr, "RMCMD\0", 6) ;
  return ret ;
}
static int set_cmd_header_idx(rm_cmd_t *cmd,uint8_t idx)
{
  int ret = 0;
  char * ptr = cmd->header;
  ptr[6] = idx & 0xff;
  return ret ;
}

static int get_cmd_header_idx(rm_cmd_t *cmd,uint8_t *idx)
{
  int ret = 0;
  char * ptr = cmd->header;
  *idx =  ptr[6];
  return ret ;
}


static int set_cmd_header_type(rm_cmd_t *cmd,uint8_t type)
{
  int ret = 0;
  char * ptr = cmd->header;
  ptr[7] =  0xff & type ;
  return ret ;
}


#if 0
static POSSIBLY_UNUSED int get_cmd_header_type(rm_cmd_t *cmd,uint8_t *type)
{
  int ret = 0;
  char * ptr = cmd->header;
  *type =  ptr[7];
  return ret ;
}

#endif


static int set_cmd_header_data_length(rm_cmd_t *cmd,uint32_t sz)
{
  int ret = 0;
  char * ptr = &cmd->header[12];
  ptr[0] = sz & 0xff;
  ptr[1] = (sz & 0xff00)>>8;
  ptr[2] = (sz & 0xff0000)>>16;
  ptr[3] = (sz & 0xff000000)>>24;

  return ret ;
}

static int get_cmd_header_data_length(rm_cmd_t *cmd, uint32_t *sz)
{
  int ret = 0;
  char * ptr = &cmd->header[12];
  *sz = ((ptr[3] << 24) & 0xff000000)  | ((ptr[2] << 16) & 0xff0000) | ((ptr[1] << 8) & 0xff00) | (ptr[0] & 0xff);
  return ret ;
}


#if 0
static POSSIBLY_UNUSED int set_cmd_header_crc(rm_cmd_t *cmd, uint32_t crc)
{
  int ret = 0;
  char * ptr = &cmd->header[8];
  ptr[0] = crc & 0xff;
  ptr[1] = (crc & 0xff00)>>8;
  ptr[2] = (crc & 0xff0000)>>16;
  ptr[3] = (crc & 0xff000000)>>24;

  return ret ;
}

static POSSIBLY_UNUSED int get_cmd_header_crc(rm_cmd_t *cmd, uint32_t *crc)
{
  int ret = 0;
  char * ptr = &cmd->header[8];
  *crc = (ptr[2] << 24)  | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0];
  return ret ;
}
#endif

#if 0
static   int get_cmd_data_buffer_ptr(rm_cmd_t *cmd, buff_ptr_t * ptr )
{
   uint32_t size = 0;
   ptr->buff = cmd->data;
   get_cmd_header_data_length(cmd,&size);
   ptr->buf_sz = size;
   return 0;
}
#endif

static int uart_rx_process(char *buffer,int read_sz );
static int uart_write(void *cmd, uint32_t cmd_length);
static int rpc_trans(rm_cmd_t * cmd, rm_cmd_t ** result_cmd );

/*
typedef struct __attribute__((packed))  rm_cmd_fopen_param{
    buff_ptr_t filename;
    buff_ptr_t modes;
}rm_cmd_fopen_param_t ;
*/

typedef struct __attribute__((packed)) rm_cmd_fopen_ret_param{
    uint8_t  hfile[4];
}rm_cmd_fopen_ret_param_t ;

static int rm_cmd_fopen_param_set_name(buff_ptr_t* params, char * __filename)
{
    int sz = strlen(__filename);
    params->buf_sz = sz;

    memcpy(params->buff, __filename, sz );

    return 0;
}

static int rm_cmd_fopen_param_set_modes(buff_ptr_t* params, char * modes)
{
    int sz = strlen(modes);
    params->buf_sz = sz;
    memcpy(params->buff, modes, sz );
    return 0;
}



static FILE *rm_cmd_fopen_get_return_file(rm_cmd_t *cmd)
{

    FILE *file = NULL ;
    uint8_t idx = 0xff ;
    rm_cmd_fopen_ret_param_t  *ret ;
    get_cmd_header_idx(cmd, &idx );
    if (idx != RM_CMD_FOPEN) {
      return file;
    }
    ret = (rm_cmd_fopen_ret_param_t*)cmd->data;
    file = (FILE *)(*(uint32_t*)ret->hfile);
    return file;
}

/* Open a file and create a new stream for it.
   This function is a possible cancellation point and therefore not
   marked with __THROW.


*/
FILE *fopen (const char *__restrict __filename,
             const char *__restrict __modes)
{
    rm_cmd_t *rpc_teturn = NULL;
    rm_cmd_t * cmd = NULL;
    int  data_sz =  sizeof(buff_ptr_t)*2 -2 + strlen(__modes) + strlen(__filename);
    cmd = (rm_cmd_t * )rm_cmd_malloc(RM_CMD_HEADER_LENGTH + data_sz);

    buff_ptr_t *params = (buff_ptr_t *)((char *)cmd + RM_CMD_HEADER_LENGTH );
    rm_cmd_fopen_param_set_name( params,  (char *)__filename);
    rm_cmd_fopen_param_set_modes((buff_ptr_t*)((char*)params + sizeof(buff_ptr_t) -1 + params->buf_sz ), (char *) __modes);

    set_cmd_header_idx(cmd,RM_CMD_FOPEN);
    set_cmd_header_type(cmd,RM_CMD_TYPE_REQ);
    set_cmd_header_data_length(cmd,data_sz);
    rpc_trans(cmd,&rpc_teturn);
    return rm_cmd_fopen_get_return_file(rpc_teturn);
}


/* Close STREAM.
   This function is a possible cancellation point and therefore not
   marked with __THROW.
*/

typedef struct  __attribute__((packed))  rm_cmd_fclose_param{
    uint32_t  hfile;
}rm_cmd_fclose_param_t ;

typedef struct  __attribute__((packed))  rm_cmd_fclose_ret_param{
    uint32_t  hfile;
}rm_cmd_fclose_ret_param_t ;


int fclose (FILE *__stream)
{
    rm_cmd_t *rpc_teturn = NULL;
    rm_cmd_t * cmd = NULL;
    int  data_sz =  sizeof(rm_cmd_fclose_param_t);
    cmd = (rm_cmd_t * )rm_cmd_malloc(RM_CMD_HEADER_LENGTH + data_sz);

    rm_cmd_fclose_param_t *params = (rm_cmd_fclose_param_t *)((char *)cmd + RM_CMD_HEADER_LENGTH );
    params->hfile = (uint32_t)__stream;
    set_cmd_header_idx(cmd,RM_CMD_FCLOSE);
    set_cmd_header_type(cmd,RM_CMD_TYPE_REQ);
    set_cmd_header_data_length(cmd,data_sz);
    rpc_trans(cmd,&rpc_teturn);

    //free remote cmd space
    rm_cmd_free(cmd);

    return 0;
}


/* Flush STREAM, or all streams if STREAM is NULL.
   This function is a possible cancellation point and therefore not
   marked with __THROW.
*/

typedef struct  __attribute__((packed))  rm_cmd_fflush_param{
    uint32_t  hfile;
}rm_cmd_fflush_param_t ;

typedef struct  __attribute__((packed))  rm_cmd_fflush_ret_param{
    uint32_t  hfile;
}rm_cmd_fflush_ret_param_t ;

int sfsferror (FILE *__stream)
{
   return 0;
}
int sfsfflush (FILE *__stream)
{
    return 0;
    rm_cmd_t *rpc_teturn = NULL;
    rm_cmd_t * cmd = NULL;
    int  data_sz =  sizeof(rm_cmd_fflush_param_t);
    cmd = (rm_cmd_t * )rm_cmd_malloc(RM_CMD_HEADER_LENGTH + data_sz);

    rm_cmd_fflush_param_t *params = (rm_cmd_fflush_param_t *)((char *)cmd + RM_CMD_HEADER_LENGTH );
    params->hfile = (uint32_t)__stream;
    set_cmd_header_idx(cmd,RM_CMD_FCLOSE);
    set_cmd_header_type(cmd,RM_CMD_TYPE_REQ);
    set_cmd_header_data_length(cmd,data_sz);
    rpc_trans(cmd,&rpc_teturn);

    //free remote cmd space
    rm_cmd_free(cmd);

    return 0;
}


size_t fwrite (const void *__restrict __ptr, size_t __size,
                size_t __n, FILE *__restrict __s);
/*
  Write a string to STREAM.
  This function is a possible cancellation point and therefore not
  marked with __THROW.
*/

int fputs (const char *__restrict __s, FILE *__restrict __stream)
{
  return fwrite(__s, strlen(__s), 1, __stream);
}
/*
  Write a string, followed by a newline, to stdout.
  This function is a possible cancellation point and therefore not
  marked with __THROW.
*/
int puts (const char *__s)
{
 // char nl = '\n';
 // fwrite(__s, strlen(__s) + 1, 1, stdout);
 // fwrite(&nl, 1, 1, stdout);
  return 0;
}

int getc(FILE *stream)
{
  char c = 0;
  fread( &c,1,1,stream);
  return c;
}

int sfsputc(int c, FILE *stream)
{
   char cha = 0xff & c;
  return fwrite(&cha, 1, 1, stream);
}

/*
  Push a character back onto the input buffer of STREAM.
  This function is a possible cancellation point and therefore not
  marked with __THROW.
*/
int ungetc (int __c, FILE *__stream)
{
   char c = 0xff & __c;
  return fwrite(&c, 1, 1, __stream);
}


/*
  Read chunks of generic data from STREAM.
  This function is a possible cancellation point and therefore not
  marked with __THROW.
*/

typedef struct  __attribute__((packed))  rm_cmd_fread_param{
    uint16_t rsz;
    uint32_t  hfile;
}rm_cmd_fread_param_t ;

typedef struct  __attribute__((packed))  rm_cmd_fread_ret_param{
    uint32_t  hfile;
    buff_ptr_t data;
}rm_cmd_fread_ret_param_t ;


static size_t _fread (void *__restrict __ptr, size_t __size,
              size_t __n, FILE *__restrict __stream)
{
    size_t rdsz = 0;
    rm_cmd_t * cmd = NULL;
    rm_cmd_t *rpc_teturn = NULL;
    uint32_t  data_sz =  sizeof(rm_cmd_fread_param_t) ;
    rm_cmd_fread_param_t *params = NULL;


    cmd = (rm_cmd_t * )rm_cmd_malloc(RM_CMD_HEADER_LENGTH + data_sz);
    params = (rm_cmd_fread_param_t *)((char *)cmd + RM_CMD_HEADER_LENGTH);
    params->rsz = __size * __n;
    params->hfile = (uint32_t)__stream;
    set_cmd_header_idx(cmd,RM_CMD_FREAD);
    set_cmd_header_type(cmd,RM_CMD_TYPE_REQ);
    set_cmd_header_data_length(cmd,data_sz);
    // send request to server
    rpc_trans(cmd,&rpc_teturn);

    //copy data to user buffer
    data_sz = 0;
    get_cmd_header_data_length(rpc_teturn, &data_sz);
    if (data_sz){
      rm_cmd_fread_ret_param_t * ret = (rm_cmd_fread_ret_param_t *)rpc_teturn->data ;
      if (ret->data.buf_sz) {
        memcpy(__ptr, ret->data.buff, ret->data.buf_sz);
        rdsz = ret->data.buf_sz;

      }
    }

    //free remote cmd space
    rm_cmd_free(cmd);
    return rdsz;
}

#define RM_CMD_SEND_FRAME_SIZE 512
size_t fread (void *__restrict __ptr, size_t __size,
              size_t __n, FILE *__restrict __stream)
{
   uint32_t sends = __size* __n;
   uint16_t  times = sends/RM_CMD_SEND_FRAME_SIZE;
   uint16_t remain = sends%RM_CMD_SEND_FRAME_SIZE;
   size_t reads = 0;
   for(int i = 0; i < times; i++){
     reads += _fread ((char*)__ptr + RM_CMD_SEND_FRAME_SIZE *i  , RM_CMD_SEND_FRAME_SIZE,
              1,  __stream);
   }
   if (remain){
    reads += _fread ((char*)__ptr + RM_CMD_SEND_FRAME_SIZE *times  , remain,
              1,  __stream);
   }
   return reads;
}

/* Write chunks of generic data to STREAM.
   This function is a possible cancellation point and therefore not
   marked with __THROW.
*/



typedef struct  __attribute__((packed))  rm_cmd_fwrite_param{
    uint32_t  hfile;
    buff_ptr_t data;

}rm_cmd_fwrite_param_t ;

typedef struct  __attribute__((packed))  rm_cmd_fwrite_ret_param{
    uint16_t wsz;
    uint32_t  hfile;
}rm_cmd_fwrite_ret_param_t ;


static size_t _fwrite (const void *__restrict __ptr, size_t __size,
                size_t __n, FILE *__restrict __s)
{
    rm_cmd_t *rpc_teturn = NULL;
    rm_cmd_t * cmd = NULL;
    uint32_t  data_sz = 0;
    size_t wrtd_size = 0;
    rm_cmd_fwrite_param_t *params = NULL;
    rm_cmd_fwrite_ret_param_t *ret_params = NULL;
    FILE *file = NULL;

    // call local service to handle req
    file = __s;
    wrtd_size = __size * __n;

    if (file == 0 ){
      printf("%s invalid file handle !!!!!!", __func__);
      return 0;

    }

    data_sz =  sizeof(rm_cmd_fwrite_param_t) -1 + wrtd_size;

    cmd = (rm_cmd_t * )rm_cmd_malloc(RM_CMD_HEADER_LENGTH + data_sz);

    params = (rm_cmd_fwrite_param_t *)((char *)cmd + RM_CMD_HEADER_LENGTH);
    params->data.buf_sz = wrtd_size;
    memcpy(params->data.buff, __ptr,wrtd_size );
    params->hfile = (uint32_t)file;

    set_cmd_header_idx(cmd,RM_CMD_FWRITE);
    set_cmd_header_type(cmd,RM_CMD_TYPE_REQ);
    set_cmd_header_data_length(cmd,data_sz);
    // send request to server, wait for result
    rpc_trans(cmd,&rpc_teturn);


    // check result , free rm cmd space

    data_sz = 0;

    get_cmd_header_data_length(rpc_teturn, &data_sz);
    if (data_sz){
      ret_params = (rm_cmd_fwrite_ret_param_t *)rpc_teturn->data ;
      wrtd_size = ret_params->wsz;
     }

    //free remote cmd space

    rm_cmd_free(cmd);
    return wrtd_size;
}

size_t fwrite (const void *__restrict __ptr, size_t __size,
                size_t __n, FILE *__restrict __s)
{
   uint32_t sends = __size* __n;
   uint32_t  times = sends/RM_CMD_SEND_FRAME_SIZE;
   uint16_t remain = sends%RM_CMD_SEND_FRAME_SIZE;
   size_t writes = 0;
   for(int i = 0; i < times; i++){
     writes += _fwrite ((char*)__ptr + RM_CMD_SEND_FRAME_SIZE *i,1 ,RM_CMD_SEND_FRAME_SIZE, __s);
   }
   if (remain){
    writes += _fwrite ((char*)__ptr + RM_CMD_SEND_FRAME_SIZE *times ,1 , remain, __s);
   }
   return writes;
}


/*
   Seek to a certain position on STREAM.
   This function is a possible cancellation point and therefore not
   marked with __THROW.
*/

typedef struct  __attribute__((packed))  rm_cmd_fseek_param{
    long int off;
    int whence;
    uint32_t  hfile;
}rm_cmd_fseek_param_t ;

typedef struct  __attribute__((packed))  rm_cmd_fseek_ret_param{
    long int off;
    uint32_t  hfile;
}rm_cmd_fseek_ret_param_t ;


int fseek (FILE *__stream, long int __off, int __whence)
{
    rm_cmd_t *rpc_teturn = NULL;
    rm_cmd_t * cmd = NULL;
    uint32_t  data_sz = 0;
    int off = 0;
    rm_cmd_fseek_param_t *params = NULL;
    rm_cmd_fseek_ret_param_t *ret_params = NULL;
    FILE *file = NULL;

    // call local service to handle req
    file = __stream;

    if (file == 0 ){
      printf("%s invalid file handle !!!!!!", __func__);
      return 0;

    }

    data_sz =  sizeof(rm_cmd_fseek_param_t);

    cmd = (rm_cmd_t * )rm_cmd_malloc(RM_CMD_HEADER_LENGTH + data_sz);

    params = (rm_cmd_fseek_param_t *)((char *)cmd + RM_CMD_HEADER_LENGTH);
    params->off = __off;
    params->whence = __whence;
    params->hfile = (uint32_t)file;

    set_cmd_header_idx(cmd,RM_CMD_FSEEK);
    set_cmd_header_type(cmd,RM_CMD_TYPE_REQ);
    set_cmd_header_data_length(cmd,data_sz);
    // send request to server, wait for result
    rpc_trans(cmd,&rpc_teturn);

    // check result , free remote cmd space
    get_cmd_header_data_length(rpc_teturn, &data_sz);
    if (data_sz){
      ret_params = (rm_cmd_fseek_ret_param_t *)rpc_teturn->data ;
      off = ret_params->off;
     }

    rm_cmd_free(cmd);
    return off;
}


/*
  Return the current position of STREAM.
  This function is a possible cancellation point and therefore not
  marked with __THROW.
*/
typedef struct  __attribute__((packed))  rm_cmd_ftell_param{
    uint32_t  hfile;
}rm_cmd_ftell_param_t ;

typedef struct  __attribute__((packed))  rm_cmd_ftell_ret_param{
    long int off;
    uint32_t  hfile;
}rm_cmd_ftell_ret_param_t ;


long ftell (FILE *__stream)
{
    rm_cmd_t *rpc_teturn = NULL;
    rm_cmd_t * cmd = NULL;
    uint32_t  data_sz = 0;
    long off = 0;
    rm_cmd_ftell_param_t *params = NULL;
    rm_cmd_ftell_ret_param_t *ret_params = NULL;
    FILE *file = NULL;

    // call local service to handle req
    file = __stream;

    if (file == 0 ){
      printf("%s invalid file handle !!!!!!", __func__);
      return 0;

    }

    data_sz =  sizeof(rm_cmd_ftell_param_t);

    cmd = (rm_cmd_t * )rm_cmd_malloc(RM_CMD_HEADER_LENGTH + data_sz);

    params = (rm_cmd_ftell_param_t *)((char *)cmd + RM_CMD_HEADER_LENGTH);
    params->hfile = (uint32_t)file;

    set_cmd_header_idx(cmd,RM_CMD_FTELL);
    set_cmd_header_type(cmd,RM_CMD_TYPE_REQ);
    set_cmd_header_data_length(cmd,data_sz);
    // send request to server, wait for result
    rpc_trans(cmd,&rpc_teturn);

    // check result , free remote cmd space
    get_cmd_header_data_length(rpc_teturn, &data_sz);
    if (data_sz){
      ret_params = (rm_cmd_ftell_ret_param_t *)rpc_teturn->data ;
      off = ret_params->off;
     }

    rm_cmd_free(cmd);
    return off;
}

/*rx tx handler on uart port*/
#define RM_CMD_RCVBUFF_SIZE 2048
static char rcv_buffer[RM_CMD_RCVBUFF_SIZE];
static char temp_buffer[RM_CMD_RCVBUFF_SIZE/2];
static uint16_t wrptr = 0;


static  rm_cmd_t *find_rcv_cmd(void)
{
  rm_cmd_t * cmd = NULL;
  uint16_t offset = 0;
  if( 0 == wrptr ) return cmd;
  cmd = (rm_cmd_t *)strstr((char *)rcv_buffer,"RMCMD");
  if (cmd){
      offset =  (uint32_t)(cmd) -(uint32_t)(rcv_buffer);
      if (offset){
         memmove((char *)rcv_buffer, (char *)cmd, wrptr - offset);
         wrptr -= offset;
         cmd = (rm_cmd_t *)rcv_buffer;
      }
  }
  return cmd;
}


static int rm_cmd_receive_cmd(rm_cmd_t * cmd)
{
   rm_cmd_idx_t idx;
   get_cmd_header_idx(cmd, (uint8_t*)&idx);

   switch (idx){
       case  RM_CMD_FOPEN:
       case  RM_CMD_FCLOSE:
       case  RM_CMD_FREAD:
       case  RM_CMD_FWRITE:
       case  RM_CMD_FTELL:
       case  RM_CMD_FSEEK:
          printf("%s  cmd: %s ",rm_cmds[idx]);

          break;
       default:
          printf("%s invalid cmd !!!!!!",__func__);
          break;
   }

   return 0;
}

static rm_cmd_t * rcved_cmd = NULL;

static bool rm_cmd_rx_process(char *buffer, uint16_t sz )
{
   bool complete = false;
   rm_cmd_t * cmd = NULL;
   uint32_t data_length = 0;
   uint32_t buffer_free_size = RM_CMD_RCVBUFF_SIZE - wrptr;
   if (buffer_free_size > sz){
     memcpy(rcv_buffer + wrptr, buffer, sz);
     wrptr += sz;
   }else{
     printf("%s no more rm cmd receive space !!!!!! ", __func__);
     wrptr = 0;
     return 0;
   }
   if (wrptr <= 0){
     return 0;
   }
   cmd = find_rcv_cmd();

   if (cmd){
      get_cmd_header_data_length(cmd, &data_length);
      #ifdef RM_FILESYS_SERVER

      if ((data_length + RM_CMD_HEADER_LENGTH)  <= (wrptr  - ((uint64_t)cmd -(uint64_t)rcv_buffer ) ) ){ //get a complete package

      #else
      if ((data_length + RM_CMD_HEADER_LENGTH)  <= (wrptr  - ((uint32_t)cmd -(uint32_t)rcv_buffer ) ) ){ //get a complete package
      #endif
          rm_cmd_receive_cmd(cmd);
          rcved_cmd = cmd;
          wrptr = 0;
          complete = true;
      }

   }

  return complete;
}

/*
    call uart driver, send rm_cmd to peer, read request result from peer
*/


static int rpc_trans(rm_cmd_t * cmd, rm_cmd_t ** result_cmd )
{
  uint32_t cmd_length = 0 ;
  bool complete = false;
  int read_sz = RM_CMD_RCVBUFF_SIZE/2;
  int rsz = 0;

  get_cmd_header_data_length(cmd, &cmd_length);

  cmd_length += RM_CMD_HEADER_LENGTH ;
  set_cmd_header_magic_string(cmd);


  uart_write(cmd, cmd_length);

  if (result_cmd){ //wait result from peer
     while(1){
        rsz = uart_rx_process((char*)temp_buffer,read_sz );
        if (rsz <= 0){
          continue;
        }
        printf("%s rsz:%d",__func__, rsz);
        complete = rm_cmd_rx_process((char *)temp_buffer, rsz);
        if (complete){
            *result_cmd = rcved_cmd;

            break;
        }
     }
  }

  return 0;
}

static int uart_write(void *buf, uint32_t len)
{
    int ret = 0;
    ret = hal_trace_output(buf, len);
    return ret;
}


/*
    buffer usage:
      read pointer always less than write  pointer.
*/

static uint8_t rpc_uart_rx_buffer[RM_CMD_RCVBUFF_SIZE];
static uint8_t rpc_uart_rcv_buffer[RM_CMD_RCVBUFF_SIZE];
static volatile uint16_t uart_wrptr = 0;
static volatile uint16_t uart_rdptr = 0;


/*
    called in the contex of interrupt disable
*/
unsigned int remote_cmd_receive_buffer_push(unsigned char *data,unsigned size)
{
   int buffer_free_size = 0;
   uint32_t st;
   st = int_lock();
   _re_push:
   buffer_free_size = RM_CMD_RCVBUFF_SIZE - uart_wrptr;
   if (buffer_free_size > size){
     memcpy(rpc_uart_rcv_buffer + uart_wrptr, data, size);
     uart_wrptr += size;
   }else{

     memmove(rpc_uart_rcv_buffer, rpc_uart_rcv_buffer + uart_rdptr, uart_wrptr - uart_rdptr );

     uart_wrptr = uart_wrptr - uart_rdptr;
     uart_rdptr = 0;
     goto _re_push;
     if (RM_CMD_RCVBUFF_SIZE - uart_wrptr < size){
        printf("%s no more rm cmd receive space !!!!!! ", __func__);
        uart_wrptr = 0;
        uart_rdptr = 0;
     }

   }
   int_unlock(st);
   return 0;
}


void sfs_setup_uart_handler(void)
{
    hal_trace_rx_open(rpc_uart_rx_buffer ,RM_CMD_RCVBUFF_SIZE, remote_cmd_receive_buffer_push);
}

static int uart_rx_process(char *buffer,int readsz )
{
   int ret = 0;
   int datainbuffer = 0;
   uint32_t st;
   st = int_lock();

   if (uart_rdptr > uart_wrptr) {
      uart_rdptr = 0;
   }

   datainbuffer = uart_wrptr - uart_rdptr;
   if (datainbuffer == 0){
     int_unlock(st);
     return 0;
   }

   if (datainbuffer < readsz){
     memcpy(buffer, rpc_uart_rcv_buffer + uart_rdptr, datainbuffer );
     ret = datainbuffer;
   }else {
     memcpy(buffer, rpc_uart_rcv_buffer + uart_rdptr, readsz );
     ret = readsz;
   }
   uart_rdptr += ret;
   int_unlock(st);
   return ret;
}

#endif
