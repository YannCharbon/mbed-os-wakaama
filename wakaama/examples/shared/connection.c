/*******************************************************************************
 *
 * Copyright (c) 2013, 2014 Intel Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v20.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    David Navarro, Intel Corporation - initial API and implementation
 *    Pascal Rieux - Please refer to git log
 *
 *******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "connection.h"
#include "commandline.h"

#include "socket_api.h"
#include "ip6string.h"
#include "mbed_trace.h"
#include "net_interface.h"

static int cur_sock;
static uint8_t buffer[2048];
static uint8_t pending_rx_buffer[2048];
static int pending_rx_pkt_len = 0;
static ns_address_t pending_rx_addr;

extern void lwm2m_handle_incoming_socket_data_c_wrap(ns_address_t *addr, uint8_t *buf, size_t len);

void socket_recv_callback(void *socket_cb) {
    //wait_us(200);
    //printf("Socket receive callback (event_type %d, socket_id %d, d_len %d)\n", socket_cb->event_type, socket_cb->socket_id, socket_cb->d_len);
    //ns_address_t addr;
    //int numBytes = socket_recvfrom(cur_sock, buffer, 2048, 0, &addr);
    //printf("Received %d byte from %s\n", trace_ipv6(addr.address));
    //printf("Socket receive callback\n");
    //socket_callback_t *socket_callback = (socket_callback_t *)socket_cb;
    //int16_t length;
//
    //if (socket_callback->event_type == SOCKET_DATA) {
    //    if (socket_callback->d_len > 0) {
    //        memset(pending_rx_buffer, 0, 2048);
//
    //        length = socket_read(socket_callback->socket_id, &pending_rx_addr, pending_rx_buffer, socket_callback->d_len);
    //        if (length > 0) {
    //            printf("Received %d bytes\n", length);
    //            pending_rx_pkt_len = length;
    //        } else {
    //            pending_rx_pkt_len = 0;
    //        }
    //    }
    //}
    //printf("%s\n", __func__);
    socket_callback_t *socket_callback = (socket_callback_t *)socket_cb;

	memset(pending_rx_buffer, 0, sizeof(pending_rx_buffer));
	bool something_in_socket = true;
	int length = 0;
	while (something_in_socket)
	{
		//int rc = recv(n->my_socket, &buffer[bytes], (size_t)(len - bytes), 0);
		//length = socket_recv(mqttc_ptr->ipstack->my_socket, &receive_buffer, sizeof(receive_buffer) - 1, 0);
        length = socket_recvfrom(socket_callback->socket_id, pending_rx_buffer, 2048, 0, &pending_rx_addr);
		//INFO("Mbed data read size : %d", rc);
		if (length > 0) {
            //printf("Length %d , Received: %x \n", length, pending_rx_buffer[0]);
            lwm2m_handle_incoming_socket_data_c_wrap(&pending_rx_addr, pending_rx_buffer, length);
        }
        else if (length!=NS_EWOULDBLOCK) {
            printf("Error happened when receiving %d\n", length);
            something_in_socket = false;
        }
        else {
            // there was nothing to read.
            something_in_socket = false;
        }
	}
}

void test_socket_recv_callback(void *socket_cb) {
    //printf("Socket receive callback\n");
    //ns_address_t addr;
    //int numBytes = socket_recvfrom(cur_sock, buffer, 2048, 0, &addr);
    //printf("Received %d byte from %s\n", trace_ipv6(addr.address));
}


int create_socket(int port_number, int ai_family)
{
    (void)ai_family;
    int sock = socket_open(SOCKET_UDP, port_number, &socket_recv_callback);
    if (sock < 0) {
        printf("Could not open socket\n");
        return -1;
    }
    cur_sock = sock;
    // Normally socket_open already binds
    ns_address_t binding;
    //memcpy(binding.address, ns_in6addr_any, 16);
    //binding.type = ADDRESS_IPV6;
    //binding.identifier = port_number;
    //int ret = socket_bind(sock, &binding);
    //if (ret < 0) {
    //    printf("Could not bind socket\n");
    //    return -1;
    //}
    return sock;
}

connection_t * connection_find(connection_t * connList,
                               ns_address_t * addr,
                               size_t addrLen)
{
    connection_t * connP;

    connP = connList;
    while (connP != NULL)
    {
        if ((connP->addrLen == addrLen)
         && (memcmp(&(connP->addr), addr, addrLen) == 0))
        {
            return connP;
        }
        connP = connP->next;
    }

    return connP;
}

connection_t * connection_new_incoming(connection_t * connList,
                                       int sock,
                                       ns_address_t * addr,
                                       size_t addrLen)
{
    connection_t * connP;

    connP = (connection_t *)lwm2m_malloc(sizeof(connection_t));
    if (connP != NULL)
    {
        connP->sock = sock;
        memcpy(&(connP->addr), addr, addrLen);
        connP->addrLen = addrLen;
        connP->next = connList;
    }

    return connP;
}

connection_t * connection_create(connection_t * connList,
                                 int sock,
                                 char * host,
                                 char * port,
                                 int addressFamily)
{
    int ret = 0;
    connection_t * connP = NULL;

    (void)addressFamily;

    ns_address_t host_addr;
    host_addr.type = ADDRESS_IPV6;

    printf("%s : host is %s\n", __func__, host);

    if (stoip6(host, strlen(host), host_addr.address) == false) {
        printf("%s: Failed to convert host address\n", __func__);
        return NULL;
    }

    host_addr.identifier = atoi(port);

    //int test_sock = socket_open(SOCKET_UDP, host_addr.identifier, &test_socket_recv_callback);
//
    //if (test_sock < 0) {
    //    printf("%s: Failed to open socket\n", __func__);
    //    return NULL;
    //}

    //ret = socket_connect(sock, &host_addr, 0);

    //if (ret >= 0) {
        connP = connection_new_incoming(connList, sock, &host_addr, sizeof(host_addr));
    //}

    //socket_close(test_sock);

    return connP;
}

void connection_free(connection_t * connList)
{
    while (connList != NULL)
    {
        connection_t * nextP;

        nextP = connList->next;
        lwm2m_free(connList);

        connList = nextP;
    }
}

int connection_send(connection_t *connP,
                    uint8_t * buffer,
                    size_t length)
{
    int nbSent;
    size_t offset;

    offset = 0;
    //while (offset != length)
    //{
    //    //printf("Socket sendto\n");
    //    nbSent = socket_sendto(connP->sock, &connP->addr, buffer + offset, length - offset);
    //    if (nbSent == -1) return -1;
    //    offset += nbSent;
    //    printf("Successfully sent %d bytes\n", nbSent);
    //}
    int ret = socket_sendto(connP->sock, &connP->addr, buffer + offset, length - offset);
    if (ret == 0) {
        printf("Successfully sent %d bytes to %s:%d\n", length, trace_ipv6(connP->addr.address), connP->addr.identifier);
    }

    return 0;
}

uint8_t lwm2m_buffer_send(void * sessionH,
                          uint8_t * buffer,
                          size_t length,
                          void * userdata)
{
    connection_t * connP = (connection_t*) sessionH;

    (void)userdata; /* unused */

    if (connP == NULL)
    {
        fprintf(stderr, "#> failed sending %zu bytes, missing connection\r\n", length);
        return COAP_500_INTERNAL_SERVER_ERROR ;
    }

    if (-1 == connection_send(connP, buffer, length))
    {
        fprintf(stderr, "#> failed sending %zu bytes\r\n", length);
        return COAP_500_INTERNAL_SERVER_ERROR ;
    }

    return COAP_NO_ERROR;
}

bool lwm2m_session_is_equal(void * session1,
                            void * session2,
                            void * userData)
{
    (void)userData; /* unused */

    return (session1 == session2);
}
