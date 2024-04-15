#ifndef OBJECT_DEVICE_H
#define OBJECT_DEVICE_H

#include "liblwm2m.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define PRV_MANUFACTURER "Open Mobile Alliance"
#define PRV_MODEL_NUMBER "Lightweight M2M Client"
#define PRV_BINDING_MODE "U"

// Resource Id's:
#define RES_O_MANUFACTURER 0
#define RES_O_MODEL_NUMBER 1
#define RES_O_SERIAL_NUMBER 2
#define RES_O_FIRMWARE_VERSION 3
#define RES_M_REBOOT 4
#define RES_O_FACTORY_RESET 5
#define RES_O_AVL_POWER_SOURCES 6
#define RES_O_POWER_SOURCE_VOLTAGE 7
#define RES_O_POWER_SOURCE_CURRENT 8
#define RES_O_BATTERY_LEVEL 9
#define RES_O_MEMORY_FREE 10
#define RES_M_ERROR_CODE 11
#define RES_O_RESET_ERROR_CODE 12
#define RES_O_CURRENT_TIME 13
#define RES_O_UTC_OFFSET 14
#define RES_O_TIMEZONE 15
#define RES_M_BINDING_MODES 16
#define RES_O_DEVICE_TYPE 17
#define RES_O_HARDWARE_VERSION 18
#define RES_O_SOFTWARE_VERSION 19
#define RES_O_BATTERY_STATUS 20
#define RES_O_MEMORY_TOTAL 21

class Object_Device
{
public:
    lwm2m_object_t *get_object_device(void);
    void free_object_device(lwm2m_object_t *objectP);

    friend class CallbackSetter;

private:
    static uint8_t prv_set_value(lwm2m_data_t *dataP);
    static uint8_t prv_device_read(lwm2m_context_t *contextP,
                                   uint16_t instanceId,
                                   int *numDataP,
                                   lwm2m_data_t **dataArrayP,
                                   lwm2m_object_t *objectP);
    static uint8_t prv_device_discover(lwm2m_context_t *contextP,
                                       uint16_t instanceId,
                                       int *numDataP,
                                       lwm2m_data_t **dataArrayP,
                                       lwm2m_object_t *objectP);
    static uint8_t prv_device_execute(lwm2m_context_t *contextP,
                                      uint16_t instanceId,
                                      uint16_t resourceId,
                                      uint8_t *buffer,
                                      int length,
                                      lwm2m_object_t *objectP);
};

class CallbackSetter
{
public:
    static void setCallBack(lwm2m_object_t& object){
        object.readFunc = &Object_Device::prv_device_read;
        object.executeFunc = &Object_Device::prv_device_execute;
        object.discoverFunc = &Object_Device::prv_device_discover;
    }
};

#endif