/**
 *  @file object_definition.h
 *  @brief This header file contain one function that dynamically create every object and resource and stores 
 *         it in a vector that is returned. This vector is the used when constructing a NodeClient.
 *
 *  @author Bastien Pillonel
 *
 *  @date 6/21/2024
 */

#ifndef OBJECTS_DEFINITION_H
#define OBJECTS_DEFINITION_H

#include "node_client.h"

#define PRV_MANUFACTURER "OMA"          // ... to fit in er_coap_h REST_MAX_CHUNK_SIZE "Open Mobile Alliance"
#define PRV_MODEL_NUMBER "LWM2M Client" // dito: "Lightweight M2M Client"
#define PRV_SERIAL_NUMBER "345000123"
#define PRV_FIRMWARE_VERSION "1.0"
#define PRV_POWER_SOURCE_1 1
#define PRV_POWER_SOURCE_2 5
#define PRV_POWER_VOLTAGE_1 3800
#define PRV_POWER_VOLTAGE_2 5000
#define PRV_POWER_CURRENT_1 125
#define PRV_POWER_CURRENT_2 900
#define PRV_BATTERY_LEVEL 100
#define PRV_MEMORY_FREE 15
#define PRV_ERROR_CODE 0
#define PRV_TIME_ZONE "Europe"
#define PRV_BINDING_MODE "UQ"
#define PRV_OFFSET_MAXLEN 7 //+HH:MM\0 at max
#define PRV_TLV_BUFFER_SIZE 128
#define PRV_SHORT_SERVER_ID 123
#define PRV_LIFETIME 60
#define PRV_STORING false
#define PRV_BINDING "UQ"
#define PRV_SERVER_URI "coap://[2a01:111:f100:9001::1761:93fa]:5684"
#define PRV_SERVER_ID 123
#define PRV_IS_BOOTSTRAP false
#define PRV_CLIENT_HOLDOFF 10

#define SECURITY_OBJECT_ID 0
#define SERVER_OBJECT_ID 1
#define DEVICE_OBJECT_ID 3
#define DEVICE_EXTENSION_OBJECT_ID 3410
#define BATTERY_OBJECT_ID 3411
#define LPWAN_OBJECT_ID 3412
#define DATA_BRIDGE_OBJECT_ID 3414
#define TIME_SYNCHRONISATION_OBJECT_ID 3415
#define OUTDOOR_LAMP_CONTROLLER_OBJECT_ID 3416
#define LUMINAIR_ASSET_OBJECT_ID 3417
#define DIGITAL_INPUT_OBJECT_ID 3200
#define DIGITAL_OUTPUT_OBJECT_ID 3201
#define ANALOG_INPUT_OBJECT_ID 3202
#define SENSOR_OBJECT_ID 3300
#define GENERIC_ACTUATOR_OBJECT_ID 3413
#define ELECTRICAL_MEASUREMENT_OBJECT_ID 3418
#define PHOTOCELL_OBJECT_ID 3419
#define LED_COLOR_OBJECT_ID 3420

std::vector<NodeObject *> *initializeObjects();

#endif