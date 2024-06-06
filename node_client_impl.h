#include "node_client.h"

int NodeClient::InitNetwork()
{
    int ret = 0;
    std::cout << "Obj vec address1 = " << _objects << " and size is " /*<< _objects->size()*/ << std::endl;

    // try to connect and get ip via DHCP.
    printf("=====================\n");
    printf("obtaining ip address...\n");

    eth = EthInterface::get_default_instance();
    ret = eth->connect();

    std::cout << "Obj vec address2 = " << _objects << " and size is " /*<< _objects->size()*/ << std::endl;

    if (ret != 0)
    {
        printf("DHCP Error - No IP");
        return ret;
    }

    std::cout << "Obj vec address3 = " << _objects << " and size is " /*<< _objects->size()*/ << std::endl;

    // Show the network address
    SocketAddress a;
    eth->get_ip_address(&a);
    printf("IP address: %s\n", a.get_ip_address() ? a.get_ip_address() : "None");

    std::cout << "Obj vec address4 = " << _objects << " and size is " /*<< _objects->size()*/ << std::endl;

    NodeClient::_printInterfaceAddr(1);

    std::cout << "Obj vec address5 = " << _objects << " and size is " << _objects->size() << std::endl;

    std::cout << "Instance addr is " << this << std::endl;
    return ret;
}

void NodeClient::_printInterfaceAddr(int id)
{
    printf("Interface %d:\n", id);

    uint8_t address_buf[128];
    int address_count = 0;
    char buf[128];

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
    std::cout << "Instance addr is " << this << std::endl;
    if (_objects)
        std::cout << "objects non null" << std::endl;

    std::cout << "Obj vec address = " << _objects << " and size is " /*<< _objects->size()*/ << std::endl;

    lwm2m_object_t **objArray = new lwm2m_object_t*[_objects->size()];

    memset(&data, 0, sizeof(client_data_t));
    data.addressFamily = ADDRESS_IPV6;

    int result;

#ifdef USE_DTLS
    char *pskId = CLIENT_IDENTITY;
    char *psk = CLIENT_KEYSTR;

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

    char serverUri[50];
    int serverId = 123;
#ifdef USE_DTLS
    sprintf(serverUri, "coaps://[%s]:%s", M2M_SERVER_URL, SERVER_DTLS_PORT);
#else
    sprintf(serverUri, "coap://[%s]:%s", M2M_SERVER_URL, SERVER_PORT);
#endif

    objArray[0] = get_security_object(serverId, serverUri, pskId, pskBuffer, pskLen, false);
    //lwm2m_object_t *securityObject = _objects->at(0)->Get();
    //((security_instance_t *)(securityObject->instanceList->next))->secretKey = (char *) lwm2m_malloc(pskLen);
    //memcpy(((security_instance_t *)(securityObject->instanceList->next))->secretKey, pskBuffer, pskLen);
    //((security_instance_t *)(securityObject->instanceList->next))->secretKeyLen = pskLen;
    //objArray[0] = _objects->at(0)->Get();
    /*if (NULL == objArray[0])
    {
        fprintf(stderr, "Failed to create security object\r\n");
        return -1;
    }
    data.securityObjP = objArray[0];*/

    objArray[0] = get_security_object(serverId, serverUri, pskId, pskBuffer, pskLen, false);
    data.securityObjP = objArray[0];

    for (size_t i = 1; i < _objects->size(); ++i)
    {
        objArray[i] = _objects->at(i)->Get();
        if (!objArray[i])
        {
            fprintf(stderr, "Failed to create object %lu\r\n", i);
            return -1;
        }
    }

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
    result = lwm2m_configure(lwm2mH, CLIENT_ENDPOINT_NAME, NULL, NULL, OBJ_COUNT, objArray);
    if (result != 0)
    {
        fprintf(stderr, "lwm2m_configure() failed: 0x%X\r\n", result);
        return -1;
    }
    fprintf(stdout, "LWM2M Client \"%s\" started on port %d.\r\nUse Ctrl-C to exit.\r\n\n", CLIENT_ENDPOINT_NAME, CLIENT_LOCAL_PORT);

    lwm2mMainThread.start(&NodeClient::_lwm2mMainThreadTask);

    while (1)
    {
        ThisThread::sleep_for(1s);
        //mbed_stats_heap_t heap_stats;
        //mbed_stats_heap_get(&heap_stats);
    }

    delete objArray;

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

void NodeClient::_lwm2mHandleIncomingSocketDataCppWrap(ns_address_t *addr, uint8_t *buf, size_t len)
{
    printf("New packet arrived !\n");
    if (connectionlayer_handle_packet(data.connLayer, addr, buf, len) == -1)
    {
        /*
         * This packet comes from an unknown peer
         */
        fprintf(stderr, "received bytes ignored!\r\n");
    }

    lwm2mMainThreadTimeoutTimer.detach();
    lwm2mMainThread.flags_set(0x1);
}

NodeClient::~NodeClient(){
    for(NodeObject *object : *_objects){
        delete object;
    }
}

void lwm2m_main_thread_timeout_timer_cb()
{
    lwm2mMainThread.flags_set(0x1);
}

extern "C" void lwm2m_handle_incoming_socket_data(ns_address_t *addr, uint8_t *buf, size_t len)
{
    NodeClient::_lwm2mHandleIncomingSocketDataCppWrap(addr, buf, len);
}

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

    // parse uri in the form "coaps://[host]:[port]"
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
    printf("Crash 10\r\n");
    newConnP = (connection_t *)dtlsconnection_create(dataP->connLayer, secObjInstID, dataP->sock, host, port,
                                                     dataP->addressFamily);
#else
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