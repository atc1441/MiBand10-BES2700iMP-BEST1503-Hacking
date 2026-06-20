/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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

#define RM_FILESYS_SERVER

#ifdef RM_FILESYS_SERVER

#include<map>
using namespace std;

void *rm_cmd_malloc(size_t __size)
{
    return malloc(__size);
}

void rm_cmd_free(void *ptr)
{
    free(ptr);
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
} rm_cmd_idx_t;

typedef struct __attribute__ ((packed)) {
    char header[RM_CMD_HEADER_LENGTH];
    char data[1];
} rm_cmd_t;

typedef struct __attribute__ ((packed)) {
    uint32_t buf_sz;
    char buff[1];
} buff_ptr_t;

static int rpc_trans(rm_cmd_t * cmd, rm_cmd_t ** result_cmd);

static int set_cmd_header_magic_string(rm_cmd_t * cmd)
{
    int ret = 0;
    char *ptr = cmd->header;
    memcpy(ptr, "RMCMD\0", 6);
    return ret;
}

static int set_cmd_header_idx(rm_cmd_t * cmd, uint8_t idx)
{
    int ret = 0;
    char *ptr = cmd->header;
    ptr[6] = idx & 0xff;
    return ret;
}

static int get_cmd_header_idx(rm_cmd_t * cmd, uint8_t * idx)
{
    int ret = 0;
    char *ptr = cmd->header;
    *idx = ptr[6];
    return ret;
}

static int set_cmd_header_type(rm_cmd_t * cmd, uint8_t type)
{
    int ret = 0;
    char *ptr = cmd->header;
    ptr[7] = 0xff & type;
    return ret;
}

#if 0
static POSSIBLY_UNUSED int get_cmd_header_type(rm_cmd_t * cmd, uint8_t * type)
{
    int ret = 0;
    char *ptr = cmd->header;
    *type = ptr[7];
    return ret;
}

#endif

static int set_cmd_header_data_length(rm_cmd_t * cmd, uint32_t sz)
{
    int ret = 0;
    char *ptr = &cmd->header[12];
    ptr[0] = sz & 0xff;
    ptr[1] = (sz & 0xff00) >> 8;
    ptr[2] = (sz & 0xff0000) >> 16;
    ptr[3] = (sz & 0xff000000) >> 24;

    return ret;
}

static int get_cmd_header_data_length(rm_cmd_t * cmd, uint32_t * sz)
{
    int ret = 0;
    char *ptr = &cmd->header[12];
    *sz = ((ptr[3] << 24) & 0xff000000) | ((ptr[2] << 16) & 0xff0000) | ((ptr[1] << 8) & 0xff00) | (ptr[0] & 0xff);
    return ret;
}

#if 0
static POSSIBLY_UNUSED int set_cmd_header_crc(rm_cmd_t * cmd, uint32_t crc)
{
    int ret = 0;
    char *ptr = &cmd->header[8];
    ptr[0] = crc & 0xff;
    ptr[1] = (crc & 0xff00) >> 8;
    ptr[2] = (crc & 0xff0000) >> 16;
    ptr[3] = (crc & 0xff000000) >> 24;

    return ret;
}

static POSSIBLY_UNUSED int get_cmd_header_crc(rm_cmd_t * cmd, uint32_t * crc)
{
    int ret = 0;
    char *ptr = &cmd->header[8];
    *crc = (ptr[2] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0];
    return ret;
}
#endif

#if 0
static int get_cmd_data_buffer_ptr(rm_cmd_t * cmd, buff_ptr_t * ptr)
{
    uint32_t size = 0;
    ptr->buff = cmd->data;
    get_cmd_header_data_length(cmd, &size);
    ptr->buf_sz = size;
    return 0;
}
#endif

static int uart_rx_process(char *buffer, int read_sz);
static int uart_write(void *cmd, int cmd_length);
static int rpc_trans(rm_cmd_t * cmd, rm_cmd_t ** result_cmd);

typedef struct __attribute__ ((packed)) rm_cmd_fopen_ret_param {
    uint8_t hfile[4];
} rm_cmd_fopen_ret_param_t;

