/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
 * Application layer function of head tracking 3dof algo with lsm6dsl sensor
 ****************************************************************************/
#ifndef _WINDOW_NODE_H_
#define _WINDOW_NODE_H_

struct WindowNode {
    float data;
    struct WindowNode* next;
};

#ifdef __cplusplus
extern "C"
{
#endif

struct WindowNode* createNode(float value);

extern void insertNode(struct WindowNode** head, float value);

extern float avgList(struct WindowNode* head);

extern void writeDataToLinkedList(struct WindowNode** head, float value);

#ifdef __cplusplus
}
#endif


#endif