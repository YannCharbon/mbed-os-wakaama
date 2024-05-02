/*
 * Copyright (c) 2023
 * Yann Charbon <yann.charbon@heig-vd.h>
 *
 */

/*******************************************************************************
 *
 * Copyright (c) 2013, 2014, 2015 Intel Corporation and others.
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
 *    Benjamin Cabé - Please refer to git log
 *    Fabien Fleutot - Please refer to git log
 *    Simon Bernard - Please refer to git log
 *    Julien Vermillard - Please refer to git log
 *    Axel Lorente - Please refer to git log
 *    Toby Jaffey - Please refer to git log
 *    Bosch Software Innovations GmbH - Please refer to git log
 *    Pascal Rieux - Please refer to git log
 *    Christian Renz - Please refer to git log
 *
 *******************************************************************************/

/*
 Copyright (c) 2013, 2014 Intel Corporation

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

     * Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
     * Neither the name of Intel Corporation nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.

 David Navarro <david.navarro@intel.com>
 Bosch Software Innovations GmbH - Please refer to git log

*/

#include "mbed.h"
#include "NetworkInterface.h"
#include "EthernetInterface.h"
#include "UDPSocket.h"
#include "mesh_system.h"
#include "net_interface.h"
#include "ip6string.h"

#include "object_device.h"

using namespace std::chrono;

// Network interface
NetworkInterface *eth;


static Thread lwm2m_main_thread(osPriorityNormal, OS_STACK_SIZE, nullptr, "lwm2m_main_thread");

void lwm2m_main_thread_task();

static Timeout lwm2m_main_thread_timeout_timer;

void lwm2m_main_thread_timeout_timer_cb() {
    lwm2m_main_thread.flags_set(0x1);
}

extern "C" {
#include "liblwm2m.h"
#include "connection.h"
#if defined(USE_DTLS)
#include "dtlsconnection.h"
#endif
#include "object_utils.h"

#define STR(x)  #x

#define CLIENT_ENDPOINT_NAME    "mbedM2M"
#define CLIENT_IDENTITY         "mbedM2M"
#define CLIENT_KEYSTR           "65875A0C3D4646A99BFC4D5967EE7DB3"
#define SERVER_DTLS_PORT        "5684"
#define SERVER_PORT             "5683"
#define CLIENT_LOCAL_PORT       0       // Let OS decide

//Connect to the Leshan test server as default: http://leshan.eclipse.org
#define M2M_SERVER_URL "2a01:111:f100:9001::1761:93fa" // LESHAN

extern lwm2m_object_t * get_object_device(void);
extern void free_object_device(lwm2m_object_t * objectP);
extern lwm2m_object_t * get_server_object(void);
extern void free_server_object(lwm2m_object_t * object);
lwm2m_object_t * get_security_object(int serverId, const char* serverUri, char * bsPskId, char * psk, uint16_t pskLen, bool isBootstrap);
extern void free_security_object(lwm2m_object_t * objectP);
extern char * get_server_uri(lwm2m_object_t * objectP, uint16_t secObjInstID);
extern lwm2m_object_t * get_test_object(void);
extern void free_test_object(lwm2m_object_t * object);
}

#define MAX_PACKET_SIZE 2048

int g_reboot = 0;

typedef struct
{
    lwm2m_object_t * securityObjP;
    int sock;
    connection_t * connList;
    int addressFamily;
    lwm2m_context_t *ctx;
    lwm2m_connection_layer_t *connLayer;
} client_data_t;

client_data_t data;
lwm2m_context_t * lwm2mH = NULL;

static void print_interface_addr(int id) {
    printf("Interface %d:\n", id);

    uint8_t address_buf[128];
    int address_count = 0;
    char buf[128];

    int ret = arm_net_address_list_get(id, 128, address_buf, &address_count);

    if (ret == 0) {
        uint8_t *t_buf = address_buf;
        for (int i = 0; i < address_count; ++i) {
            ip6tos(t_buf, buf);
            printf(" [%d] %s\n", i, buf);
            t_buf += 16;
        }
    } else if (ret == -1) {
        printf("Unknown interface ID\n");
    }
}