static int rm_cmd_fopen_param_set_name(buff_ptr_t * params, char *__filename)
{
    int sz = strlen(__filename);
    params->buf_sz = sz;
    memcpy(params->buff, __filename, sz);
    return 0;
}

static int rm_cmd_fopen_param_set_modes(buff_ptr_t * params, char *modes)
{
    int sz = strlen(modes);
    params->buf_sz = sz;
    memcpy(params->buff, modes, sz);
    return 0;
}

static FILE *rm_cmd_fopen_get_return_file(rm_cmd_t * cmd)
{

    FILE *file = NULL;
    uint8_t idx = 0xff;
    rm_cmd_fopen_ret_param_t *ret;
    get_cmd_header_idx(cmd, &idx);
    if (idx != RM_CMD_FOPEN) {
        return file;
    }
    ret = (rm_cmd_fopen_ret_param_t *) cmd->data;
    file = (FILE *) (*(uint32_t *) ret->hfile);
    return file;
}

map < uint32_t, uint64_t > files;
map < uint32_t, uint64_t >::iterator iter;

static uint32_t get_4byte_fp(FILE * file)
{
    uint32_t handle = 0;
    handle = 0xffffffff & (uint64_t) file;
    files.insert(pair < uint32_t, uint64_t > (handle, (uint64_t) file));
    return handle;
}

static FILE *get_file_handle(uint32_t file)
{
    FILE *handle = 0;

    iter = files.find(file);
    handle = (FILE *) iter->second;

    return handle;
}

static void remove_file(uint32_t file)
{
    iter = files.find(file);
    files.erase(iter);
}

static int fopen_srv(buff_ptr_t * params)
{

    rm_cmd_t *cmd = NULL;
    int data_sz = 0;
    char file_name[128];
    char *modes = NULL;

    rm_cmd_fopen_ret_param_t *ret_params = NULL;

    FILE *file = NULL;

    // call local service to handle req
    buff_ptr_t *filename_ = params;
    buff_ptr_t *mode_ = (buff_ptr_t *) ((char *)params + sizeof(buff_ptr_t) - 1 + filename_->buf_sz);

    data_sz = filename_->buf_sz;
    memcpy(file_name, (char *)filename_->buff, data_sz);
    file_name[data_sz] = 0;

    if (data_sz == 0) {
        printf("%s invalid file name !!!!!!\r\n", __func__);
        file = NULL;
    } else {
        printf("%s %s\r\n", __func__, file_name);
        modes = (char *)mode_->buff;
        file = fopen(file_name, modes);
        printf("%s %s m: %s h:%p \r\n", __func__, file_name, modes, file);
    }

    // send back  result

    data_sz = sizeof(rm_cmd_fopen_ret_param_t);
    cmd = (rm_cmd_t *) malloc(RM_CMD_HEADER_LENGTH + data_sz);

    ret_params = (rm_cmd_fopen_ret_param_t *) ((char *)cmd + RM_CMD_HEADER_LENGTH);
    *(uint32_t *) ret_params->hfile = get_4byte_fp(file);

    printf("%s file: %x\r\n", __func__, *(uint32_t *) ret_params->hfile);

    set_cmd_header_idx(cmd, RM_CMD_FOPEN);
    set_cmd_header_type(cmd, RM_CMD_TYPE_ANSWER);
    set_cmd_header_data_length(cmd, data_sz);

    // send to client in block mode, no confirm from client
    rpc_trans(cmd, NULL);

    rm_cmd_free(cmd);
    return 0;
}

/* Close STREAM.
   This function is a possible cancellation point and therefore not
   marked with __THROW.
*/

typedef struct __attribute__ ((packed)) rm_cmd_fclose_param {
    uint32_t hfile;
} rm_cmd_fclose_param_t;

typedef struct __attribute__ ((packed)) rm_cmd_fclose_ret_param {
    uint32_t hfile;
} rm_cmd_fclose_ret_param_t;

