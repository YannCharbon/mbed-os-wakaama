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
 *  @file node_object_impl.h
 *  @brief This header file contain the definition of the object class methods representing a generic object following uCIFI standard
 *
 *  @author Bastien Pillonel
 *
 *  @date 6/21/2024
 */

#ifndef NODE_OBJECT_IMPL_H
#define NODE_OBJECT_IMPL_H

#include "node_object.h"

uint8_t NodeObject::_objectCreate(lwm2m_context_t *contextP,
                                  uint16_t instanceId,
                                  int numData,
                                  lwm2m_data_t *dataArray,
                                  lwm2m_object_t *objectP)
{
    ObjectList *objectInstance;
    uint8_t result;

    // Create new ObjectList structure (corresponding to the object instance)
    objectInstance = (ObjectList *)lwm2m_malloc(sizeof(ObjectList));
    if (!objectInstance)
        return COAP_500_INTERNAL_SERVER_ERROR;
    memset(objectInstance, 0, sizeof(ObjectList));

    // Insert new object instance node at the end of the linked list
    objectInstance->instanceId = instanceId;
    objectInstance->next = nullptr;
    objectInstance->objectInstance = this;
    objectP->instanceList = LWM2M_LIST_ADD(objectP->instanceList, objectInstance);

    // Write value from server side
    result = _objectWrite(contextP, instanceId, numData, dataArray, objectP, LWM2M_WRITE_REPLACE_RESOURCES);
    if (result != COAP_204_CHANGED)
    {
        (void)_objectDelete(contextP, instanceId, objectP);
    }
    else
    {
        result = COAP_201_CREATED;
    }

    return result;
}

uint8_t NodeObject::_objectDelete(lwm2m_context_t *contextP,
                                  uint16_t instanceId,
                                  lwm2m_object_t *objectP)
{
    ObjectList *objectInstance;

    /* unused parameter */
    (void)contextP;

    // Remove ObjectList corresponding to the instance from object instance list
    objectP->instanceList = lwm2m_list_remove(objectP->instanceList, instanceId, (lwm2m_list_t **)&objectInstance);
    if (!objectInstance)
        return COAP_404_NOT_FOUND;

    lwm2m_free(objectInstance);

    return COAP_202_DELETED;
}

uint8_t NodeObject::_objectDiscover(lwm2m_context_t *contextP,
                                    uint16_t instanceId,
                                    int *numDataP,
                                    lwm2m_data_t **dataArrayP,
                                    lwm2m_object_t *objectP)
{
    uint8_t result;
    int i = 0;

    /* Unused parameter */
    (void)contextP;

    result = COAP_205_CONTENT;

    // Is the server asking for full object
    if (*numDataP == 0)
    {
        size_t nbRes = _resources.size();
        *dataArrayP = lwm2m_data_new(nbRes);
        if (!(*dataArrayP))
            return COAP_500_INTERNAL_SERVER_ERROR;

        *numDataP = nbRes;

        // Fulfil dataArray's ids with all resources ids of object
        for (const auto &pair : _resources)
        {
            (*dataArrayP)[i].id = static_cast<uint16_t>(pair.first);
            ++i;
        }
    }
    // Server is only asking for certain resources
    else
    {
        for (i = 0; i < *numDataP && result == COAP_205_CONTENT; ++i)
        {
            if (_resources.find((*dataArrayP)[i].id) == _resources.end())
            {
                result = COAP_404_NOT_FOUND;
            }
        }
    }

    return result;
}

