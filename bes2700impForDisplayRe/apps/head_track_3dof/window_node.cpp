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
 * Application layer function of head tracking 3dof with lsm6dsl sensor
 ****************************************************************************/
#ifdef HEAD_TRACK_ENABLE
#include "window_node.h"
#include <stdio.h>
#include <stdlib.h>

struct WindowNode* createNode(float value)
{
    struct WindowNode* newNode = (struct WindowNode*)malloc(sizeof(struct WindowNode));
    newNode->data = value;
    newNode->next = NULL;
    return newNode;
}

void insertNode(struct WindowNode** head, float value)
{
    struct WindowNode* newNode = createNode(value);

    if (*head == NULL) {
        *head = newNode;
        newNode->next = newNode;
    } else {
        newNode->next = (*head)->next;
        (*head)->next = newNode;
        *head = newNode;
    }
}

float avgList(struct WindowNode* head)
{
    int k = 0;
    if (head == NULL) {
        return 0.0;
    }

    float sum = 0.0;
    struct WindowNode* current = head;
    do {
        sum += current->data;
        current = current->next;
        k++;
    } while (current != head);

    return sum/k;
}

void writeDataToLinkedList(struct WindowNode** head, float value)
{
    if (*head == NULL) {
        *head = createNode(value);
        (*head)->next = *head;
    } else {
        (*head)->data = value;
        *head = (*head)->next;
    }
}
#endif