static int fclose_srv(rm_cmd_fclose_param_t * params)
{
    rm_cmd_t *cmd = NULL;
    uint32_t file = params->hfile;
    rm_cmd_fclose_ret_param_t *ret_params = NULL;

    printf("%s %x\r\n", __func__, file);

    if (file) {
        fclose(get_file_handle(file));

    }

    int data_sz = sizeof(rm_cmd_fclose_ret_param_t);

    cmd = (rm_cmd_t *) malloc(RM_CMD_HEADER_LENGTH + data_sz);
    ret_params = (rm_cmd_fclose_ret_param_t *) ((char *)cmd + RM_CMD_HEADER_LENGTH);
    ret_params->hfile = file;

    set_cmd_header_idx(cmd, RM_CMD_FCLOSE);
    set_cmd_header_type(cmd, RM_CMD_TYPE_ANSWER);
    set_cmd_header_data_length(cmd, data_sz);

    // send result to client, free remote cmd space
    rpc_trans(cmd, NULL);

    rm_cmd_free(cmd);

    return 0;
}

/* Flush STREAM, or all streams if STREAM is NULL.
   This function is a possible cancellation point and therefore not
   marked with __THROW.
*/

typedef struct __attribute__ ((packed)) rm_cmd_fflush_param {
    uint32_t hfile;
} rm_cmd_fflush_param_t;

typedef struct __attribute__ ((packed)) rm_cmd_fflush_ret_param {
    uint32_t hfile;
} rm_cmd_fflush_ret_param_t;

static int fflush_srv(rm_cmd_fflush_param_t * params)
{
    rm_cmd_t *cmd = NULL;
    uint32_t file = params->hfile;
    rm_cmd_fflush_ret_param_t *ret_params = NULL;
    if (file) {
        fclose(get_file_handle(file));
    }

    int data_sz = sizeof(rm_cmd_fflush_ret_param_t);

    cmd = (rm_cmd_t *) malloc(RM_CMD_HEADER_LENGTH + data_sz);
    ret_params = (rm_cmd_fflush_ret_param_t *) ((char *)cmd + RM_CMD_HEADER_LENGTH);
    ret_params->hfile = file;

    set_cmd_header_idx(cmd, RM_CMD_FCLOSE);
    set_cmd_header_type(cmd, RM_CMD_TYPE_ANSWER);
    set_cmd_header_data_length(cmd, data_sz);

    // send result to client, free remote cmd space
    rpc_trans(cmd, NULL);

    rm_cmd_free(cmd);

    return 0;
}

/*
  Read chunks of generic data from STREAM.
  This function is a possible cancellation point and therefore not
  marked with __THROW.
*/

typedef struct __attribute__ ((packed)) rm_cmd_fread_param {
    uint16_t rsz;
    uint32_t hfile;
} rm_cmd_fread_param_t;

typedef struct __attribute__ ((packed)) rm_cmd_fread_ret_param {
    uint32_t hfile;
    buff_ptr_t data;
} rm_cmd_fread_ret_param_t;

static size_t fread_srv(rm_cmd_fread_param_t * params)
{
    rm_cmd_t *cmd = NULL;
    uint32_t data_sz = 0;
    size_t rdd_size = 0;
    rm_cmd_fread_ret_param_t *ret_params = NULL;
    uint32_t file = NULL;
    char *buffer = NULL;
    // call local service to handle req
    file = params->hfile;
    data_sz = params->rsz;

    if (file == 0) {
        printf("%s invalid file handle !!!!!!\r\n", __func__);
        file = NULL;
    } else {
        buffer = (char *)rm_cmd_malloc(data_sz);
        rdd_size = fread(buffer, 1, data_sz, get_file_handle(file));
    }

    // send back  result

    data_sz = sizeof(rm_cmd_fread_ret_param_t) - 1 + rdd_size;
    cmd = (rm_cmd_t *) rm_cmd_malloc(RM_CMD_HEADER_LENGTH + data_sz);
    ret_params = (rm_cmd_fread_ret_param_t *) ((char *)cmd + RM_CMD_HEADER_LENGTH);
    if (rdd_size) {
        memcpy(&ret_params->data.buff[0], buffer, rdd_size);
        ret_params->data.buf_sz = rdd_size;
        rm_cmd_free(buffer);
    }

    ret_params->hfile = file;
    set_cmd_header_idx(cmd, RM_CMD_FREAD);
    set_cmd_header_type(cmd, RM_CMD_TYPE_ANSWER);
    set_cmd_header_data_length(cmd, data_sz);

    // send to client in block mode, no confirm from client, free rm cmd space
    rpc_trans(cmd, NULL);

    rm_cmd_free(cmd);
    return 0;

}

