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
 *  @file node_client_impl.h
 *  @brief This header file contain the definition of the client class methods.
 *
 *  @author Bastien Pillonel
 *
 *  @date 6/21/2024
 */

#ifndef NODE_CLIENT_IMPL_H
#define NODE_CLIENT_IMPL_H

#include "node_client.h"

int NodeClient::InitNetwork()
{
    int ret = 0;

    // Try to connect and get ip via DHCP.
    printf("=====================\n");
    printf("obtaining ip address...\n");

    _eth = EthInterface::get_default_instance();
    ret = _eth->connect();

    if (ret != 0)
    {
        printf("DHCP Error - No IP");
        return ret;
    }

    // Show the network address
    SocketAddress a;
    _eth->get_ip_address(&a);
    printf("IP address: %s\n", a.get_ip_address() ? a.get_ip_address() : "None");

    NodeClient::_printInterfaceAddr(1);

    return ret;
}

void NodeClient::_printInterfaceAddr(int id)
{
    printf("Interface %d:\n", id);

    uint8_t address_buf[128];
    int address_count = 0;
    char buf[128];

    // Get address of interface
    int ret = arm_net_address_list_get(id, 128, address_buf, &address_count);

    if (ret == 0)
    {
        uint8_t *t_buf = address_buf;
        for (int i = 0; i < address_count; ++i)
        {
            ip6tos(t_buf, buf);
            printf(" [%d] %s\n", i, buf);
            t_buf += 16;
        }
    }
    else if (ret == -1)
    {
        printf("Unknown interface ID\n");
    }
}

void NodeClient::_printstate(lwm2m_context_t *lwm2mH)
{
    lwm2m_server_t *targetP;

    printf("State: ");
    switch (lwm2mH->state)
    {
    case STATE_INITIAL:
        printf("STATE_INITIAL");
        break;
    case STATE_BOOTSTRAP_REQUIRED:
        printf("STATE_BOOTSTRAP_REQUIRED");
        break;
    case STATE_BOOTSTRAPPING:
        printf("STATE_BOOTSTRAPPING");
        break;
    case STATE_REGISTER_REQUIRED:
        printf("STATE_REGISTER_REQUIRED");
        break;
    case STATE_REGISTERING:
        printf("STATE_REGISTERING");
        break;
    case STATE_READY:
        printf("STATE_READY");
        break;
    default:
        printf("Unknown !");
        break;
    }
    printf("\r\n");

    targetP = lwm2mH->bootstrapServerList;

    if (lwm2mH->bootstrapServerList == NULL)
    {
        printf("No Bootstrap Server.\r\n");
    }
    else
    {
        printf("Bootstrap Servers:\r\n");
        for (targetP = lwm2mH->bootstrapServerList; targetP != NULL; targetP = targetP->next)
        {
            printf(" - Security Object ID %d", targetP->secObjInstID);
            printf("\tHold Off Time: %lu s", (unsigned long)targetP->lifetime);
            printf("\tstatus: ");
            switch (targetP->status)
            {
            case STATE_DEREGISTERED:
                printf("DEREGISTERED\r\n");
                break;
            case STATE_BS_HOLD_OFF:
                printf("CLIENT HOLD OFF\r\n");
                break;
            case STATE_BS_INITIATED:
                printf("BOOTSTRAP INITIATED\r\n");
                break;
            case STATE_BS_PENDING:
                printf("BOOTSTRAP PENDING\r\n");
                break;
            case STATE_BS_FINISHED:
                printf("BOOTSTRAP FINISHED\r\n");
                break;
            case STATE_BS_FAILED:
                printf("BOOTSTRAP FAILED\r\n");
                break;
            default:
                printf("INVALID (%d)\r\n", (int)targetP->status);
            }
            printf("\r\n");
        }
    }

    if (lwm2mH->serverList == NULL)
    {
        printf("No LWM2M Server.\r\n");
    }
    else
    {
        printf("LWM2M Servers:\r\n");
        for (targetP = lwm2mH->serverList; targetP != NULL; targetP = targetP->next)
        {
            printf(" - Server ID %d", targetP->shortID);
            printf("\tstatus: ");
            switch (targetP->status)
            {
            case STATE_DEREGISTERED:
                printf("DEREGISTERED\r\n");
                break;
            case STATE_REG_PENDING:
                printf("REGISTRATION PENDING\r\n");
                break;
            case STATE_REGISTERED:
                printf("REGISTERED\tlocation: \"%s\"\tLifetime: %lus\r\n", targetP->location, (unsigned long)targetP->lifetime);
                break;
            case STATE_REG_UPDATE_PENDING:
                printf("REGISTRATION UPDATE PENDING\r\n");
                break;
            case STATE_REG_UPDATE_NEEDED:
                printf("REGISTRATION UPDATE REQUIRED\r\n");
                break;
            case STATE_DEREG_PENDING:
                printf("DEREGISTRATION PENDING\r\n");
                break;
            case STATE_REG_FAILED:
                printf("REGISTRATION FAILED\r\n");
                break;
            default:
                printf("INVALID (%d)\r\n", (int)targetP->status);
            }
            printf("\r\n");
        }
    }
}

