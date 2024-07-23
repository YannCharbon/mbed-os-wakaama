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
 *    Bosch Software Innovations GmbH - Please refer to git log
 *    Pascal Rieux - Please refer to git log
 *    Ville Skyttä - Please refer to git log
 *    Scott Bertin, AMETEK, Inc. - Please refer to git log
 *
 *******************************************************************************/

/**
 *  @file node_client.h
 *  @brief This header file contain the declaration of the client class representing a LwM2M client.
 *
 *  @author Bastien Pillonel
 *
 *  @date 6/21/2024
 */

#ifndef NODE_CLIENT_H
#define NODE_CLIENT_H

#include <vector>
#include <stdio.h>

#include "mbed.h"
#include "node_object.h"
#include "NetworkInterface.h"
#include "EthernetInterface.h"
#include "UDPSocket.h"
#include "mesh_system.h"
#include "net_interface.h"
#include "ip6string.h"
#include "mbed_mem_trace.h"

extern "C"
{
#include "liblwm2m.h"
#include "connection.h"
#if defined(USE_DTLS)
#include "dtlsconnection.h"
#endif
#include "object_utils.h"

// Connect to the Leshan test server as default: http://leshan.eclipse.org
#define M2M_SERVER_URL "2a01:111:f100:9001::1761:93fa" // LESHAN
#define CLIENT_ENDPOINT_NAME "mbedM2M"
#define CLIENT_IDENTITY "mbedM2M"
#define CLIENT_KEYSTR "65875A0C3D4646A99BFC4D5967EE7DB3"
#define SERVER_DTLS_PORT "5684"
#define SERVER_PORT "5683"
#define CLIENT_LOCAL_PORT 0 // Let OS decide

    lwm2m_object_t *get_security_object(int serverId, const char *serverUri, char *bsPskId, char *psk, uint16_t pskLen, bool isBootstrap);
    extern void free_security_object(lwm2m_object_t *objectP);
    extern char *get_server_uri(lwm2m_object_t *objectP, uint16_t secObjInstID);
}

static Thread lwm2mMainThread(osPriorityNormal, OS_STACK_SIZE, nullptr, "lwm2mMainThread");
static Timeout lwm2mMainThreadTimeoutTimer;

typedef struct
{
    lwm2m_object_t *securityObjP;
    int sock;
    connection_t *connList;
    int addressFamily;
    lwm2m_context_t *ctx;
    lwm2m_connection_layer_t *connLayer;
} client_data_t;

client_data_t data;
lwm2m_context_t *lwm2mH = NULL;

/**
 * @brief NodeClient represent a LwM2M client storing every object and resource associated.
 *
 */
class NodeClient
{
public:
    /**
     * @brief Construct a new Node Client object by default
     *
     */
    NodeClient() {}

    /**
     * @brief Construct a new Node Client object by copy
     * 
     * @param src 
     */
    NodeClient(const NodeClient &src) : _objects(src._objects), _eth(src._eth), _url(src._url), _port(src._port), _clientKey(src._clientKey), _endpointName(src._endpointName), _clientIdentity(src._clientIdentity) {}

    /**
     * @brief Construct a new Node Client object by moving
     * 
     * @param src 
     */
    NodeClient(NodeClient &&src) : _objects(std::move(src._objects)), _eth(src._eth), _url(src._url), _port(src._port), _clientKey(src._clientKey), _endpointName(src._endpointName), _clientIdentity(src._clientIdentity) {
        src._eth = nullptr;
        src._url = nullptr;
        src._port = nullptr;
        src._clientKey = nullptr;
        src._endpointName = nullptr;
        src._clientIdentity = nullptr;
    }

    /**
     * @brief Construct a new Node Client object by specific a vector of object stored in the client
     * 
     * @param objects respresents a list of objects on the client device.
     */
    NodeClient(std::vector<NodeObject *> *objects) : _objects(objects), _eth(), _url(""), _port(""), _clientKey(""), _endpointName(""), _clientIdentity("")  {}

    /**
     * @brief Construct a new Node Client object by specific a vector of object stored in the client, a network interface, the server URL and server port
     * 
     * @param objects respresents a list of objects on the client device.
     * @param eth network interface for NodeClient
     * @param url server url
     * @param port server port
     * @param clientKey psk client key
     * @param endpointName name of client endpoint
     * @param clientIdentity client identity for psk
     */
    NodeClient(std::vector<NodeObject *> *objects, NetworkInterface *eth, const char *url, const char *port, char *clientKey, char *endpointName, char *clientIdentity) : _objects(objects), _eth(eth), _url(url), _port(port), _clientKey(clientKey), _endpointName(endpointName), _clientIdentity(clientIdentity) {}

    /**
     * @brief Destroy the Node Client object
     * 
     */
    ~NodeClient();

    /**
     * @brief Setup client to ensure a correct connection with the server
     * 
     * @return int error code
     */
    int InitNetwork();

    /**
     * @brief Let the client connects itself to the server and configuring object so that
     * server can render correct objects that are stored on the client.
     * 
     * @return int error code
     */
    int StartClient();

    /**
     * @brief Wrapper for incoming packet handler function
     * 
     * @param addr address from incoming packet
     * @param buf payload
     * @param len payload length
     */
    static void Lwm2mHandleIncomingSocketDataCppWrap(ns_address_t *addr, uint8_t *buf, size_t len);

    /**
     * @brief Set the Objects attribute
     * 
     * @param objects vector of NodeObject avaliable on the client
     */
    void SetObjects(std::vector<NodeObject *> *objects);

    /**
     * @brief Set the Network Interface attribute
     * 
     * @param eth new network interface of NodeClient 
     */
    void SetNetworkInterface(NetworkInterface *eth);

    /**
     * @brief Set the Url object
     * 
     * @param url of the LwM2M server
     */
    void SetUrl(const char *url);

    /**
     * @brief Set the Port object
     * 
     * @param port of the LwM2M server
     */
    void SetPort(const char *port);

    /**
     * @brief Set the Client Key object
     * 
     * @param clientKey psk client key
     */
    void SetClientKey(char *clientKey);

    /**
     * @brief Set the Endpoint Name object
     * 
     * @param endpointName endpoint name of client
     */
    void SetEndpointName(char *endpointName);

    /**
     * @brief Set the Client Identity object
     * 
     * @param clientIdentity psk client identity
     */
    void SetClientIdentity(char *clientIdentity);

private:
    std::vector<NodeObject *> *_objects;
    NetworkInterface *_eth;
    const char *_url;
    const char *_port;
    char *_clientKey;
    char *_endpointName;
    char *_clientIdentity;

    /**
     * @brief Display interface addr infos
     * 
     * @param id of interface
     */
    static void _printInterfaceAddr(int id);

    /**
     * @brief Display state of client-server LwM2M connection
     * 
     * @param lwm2mH context structure for client-server connection state 
     */
    static void _printstate(lwm2m_context_t *lwm2mH);

    /**
     * @brief Main thread task, send packet to the server and handle timeout depanding on connection state
     * 
     */
    static void _lwm2mMainThreadTask();
};

#include "node_client_impl.h"

#endif