/* Write chunks of generic data to STREAM.
   This function is a possible cancellation point and therefore not
   marked with __THROW.
*/

typedef struct __attribute__ ((packed)) rm_cmd_fwrite_param {
    uint32_t hfile;
    buff_ptr_t data;
} rm_cmd_fwrite_param_t;

typedef struct __attribute__ ((packed)) rm_cmd_fwrite_ret_param {
    uint16_t wsz;
    uint32_t hfile;
} rm_cmd_fwrite_ret_param_t;

static size_t fwrite_srv(rm_cmd_fwrite_param_t * params)
{
    rm_cmd_t *cmd = NULL;
    uint32_t data_sz = 0;
    size_t wrtd_size = 0;
    rm_cmd_fwrite_ret_param_t *ret_params = NULL;
    uint32_t file = NULL;

    // call local service to handle req
    file = params->hfile;
    data_sz = params->data.buf_sz;;

    if (file == 0) {
        printf("%s invalid file handle !!!!!!\r\n", __func__);
        return 0;

    }

    wrtd_size = fwrite(params->data.buff, 1, data_sz, get_file_handle(file));
    //printf("%s fi:%x h:%p ws:%d", __func__, file,get_file_handle(file),wrtd_size);

    data_sz = sizeof(rm_cmd_fwrite_ret_param_t);

    cmd = (rm_cmd_t *) rm_cmd_malloc(RM_CMD_HEADER_LENGTH + data_sz);

    ret_params = (rm_cmd_fwrite_ret_param_t *) ((char *)cmd + RM_CMD_HEADER_LENGTH);
    ret_params->wsz = wrtd_size;
    ret_params->hfile = file;

    set_cmd_header_idx(cmd, RM_CMD_FWRITE);
    set_cmd_header_type(cmd, RM_CMD_TYPE_ANSWER);
    set_cmd_header_data_length(cmd, data_sz);
    // send request to server, free rm cmd space result
    rpc_trans(cmd, NULL);

    //free remote cmd space

    rm_cmd_free(cmd);
    return 0;
}

/*
   Seek to a certain position on STREAM.
   This function is a possible cancellation point and therefore not
   marked with __THROW.
*/

typedef struct __attribute__ ((packed)) rm_cmd_fseek_param {
    int off;
    int whence;
    uint32_t hfile;
} rm_cmd_fseek_param_t;

typedef struct __attribute__ ((packed)) rm_cmd_fseek_ret_param {
    int off;
    uint32_t hfile;
} rm_cmd_fseek_ret_param_t;

static int fseek_srv(rm_cmd_fseek_param_t * params)
{
    rm_cmd_t *cmd = NULL;
    uint32_t data_sz = 0;
    int off = 0;
    rm_cmd_fseek_ret_param_t *ret_params = NULL;
    uint32_t file = NULL;

    // call local service to handle req
    file = params->hfile;

    if (file == 0) {
        printf("%s invalid file handle !!!!!!\r\n", __func__);
        return 0;

    }

    off = fseek(get_file_handle(file), params->off, params->whence);
    data_sz = sizeof(rm_cmd_fseek_ret_param_t);

    cmd = (rm_cmd_t *) rm_cmd_malloc(RM_CMD_HEADER_LENGTH + data_sz);

    ret_params = (rm_cmd_fseek_ret_param_t *) ((char *)cmd + RM_CMD_HEADER_LENGTH);
    ret_params->off = off;
    ret_params->hfile = file;

    set_cmd_header_idx(cmd, RM_CMD_FSEEK);
    set_cmd_header_type(cmd, RM_CMD_TYPE_ANSWER);
    set_cmd_header_data_length(cmd, data_sz);
    //send result to client, free remote cmd space
    rpc_trans(cmd, NULL);

    rm_cmd_free(cmd);
    return off;
}