int NodeClient::StartClient()
{
    // Ensure only one client instance is started 
    static bool clientStarted = false;
    if (clientStarted == true)
        return -1;
    clientStarted = true;

    lwm2m_object_t **objArray = new lwm2m_object_t *[_objects->size() + 1];
    memset(&data, 0, sizeof(client_data_t));
    data.addressFamily = ADDRESS_IPV6;
    int result;
 
#ifdef USE_DTLS
    // Configure psk id and key when DTLS is requested
    char *pskId = _clientIdentity;
    char *psk = _clientKey;

    uint16_t pskLen = 0;
    char *pskBuffer = NULL;
#else
    char *pskId = NULL;
    char *psk = NULL;
    uint16_t pskLen = -1;
    char *pskBuffer = NULL;
#endif

    fprintf(stderr, "Trying to bind LWM2M Client to port %d\r\n", CLIENT_LOCAL_PORT);
    data.sock = create_socket(CLIENT_LOCAL_PORT, data.addressFamily);
    if (data.sock < 0)
    {
        fprintf(stderr, "Failed to open socket: %d %s\r\n", errno, strerror(errno));
        return -1;
    }

#ifdef USE_DTLS
    // Pre-Shared-Key configuration
    if (psk != NULL)
    {
        pskLen = strlen(psk) / 2;
        pskBuffer = (char *)malloc(pskLen);

        if (NULL == pskBuffer)
        {
            fprintf(stderr, "Failed to create PSK binary buffer\r\n");
            return -1;
        }
        // Hex string to binary
        char *h = psk;
        char *b = pskBuffer;
        char xlate[] = "0123456789ABCDEF";

        for (; *h; h += 2, ++b)
        {
            char *l = strchr(xlate, toupper(*h));
            char *r = strchr(xlate, toupper(*(h + 1)));

            if (!r || !l)
            {
                fprintf(stderr, "Failed to parse Pre-Shared-Key HEXSTRING\r\n");
                return -1;
            }

            *b = ((l - xlate) << 4) + (r - xlate);
        }
    }
#endif

    // Configuring server URI
    char serverUri[50];
    int serverId = 123;

    sprintf(serverUri, "coaps://[%s]:%s", _url, _port);
/*#ifdef USE_DTLS
    sprintf(serverUri, "coaps://[%s]:%s", M2M_SERVER_URL, SERVER_DTLS_PORT);
#else
    sprintf(serverUri, "coap://[%s]:%s", M2M_SERVER_URL, SERVER_PORT);
#endif*/
    // Get object security from object_security.c file 
    objArray[0] = get_security_object(serverId, serverUri, pskId, pskBuffer, pskLen, false);
    data.securityObjP = objArray[0];

    // Get object from NodeObject instance stored in the client
    for (size_t i = 0; i < _objects->size(); ++i)
    {
        objArray[i + 1] = _objects->at(i)->Get();
        if (!objArray[i + 1])
        {
            fprintf(stderr, "Failed to create object %lu\r\n", i);
            return -1;
        }
    }

    // Init connection context structure
    printf("lwm2m_init\n");
    lwm2mH = lwm2m_init(&data);
    if (NULL == lwm2mH)
    {
        fprintf(stderr, "lwm2m_init() failed\r\n");
        return -1;
    }

    data.ctx = lwm2mH;
    data.connLayer = connectionlayer_create(lwm2mH);

    printf("lwm2m_configure\n");
    result = lwm2m_configure(lwm2mH, _endpointName, NULL, NULL, _objects->size() + 1, objArray);
    if (result != 0)
    {
        fprintf(stderr, "lwm2m_configure() failed: 0x%X\r\n", result);
        return -1;
    }
    fprintf(stdout, "LWM2M Client \"%s\" started on port %d.\r\nUse Ctrl-C to exit.\r\n\n", _endpointName, CLIENT_LOCAL_PORT);

    lwm2mMainThread.start(&NodeClient::_lwm2mMainThreadTask);

    //int i = 0;
    while (1)
    {
        ThisThread::sleep_for(1s);
        // Test to update the resource device operating hours on device extension to see if server is able to update the red value
        //((_objects->at(2))->GetResource(9))->SetValue<int>(i++);
    }

    delete objArray;
    clientStarted = false;

    return 0;
}

