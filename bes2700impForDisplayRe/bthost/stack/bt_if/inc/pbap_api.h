/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifndef __BT_APP_PBAP_API_H__
#define __BT_APP_PBAP_API_H__

#ifdef BT_PBAP_SUPPORT

#ifdef __cplusplus
extern "C" {
#endif

#define PBAP_RSP_SUCCESS 0xa0

typedef enum {
    PBAP_CHANNEL_CLOSED = 0,
    PBAP_OBEX_CONNECTED,
    PBAP_CHANNEL_OPENED,
    PBAP_PHONEBOOK_SIZE_RSP,
    PBAP_NEW_MISSED_CALLS_RSP,
    PBAP_VCARD_LISTING_ITEM_RSP,
    PBAP_VCARD_ENTRY_ITEM_RSP,
    PBAP_SET_PHONEBOOK_DONE,
    PBAP_ABORT_CURR_OP_DONE,
    PBAP_PULL_PHONEBOOK_DONE,
    PBAP_PULL_VCARD_LISTING_DONE,
    PBAP_PULL_VCARD_ENTRY_DONE,
} pbap_event_t;

/**
 * The PropertySelector is a property mask (64-bit value), used to
 * indicate the properties contained in the requested vCard objects.
 * The PCE can use this header to receive only the desired content
 * of the requested vCard(s). All properties of the vCard shall
 * be returned if this header is not specified or contains all zeros.
 *
 * Mandatory properties for vCard 2.1 are VERSION, N and TEL.
 * Mandatory properties for vCard 3.0 are VERSION, N, FN and TEL.
 * If the entry does not have any TEL information, an empty TEL
 * property will be included.
 *
 * With the exception of the above propertities, the properties
 * that are requested by the PCE can be ignored by the PSE in case
 * the PSE doesn't support them. However, all the properties that
 * are indeed supported by the PSE shall be returned if their value
 * is not NULL.
 *
 * The PSE shall not return any other properties than those requested
 * by the PCE. In the case of multiple instances of the same property,
 * for instance when more than 1 telephone number is present in a
 * phonebook entry, all of the instances shall be returned by the PSE.
 *
 */

#define PBAP_PROPERTY_BYTE0_VERSION     0x01    /* vCard Version */
#define PBAP_PROPERTY_BYTE0_FN          0x02    /* Formatted Name */
#define PBAP_PROPERTY_BYTE0_N           0x04    /* Structured Presentation of Name */
#define PBAP_PROPERTY_BYTE0_PHOTO       0x08    /* Associated Image or Photo */
#define PBAP_PROPERTY_BYTE0_BDAY        0x10    /* Birthday */
#define PBAP_PROPERTY_BYTE0_ADR         0x20    /* Delivery Address */
#define PBAP_PROPERTY_BYTE0_LABEL       0x40    /* Delivery */
#define PBAP_PROPERTY_BYTE0_TEL         0x80    /* Telephone Number */
#define PBAP_PROPERTY_BYTE1_EMAIL       0x01    /* Electronic Mail Address */
#define PBAP_PROPERTY_BYTE1_MAILER      0x02    /* Electronic Mail */
#define PBAP_PROPERTY_BYTE1_TZ          0x04    /* Time Zone */
#define PBAP_PROPERTY_BYTE1_GEO         0x08    /* Geographic Position */
#define PBAP_PROPERTY_BYTE1_TITLE       0x10    /* Job */
#define PBAP_PROPERTY_BYTE1_ROLE        0x20    /* Role within the Organization */
#define PBAP_PROPERTY_BYTE1_LOGO        0x40    /* Organization Logo */
#define PBAP_PROPERTY_BYTE1_AGENT       0x80    /* vCard of Person Representing */
#define PBAP_PROPERTY_BYTE2_ORG         0x01    /* Name of Organization */
#define PBAP_PROPERTY_BYTE2_NOTE        0x02    /* Comments */
#define PBAP_PROPERTY_BYTE2_REV         0x04    /* Revision */
#define PBAP_PROPERTY_BYTE2_SOUND       0x08    /* Pronunciation of Name */
#define PBAP_PROPERTY_BYTE2_URL         0x10    /* Uniform Resource Locator */
#define PBAP_PROPERTY_BYTE2_UID         0x20    /* Unique ID */
#define PBAP_PROPERTY_BYTE2_KEY         0x40    /* Public Encryption Key */
#define PBAP_PROPERTY_BYTE2_NICKNAME    0x80    /* Nickname */
#define PBAP_PROPERTY_BYTE3_CATEGORIES  0x01    /* Categories */
#define PBAP_PROPERTY_BYTE3_PROID       0x02    /* Product ID */
#define PBAP_PROPERTY_BYTE3_CLASS       0x04    /* Class information */
#define PBAP_PROPERTY_BYTE3_SORT_STRING 0x08    /* String used for sorting operations */
#define PBAP_PROPERTY_BYTE3_X_IRMC_CALL_DATETIME    0x10    /* Time stamp */
#define PBAP_PROPERTY_BYTE3_X_BT_SPEEDDIAKKEY       0x20    /* Speed-dial shortcut */
#define PBAP_PROPERTY_BYTE3_X_BT_UCI                0x40    /* Uniform Caller Identifer */
#define PBAP_PROPERTY_BYTE3_X_BT_UID                0x80    /* Bluetooth Contact Unique Identifier */
#define PBAP_PROPERTY_BYTE4_PROPRIETARY_FILTER      0x80    /* indicates the usage of a proprietary filter */

#define PBAP_MAX_VCARD_ENTRY_NAME_SIZE 42

#define PBAP_MAX_VCARD_CONTACT_NAME_SIZE 80

#define PBAP_MAX_VCARD_FORAMTTED_NAME_SIZE 80

#define PBAP_MAX_VCARD_CONTACT_ADDR_SIZE 128

#define PBAP_MAX_VCARD_CONTACT_TEL_SIZE 32

#define PBAP_MAX_VCARD_CONTACT_EMAIL_SIZE 64

#define PBAP_MAX_VCARD_CONTACT_X_BT_UID_PREFIX_SIZE 9

#define PBAP_MAX_VCARD_CONTACT_X_BT_UID_SIZE 40

#define PBAP_MAX_VCARD_CONTACT_DATE_SIZE 32

#define PBAP_MAX_VCARD_CONTACT_TYPE_SIZE 16

/**
 * Phonebook download feature is used to download the entire content of a phonebook
 * object. It is especially designed for devices with relatively large storage capacity
 * that are typically attempting to download and locally store the entire phonebook.
 *
 * Note that this feature is very basic. In particular, phonebook entries are delivered
 * in ascending index order only. No sorting is possible. The PullPhonebook function
 * is used to download the phonebook object of interest.
 *
 * PullPhonebook function retrieves an entire phonebook object from the object
 * exchange server. The PullPhonebook functions is typically used from the root
 * folder. However, when an implementation is using both phonebook downloading
 * and phonebook browsing services. PullPhonebook can be used from wichever
 * virtual folder the PSE is currently set to.
 *
 * The vCard objects in the response shall only contain the properties as included
 * using the PropertySelector parameter and shall be presented using the Format
 * as indicated using the Format parameter. The response shall only contain vCard
 * objects that comply with the vCardSelector parameter.
 *
 * The name header shall contain the absolute path in the virtual folders
 * architecture of the PSE, appended with the name of file representation
 * of one of the Phonebook Objects. Example: telecom/pb.vcf or SIM1/telecom/pb.vcf
 * for the main phonebook objects.
 *
 */

#define PBAP_MAIN_PHONEBOOK_OBJECT_PATH_NAME        "telecom/pb.vcf"
#define PBAP_MISSED_CALL_HISTORY_OBJECT_PATH_NAME   "telecom/mch.vcf"
#define PBAP_INCOMING_CALL_HISTORY_OBJECT_PATH_NAME "telecom/ich.vcf"
#define PBAP_OUTGOING_CALL_HISTORY_OBJECT_PATH_NAME "telecom/och.vcf"

#define PBAP_SIM1_MAIN_PHONEBOOK_OBJECT_PATH_NAME        "SIM1/telecom/pb.vcf"
#define PBAP_SIM1_MISSED_CALL_HISTORY_OBJECT_PATH_NAME   "SIM1/telecom/mch.vcf"
#define PBAP_SIM1_INCOMING_CALL_HISTORY_OBJECT_PATH_NAME "SIM1/telecom/ich.vcf"
#define PBAP_SIM1_OUTGOING_CALL_HISTORY_OBJECT_PATH_NAME "SIM1/telecom/och.vcf"

typedef enum {
    PBAP_VCARD_SELECTOR_OPERATOR_OR = 0x00, // default operator
    PBAP_VCARD_SELECTOR_OPERATOR_AND = 0x01,
} pbap_vcard_selector_operator_enum_t;

struct btif_pbap_channel_t;

struct pbap_callback_parm_t;

typedef void (*btif_pbap_callback_t)(uint8_t device_id, struct btif_pbap_channel_t *pbap_ctl, const struct pbap_callback_parm_t *parm);

struct btif_pbap_channel_t *btif_alloc_pbap_channel(void);

void btif_pbap_init(btif_pbap_callback_t btif_pbap_callback);

bt_status_t btif_pbap_connect(bt_bdaddr_t *addr);

bt_status_t btif_pbap_disconnect(struct btif_pbap_channel_t *pbap_chan);

bt_status_t btif_pbap_send_obex_disconnect_req(struct btif_pbap_channel_t *pbap_chan);

bt_status_t btif_pbap_send_abort_req(struct btif_pbap_channel_t *pbap_chan);

void btif_pbap_set_auth_connect_req(struct btif_pbap_channel_t *pbap_chan, bool auth_req);

bool btif_pbap_is_connected(struct btif_pbap_channel_t *pbap_chan);

int btif_pbap_get_state(struct btif_pbap_channel_t *pbap_chan);

struct pbap_pull_phonebook_parameters {
    bool has_property_selector_paramter; // which vcard fields should be returned, default all fields are returned
    uint8_t property_selector_parameter_byte7;
    uint8_t property_selector_parameter_byte6;
    uint8_t property_selector_parameter_byte5;
    uint8_t property_selector_parameter_byte4;
    uint8_t property_selector_parameter_byte3;
    uint8_t property_selector_parameter_byte2;
    uint8_t property_selector_parameter_byte1;
    uint8_t property_selector_parameter_byte0;
    bool dont_use_default_max_list_count; // default is 65535
    uint16_t max_list_count; // max number of entries are returned
    uint16_t list_start_offset; // specifies which one is the start entry returned in the response, default is 0
    bool reset_new_missed_calls;
    bool has_vcard_selector_parameter; // only return vcards that the specified field's value is not empty
    uint8_t vcard_selector_parameter_byte7;
    uint8_t vcard_selector_parameter_byte6;
    uint8_t vcard_selector_parameter_byte5;
    uint8_t vcard_selector_parameter_byte4;
    uint8_t vcard_selector_parameter_byte3;
    uint8_t vcard_selector_parameter_byte2;
    uint8_t vcard_selector_parameter_byte1;
    uint8_t vcard_selector_parameter_byte0;
    pbap_vcard_selector_operator_enum_t vcard_selector_operator;
};

struct pbap_vcard_entry_item_t {
    uint8_t formatted_name_len;
    uint8_t contact_name_len;
    uint8_t contact_address_len;
    uint8_t contact_tel_len;
    uint8_t contact_email_len;
    uint8_t contact_x_bt_uid_len;
    uint8_t contact_date_len;
    uint8_t contact_type_len;
    char formatted_name[PBAP_MAX_VCARD_FORAMTTED_NAME_SIZE+1];
    char contact_name[PBAP_MAX_VCARD_CONTACT_NAME_SIZE+1];
    char contact_addr[PBAP_MAX_VCARD_CONTACT_ADDR_SIZE+1];
    char contact_tel[PBAP_MAX_VCARD_CONTACT_TEL_SIZE+1];
    char contact_email[PBAP_MAX_VCARD_CONTACT_EMAIL_SIZE+1];
    char context_x_bt_uid_start[PBAP_MAX_VCARD_CONTACT_X_BT_UID_PREFIX_SIZE];
    char contact_x_bt_uid[PBAP_MAX_VCARD_CONTACT_X_BT_UID_SIZE+1];
    char contact_date[PBAP_MAX_VCARD_CONTACT_DATE_SIZE+1];
    char contact_type[PBAP_MAX_VCARD_CONTACT_TYPE_SIZE+1];
};

bool btif_pbap_pull_phonebook(struct btif_pbap_channel_t *pbap_chan,
        const char* phonebook_object_path_name,
        struct pbap_pull_phonebook_parameters *parameters);

/**
 * SetPhonebook operation is used to set the "current folder" on the receiving
 * side. Note that the obex SETPATH command allows only for setting the
 * current folder to the root, parent, or a child folder. For example in order
 * to set the current phone book folder to "pb", from the root, it is necessary
 * to apply SETPATH twice: the first is necessary to change into "telecom" and
 * the second is necessary to change into "pb".
 *
 */

#define PBAP_TELECOM_FOLDER_NAME                "telecom"
#define PBAP_SIM1_FOLDER_NAME                   "SIM1"
#define PBAP_MAIN_PHONEBOOK_FOLDER_NAME         "pb"
#define PBAP_MISSED_CALL_HISTORY_FOLDER_NAME    "mch"
#define PBAP_INCOMING_CALL_HISTORY_FOLDER_NAME  "ich"
#define PBAP_OUTGOING_CALL_HISTORY_FOLDER_NAME  "och"

bool btif_pbap_set_phonebook_to_root(struct btif_pbap_channel_t *pbap_chan);

bool btif_pbap_set_phonebook_to_parent(struct btif_pbap_channel_t *pbap_chan);

bool btif_pbap_set_phonebook_to_child(struct btif_pbap_channel_t *pbap_chan, const char* folder_name);

/**
 * The PullvCardListing function retrieves the PSE's Phonebook listing object.
 *
 * The vCard handles in the vCard-listing object shall be ordered as directed
 * in the Order parameter. Only vCards matching the vCardSelector and
 * vCardSelectorOperator criteria and the SearchValue for the property as
 * indicated in the SearchPeroperty shall be contained in the listing object.
 * The vCard-listing object shall contain up to MaxListCount vCard entries,
 * starting with an offset as indicated using the ListStartOffset parameter.
 *
 * The name header specifies the name of the folder to be retrieved. The value
 * shall not include any path information, since the PullvCardListing function
 * uses relative paths. An empty name header may be sent to retrieve the vCard
 * Listing object of the current folder. However, it is illegal to issue a
 * PullvCardListing request with an empty name header from the "telecom/"
 * folder. Any such request should be responded to with a "Not Found"
 * error message.
 *
 */

typedef enum {
    PBAP_ORDER_INDEXED = 0x00, // default order
    PBAP_ORDER_ALPHANUM = 0x01,
    PBAP_ORDER_PHONETIC = 0x02,
} pbap_order_enum_t;

typedef enum {
    PBAP_SEARCH_PROPERTY_NAME = 0x00, // default search property
    PBAP_SEARCH_PROPERTY_NUMBER = 0x01,
    PBAP_SEARCH_PROPERTY_SOUND = 0x02,
} pbap_search_property_enum_t;

struct pbap_pull_vcard_listing_parameters {
    bool use_search_property_to_filter_vcard;
    pbap_search_property_enum_t search_property;
    const char* search_text;
    pbap_order_enum_t order;
    bool dont_use_default_max_list_count; // default is 65535
    uint16_t max_list_count; // max number of entries are returned
    uint16_t list_start_offset; // specifies which one is the start entry returned in the response, default is 0
    bool reset_new_missed_calls;
    bool has_vcard_selector_parameter; // only return vcards that the specified field's value is not empty
    uint8_t vcard_selector_parameter_byte7;
    uint8_t vcard_selector_parameter_byte6;
    uint8_t vcard_selector_parameter_byte5;
    uint8_t vcard_selector_parameter_byte4;
    uint8_t vcard_selector_parameter_byte3;
    uint8_t vcard_selector_parameter_byte2;
    uint8_t vcard_selector_parameter_byte1;
    uint8_t vcard_selector_parameter_byte0;
    pbap_vcard_selector_operator_enum_t vcard_selector_operator;
};

struct pbap_vcard_listing_item_t { // the text encoded in utf-8
    uint8_t entry_nane_len;
    uint8_t contact_name_len;
    char vcard_entry_name[PBAP_MAX_VCARD_ENTRY_NAME_SIZE+1];
    char vcard_contact_name[PBAP_MAX_VCARD_CONTACT_NAME_SIZE+1];
};

bool btif_pbap_pull_vcard_listing(struct btif_pbap_channel_t *pbap_chan,
        const char* folder_name,
        struct pbap_pull_vcard_listing_parameters *parameters);

/**
 * The PullvCardEntry function retrieves a specific vCard from the object
 * exchange server.
 * 
 * The vCards shall only contain the properties as indicated using the
 * PropertySelector parameter and shall be presented using the Format
 * as indicated using the Format parameter.
 *
 * The Name header shall be used to indicate the name or, if supported,
 * the X-BT-UID of the object to be retrieved. No path information shall
 * be included since the PullvCardEntry function uses relative path
 * information when the value is a name.
 *
 * If the "X-BT-UIDs vCard Property" PCE and PSE feature bits are set,
 * the value of the Name header may have the format "X-BT-UID:XXXX...XXXX",
 * In this case the PSE shall return the corresponding vCard whether the
 * client is in the vCard's folder or not.
 *
 * If the vCard with the given X-BT-UID or name could not be found,
 * the function shall return the error "NOT FOUND".
 *
 */

struct pbap_pull_vcard_entry_parameters {
    bool has_property_selector_paramter; // which vcard fields should be returned, default all fields are returned
    uint8_t property_selector_parameter_byte7;
    uint8_t property_selector_parameter_byte6;
    uint8_t property_selector_parameter_byte5;
    uint8_t property_selector_parameter_byte4;
    uint8_t property_selector_parameter_byte3;
    uint8_t property_selector_parameter_byte2;
    uint8_t property_selector_parameter_byte1;
    uint8_t property_selector_parameter_byte0;
};

bool btif_pbap_pull_vcard_entry(struct btif_pbap_channel_t *pbap_chan,
        const char *vcard_entry_name,
        struct pbap_pull_vcard_entry_parameters *parameters);

struct pbap_callback_parm_t {
    pbap_event_t event;
    uint8_t error_code;
    uint16_t phonebook_size;
    uint8_t new_missed_calls;
    const struct pbap_vcard_listing_item_t *listing_item;
    const struct pbap_vcard_entry_item_t *entry_item;
};

#ifdef __cplusplus
}
#endif

#endif /* BT_PBAP_SUPPORT */

#endif /* __BT_APP_PBAP_API_H__ */
 