/*
  Return the current position of STREAM.
  This function is a possible cancellation point and therefore not
  marked with __THROW.
*/
typedef struct __attribute__ ((packed)) rm_cmd_ftell_param {
    uint32_t hfile;
} rm_cmd_ftell_param_t;

typedef struct __attribute__ ((packed)) rm_cmd_ftell_ret_param {
    long int off;
    uint32_t hfile;
} rm_cmd_ftell_ret_param_t;

static int ftell_srv(rm_cmd_ftell_param_t * params)
{
    rm_cmd_t *cmd = NULL;
    uint32_t data_sz = 0;
    int off = 0;
    rm_cmd_ftell_ret_param_t *ret_params = NULL;
    uint32_t file = NULL;

    // call local service to handle req
    file = params->hfile;

    if (file == 0) {
        printf("%s invalid file handle !!!!!!\r\n", __func__);
        return 0;

    }

    off = ftell(get_file_handle(file));
    data_sz = sizeof(rm_cmd_ftell_ret_param_t);

    cmd = (rm_cmd_t *) rm_cmd_malloc(RM_CMD_HEADER_LENGTH + data_sz);

    ret_params = (rm_cmd_ftell_ret_param_t *) ((char *)cmd + RM_CMD_HEADER_LENGTH);
    ret_params->off = off;
    ret_params->hfile = file;

    set_cmd_header_idx(cmd, RM_CMD_FSEEK);
    set_cmd_header_type(cmd, RM_CMD_TYPE_ANSWER);
    set_cmd_header_data_length(cmd, data_sz);
    //send result to client, free remote cmd space
    rpc_trans(cmd, NULL);

    rm_cmd_free(cmd);
    return off;
}

/*rx tx handler on uart port*/
#define RM_CMD_RCVBUFF_SIZE 2048
static char rcv_buffer[RM_CMD_RCVBUFF_SIZE];
static char temp_buffer[RM_CMD_RCVBUFF_SIZE / 2];
static uint16_t wrptr = 0;

static rm_cmd_t *rcv_cmd_ptr = NULL;

static rm_cmd_t *find_rcv_cmd(void)
{
    rm_cmd_t *cmd = NULL;

    cmd = (rm_cmd_t *) memmem((char *)rcv_buffer, RM_CMD_RCVBUFF_SIZE, "RMCMD", 5);
    return cmd;
}

static char *rm_cmds[] = {
    "RM_CMD_FOPEN",
    "RM_CMD_FCLOSE",
    "RM_CMD_FREAD",
    "RM_CMD_FWRITE",
    "RM_CMD_FTELL",
    "RM_CMD_FSEEK"
};

static int rm_cmd_receive_cmd(rm_cmd_t * cmd)
{
    rm_cmd_idx_t idx;
    get_cmd_header_idx(cmd, (uint8_t *) & idx);

    switch (idx) {
    case RM_CMD_FOPEN:
    case RM_CMD_FCLOSE:
    case RM_CMD_FREAD:
    case RM_CMD_FWRITE:
    case RM_CMD_FTELL:
    case RM_CMD_FSEEK:

#ifdef SRV_DEBUG
        printf("cmd: %s\r\n ", rm_cmds[idx]);
#endif
        break;
    default:
        printf("%s invalid cmd !!!!!!\r\n", __func__);
        break;
    }

    return 0;
}

