/**
 *  @file node_object.h
 *  @brief This header file contain the declaration of the object class representing a generic object following uCIFI standard
 *
 *  @author Bastien Pillonel
 *
 *  @date 5/2/2024
 */

#ifndef OBJECT_H
#define OBJECT_H

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

template <typename T, typename = std::void_t<>>
struct HasRequiredFields : std::false_type
{
};

template <typename T>
struct HasRequiredFields<T, std::void_t<decltype(std::declval<T>().next), decltype(std::declval<T>().instanceId)>> : std::true_type
{
};

template <typename UserStruct>
class NodeObject
{
private:
    size_t _objectId;
    std::map<size_t, std::unique_ptr<Resource>> _resources;

    struct ObjectList
    {
        struct ObjectList *next;
        uint16_t instanceId;
    };

    UserStruct _userStruct;

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
    NodeObject() : _objectId(0), _userStruct(nullptr), _resources({}) {}

    NodeObject(const NodeObject &) = delete;
    NodeObject &operator=(const NodeObject &) = delete;

    NodeObject(const size_t objectId, const UserStruct &userStruct, const std::vector<std::type_info const *> &types, const std::vector<ResourceOp> &resourcesOp, const std::vector<std::string> &names, const std::vector<Units> &units)
    {

        // Ensure all vector describing resources have same number of resources
        if ((types.size() == resourcesOp.size()) && (resourcesOp.size() == names.size()) && (names.size() == units.size()) && HasRequiredFields<UserStruct>::value)
        {
            _userStruct = userStruct;
            _objectId = objectId;

            // Create resources based on the vectors describing each resource
            Resource *res;
            // Positioning dataExplorer pointer after the first two fields that are userStruct ptr and instanceId
            void *dataExplorer = ((void *)&userStruct + sizeof(UserStruct *) + sizeof(uint16_t));
            for (size_t i = 0; i < names.size(); ++i)
            {
                if (*types[i] == typeid(int))
                {
                    res = new Resource(*((int *)dataExplorer), resourcesOp[i], names[i], units[i]);
                    dataExplorer += sizeof(int);
                }
                else if (*types[i] == typeid(bool))
                {
                    res = new Resource(*((bool *)dataExplorer), resourcesOp[i], names[i], units[i]);
                    dataExplorer += sizeof(bool);
                }
                else if (*types[i] == typeid(float))
                {
                    res = new Resource(*((float *)dataExplorer), resourcesOp[i], names[i], units[i]);
                    dataExplorer += sizeof(float);
                }
                else if (*types[i] == typeid(double))
                {
                    res = new Resource(*((double *)dataExplorer), resourcesOp[i], names[i], units[i]);
                    dataExplorer += sizeof(double);
                }
                else if (*types[i] == typeid(std::string))
                {
                    std::cout << "Constructor string = " << std::endl;
                    res = new Resource(*((std::string *)dataExplorer), resourcesOp[i], names[i], units[i]);
                    dataExplorer += sizeof(std::string);
                }
                else
                {
                    std::cout << "Failed to instanciate resources, types not corresponding" << std::endl;
                    _userStruct = {};
                    _objectId = 0;
                    break;
                }

                _resources[i] = std::make_unique<Resource>(*res);
            }
        }
        else
        {
            std::cout << "Failed to instanciate resources, vector size are not the same" << std::endl;
            _userStruct = {};
            _objectId = 0;
        }
    }

    ~NodeObject()
    {
        for (auto &pair : _resources)
        {
            pair.second.reset();
        }
    }

    lwm2m_object_t *Get();
};

#include "node_object_impl.h"

#endif