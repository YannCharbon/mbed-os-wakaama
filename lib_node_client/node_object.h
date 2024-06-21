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
 *  @file node_object.h
 *  @brief This header file contain the declaration of the object class representing a generic object following uCIFI standard
 *
 *  @author Bastien Pillonel
 *
 *  @date 6/20/2024
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

/**
 * @brief NodeObject class representing an object according to uCIFI standard
 * 
 */
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
        NodeObject *objectInstance;
    };

    /**
     * @brief Read callback used when read action is taken on a resource belonging to an object
     * 
     * @param contextP structure giving information about client-server connection context
     * @param instanceId object instance id
     * @param numDataP number of resources concerned by reading action 
     * @param dataArrayP data provided by the client to the server
     * @param objectP structure representing the object
     * @return uint8_t error code
     */
    uint8_t _objectRead(lwm2m_context_t *contextP,
                        uint16_t instanceId,
                        int *numDataP,
                        lwm2m_data_t **dataArrayP,
                        lwm2m_object_t *objectP);

    /**
     * @brief Write callback used when write action is taken on a resource belonging to an object
     * 
     * @param contextP structure giving information about client-server connection context
     * @param instanceId object instance id
     * @param numData number of resources concerned by writing action 
     * @param dataArray data provided by the server to the client
     * @param objectP structure representing the object
     * @param writeType when writing user is able to choose between replacing or updating the resource
     * @return uint8_t error code
     */
    uint8_t _objectWrite(lwm2m_context_t *contextP,
                         uint16_t instanceId,
                         int numData,
                         lwm2m_data_t *dataArray,
                         lwm2m_object_t *objectP,
                         lwm2m_write_type_t writeType);

    /**
     * @brief Execute callback used when execut action is taken on a resource belonging to an object
     * 
     * @param contextP structure giving information about client-server connection context
     * @param instanceId object instance id
     * @param resourceId object resource id
     * @param buffer not used (compatibility with liblwm2m.h)
     * @param length not used (compatibility with liblwm2m.h)
     * @param objectP structure representing the object
     * @return uint8_t error code
     */
    uint8_t _objectExec(lwm2m_context_t *contextP,
                        uint16_t instanceId,
                        uint16_t resourceId,
                        uint8_t *buffer,
                        int length,
                        lwm2m_object_t *objectP);

    /**
     * @brief Discover callback is call when configuring object and resource on the server
     * 
     * @param contextP structure giving information about client-server connection context
     * @param instanceId object instance id
     * @param numDataP number of resources concerned by discover action 
     * @param dataArrayP data provided by the server to the client
     * @param objectP structure representing the object
     * @return uint8_t error code
     */
    uint8_t _objectDiscover(lwm2m_context_t *contextP,
                            uint16_t instanceId,
                            int *numDataP,
                            lwm2m_data_t **dataArrayP,
                            lwm2m_object_t *objectP);

    /**
     * @brief Create callback is called when server request to creat a new object on the client.
     * Server only allows user to enter resource writable new value to the new object instance created.
     * Read resource value will be copied from original object instance and callbacks will be copied too. 
     * 
     * @param contextP structure giving information about client-server connection context
     * @param instanceId object instance id
     * @param numData number of resources concerned by discover action 
     * @param dataArray data provided by the server to the client
     * @param objectP structure representing the object
     * @return uint8_t error code
     */
    uint8_t _objectCreate(lwm2m_context_t *contextP,
                          uint16_t instanceId,
                          int numData,
                          lwm2m_data_t *dataArray,
                          lwm2m_object_t *objectP);

    /**
     * @brief Delete callback is called when server wants to delete an object instance
     * 
     * @param contextP structure giving information about client-server connection context
     * @param instanceId object instance id
     * @param objectP structure representing the object
     * @return uint8_t error code
     */
    uint8_t _objectDelete(lwm2m_context_t *contextP,
                          uint16_t instanceId,
                          lwm2m_object_t *objectP);

    /**
     * @brief Static create callback (needed for compatibility with liblwm2m). Create a new object instance from the original. 
     * 
     * @param contextP 
     * @param instanceId 
     * @param numData 
     * @param dataArray 
     * @param objectP 
     * @return uint8_t 
     */
    static uint8_t objectCreateStatic(lwm2m_context_t *contextP,
                                      uint16_t instanceId,
                                      int numData,
                                      lwm2m_data_t *dataArray,
                                      lwm2m_object_t *objectP);

    /**
     * @brief Static delete callback (needed for compatibility with liblwm2m). Delete corresponding object instance. 
     * 
     * @param contextP 
     * @param instanceId 
     * @param objectP 
     * @return uint8_t 
     */
    static uint8_t objectDeleteStatic(lwm2m_context_t *contextP,
                                      uint16_t instanceId,
                                      lwm2m_object_t *objectP);

    /**
     * @brief Static discover callback (needed for compatibility with liblwm2m). Discover corresponding object instance. 
     * 
     * @param contextP 
     * @param instanceId 
     * @param numDataP 
     * @param dataArrayP 
     * @param objectP 
     * @return uint8_t 
     */
    static uint8_t objectDiscoverStatic(lwm2m_context_t *contextP,
                                        uint16_t instanceId,
                                        int *numDataP,
                                        lwm2m_data_t **dataArrayP,
                                        lwm2m_object_t *objectP);

    /**
     * @brief Static read callback (needed for compatibility with liblwm2m). Read corresponding object instance. 
     * 
     * @param contextP 
     * @param instanceId 
     * @param numDataP 
     * @param dataArrayP 
     * @param objectP 
     * @return uint8_t 
     */
    static uint8_t objectReadStatic(lwm2m_context_t *contextP,
                                    uint16_t instanceId,
                                    int *numDataP,
                                    lwm2m_data_t **dataArrayP,
                                    lwm2m_object_t *objectP);

    /**
     * @brief Static write callback (needed for compatibility with liblwm2m). Write corresponding object instance. 
     * 
     * @param contextP 
     * @param instanceId 
     * @param numData 
     * @param dataArray 
     * @param objectP 
     * @param writeType 
     * @return uint8_t 
     */
    static uint8_t objectWriteStatic(lwm2m_context_t *contextP,
                                     uint16_t instanceId,
                                     int numData,
                                     lwm2m_data_t *dataArray,
                                     lwm2m_object_t *objectP,
                                     lwm2m_write_type_t writeType);

    /**
     * @brief Static execute callback (needed for compatibility with liblwm2m). Execute corresponding object instance. 
     * 
     * @param contextP 
     * @param instanceId 
     * @param resourceId 
     * @param buffer 
     * @param length 
     * @param objectP 
     * @return uint8_t 
     */
    static uint8_t objectExecStatic(lwm2m_context_t *contextP,
                                    uint16_t instanceId,
                                    uint16_t resourceId,
                                    uint8_t *buffer,
                                    int length,
                                    lwm2m_object_t *objectP);