static bool rm_cmd_rx_process(char *buffer, uint16_t sz)
{
    bool complete = false;
    rm_cmd_t *cmd = NULL;
    uint32_t data_length = 0;
    uint32_t buffer_free_size = RM_CMD_RCVBUFF_SIZE - wrptr;
    if (buffer_free_size > sz) {
        memcpy(rcv_buffer + wrptr, buffer, sz);
        wrptr += sz;
    } else {
        printf("%s no more rm cmd receive space !!!!!!\r\n ", __func__);
        wrptr = 0;
        return 0;
    }

#ifdef SRV_DEBUG
    printf("%s\r\n", __func__);
#endif
    cmd = find_rcv_cmd();

    if (cmd) {
        get_cmd_header_data_length(cmd, &data_length);
#ifdef RM_FILESYS_SERVER
        if ((data_length + RM_CMD_HEADER_LENGTH) <= (wrptr - ((uint64_t) cmd - (uint64_t) rcv_buffer))) {    //get a complete package
#else
        if ((data_length + RM_CMD_HEADER_LENGTH) <= (wrptr - ((uint32_t) cmd - (uint32_t) rcv_buffer))) {    //get a complete package
#endif
            rm_cmd_receive_cmd(cmd);

            rcv_cmd_ptr = cmd;
            complete = true;
        }

    }

    return complete;
}

/*
    call uart driver, send rm_cmd to peer, read request result from peer
*/

static int rpc_trans(rm_cmd_t * cmd, rm_cmd_t ** result_cmd)
{
    uint32_t data_length = 0;
    bool complete = false;
    int read_sz = RM_CMD_RCVBUFF_SIZE / 2;
    int rsz = 0;
    uint16_t cmd_length = 0;
    get_cmd_header_data_length(cmd, &data_length);
    cmd_length = RM_CMD_HEADER_LENGTH + data_length;
    set_cmd_header_magic_string(cmd);
    uart_write(cmd, cmd_length);
    if (result_cmd) {    //wait result from peer
        while (1) {
            rsz = uart_rx_process((char *)temp_buffer, read_sz);
            if (rsz <= 0) {
                continue;
            }
            printf("%s rsz:%d \r\n", __func__, rsz);
            complete = rm_cmd_rx_process((char *)temp_buffer, rsz);
            if (complete) {
                *result_cmd = (rm_cmd_t *) rcv_buffer;
                break;
            }
        }
    }

    return 0;
}

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

static int uart_fd = 0;

static int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio, oldtio;
    if (tcgetattr(fd, &oldtio) != 0) {
        perror("SetupSerial 1 \r\n");
        return -1;
    }
    memset(&newtio, 0, sizeof(newtio));
    newtio.c_cflag |= CLOCAL | CREAD;

    newtio.c_cflag &= ~CSIZE;
    switch (nBits) {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    }
    switch (nEvent) {
    case 'O':
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'E':
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
    case 'N':
        newtio.c_cflag &= ~PARENB;
        break;
    }
    switch (nSpeed) {
    case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;
    case 4800:
        cfsetispeed(&newtio, B4800);
        cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    case 460800:
        cfsetispeed(&newtio, B460800);
        cfsetospeed(&newtio, B460800);
        break;
    case 921600:
        cfsetispeed(&newtio, B921600);
        cfsetospeed(&newtio, B921600);
        break;
    default:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    }
    if (nStop == 1)
        newtio.c_cflag &= ~CSTOPB;
    else if (nStop == 2)
        newtio.c_cflag |= CSTOPB;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;
    tcflush(fd, TCIFLUSH);
    if ((tcsetattr(fd, TCSANOW, &newtio)) != 0) {
        perror("com set error\r\n");
        return -1;
    }
//  printf("set done!\n\r");
    return 0;
}

static int uart_send(int fd, void *buf, int len)
{
    int ret = 0;
    int count = 0;

    tcflush(fd, TCIFLUSH);

    while (len > 0) {

        ret = write(fd, (char *)buf + count, len);
        if (ret < 1) {
            break;
        }
        count += ret;
        len = len - ret;
    }

    return count;
}

static int uart_write(void *cmd, int cmd_length)
{
    return uart_send(uart_fd, cmd, cmd_length);
}