int init_network()
{
    int ret = 0;

    //try to connect and get ip via DHCP.
    printf("=====================\n");
    printf("obtaining ip address...\n");

    eth = EthInterface::get_default_instance();
    ret = eth->connect();

    if(ret != 0){
        printf("DHCP Error - No IP");
        return ret;
    }

    // Show the network address
    SocketAddress a;
    eth->get_ip_address(&a);
    printf("IP address: %s\n", a.get_ip_address() ? a.get_ip_address() : "None");

    print_interface_addr(1);

    return ret;
}

void * lwm2m_connect_server(uint16_t secObjInstID,
                            void * userData)
{
    client_data_t * dataP;
    char * uri;
    char * host;
    char * port;
    connection_t * newConnP = NULL;

    dataP = (client_data_t *)userData;

    uri = get_server_uri(dataP->securityObjP, secObjInstID);

    if (uri == NULL) return NULL;

    printf("Connecting to %s\r\n", uri);

    // parse uri in the form "coaps://[host]:[port]"
    if (0 == strncmp(uri, "coaps://", strlen("coaps://")))
    {
        host = uri+strlen("coaps://");
    }
    else if (0 == strncmp(uri, "coap://", strlen("coap://")))
    {
        host = uri+strlen("coap://");
    }
    else
    {
        goto exit;
    }
    port = strrchr(host, ':');
    if (port == NULL) goto exit;
    // remove brackets
    if (host[0] == '[')
    {
        host++;
        if (*(port - 1) == ']')
        {
            *(port - 1) = 0;
        }
        else goto exit;
    }
    // split strings
    *port = 0;
    port++;

#if defined(USE_DTLS)
    newConnP = (connection_t *)dtlsconnection_create(dataP->connLayer, secObjInstID, dataP->sock, host, port,
                                                         dataP->addressFamily);
#else
    newConnP = connection_create(dataP->connLayer, dataP->sock, host, port, dataP->addressFamily);
#endif

    if (newConnP == NULL) {
        printf("Connection creation failed.\r\n");
    }
    else {
        dataP->connList = newConnP;
    }

exit:
    lwm2m_free(uri);
    return (void *)newConnP;
}