public:
    /**
     * @brief Construct a new Node Object object by default
     * 
     */
    NodeObject() : _objectId(0), _instanceId(0), _resources({}) {}

    /**
     * @brief Construct a new Node Object object by copy
     * 
     * @param src 
     */
    NodeObject(const NodeObject &src) : _objectId(src._objectId), _instanceId(src._instanceId) {
        for(auto pair : src._resources){
            _resources[pair.first] = new Resource(*pair.second);
        }
    }

    /**
     * @brief Construct a new Node Object object by moving
     * 
     * @param src 
     */
    NodeObject(NodeObject &&src) : _objectId(src._objectId), _instanceId(src._instanceId), _resources(std::move(src._resources))
    {
        src._objectId = 0;
        src._instanceId = 0;
    }

    /**
     * @brief Construct a new Node Object object by specifying attribute
     * 
     * @param objId objcet id
     * @param instId object instance id
     * @param resources list of resource instances belonging to object instance
     */
    NodeObject(uint16_t objId, uint16_t instId, std::vector<Resource *> resources) : _objectId(objId), _instanceId(instId)
    {
        for (Resource *res : resources)
        {
            _resources[res->GetId()] = res;
        }
    }

    /**
     * @brief Get the Resource object
     * 
     * @param id instance id of object
     * @return Resource* pointer on the object
     */
    Resource *GetResource(size_t id);

    /**
     * @brief Destroy the Node Object object
     * 
     */
    ~NodeObject();

    /**
     * @brief Get lwm2m_object_t structure represanting our object (used for compatibility with liblwm2m.h)
     * 
     * @return lwm2m_object_t* 
     */
    lwm2m_object_t *Get();
};

#include "node_object_impl.h"

#endif