static int uart_recv_timeout(void *buf, int len, int timeout_ms)
{
    int ret;
    size_t rsum = 0;
    fd_set rset;
    struct timeval t;
    char *c;
    while (rsum < len) {
        t.tv_sec = timeout_ms / 1000;
        t.tv_usec = (timeout_ms - t.tv_sec * 1000) * 1000;
        FD_ZERO(&rset);
        //printf("uart_fd:%x", uart_fd);
        FD_SET(uart_fd, &rset);
        ret = select(uart_fd + 1, &rset, NULL, NULL, &t);
        //printf("select result:%d", ret);
        if (ret == -1) {
            perror("select()\r\n");

        } else if (ret && FD_ISSET(uart_fd, &rset)) {
            ret = read(uart_fd, (char *)buf + rsum, len - rsum);
            if (ret < 0) {
                printf("ret:%d < 0\r\n", ret);
                return ret;
            } else {
#ifdef SRV_DEBUG
                printf("Data len %d<\r\n", ret);
                for (int i = 0; i < ret; i++) {
                    c = (char *)((char *)buf + rsum + i);
                    printf("0x%x ", *c & 0xff);
                }
                printf(">\r\n");
#endif

                rsum += ret;
                return rsum;
            }
        } else {
            //printf("No data within uart_fd.\n");
            continue;

        }

    }

    return rsum;
}

static int uart_rx_process(char *buffer, int read_sz)
{
    return uart_recv_timeout(buffer, read_sz, 100);
}

int rpc_receive_request(void)
{
    bool complete = false;
    int read_sz = 0;
    read_sz = uart_rx_process(temp_buffer, RM_CMD_RCVBUFF_SIZE / 2);
    if (read_sz <= 0) {
        return 0;
    }
#ifdef SRV_DEBUG
    printf("%s rsz:%d\r\n", __func__, read_sz);
#endif

    complete = rm_cmd_rx_process(temp_buffer, read_sz);
    if (!complete) {
        return 0;
    }

    rm_cmd_idx_t idx;
    uint8_t idx_ = 0;
    rm_cmd_t *cmd = (rm_cmd_t *) rcv_cmd_ptr;

    get_cmd_header_idx(cmd, &idx_);
    idx = (rm_cmd_idx_t) idx_;
    //printf("%s cmd:%s\r\n",__func__, rm_cmds[idx]);
    switch (idx) {
    case RM_CMD_FOPEN:
        fopen_srv((buff_ptr_t *) cmd->data);
        break;
    case RM_CMD_FCLOSE:
        fclose_srv((rm_cmd_fclose_param_t *) cmd->data);
        break;
    case RM_CMD_FREAD:
        fread_srv((rm_cmd_fread_param_t *) cmd->data);
        break;
    case RM_CMD_FWRITE:
        fwrite_srv((rm_cmd_fwrite_param_t *) cmd->data);
        break;
    case RM_CMD_FTELL:
        ftell_srv((rm_cmd_ftell_param_t *) cmd->data);
        break;
    case RM_CMD_FSEEK:
        fseek_srv((rm_cmd_fseek_param_t *) cmd->data);
        break;
    default:
        printf("%s invalid cmd !!!!!!\r\n", __func__);
        break;
    }
    //clear receive buffer
    complete = 0;
    memset(rcv_buffer, 0, RM_CMD_RCVBUFF_SIZE);
    wrptr = 0;
    return 0;
}

/*
   usage: sfstool /dev/ttyS0  work_dir
*/

int main(int argc, const char *argv[])
{
    char *uart = "/dev/ttyUSB0";
    char *work_dir = "~/";
    int baudrate = 115200;
    if (argc >= 4) {
        uart = (char *)argv[1];
        baudrate = atoi(argv[2]);
        work_dir = (char *)argv[3];
    }
    //change current dir to work dir
    chdir(work_dir);

    uart_fd = open(uart, O_RDWR);
    if (uart_fd <= 0) {
        printf("open %s  failed\r\n", uart);
        exit(0);
    }

    printf("open %s b %d fd: %d success\r\n", uart, baudrate, uart_fd);
    set_opt(uart_fd, baudrate, 8, 'N', 1);

    while (1) {
        rpc_receive_request();
    }

}
#endif