uint8_t NodeObject::_objectRead(lwm2m_context_t *contextP,
                                uint16_t instanceId,
                                int *numDataP,
                                lwm2m_data_t **dataArrayP,
                                lwm2m_object_t *objectP)
{
    ObjectList *objectInstance;
    uint8_t result;
    int i = 0;

    /* Unused parameter */
    (void)contextP;

    objectInstance = (ObjectList *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (!objectInstance)
        return COAP_404_NOT_FOUND;

    // Is the server asking for the full instance ?
    if (*numDataP == 0)
    {
        size_t nbRes = 0;

        // Count only resource that are readable
        for (const auto &pair : _resources)
            if (pair.second->GetOp() == ResourceOp::RES_RDWR || pair.second->GetOp() == ResourceOp::RES_RD)
                nbRes++;

        *dataArrayP = lwm2m_data_new(nbRes);
        if (!(*dataArrayP))
            return COAP_500_INTERNAL_SERVER_ERROR;

        *numDataP = nbRes;

        // Store all resources ids that are readable
        for (const auto &pair : _resources)
        {
            if (pair.second->GetOp() == ResourceOp::RES_RDWR || pair.second->GetOp() == ResourceOp::RES_RD)
            {
                (*dataArrayP)[i].id = static_cast<uint16_t>(pair.first);
                ++i;
            }
        }
    }

    i = 0;
    do
    {
        if ((*dataArrayP)[i].type == LWM2M_TYPE_MULTIPLE_RESOURCE)
        {
            result = COAP_404_NOT_FOUND;
        }
        else
        {
            // Find resource instance associated to id
            auto resourceIt = _resources.find((*dataArrayP)[i].id);
            if (resourceIt == _resources.end())
            {
                result = COAP_404_NOT_FOUND;
            }
            else
            {
                // Check type of resource to use correct encoding function
                Resource *objectRes = (*resourceIt).second;
                result = COAP_205_CONTENT;

                if ((*objectRes).Type() == typeid(int))
                    lwm2m_data_encode_int(*((*objectRes).Read<int>()), (*dataArrayP) + i);
                else if ((*objectRes).Type() == typeid(bool))
                    lwm2m_data_encode_bool(*((*objectRes).Read<bool>()), (*dataArrayP) + i);
                else if ((*objectRes).Type() == typeid(float))
                    lwm2m_data_encode_float(*((*objectRes).Read<float>()), (*dataArrayP) + i);
                else if ((*objectRes).Type() == typeid(double))
                    lwm2m_data_encode_float(*((*objectRes).Read<double>()), (*dataArrayP) + i);
                else if ((*objectRes).Type() == typeid(std::string))
                {
                    lwm2m_data_encode_string((*((*objectRes).Read<std::string>())).c_str(), (*dataArrayP) + i);
                }
                // Resources type is multiple instances
                else if ((*objectRes).Type() == typeid(std::map<size_t, Resource *>))
                {
                    const std::map<size_t, Resource *> *resourcesInstList = ((*objectRes).GetValue<std::map<size_t, Resource *>>());
                    size_t count;
                    lwm2m_data_t *subData;

                    // Prepare subData array to get every resource instances value and id
                    if ((*dataArrayP)[i].type == LWM2M_TYPE_MULTIPLE_RESOURCE)
                    {
                        count = (*dataArrayP)->value.asChildren.count;
                        subData = (*dataArrayP)->value.asChildren.array;
                    }
                    else
                    {
                        count = resourcesInstList->size();
                        subData = lwm2m_data_new(count);
                        size_t idx = 0;
                        for (auto pair : (*resourcesInstList))
                        {
                            subData[idx].id = pair.first;
                            idx++;
                        }
                        lwm2m_data_encode_instances(subData, count, (*dataArrayP) + i);
                    }

                    // For every resources in the resource encode the value with corresponding function
                    Resource *resourceInstance;
                    for (size_t idx = 0; idx < count; ++idx)
                    {
                        std::cout << "Sub id is " << subData[idx].id << std::endl;
                        auto resourceInstIt = resourcesInstList->find(subData[idx].id);
                        resourceInstance = (*resourceInstIt).second;
                        if ((*resourceInstance).Type() == typeid(int))
                            lwm2m_data_encode_int(*((*resourceInstance).Read<int>()), subData + idx);
                        else if ((*resourceInstance).Type() == typeid(bool))
                            lwm2m_data_encode_bool(*((*resourceInstance).Read<bool>()), subData + idx);
                        else if ((*resourceInstance).Type() == typeid(float))
                            lwm2m_data_encode_float(*((*resourceInstance).Read<float>()), subData + idx);
                        else if ((*resourceInstance).Type() == typeid(double))
                            lwm2m_data_encode_float(*((*resourceInstance).Read<double>()), subData + idx);
                        else if ((*resourceInstance).Type() == typeid(std::string))
                            lwm2m_data_encode_string((*((*resourceInstance).Read<std::string>())).c_str(), subData + idx);
                    }
                }
                else
                    result = COAP_404_NOT_FOUND;
            }
        }
        ++i;
    } while (i < *numDataP && result == COAP_205_CONTENT);
    return result;
}

uint8_t NodeObject::_objectWrite(lwm2m_context_t *contextP,
                                 uint16_t instanceId,
                                 int numData,
                                 lwm2m_data_t *dataArray,
                                 lwm2m_object_t *objectP,
                                 lwm2m_write_type_t writeType)
{
    ObjectList *objectInstance;
    int i;
    uint8_t result;

    objectInstance = (ObjectList *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (!objectInstance)
        return COAP_404_NOT_FOUND;

    // If replace action is asked from server side, delete and recreate the object
    if (writeType == LWM2M_WRITE_REPLACE_INSTANCE)
    {
        result = _objectDelete(contextP, instanceId, objectP);
        if (result == COAP_202_DELETED)
        {
            result = _objectCreate(contextP, instanceId, numData, dataArray, objectP);
            if (result == COAP_201_CREATED)
            {
                result = COAP_204_CHANGED;
            }
        }
        return result;
    }

    i = 0;
    do
    {
        // Find resource corresponding to id
        auto resourceIt = _resources.find(dataArray[i].id);
        if (resourceIt == _resources.end())
        {
            result = COAP_404_NOT_FOUND;
        }
        else
        {
            // Execute coresponding decoding function to write on the right resource
            Resource *objectRes = (*resourceIt).second;

            if ((*objectRes).Type() == typeid(int))
            {
                int64_t valI;
                lwm2m_data_decode_int(dataArray + i, &valI);
                result = ((*objectRes).Write<int>((int)valI) == RES_SUCCESS ? COAP_204_CHANGED : COAP_404_NOT_FOUND);
            }
            else if ((*objectRes).Type() == typeid(bool))
            {
                bool valB;
                lwm2m_data_decode_bool(dataArray + i, &valB);
                result = ((*objectRes).Write<bool>(valB) == RES_SUCCESS ? COAP_204_CHANGED : COAP_404_NOT_FOUND);
            }
            else if ((*objectRes).Type() == typeid(float))
            {
                double valF;
                lwm2m_data_decode_float(dataArray + i, &valF);
                result = ((*objectRes).Write<float>((float)valF) == RES_SUCCESS ? COAP_204_CHANGED : COAP_404_NOT_FOUND);
            }
            else if ((*objectRes).Type() == typeid(double))
            {
                double valD;
                lwm2m_data_decode_float(dataArray + i, &valD);
                result = ((*objectRes).Write<double>(valD) == RES_SUCCESS ? COAP_204_CHANGED : COAP_404_NOT_FOUND);
            }
            else if ((*objectRes).Type() == typeid(std::string))
            {
                std::string stringValue = std::string((char *)(dataArray[i].value.asBuffer.buffer));
                stringValue.resize(dataArray[i].value.asBuffer.length);
                result = ((*objectRes).Write<std::string>(stringValue) == RES_SUCCESS ? COAP_204_CHANGED : COAP_404_NOT_FOUND);
            }
            // Multiple instance resource
            else if ((*objectRes).Type() == typeid(std::map<size_t, Resource *>))
            {
                // Create subData array to store id and value of resources inside the resource
                size_t count = dataArray[i].value.asChildren.count;
                lwm2m_data_t *subData = dataArray[i].value.asChildren.array;

                std::map<size_t, Resource *> *resourcesInstList = ((*objectRes).GetValue<std::map<size_t, Resource *>>());
                Resource *resourceInstance;

                // Ensure resource id asked for creating new resource instance isn't already taken
                auto resourceInstIt = resourcesInstList->find(subData[0].id);
                if (resourceInstIt != resourcesInstList->end())
                    return COAP_405_METHOD_NOT_ALLOWED;

                // Ensure resource map isn't empty
                resourceInstIt = resourcesInstList->find(0);
                if (resourceInstIt == resourcesInstList->end())
                    return COAP_405_METHOD_NOT_ALLOWED;

                // For every resource instances inside the resource, write corresponding value inside correct resource instance
                resourceInstance = (*resourceInstIt).second;
                for (size_t idx = 0; idx < count; ++idx)
                {
                    if ((*resourceInstance).Type() == typeid(int))
                    {
                        int64_t valI;
                        lwm2m_data_decode_int(subData + idx, &valI);
                        (*resourcesInstList)[subData[idx].id] = new Resource(*resourceInstance);
                        result = (((*resourcesInstList)[subData[idx].id])->Write<int>(valI) == RES_SUCCESS ? COAP_204_CHANGED : COAP_404_NOT_FOUND);
                    }
                    else if ((*resourceInstance).Type() == typeid(bool))
                    {
                        bool valB;
                        lwm2m_data_decode_bool(subData + idx, &valB);
                        (*resourcesInstList)[subData[idx].id] = new Resource(*resourceInstance);
                        result = (((*resourcesInstList)[subData[idx].id])->Write<bool>(valB) == RES_SUCCESS ? COAP_204_CHANGED : COAP_404_NOT_FOUND);
                    }
                    else if ((*resourceInstance).Type() == typeid(float))
                    {
                        double valF;
                        lwm2m_data_decode_float(subData + idx, &valF);
                        (*resourcesInstList)[subData[idx].id] = new Resource(*resourceInstance);
                        result = (((*resourcesInstList)[subData[idx].id])->Write<float>(valF) == RES_SUCCESS ? COAP_204_CHANGED : COAP_404_NOT_FOUND);
                    }
                    else if ((*resourceInstance).Type() == typeid(double))
                    {
                        double valD;
                        lwm2m_data_decode_float(subData + idx, &valD);
                        (*resourcesInstList)[subData[idx].id] = new Resource(*resourceInstance);
                        result = (((*resourcesInstList)[subData[idx].id])->Write<double>(valD) == RES_SUCCESS ? COAP_204_CHANGED : COAP_404_NOT_FOUND);
                    }
                    else if ((*resourceInstance).Type() == typeid(std::string))
                    {
                        std::string stringValue = std::string((char *)(subData[idx].value.asBuffer.buffer));
                        stringValue.resize(subData[idx].value.asBuffer.length);
                        (*resourcesInstList)[subData[idx].id] = new Resource(*resourceInstance);
                        result = (((*resourcesInstList)[subData[idx].id])->Write<std::string>(stringValue) == RES_SUCCESS ? COAP_204_CHANGED : COAP_404_NOT_FOUND);
                    }
                }
            }
            else
                result = COAP_404_NOT_FOUND;
        }

        ++i;
    } while (i < numData && result == COAP_204_CHANGED);

    return result;
}

uint8_t NodeObject::_objectExec(lwm2m_context_t *contextP,
                                uint16_t instanceId,
                                uint16_t resourceId,
                                uint8_t *buffer,
                                int length,
                                lwm2m_object_t *objectP)
{
    ObjectList *objectInstance;
    uint8_t result;

    /* Unused parameter */
    (void)contextP;

    objectInstance = (ObjectList *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (!objectInstance)
    {
        return COAP_404_NOT_FOUND;
    }

    // Find corresponding resource from id
    auto resourceIt = _resources.find(static_cast<size_t>(resourceId));
    if (resourceIt == _resources.end())
    {
        result = COAP_404_NOT_FOUND;
    }
    else
    {
        // For each resource to execute, call resource's execute method
        Resource objectRes(*((*resourceIt).second));

        if (objectRes.Type() == typeid(int))
            result = (objectRes.Exec<int>() == RES_SUCCESS ? COAP_204_CHANGED : COAP_405_METHOD_NOT_ALLOWED);
        else if (objectRes.Type() == typeid(bool))
            result = (objectRes.Exec<bool>() == RES_SUCCESS ? COAP_204_CHANGED : COAP_405_METHOD_NOT_ALLOWED);
        else if (objectRes.Type() == typeid(float))
            result = (objectRes.Exec<float>() == RES_SUCCESS ? COAP_204_CHANGED : COAP_405_METHOD_NOT_ALLOWED);
        else if (objectRes.Type() == typeid(double))
            result = (objectRes.Exec<double>() == RES_SUCCESS ? COAP_204_CHANGED : COAP_405_METHOD_NOT_ALLOWED);
        else if (objectRes.Type() == typeid(std::string))
            result = (objectRes.Exec<std::string>() == RES_SUCCESS ? COAP_204_CHANGED : COAP_405_METHOD_NOT_ALLOWED);
        else
            result = COAP_405_METHOD_NOT_ALLOWED;
    }

    return result;
}

uint8_t NodeObject::objectCreateStatic(lwm2m_context_t *contextP,
                                       uint16_t instanceId,
                                       int numData,
                                       lwm2m_data_t *dataArray,
                                       lwm2m_object_t *objectP)
{
    // Create new instance of object by copy, writing value coming from server will be written inside instance _objectCreate() method
    NodeObject *instance = static_cast<NodeObject *>(objectP->userData);
    NodeObject *createdInstance = new NodeObject(*instance);
    return createdInstance->_objectCreate(contextP, instanceId, numData, dataArray, objectP);
}

uint8_t NodeObject::objectDeleteStatic(lwm2m_context_t *contextP,
                                       uint16_t instanceId,
                                       lwm2m_object_t *objectP)
{
    // Not allowed to delete original instance
    if (instanceId == 0)
    {
        return COAP_405_METHOD_NOT_ALLOWED;
    }

    // Find corresponding object instance from id
    NodeObject *instance = nullptr;
    for (ObjectList *objectInstance = (ObjectList *)objectP->instanceList; objectInstance != nullptr; objectInstance = (ObjectList *)objectInstance->next)
    {
        if (objectInstance->instanceId == instanceId)
        {
            instance = static_cast<NodeObject *>(objectInstance->objectInstance);
            break;
        }
    }

    // Call delete callback on founded instance, and free memory
    uint8_t result = instance->_objectDelete(contextP, instanceId, objectP);
    if(instance)
        delete instance;
    return result;
}

uint8_t NodeObject::objectDiscoverStatic(lwm2m_context_t *contextP,
                                         uint16_t instanceId,
                                         int *numDataP,
                                         lwm2m_data_t **dataArrayP,
                                         lwm2m_object_t *objectP)
{
    // Find object instance from id
    NodeObject *instance = nullptr;
    for (ObjectList *objectInstance = (ObjectList *)objectP->instanceList; objectInstance != nullptr; objectInstance = (ObjectList *)objectInstance->next)
    {
        if (objectInstance->instanceId == instanceId)
        {
            instance = static_cast<NodeObject *>(objectInstance->objectInstance);
            break;
        }
    }

    // Call discover callback on founded instance
    return instance->_objectDiscover(contextP, instanceId, numDataP, dataArrayP, objectP);
}

uint8_t NodeObject::objectReadStatic(lwm2m_context_t *contextP,
                                     uint16_t instanceId,
                                     int *numDataP,
                                     lwm2m_data_t **dataArrayP,
                                     lwm2m_object_t *objectP)
{
    // Find object instance from id
    NodeObject *instance = nullptr;
    for (ObjectList *objectInstance = (ObjectList *)objectP->instanceList; objectInstance != nullptr; objectInstance = (ObjectList *)objectInstance->next)
    {
        if (objectInstance->instanceId == instanceId)
        {
            instance = static_cast<NodeObject *>(objectInstance->objectInstance);
            break;
        }
    }

    // Call read callback on founded instance
    return instance->_objectRead(contextP, instanceId, numDataP, dataArrayP, objectP);

}

uint8_t NodeObject::objectWriteStatic(lwm2m_context_t *contextP,
                                      uint16_t instanceId,
                                      int numData,
                                      lwm2m_data_t *dataArray,
                                      lwm2m_object_t *objectP,
                                      lwm2m_write_type_t writeType)
{
    // Find object from id
    NodeObject *instance = nullptr;
    for (ObjectList *objectInstance = (ObjectList *)objectP->instanceList; objectInstance != nullptr; objectInstance = (ObjectList *)objectInstance->next)
    {
        if (objectInstance->instanceId == instanceId)
        {
            instance = static_cast<NodeObject *>(objectInstance->objectInstance);
            break;
        }
    }

    // Call write callback on founded instance
    return instance->_objectWrite(contextP, instanceId, numData, dataArray, objectP, writeType);
}

uint8_t NodeObject::objectExecStatic(lwm2m_context_t *contextP,
                                     uint16_t instanceId,
                                     uint16_t resourceId,
                                     uint8_t *buffer,
                                     int length,
                                     lwm2m_object_t *objectP)
{
    // Find object from id
    NodeObject *instance = nullptr;
    for (ObjectList *objectInstance = (ObjectList *)objectP->instanceList; objectInstance != nullptr; objectInstance = (ObjectList *)objectInstance->next)
    {
        if (objectInstance->instanceId == instanceId)
        {
            instance = static_cast<NodeObject *>(objectInstance->objectInstance);
            break;
        }
    }

    // Call execute callback on founded instance
    return instance->_objectExec(contextP, instanceId, resourceId, buffer, length, objectP);
}

lwm2m_object_t *NodeObject::Get()
{
    lwm2m_object_t *objectDescr;

    objectDescr = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));

    if (objectDescr)
    {
        memset(objectDescr, 0, sizeof(lwm2m_object_t));
        objectDescr->objID = _objectId;

        ObjectList *objectInstance = (ObjectList *)lwm2m_malloc(sizeof(ObjectList));
        if (!objectInstance)
        {
            lwm2m_free(objectDescr);
            return nullptr;
        }

        // Store our first object instance node in the list of instance
        objectInstance->instanceId = _instanceId;
        objectInstance->next = nullptr;
        objectInstance->objectInstance = this;
        objectDescr->instanceList = LWM2M_LIST_ADD(objectDescr->instanceList, objectInstance);

        // Bind action callback
        objectDescr->createFunc = &NodeObject::objectCreateStatic;
        objectDescr->deleteFunc = &NodeObject::objectDeleteStatic;
        objectDescr->discoverFunc = &NodeObject::objectDiscoverStatic;
        objectDescr->readFunc = &NodeObject::objectReadStatic;
        objectDescr->writeFunc = &NodeObject::objectWriteStatic;
        objectDescr->executeFunc = &NodeObject::objectExecStatic;
        objectDescr->userData = this;
    }

    return objectDescr;
}

Resource *NodeObject::GetResource(size_t id)
{
    auto resourceIt = _resources.find(id);
    if (resourceIt != _resources.end())
        return (*resourceIt).second;
    else
        return nullptr;
}

NodeObject::~NodeObject()
{
    // Delete every resource dynamically allocated
    for (auto pair : _resources)
    {
        delete pair.second;
    }
}

#endif