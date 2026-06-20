#ifndef __APP_TOTA_DATA_TRANSCEIVER_H__
#define __APP_TOTA_DATA_TRANSCEIVER_H__


#define MAX_DATA_LENGTH         (4096)

typedef enum{
    WRITE_FLASH,
    WRITE_RAM,
    READ_FLASH,
    READ_RAM,
}mode_e;


/* ---REQUEST---> */
/* <---ACCEPT--- */
/* <--- DATA ---> */
/* <---SUCCESS--- */
typedef enum{
    REQUEST,
    ACCEPT,
    REJECT,
    SUCCESS,
    FAILED,
    READ_DONE,
    TIMEOUT,
}status_e;

typedef enum{
    HANDLE_FINISH,
    HANDLE_OK,
    HANDLE_ERROR,
    HANDLE_SEND_DATA_DONE
}handle_status_e;

typedef struct{
    /* write flash or just in ram */
    uint8_t mode;
    /* data streaming status */
    uint8_t status;
    /* the max packet size */
    uint16_t max_packet_size;
    /* the length of the data will be transfered */
    uint32_t data_length;
    /* may need: the crc of the data */
    uint32_t crc;
    /* address for write or read */
    uint32_t addr;
}DATA_TRANSCEIVER_CTL_T;

#endif