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

#define THINGSBOARD_SERVER_URL "2a05:d018:1db1:3d01:c117:6d9a:bd23:5c50"
#define THINGSBOARD_SERVER_PORT "6683"
#define THINGSBOARD_DTLS_SERVER_PORT "6684"
#define THINGSBOARD_BOOTSTRAP_SERVER_PORT "5683"
#define THINGSBOARD_DTLS_BOOTSTRAP_SERVER_PORT "5684"

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

class NodeClient
{
public:
    NodeClient() : _objects({}) {}
    NodeClient(const NodeClient &src) : _objects(src._objects) {}
    NodeClient(NodeClient &&src) : _objects(std::move(src._objects)) {}
    NodeClient(std::vector<NodeObject *> *objects) : _objects(objects) {}

    ~NodeClient();

    int InitNetwork();
    int StartClient();

    static void _lwm2mHandleIncomingSocketDataCppWrap(ns_address_t *addr, uint8_t *buf, size_t len);

private:
    std::vector<NodeObject *> *_objects;
    NetworkInterface *_eth;

    static void _printInterfaceAddr(int id);
    static void _printstate(lwm2m_context_t *lwm2mH);
    static void _lwm2mMainThreadTask();
};

#include "node_client_impl.h"

#endif