void lwm2m_close_connection(void * sessionH,
                            void * userData)
{
    client_data_t * app_data;
    connection_t * targetP;

    app_data = (client_data_t *)userData;
    targetP = (connection_t *)sessionH;

    if (targetP == app_data->connList)
    {
        app_data->connList = targetP->next;
        lwm2m_free(targetP);
    }
    else
    {
        connection_t * parentP;

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

void print_state(lwm2m_context_t * lwm2mH)
{
    lwm2m_server_t * targetP;

    printf("State: ");
    switch(lwm2mH->state)
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
        for (targetP = lwm2mH->bootstrapServerList ; targetP != NULL ; targetP = targetP->next)
        {
            printf(" - Security Object ID %d", targetP->secObjInstID);
            printf("\tHold Off Time: %lu s", (unsigned long)targetP->lifetime);
            printf("\tstatus: ");
            switch(targetP->status)
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
        for (targetP = lwm2mH->serverList ; targetP != NULL ; targetP = targetP->next)
        {
            printf(" - Server ID %d", targetP->shortID);
            printf("\tstatus: ");
            switch(targetP->status)
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

#define OBJ_COUNT 4


#if !MBED_TEST_MODE
#ifdef NO_DEBUG
int main(int argc, char *argv[])
{
    init_network();

    lwm2m_object_t * objArray[OBJ_COUNT];

    memset(&data, 0, sizeof(client_data_t));
    data.addressFamily = ADDRESS_IPV6;

    int result;

#ifdef USE_DTLS
    char * pskId = CLIENT_IDENTITY;
    char *psk = CLIENT_KEYSTR;

    uint16_t pskLen = 0;
    char * pskBuffer = NULL;
#else
    char * pskId = NULL;
    char *psk = NULL;
    uint16_t pskLen = -1;
    char * pskBuffer = NULL;
#endif

    /*
     *This call an internal function that create an IPV6 socket on the port.
     */
    fprintf(stderr, "Trying to bind LWM2M Client to port %d\r\n", CLIENT_LOCAL_PORT);
    data.sock = create_socket(CLIENT_LOCAL_PORT, data.addressFamily);
    if (data.sock < 0)
    {
        fprintf(stderr, "Failed to open socket: %d %s\r\n", errno, strerror(errno));
        return -1;
    }

    /*
     * Now the main function fill an array with each object, this list will be later passed to liblwm2m.
     * Those functions are located in their respective object file.
     */
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

        for ( ; *h; h += 2, ++b)
        {
            char *l = strchr(xlate, toupper(*h));
            char *r = strchr(xlate, toupper(*(h+1)));

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
    sprintf (serverUri, "coaps://[%s]:%s", M2M_SERVER_URL, SERVER_DTLS_PORT);
#else
    sprintf (serverUri, "coap://[%s]:%s", M2M_SERVER_URL, SERVER_PORT);
#endif
    /*
     * Now the main function fill an array with each object, this list will be later passed to liblwm2m.
     * Those functions are located in their respective object file.
     */
    objArray[0] = get_security_object(serverId, serverUri, pskId, pskBuffer, pskLen, false);
    if (NULL == objArray[0])
    {
        fprintf(stderr, "Failed to create security object\r\n");
        return -1;
    }
    data.securityObjP = objArray[0];

    objArray[1] = get_server_object();
    if (NULL == objArray[1])
    {
        fprintf(stderr, "Failed to create server object\r\n");
        return -1;
    }

    //objArray[2] = get_object_device();
    ObjectDevice* objectDevice = new ObjectDevice();
    objArray[2] = objectDevice->GetObjectDevice();
    if (NULL == objArray[2])
    {
        fprintf(stderr, "Failed to create Device object\r\n");
        return -1;
    }

    objArray[3] = get_test_object();
    if (NULL == objArray[3])
    {
        fprintf(stderr, "Failed to create Test object\r\n");
        return -1;
    }

    /*
     * The liblwm2m library is now initialized with the functions that will be in
     * charge of communication
     */
    printf("lwm2m_init\n");
    lwm2mH = lwm2m_init(&data);
    if (NULL == lwm2mH)
    {
        fprintf(stderr, "lwm2m_init() failed\r\n");
        return -1;
    }

    data.ctx = lwm2mH;
    data.connLayer = connectionlayer_create(lwm2mH);

    /*
     * We configure the liblwm2m library with the name of the client - which shall be unique for each client -
     * the number of objects we will be passing through and the objects array
     */
    printf("lwm2m_configure\n");
    result = lwm2m_configure(lwm2mH, CLIENT_ENDPOINT_NAME, NULL, NULL, OBJ_COUNT, objArray);
    if (result != 0)
    {
        fprintf(stderr, "lwm2m_configure() failed: 0x%X\r\n", result);
        return -1;
    }
    fprintf(stdout, "LWM2M Client \"%s\" started on port %d.\r\nUse Ctrl-C to exit.\r\n\n", CLIENT_ENDPOINT_NAME, CLIENT_LOCAL_PORT);

    /*
     * We now enter in a while loop that will handle the communications from the server
     */
    lwm2m_main_thread.start(&lwm2m_main_thread_task);

    while(1) {
        ThisThread::sleep_for(1s);
    }

    return 0;
}
#endif
#endif

void lwm2m_main_thread_task() {
    while(1) {
        struct timeval tv;

        tv.tv_sec = 60;
        tv.tv_usec = 0;

        print_state(lwm2mH);

        /*
        * This function does two things:
        *  - first it does the work needed by liblwm2m (eg. (re)sending some packets).
        *  - Secondly it adjusts the timeout value (default 60s) depending on the state of the transaction
        *    (eg. retransmission) and the time before the next operation
        */
        int result = lwm2m_step(lwm2mH, &(tv.tv_sec));
        if (result != 0) {
            fprintf(stderr, "lwm2m_step() failed: 0x%X\r\n", result);
        }
        ThisThread::flags_wait_any_for(0x1, seconds(tv.tv_sec));
    }
}

void lwm2m_handle_incoming_socket_data_cpp_wrap(ns_address_t *addr, uint8_t *buf, size_t len) {
    printf("New packet arrived !\n");
    if (connectionlayer_handle_packet(data.connLayer, addr, buf, len) == -1) {
        /*
        * This packet comes from an unknown peer
        */
        fprintf(stderr, "received bytes ignored!\r\n");
    }

    lwm2m_main_thread_timeout_timer.detach();
    lwm2m_main_thread.flags_set(0x1);
}

extern "C" void lwm2m_handle_incoming_socket_data(ns_address_t *addr, uint8_t *buf, size_t len) {
    lwm2m_handle_incoming_socket_data_cpp_wrap(addr, buf, len);
}