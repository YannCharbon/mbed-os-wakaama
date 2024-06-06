/**
 *  @file node_object.h
 *  @brief This header file contain the declaration of the object class representing a generic object following uCIFI standard
 *
 *  @author Bastien Pillonel
 *
 *  @date 5/2/2024
 */

#ifndef NODE_OBJECT_H
#define NODE_OBJECT_H

#include <cstdio>
#include <iostream>
#include <map>
#include <memory>
#include <type_traits>
#include <vector>
#include <string>
#include <cstring>
#include <functional>
#include "liblwm2m.h"

#include "resource.h"

typedef struct _security_instance_
{
    struct _security_instance_ * next;        // matches lwm2m_list_t::next
    uint16_t                     instanceId;  // matches lwm2m_list_t::id
    char *                       uri;
    bool                         isBootstrap;    
    uint8_t                      securityMode;
    char *                       publicIdentity;
    uint16_t                     publicIdLen;
    char *                       serverPublicKey;
    uint16_t                     serverPublicKeyLen;
    char *                       secretKey;
    uint16_t                     secretKeyLen;
    uint8_t                      smsSecurityMode;
    char *                       smsParams; // SMS binding key parameters
    uint16_t                     smsParamsLen;
    char *                       smsSecret; // SMS binding secret key
    uint16_t                     smsSecretLen;
    uint16_t                     shortID;
    uint32_t                     clientHoldOffTime;
    uint32_t                     bootstrapServerAccountTimeout;
} security_instance_t;

class NodeObject
{
private:
    size_t _objectId;
    size_t _instanceId;
    std::map<size_t, Resource *> _resources;

    struct ObjectList
    {
        struct ObjectList *next;
        uint16_t instanceId;
    };

    uint8_t _objectRead(lwm2m_context_t *contextP,
                        uint16_t instanceId,
                        int *numDataP,
                        lwm2m_data_t **dataArrayP,
                        lwm2m_object_t *objectP);

    uint8_t _objectWrite(lwm2m_context_t *contextP,
                         uint16_t instanceId,
                         int numData,
                         lwm2m_data_t *dataArray,
                         lwm2m_object_t *objectP,
                         lwm2m_write_type_t writeType);

    uint8_t _objectExec(lwm2m_context_t *contextP,
                        uint16_t instanceId,
                        uint16_t resourceId,
                        uint8_t *buffer,
                        int length,
                        lwm2m_object_t *objectP);

    uint8_t _objectDiscover(lwm2m_context_t *contextP,
                            uint16_t instanceId,
                            int *numDataP,
                            lwm2m_data_t **dataArrayP,
                            lwm2m_object_t *objectP);

    uint8_t _objectCreate(lwm2m_context_t *contextP,
                          uint16_t instanceId,
                          int numData,
                          lwm2m_data_t *dataArray,
                          lwm2m_object_t *objectP);

    uint8_t _objectDelete(lwm2m_context_t *contextP,
                          uint16_t instanceId,
                          lwm2m_object_t *objectP);

    static uint8_t objectCreateStatic(lwm2m_context_t *contextP,
                                      uint16_t instanceId,
                                      int numData,
                                      lwm2m_data_t *dataArray,
                                      lwm2m_object_t *objectP);

    static uint8_t objectDeleteStatic(lwm2m_context_t *contextP,
                                      uint16_t instanceId,
                                      lwm2m_object_t *objectP);

    static uint8_t objectDiscoverStatic(lwm2m_context_t *contextP,
                                        uint16_t instanceId,
                                        int *numDataP,
                                        lwm2m_data_t **dataArrayP,
                                        lwm2m_object_t *objectP);

    static uint8_t objectReadStatic(lwm2m_context_t *contextP,
                                    uint16_t instanceId,
                                    int *numDataP,
                                    lwm2m_data_t **dataArrayP,
                                    lwm2m_object_t *objectP);

    static uint8_t objectWriteStatic(lwm2m_context_t *contextP,
                                     uint16_t instanceId,
                                     int numData,
                                     lwm2m_data_t *dataArray,
                                     lwm2m_object_t *objectP,
                                     lwm2m_write_type_t writeType);

    static uint8_t objectExecStatic(lwm2m_context_t *contextP,
                                    uint16_t instanceId,
                                    uint16_t resourceId,
                                    uint8_t *buffer,
                                    int length,
                                    lwm2m_object_t *objectP);

public:
    NodeObject() : _objectId(0), _instanceId(0), _resources({}) {}
    NodeObject(const NodeObject &src) : _objectId(src._objectId), _instanceId(src._instanceId), _resources(src._resources) {}
    NodeObject(NodeObject &&src) : _objectId(src._objectId), _instanceId(src._instanceId), _resources(std::move(src._resources))
    {
        src._objectId = 0;
        src._instanceId = 0;
    }
    NodeObject(uint16_t objId, uint16_t instId, std::vector<Resource *> resources) : _objectId(objId), _instanceId(instId)
    {
        for (Resource *res : resources)
        {
            _resources[res->GetId()] = res;
        }
    }

    ~NodeObject();

    lwm2m_object_t *Get();
};

#include "node_object_impl.h"

#endif