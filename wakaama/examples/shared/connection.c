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

#include "connection.h"
#include "commandline.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "socket_api.h"
#include "ip6string.h"
#include "mbed_trace.h"
#include "net_interface.h"

static int cur_sock;
static uint8_t pending_rx_buffer[2048];
static ns_address_t pending_rx_addr;

/** This stack does not do anything with the incoming packets.
 * These must be used by the application, which should call
 * connectionlayer_find_connection, connection_new_incoming and
 * connectionlayer_handle_packet
*/
extern void lwm2m_handle_incoming_socket_data(ns_address_t *addr, uint8_t *buf, size_t len);

void socket_recv_callback(void *socket_cb) {
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
            char a[40];
            ip6tos(pending_rx_addr.address, a);
            printf("[%s] : Length %d , Received: %x \n", a, length, pending_rx_buffer[0]);
            lwm2m_handle_incoming_socket_data(&pending_rx_addr, pending_rx_buffer, length);
        }
        else if (length!=NS_EWOULDBLOCK) {
            printf("Error happened when receiving %d\n", length);
            something_in_socket = false;
        }
        else {
            // there was nothing to read.
            //printf("there was nothing to read\n");
            something_in_socket = false;
        }
	}
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
    //ns_address_t binding;
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

static int connection_send(uint8_t const *buffer, size_t length, void *userData) {
    //int nbSent;
    size_t offset;
    connection_t *connP = (connection_t *)userData;
    offset = 0;
    //while (offset != length) {
    //    nbSent =
    //        sendto(connP->sock, buffer + offset, length - offset, 0, (struct sockaddr *)&(connP->addr), connP->addrLen);
    //    if (nbSent == -1)
    //        return -1;
    //    offset += nbSent;
    //}
    char a[40];
    ip6tos(connP->addr.address, a);
    int ret = socket_sendto(connP->sock, &connP->addr, buffer + offset, length - offset);
    if (ret == 0) {
        printf("Successfully sent %d bytes to [%s]:%d\n", length, a, connP->addr.identifier);
    }

    return 0;
}

static int connection_recv(lwm2m_context_t *ctx, uint8_t *buffer, size_t length, void *userData) {
    lwm2m_handle_packet(ctx, buffer, length, userData);
    return 0;
}

static connection_t *connection_find(connection_t *connList, ns_address_t *addr) {
    connection_t *connP;

    connP = connList;
    while (connP != NULL) {
        if (memcmp(connP->addr.address, addr->address, 16) == 0)
        {
            printf("Found\n");
            return connP;
        }
        connP = connP->next;
    }

    return connP;
}

static void connection_free(connection_t *conn) {
    if (conn->deinitFunc) {
        conn->deinitFunc(conn);
    }
    lwm2m_free(conn);
}

static void connectionlayer_free_connlist(connection_t *connList) {
    while (connList != NULL) {
        connection_t *nextP;

        nextP = connList->next;
        connection_free(connList);

        connList = nextP;
    }
}

lwm2m_connection_layer_t *connectionlayer_create(lwm2m_context_t *context) {
    lwm2m_connection_layer_t *layerCtx = (lwm2m_connection_layer_t *)lwm2m_malloc(sizeof(lwm2m_connection_layer_t));
    if (layerCtx == NULL) {
        return NULL;
    }
    layerCtx->ctx = context;
    layerCtx->connList = NULL;
    return layerCtx;
}

int connectionlayer_handle_packet(lwm2m_connection_layer_t *connLayerP, ns_address_t *addr,
                                  uint8_t *buffer, size_t length) {
    connection_t *connP = connection_find(connLayerP->connList, addr);
    if (connP == NULL) {
        return -1;
    }
    return connP->recvFunc(connLayerP->ctx, buffer, length, connP);
}

connection_t *connectionlayer_find_connection(lwm2m_connection_layer_t *connLayerP, ns_address_t const *addr) {
    return connection_find(connLayerP->connList, addr);
}

void connectionlayer_free(lwm2m_connection_layer_t *connLayerP) {
    if (connLayerP == NULL) {
        return;
    }
    connectionlayer_free_connlist(connLayerP->connList);
    lwm2m_free(connLayerP);
}

void connectionlayer_free_connection(lwm2m_connection_layer_t *connLayerP, connection_t *conn) {
    connection_t *connItor = connLayerP->connList;
    if (connLayerP->connList == conn) {
        connLayerP->connList = conn->next;
        connection_free(conn);
    } else {
        while (connItor != NULL && connItor->next != conn) {
            connItor = connItor->next;
        }
        if (connItor != NULL) {
            connItor->next = conn->next;
            connection_free(conn);
        }
    }
}

void connectionlayer_add_connection(lwm2m_connection_layer_t *connLayer, connection_t *conn) {
    conn->next = connLayer->connList;
    connLayer->connList = conn;
}

