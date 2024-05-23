/**
 *  @file node_object_impl.h
 *  @brief This header file contain the implementation of the object class representing a generic object following uCIFI standard
 *
 *  @author Bastien Pillonel
 *
 *  @date 5/2/2024
 */

#ifndef NODE_OBJECT_IMPL_H
#define NODE_OBJECT_IMPL_H

#include "node_object.h"

template <typename UserStruct>
uint8_t NodeObject<UserStruct>::_objectCreate(lwm2m_context_t *contextP,
                                              uint16_t instanceId,
                                              int numData,
                                              lwm2m_data_t *dataArray,
                                              lwm2m_object_t *objectP)
{
    ObjectList *objectInstance;
    uint8_t result;

    objectInstance = (ObjectList *)lwm2m_malloc(sizeof(ObjectList));
    if (!objectInstance)
        return COAP_500_INTERNAL_SERVER_ERROR;
    memset(objectInstance, 0, sizeof(ObjectList));

    objectInstance->instanceId = instanceId;
    objectP->instanceList = LWM2M_LIST_ADD(objectP->instanceList, objectInstance);

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

template <typename UserStruct>
uint8_t NodeObject<UserStruct>::_objectDelete(lwm2m_context_t *contextP,
                                              uint16_t instanceId,
                                              lwm2m_object_t *objectP)
{
    ObjectList *objectInstance;

    /* unused parameter */
    (void)contextP;

    objectP->instanceList = lwm2m_list_remove(objectP->instanceList, instanceId, (lwm2m_list_t **)&objectInstance);
    if (!objectInstance)
        return COAP_404_NOT_FOUND;

    lwm2m_free(objectInstance);

    return COAP_202_DELETED;
}

template <typename UserStruct>
uint8_t NodeObject<UserStruct>::_objectDiscover(lwm2m_context_t *contextP,
                                                uint16_t instanceId,
                                                int *numDataP,
                                                lwm2m_data_t **dataArrayP,
                                                lwm2m_object_t *objectP)
{
    uint8_t result;
    int i;

    /* Unused parameter */
    (void)contextP;

    result = COAP_205_CONTENT;

    // is the server asking for full object
    if (*numDataP == 0)
    {
        size_t nbRes = _resources.size();
        *dataArrayP = lwm2m_data_new(nbRes);
        if (!(*dataArrayP))
            return COAP_500_INTERNAL_SERVER_ERROR;

        *numDataP = nbRes;

        for (const auto &pair : _resources)
            (*dataArrayP)[i].id = static_cast<uint16_t>(pair.first);
    }
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

template <typename UserStruct>
uint8_t NodeObject<UserStruct>::_objectRead(lwm2m_context_t *contextP,
                                            uint16_t instanceId,
                                            int *numDataP,
                                            lwm2m_data_t **dataArrayP,
                                            lwm2m_object_t *objectP)
{
    ObjectList *objectInstance;
    uint8_t result;
    int i;

    /* Unused parameter */
    (void)contextP;

    objectInstance = (ObjectList *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (!objectInstance)
        return COAP_404_NOT_FOUND;

    // is the server asking for the full instance ?
    if (*numDataP == 0)
    {
        size_t nbRes = _resources.size();
        *dataArrayP = lwm2m_data_new(nbRes);
        if (!(*dataArrayP))
            return COAP_500_INTERNAL_SERVER_ERROR;

        *numDataP = nbRes;

        for (const auto &pair : _resources)
            (*dataArrayP)[i].id = static_cast<uint16_t>(pair.first);
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
            auto resourceIt = _resources.find((*dataArrayP)[i].id);
            if (resourceIt == _resources.end())
            {
                result = COAP_404_NOT_FOUND;
            }
            else
            {
                Resource objectRes(*((*resourceIt).second));
                result = COAP_205_CONTENT;

                if (objectRes.Type() == typeid(int))
                    lwm2m_data_encode_int(*(objectRes.Read<int>()), (*dataArrayP) + i);
                else if (objectRes.Type() == typeid(bool))
                    lwm2m_data_encode_bool(*(objectRes.Read<bool>()), (*dataArrayP) + i);
                else if (objectRes.Type() == typeid(float))
                    lwm2m_data_encode_float(*(objectRes.Read<float>()), (*dataArrayP) + i);
                else if (objectRes.Type() == typeid(double))
                    lwm2m_data_encode_float(*(objectRes.Read<double>()), (*dataArrayP) + i);
                else if (objectRes.Type() == typeid(std::string)){
                    std::cout << "String = " << objectRes.Read<std::string>() << std::endl;
                    lwm2m_data_encode_string((*(objectRes.Read<std::string>())).c_str(), (*dataArrayP) + i);
                }
                else
                    result = COAP_404_NOT_FOUND;
            }
        }
        ++i;
    } while (i < *numDataP && result == COAP_205_CONTENT);

    return result;
}

template <typename UserStruct>
uint8_t NodeObject<UserStruct>::_objectWrite(lwm2m_context_t *contextP,
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
        /* No multiple instance resources */
        if (dataArray[i].type == LWM2M_TYPE_MULTIPLE_RESOURCE)
        {
            result = COAP_404_NOT_FOUND;
        }
        else
        {
            auto resourceIt = _resources.find(dataArray[i].id);
            if (resourceIt == _resources.end())
            {
                result = COAP_404_NOT_FOUND;
            }
            else
            {
                Resource objectRes(*((*resourceIt).second));

                if (objectRes.Type() == typeid(int))
                {
                    int64_t valI;
                    lwm2m_data_decode_int(dataArray + i, &valI);
                    result = (objectRes.Write<int>((int)valI) == RES_SUCCESS ? COAP_204_CHANGED : COAP_404_NOT_FOUND);
                }
                else if (objectRes.Type() == typeid(bool))
                {
                    bool valB;
                    lwm2m_data_decode_bool(dataArray + i, &valB);
                    result = (objectRes.Write<bool>(valB) == RES_SUCCESS ? COAP_204_CHANGED : COAP_404_NOT_FOUND);
                }
                else if (objectRes.Type() == typeid(float))
                {
                    double valF;
                    lwm2m_data_decode_float(dataArray + i, &valF);
                    result = (objectRes.Write<float>((float)valF) == RES_SUCCESS ? COAP_204_CHANGED : COAP_404_NOT_FOUND);
                }
                else if (objectRes.Type() == typeid(double))
                {
                    double valD;
                    lwm2m_data_decode_float(dataArray + i, &valD);
                    result = (objectRes.Write<double>(valD) == RES_SUCCESS ? COAP_204_CHANGED : COAP_404_NOT_FOUND);
                }
                else if (objectRes.Type() == typeid(std::string))
                {
                    char *valS;
                    strncpy(valS, (char *)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);
                    result = (objectRes.Write<std::string>(std::string(valS)) == RES_SUCCESS ? COAP_204_CHANGED : COAP_404_NOT_FOUND);
                }
                else
                    result = COAP_404_NOT_FOUND;
            }
        }
        ++i;
    } while (i < numData && result == COAP_204_CHANGED);

    return result;
}

template <typename UserStruct>
uint8_t NodeObject<UserStruct>::_objectExec(lwm2m_context_t *contextP,
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
        return COAP_404_NOT_FOUND;

    auto resourceIt = _resources.find(static_cast<size_t>(resourceId));
    if (resourceIt == _resources.end())
    {
        result = COAP_404_NOT_FOUND;
    }
    else
    {
        Resource objectRes(*((*resourceIt).second));

        if (objectRes.Type() == typeid(int))
            result = (objectRes.Exec<int>() == RES_SUCCESS ? COAP_205_CONTENT : COAP_405_METHOD_NOT_ALLOWED);
        else if (objectRes.Type() == typeid(bool))
            result = (objectRes.Exec<bool>() == RES_SUCCESS ? COAP_205_CONTENT : COAP_405_METHOD_NOT_ALLOWED);
        else if (objectRes.Type() == typeid(float))
            result = (objectRes.Exec<float>() == RES_SUCCESS ? COAP_205_CONTENT : COAP_405_METHOD_NOT_ALLOWED);
        else if (objectRes.Type() == typeid(double))
            result = (objectRes.Exec<double>() == RES_SUCCESS ? COAP_205_CONTENT : COAP_405_METHOD_NOT_ALLOWED);
        else if (objectRes.Type() == typeid(std::string))
            result = (objectRes.Exec<std::string>() == RES_SUCCESS ? COAP_205_CONTENT : COAP_405_METHOD_NOT_ALLOWED);
        else
            result = COAP_405_METHOD_NOT_ALLOWED;
    }

    return result;
}

template <typename UserStruct>
uint8_t NodeObject<UserStruct>::objectCreateStatic(lwm2m_context_t *contextP,
                                                   uint16_t instanceId,
                                                   int numData,
                                                   lwm2m_data_t *dataArray,
                                                   lwm2m_object_t *objectP)
{
    NodeObject<UserStruct> *instance = static_cast<NodeObject<UserStruct> *>(objectP->userData);
    return instance->_objectCreate(contextP, instanceId, numData, dataArray, objectP);
}

template <typename UserStruct>
uint8_t NodeObject<UserStruct>::objectDeleteStatic(lwm2m_context_t *contextP,
                                                   uint16_t instanceId,
                                                   lwm2m_object_t *objectP)
{
    NodeObject<UserStruct> *instance = static_cast<NodeObject<UserStruct> *>(objectP->userData);
    return instance->_objectDelete(contextP, instanceId, objectP);
}

template <typename UserStruct>
uint8_t NodeObject<UserStruct>::objectDiscoverStatic(lwm2m_context_t *contextP,
                                                     uint16_t instanceId,
                                                     int *numDataP,
                                                     lwm2m_data_t **dataArrayP,
                                                     lwm2m_object_t *objectP)
{
    NodeObject<UserStruct> *instance = static_cast<NodeObject<UserStruct> *>(objectP->userData);
    return instance->_objectDiscover(contextP, instanceId, numDataP, dataArrayP, objectP);
}

template <typename UserStruct>
uint8_t NodeObject<UserStruct>::objectReadStatic(lwm2m_context_t *contextP,
                                                 uint16_t instanceId,
                                                 int *numDataP,
                                                 lwm2m_data_t **dataArrayP,
                                                 lwm2m_object_t *objectP)
{
    NodeObject<UserStruct> *instance = static_cast<NodeObject<UserStruct> *>(objectP->userData);
    return instance->_objectRead(contextP, instanceId, numDataP, dataArrayP, objectP);
}

template <typename UserStruct>
uint8_t NodeObject<UserStruct>::objectWriteStatic(lwm2m_context_t *contextP,
                                                  uint16_t instanceId,
                                                  int numData,
                                                  lwm2m_data_t *dataArray,
                                                  lwm2m_object_t *objectP,
                                                  lwm2m_write_type_t writeType)
{
    NodeObject<UserStruct> *instance = static_cast<NodeObject<UserStruct> *>(objectP->userData);
    return instance->_objectWrite(contextP, instanceId, numData, dataArray, objectP, writeType);
}

template <typename UserStruct>
uint8_t NodeObject<UserStruct>::objectExecStatic(lwm2m_context_t *contextP,
                                                 uint16_t instanceId,
                                                 uint16_t resourceId,
                                                 uint8_t *buffer,
                                                 int length,
                                                 lwm2m_object_t *objectP)
{
    NodeObject<UserStruct> *instance = static_cast<NodeObject<UserStruct> *>(objectP->userData);
    return instance->_objectExec(contextP, instanceId, resourceId, buffer, length, objectP);
}

template <typename UserStruct>
lwm2m_object_t *NodeObject<UserStruct>::Get()
{
    lwm2m_object_t *objectDescr;

    objectDescr = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));

    if (objectDescr)
    {
        memset(objectDescr, 0, sizeof(lwm2m_object_t));
        objectDescr->objID = _objectId;

        printf("InstanceID = %u\r\n", _userStruct.instanceId);

        objectDescr->instanceList = LWM2M_LIST_ADD(objectDescr->instanceList, &_userStruct);
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

#endif