void NodeClient::_lwm2mMainThreadTask()
{
    while (1)
    {
        struct timeval tv;

        tv.tv_sec = 60;
        tv.tv_usec = 0;

        NodeClient::_printstate(lwm2mH);

        /*
         * This function does two things:
         *  - first it does the work needed by liblwm2m (eg. (re)sending some packets).
         *  - Secondly it adjusts the timeout value (default 60s) depending on the state of the transaction
         *    (eg. retransmission) and the time before the next operation
         */
        int result = lwm2m_step(lwm2mH, &(tv.tv_sec));
        if (result != 0)
        {
            fprintf(stderr, "lwm2m_step() failed: 0x%X\r\n", result);
        }
        ThisThread::flags_wait_any_for(0x1, std::chrono::seconds(tv.tv_sec));
    }
}

void NodeClient::Lwm2mHandleIncomingSocketDataCppWrap(ns_address_t *addr, uint8_t *buf, size_t len)
{
    printf("New packet arrived !\n");
    if (connectionlayer_handle_packet(data.connLayer, addr, buf, len) == -1)
    {
        
        // This packet comes from an unknown peer
        
        fprintf(stderr, "received bytes ignored!\r\n");
    }

    lwm2mMainThreadTimeoutTimer.detach();
    lwm2mMainThread.flags_set(0x1);
}

NodeClient::~NodeClient()
{
    for (NodeObject *object : *_objects)
    {
        delete object;
    }
}

void NodeClient::SetObjects(std::vector<NodeObject *> *objects){
    _objects = objects;
}

void NodeClient::SetNetworkInterface(NetworkInterface *eth){
    _eth = eth;
}

void NodeClient::SetUrl(const char *url){
    _url = url;
}

void NodeClient::SetPort(const char *port){
    _port = port;
}

void NodeClient::SetClientKey(char *clientKey){
    _clientKey = clientKey;
}

void NodeClient::SetEndpointName(char *endpointName){
    _endpointName = endpointName;
}

void NodeClient::SetClientIdentity(char *clientIdentity){
    _clientIdentity = clientIdentity;
}

void lwm2m_main_thread_timeout_timer_cb()
{
    lwm2mMainThread.flags_set(0x1);
}

extern "C" void lwm2m_handle_incoming_socket_data(ns_address_t *addr, uint8_t *buf, size_t len)
{
    NodeClient::Lwm2mHandleIncomingSocketDataCppWrap(addr, buf, len);
}

/**
 * @brief External definition of lwm2m_connect_server
 * 
 * @param secObjInstID id of security object
 * @param userData parameter to lwm2m_init()
 * @return void* connection_t structure about the connection established
 */
void *lwm2m_connect_server(uint16_t secObjInstID,
                           void *userData)
{
    client_data_t *dataP;
    char *uri;
    char *host;
    char *port;
    connection_t *newConnP = NULL;

    dataP = (client_data_t *)userData;

    uri = get_server_uri(dataP->securityObjP, secObjInstID);

    if (uri == NULL)
        return NULL;

    printf("Connecting to %s\r\n", uri);

    // Parse uri in the form "coaps://[host]:[port]"
    if (0 == strncmp(uri, "coaps://", strlen("coaps://")))
    {
        host = uri + strlen("coaps://");
    }
    else if (0 == strncmp(uri, "coap://", strlen("coap://")))
    {
        host = uri + strlen("coap://");
    }
    else
    {
        goto exit;
    }
    port = strrchr(host, ':');
    if (port == NULL)
        goto exit;
    // remove brackets
    if (host[0] == '[')
    {
        host++;
        if (*(port - 1) == ']')
        {
            *(port - 1) = 0;
        }
        else
            goto exit;
    }
    // split strings
    *port = 0;
    port++;

#if defined(USE_DTLS)
    // Create connection with dtls use
    newConnP = (connection_t *)dtlsconnection_create(dataP->connLayer, secObjInstID, dataP->sock, host, port,
                                                     dataP->addressFamily);
#else
    // Create connection without dtls
    newConnP = connection_create(dataP->connLayer, dataP->sock, host, port, dataP->addressFamily);
#endif

    if (newConnP == NULL)
    {
        printf("Connection creation failed.\r\n");
    }
    else
    {
        dataP->connList = newConnP;
    }

exit:
    lwm2m_free(uri);
    return (void *)newConnP;
}

/**
 * @brief Close a session created by lwm2m_connect_server()
 * 
 * @param sessionH session handle identifying the peer 
 * @param userData parameter to lwm2m_init()
 */
void lwm2m_close_connection(void *sessionH,
                            void *userData)
{
    client_data_t *app_data;
    connection_t *targetP;

    app_data = (client_data_t *)userData;
    targetP = (connection_t *)sessionH;

    if (targetP == app_data->connList)
    {
        app_data->connList = targetP->next;
        lwm2m_free(targetP);
    }
    else
    {
        connection_t *parentP;

        parentP = app_data->connList;
        while (parentP != NULL && parentP->next != targetP)
        {
            parentP = parentP->next;
        }
        if (parentP != NULL)
        {
            parentP->next = targetP->next;
            lwm2m_free(targetP);
        }
    }
}

#endif