static void connection_new_incoming_internal(connection_t *conn, int sock, ns_address_t *addr) {
    conn->sock = sock;
    memcpy(conn->addr.address, addr->address, 16);
    conn->addr.identifier = addr->identifier;
    conn->sendFunc = connection_send;
    conn->recvFunc = connection_recv;
}

connection_t *connection_new_incoming(lwm2m_connection_layer_t *connLayerP, int sock, ns_address_t *addr) {
    connection_t *connP = (connection_t *)lwm2m_malloc(sizeof(connection_t));
    if (connP != NULL) {
        connection_new_incoming_internal(connP, sock, addr);
        connectionlayer_add_connection(connLayerP, connP);
    }

    return connP;
}

connection_t *connection_create(lwm2m_connection_layer_t *connLayerP, int sock, char *host, char *port,
                                int addressFamily) {
    connection_t *conn = (connection_t *)lwm2m_malloc(sizeof(connection_t));
    if (conn == NULL) {
        return NULL;
    }
    if (connection_create_inplace(conn, sock, host, port, addressFamily) > 0) {
        connectionlayer_add_connection(connLayerP, conn);
    } else {
        lwm2m_free(conn);
        conn = NULL;
    }
    return conn;
}

int connection_create_inplace(connection_t *conn, int sock, char *host, char *port, int addressFamily) {
    (void)addressFamily;

    ns_address_t host_addr;
    host_addr.type = ADDRESS_IPV6;

    printf("%s : host is %s\n", __func__, host);

    if (stoip6(host, strlen(host), host_addr.address) == false) {
        printf("%s: Failed to convert host address\n", __func__);
        return 0;
    }

    host_addr.identifier = atoi(port);

    connection_new_incoming_internal(conn, sock, &host_addr);
    return 1;
}

uint8_t lwm2m_buffer_send(void *sessionH, uint8_t *buffer, size_t length, void *userdata) {
    connection_t *connP = (connection_t *)sessionH;

    (void)userdata; /* unused */

    if (connP == NULL) {
        fprintf(stderr, "#> failed sending %lu bytes, missing connection\r\n", length);
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

#ifdef LWM2M_WITH_LOGS
    fprintf(stderr, "Sending %lu bytes to [%s]:%hu\r\n", length, trace_ipv6(connP->addr.address), connP->addr.identifier);

    output_buffer(stderr, buffer, length, 0);
#endif

    if (-1 == connP->sendFunc(buffer, length, connP)) {
        fprintf(stderr, "#> failed sending %lu bytes\r\n", length);
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    return COAP_NO_ERROR;
}

bool lwm2m_session_is_equal(void *session1, void *session2, void *userData) {
    (void)userData; /* unused */

    return (session1 == session2);
}

/*

int get_port(struct sockaddr *x)
{
   if (x->sa_family == AF_INET)
   {
       return ((struct sockaddr_in *)x)->sin_port;
   } else if (x->sa_family == AF_INET6) {
       return ((struct sockaddr_in6 *)x)->sin6_port;
   } else {
       printf("non IPV4 or IPV6 address\n");
       return  -1;
   }
}

int sockaddr_cmp(struct sockaddr *x, struct sockaddr *y)
{
    int portX = get_port(x);
    int portY = get_port(y);

    // if the port is invalid of different
    if (portX == -1 || portX != portY)
    {
        return 0;
    }

    // IPV4?
    if (x->sa_family == AF_INET)
    {
        // is V4?
        if (y->sa_family == AF_INET)
        {
            // compare V4 with V4
            return ((struct sockaddr_in *)x)->sin_addr.s_addr == ((struct sockaddr_in *)y)->sin_addr.s_addr;
            // is V6 mapped V4?
        } else if (IN6_IS_ADDR_V4MAPPED(&((struct sockaddr_in6 *)y)->sin6_addr)) {
            struct in6_addr* addr6 = &((struct sockaddr_in6 *)y)->sin6_addr;
            uint32_t y6to4 = addr6->s6_addr[15] << 24 | addr6->s6_addr[14] << 16 | addr6->s6_addr[13] << 8 |
addr6->s6_addr[12]; return y6to4 == ((struct sockaddr_in *)x)->sin_addr.s_addr; } else { return 0;
        }
    } else if (x->sa_family == AF_INET6 && y->sa_family == AF_INET6) {
        // IPV6 with IPV6 compare
        return memcmp(((struct sockaddr_in6 *)x)->sin6_addr.s6_addr, ((struct sockaddr_in6 *)y)->sin6_addr.s6_addr, 16)
== 0; } else {
        // unknown address type
        printf("non IPV4 or IPV6 address\n");
        return 0;
    }
}

*/
