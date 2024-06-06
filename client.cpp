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
#if !MBED_TEST_MODE

#include "mbed.h"
#include "node_client.h"
#include "vector"

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
#define PRV_BINDING_MODE "U"
#define PRV_OFFSET_MAXLEN 7 //+HH:MM\0 at max
#define PRV_TLV_BUFFER_SIZE 128
#define PRV_SHORT_SERVER_ID 123
#define PRV_LIFETIME 300
#define PRV_STORING false
#define PRV_BINDING "U"
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

void print_memory_info() {
    // allocate enough room for every thread's stack statistics
    int cnt = osThreadGetCount();
    mbed_stats_stack_t *stats = (mbed_stats_stack_t*) malloc(cnt * sizeof(mbed_stats_stack_t));

    cnt = mbed_stats_stack_get_each(stats, cnt);
    for (int i = 0; i < cnt; i++) {
        printf("Thread: 0x%lX, Stack size: %lu / %lu\r\n", stats[i].thread_id, stats[i].max_size, stats[i].reserved_size);
    }
    free(stats);

    // Grab the heap statistics
    mbed_stats_heap_t heap_stats;
    mbed_stats_heap_get(&heap_stats);
    printf("Heap size: %lu / %lu bytes\r\n", heap_stats.current_size, heap_stats.reserved_size);
}

int main(int argc, char *argv[])
{
    //print_memory_info();
    //mbed_mem_trace_set_callback(&mbed_mem_trace_default_callback);
    ThisThread::sleep_for(1s);
    // ================================== OBJECT SECURITY ==============================
    // TODO Issue with creating an NodeObjcet representing security object
    // Client won't start normaly with the NodeObject security but start with old version defined in object_security.c
#ifdef USE_DTLS
    int secuMode = LWM2M_SECURITY_MODE_PRE_SHARED_KEY;
    char *publicIdentity = CLIENT_IDENTITY;
    char *psk = CLIENT_KEYSTR;
    uint16_t pskLen = 0;
    char *pskBuffer = NULL;

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
#else
    int secuMode = LWM2M_SECURITY_MODE_NONE;
    char *publicIdentity = "";
#endif
    printf("Psk buffer is %s\n", pskBuffer);

    Resource *serverUri = new Resource(std::string(PRV_SERVER_URI), ResourceOp::RES_RDWR, "Server URI", Units::NA, 0);
    Resource *bootstrapServer = new Resource(PRV_IS_BOOTSTRAP, ResourceOp::RES_RDWR, "Bootstrap Server", Units::NA, 1);
    Resource *securityMode = new Resource(secuMode, ResourceOp::RES_RDWR, "Security Mode", Units::NA, 2);
    Resource *publicKeyOrId = new Resource(std::string(publicIdentity), ResourceOp::RES_RDWR, "Public Key or ID", Units::NA, 3);
    Resource *serverPublicKeyOrId = new Resource(std::string(""), ResourceOp::RES_RDWR, "Server Public Key or ID", Units::NA, 4);
    Resource *secretKey = new Resource(std::string(pskBuffer), ResourceOp::RES_RDWR, "Secret Key", Units::NA, 5);
    Resource *smsSecurityMode = new Resource(0, ResourceOp::RES_RDWR, "SMS Security Mode", Units::NA, 6);
    Resource *smsBindingKeyParam = new Resource(std::string(""), ResourceOp::RES_RDWR, "SMS Binding Key Param.", Units::NA, 7);
    Resource *smsBindingSecretKeys = new Resource(std::string(""), ResourceOp::RES_RDWR, "SMS Binding Secret Keys", Units::NA, 8);
    Resource *serverSmsNumber = new Resource(std::string(""), ResourceOp::RES_RDWR, "Server SMS Number", Units::NA, 9);
    Resource *securityShortServerId = new Resource(PRV_SHORT_SERVER_ID, ResourceOp::RES_RD, "Short server id", Units::NA, 10);
    Resource *clientHoldOffTime = new Resource(PRV_CLIENT_HOLDOFF, ResourceOp::RES_RDWR, "Client Hold Off Time", Units::SECONDS, 11);
    Resource *bsAccountTimeout = new Resource(0, ResourceOp::RES_RDWR, "BS Account Timeout", Units::SECONDS, 12);

    std::vector<Resource *> securityResources = {serverUri, bootstrapServer, securityMode, publicKeyOrId, serverPublicKeyOrId, secretKey, smsSecurityMode, smsBindingKeyParam, smsBindingSecretKeys, serverSmsNumber, securityShortServerId, clientHoldOffTime, bsAccountTimeout};
    NodeObject *securityObject = new NodeObject(SECURITY_OBJECT_ID, 0, securityResources);

    // ================================== OBJECT SERVER ==============================
    Resource *shortServerId = new Resource(PRV_SHORT_SERVER_ID, ResourceOp::RES_RD, "Short server id", Units::NA, 0);
    Resource *lifetime = new Resource(PRV_LIFETIME, ResourceOp::RES_RDWR, "Lifetime", Units::SECONDS, 1);
    Resource *defaultMinimumPeriod = new Resource(0, ResourceOp::RES_RDWR, "Default minimum period", Units::SECONDS, 2);
    Resource *defaultMaximumPeriod = new Resource(0, ResourceOp::RES_RDWR, "Default maximum period", Units::SECONDS, 3);
    Resource *disable = new Resource(0, ResourceOp::RES_E, "Disable", Units::NA, 4);
    Resource *disableTimeout = new Resource(0, ResourceOp::RES_RDWR, "Disable timeout", Units::SECONDS, 5);
    Resource *notificationStoringWhenDisabledOrOffline = new Resource(PRV_STORING, ResourceOp::RES_RDWR, "Notification storing when disabled or offline", Units::NA, 6);
    Resource *binding = new Resource(string(PRV_BINDING), ResourceOp::RES_RDWR, "Binding", Units::NA, 7);
    Resource *registrationUpdateTrigger = new Resource(0, ResourceOp::RES_E, "Registration update trigger", Units::NA, 8);

    std::vector<Resource *> serverResources = {shortServerId, lifetime, defaultMinimumPeriod, defaultMaximumPeriod, disable, disableTimeout, notificationStoringWhenDisabledOrOffline, binding, registrationUpdateTrigger};
    NodeObject *serverObject = new NodeObject(SERVER_OBJECT_ID, 0, serverResources);

    // ================================== OBJECT DEVICE ==============================
    Resource *manufacturer = new Resource(std::string(PRV_MANUFACTURER), ResourceOp::RES_RD, PRV_MANUFACTURER, Units::NA, 0);
    Resource *modelNumber = new Resource(std::string(PRV_MODEL_NUMBER), ResourceOp::RES_RD, PRV_MODEL_NUMBER, Units::NA, 1);
    Resource *serialNumber = new Resource(std::string(PRV_SERIAL_NUMBER), ResourceOp::RES_RD, PRV_SERIAL_NUMBER, Units::NA, 2);
    Resource *firmwareVersion = new Resource(std::string(PRV_FIRMWARE_VERSION), ResourceOp::RES_RD, PRV_FIRMWARE_VERSION, Units::NA, 3);
    Resource *reboot = new Resource(0, ResourceOp::RES_E, "Reboot", Units::NA, 4);
    Resource *factoryReset = new Resource(0, ResourceOp::RES_E, "Factory reset", Units::NA, 5);
    Resource *availablePowerSource = new Resource(0, ResourceOp::RES_RD, "Available power source", Units::NA, 6);
    Resource *powerSourceVoltage = new Resource(0, ResourceOp::RES_RD, "Power source voltage", Units::VOLT, 7);
    Resource *powerSourceCurrent = new Resource(0, ResourceOp::RES_RD, "Power source current", Units::AMPER, 8);
    Resource *serverBatteryLevel = new Resource(PRV_BATTERY_LEVEL, ResourceOp::RES_RD, "Battery level", Units::PERCENT, 9);
    Resource *memoryFree = new Resource(PRV_MEMORY_FREE, ResourceOp::RES_RD, "Free memory", Units::BYTES, 10);
    Resource *errorCode = new Resource(PRV_ERROR_CODE, ResourceOp::RES_RD, "Error code", Units::NA, 11);
    Resource *resetErrorCode = new Resource(0, ResourceOp::RES_E, "Reset error code", Units::NA, 12);
    Resource *currentTime = new Resource(0, ResourceOp::RES_RDWR, "Current time", Units::NA, 13);
    Resource *utcOffset = new Resource(string("UTC+X"), ResourceOp::RES_RDWR, "UTC offset", Units::NA, 14);
    Resource *timezone = new Resource(std::string(PRV_TIME_ZONE), ResourceOp::RES_RDWR, "Timezone", Units::NA, 15);
    Resource *supportedBindingMode = new Resource(std::string(PRV_BINDING_MODE), ResourceOp::RES_RD, "Supported binding and mode", Units::NA, 16);
    Resource *deviceType = new Resource(string("light node"), ResourceOp::RES_RD, "Device type", Units::NA, 17);
    Resource *hardwareVersion = new Resource(string("1.0"), ResourceOp::RES_RD, "Hardware version", Units::NA, 18);
    Resource *softwareVersion = new Resource(string("1.0"), ResourceOp::RES_RD, "Software version", Units::NA, 19);
    Resource *batteryStatus = new Resource(0, ResourceOp::RES_RD, "Battery status", Units::NA, 20);
    Resource *memoryTotal = new Resource(1000, ResourceOp::RES_RD, "Memory total", Units::NA, 21);
    Resource *extdevinfo = new Resource(0, ResourceOp::RES_RD, "External device info", Units::NA, 22);

    manufacturer->BindOnRead<std::string>([](string str)
                                          { std::cout << "Manufacturer read get : " << str << std::endl; });

    std::vector<Resource *> deviceResources = {manufacturer, modelNumber, serialNumber, firmwareVersion, reboot, factoryReset, availablePowerSource, powerSourceVoltage, powerSourceCurrent, serverBatteryLevel, memoryFree, errorCode, resetErrorCode, currentTime, utcOffset, timezone, supportedBindingMode, deviceType, hardwareVersion, softwareVersion, batteryStatus, memoryTotal, extdevinfo};
    NodeObject *deviceObject = new NodeObject(DEVICE_OBJECT_ID, 0, deviceResources);

    // ================================== OBJECT DEVICE EXTENSION ==============================
    Resource *gtinModelNumber = new Resource(std::string(""), ResourceOp::RES_RD, "GTIN model number", Units::NA, 1);
    Resource *manufacturerIdentifier = new Resource(std::string(""), ResourceOp::RES_RD, "Manufacturer identifier", Units::NA, 2);
    Resource *userGivenName = new Resource(std::string(""), ResourceOp::RES_RDWR, "User-given name", Units::NA, 3);
    Resource *assetIdentifier = new Resource(std::string(""), ResourceOp::RES_RDWR, "Asset identifier", Units::NA, 4);
    Resource *installationDate = new Resource(std::string(""), ResourceOp::RES_RDWR, "Installation date", Units::DATE, 5);
    Resource *softwareUpdate = new Resource(false, ResourceOp::RES_RD, "Software update", Units::NA, 6);
    Resource *maintenance = new Resource(false, ResourceOp::RES_RDWR, "Maintenance", Units::NA, 7);
    Resource *configurationReset = new Resource(0, ResourceOp::RES_E, "Configuration reset", Units::NA, 8);
    Resource *deviceOperatingHours = new Resource(0, ResourceOp::RES_RD, "Device operating hours", Units::HOURS, 9);
    Resource *additionalFirmwareInfo = new Resource(std::string(""), ResourceOp::RES_RD, "Additional firmware information", Units::NA, 10);

    std::vector<Resource *> deviceExtensionResources = {gtinModelNumber, manufacturerIdentifier, userGivenName, assetIdentifier, installationDate, softwareUpdate, maintenance, configurationReset, deviceOperatingHours, additionalFirmwareInfo};
    NodeObject *deviceExtensionObject = new NodeObject(DEVICE_EXTENSION_OBJECT_ID, 0, deviceExtensionResources);

    // ================================== OBJECT BATTERY ==============================
    Resource *batteryLevel = new Resource(PRV_BATTERY_LEVEL, ResourceOp::RES_RD, "Battery level", Units::PERCENT, 1);
    Resource *batteryCapacity = new Resource(0.0f, ResourceOp::RES_RD, "Battery capacity", Units::AMPER_HOUR, 2);
    Resource *batteryVoltage = new Resource(0.0f, ResourceOp::RES_RD, "Battery voltage", Units::VOLT, 3);
    Resource *typeOfBattery = new Resource(std::string(""), ResourceOp::RES_RDWR, "Type of battery", Units::NA, 4);
    Resource *lowBatteryThreshold = new Resource(0, ResourceOp::RES_RDWR, "Low battery threshold", Units::PERCENT, 5);
    Resource *batteryLevelTooLow = new Resource(false, ResourceOp::RES_RD, "Battery level too low", Units::NA, 6);
    Resource *batteryShutdown = new Resource(false, ResourceOp::RES_RDWR, "Battery shutdown", Units::NA, 7);
    Resource *numberOfCycles = new Resource(0, ResourceOp::RES_RD, "Number of cycles", Units::NA, 8);
    Resource *supplyLoss = new Resource(false, ResourceOp::RES_RD, "Supply loss", Units::NA, 9);
    Resource *supplyLossCounter = new Resource(0, ResourceOp::RES_RD, "Supply loss counter", Units::NA, 10);
    Resource *supplyLossCounterReset = new Resource(0, ResourceOp::RES_E, "Supply loss counter reset", Units::NA, 11);
    Resource *supplyLossReason = new Resource(std::string(""), ResourceOp::RES_RD, "Supply loss reason", Units::NA, 12);

    std::vector<Resource *> batteryResources = {batteryLevel, batteryCapacity, batteryVoltage, typeOfBattery, lowBatteryThreshold, batteryLevelTooLow, batteryShutdown, numberOfCycles, supplyLoss, supplyLossCounter, supplyLossCounterReset, supplyLossReason};
    NodeObject *batteryObject = new NodeObject(BATTERY_OBJECT_ID, 0, batteryResources);

    // ================================== OBJECT LPWAN ==============================
    Resource *typeOfNetwork = new Resource(std::string(""), ResourceOp::RES_RD, "Type of network", Units::NA, 1);
    Resource *ipv4Address = new Resource(std::string(""), ResourceOp::RES_RDWR, "IPv4 address", Units::NA, 2);
    Resource *ipv6Address = new Resource(std::string(""), ResourceOp::RES_RDWR, "IPv6 address", Units::NA, 3);
    Resource *networkAddress = new Resource(std::string(""), ResourceOp::RES_RDWR, "Network address", Units::NA, 4);
    Resource *secondaryNetworkAddress = new Resource(std::string(""), ResourceOp::RES_RDWR, "Secondary network address", Units::NA, 5);
    Resource *macAddress = new Resource(std::string(""), ResourceOp::RES_RDWR, "MAC address", Units::NA, 6);
    Resource *peerAddress = new Resource(std::string(""), ResourceOp::RES_RD, "Peer address", Units::NA, 7);
    Resource *multicastGroupAddress = new Resource(std::string(""), ResourceOp::RES_RDWR, "Multicast group address", Units::NA, 8);
    Resource *multicastGroupKey = new Resource(std::string(""), ResourceOp::RES_RDWR, "Multicast group key", Units::NA, 9);
    Resource *dataRate = new Resource(0, ResourceOp::RES_RDWR, "Data rate", Units::NA, 10);
    Resource *transmitPower = new Resource(0.0f, ResourceOp::RES_RD, "Transmit power", Units::DECIBEL_MILLIWATT, 11);
    Resource *frequency = new Resource(0.0f, ResourceOp::RES_RDWR, "Frequency", Units::HERTZ, 12);
    Resource *sessionTime = new Resource(std::string(""), ResourceOp::RES_RDWR, "Session time", Units::NA, 13);
    Resource *sessionDuration = new Resource(0, ResourceOp::RES_RD, "Session duration", Units::SECONDS, 14);
    Resource *meshNode = new Resource(false, ResourceOp::RES_RDWR, "Mesh node", Units::NA, 15);
    Resource *maximumRepeatTime = new Resource(0, ResourceOp::RES_RDWR, "Maximum repeat time", Units::NA, 16);
    Resource *numberOfRepeats = new Resource(0, ResourceOp::RES_RD, "Number of repeats", Units::NA, 17);
    Resource *signalToNoiseRatio = new Resource(0.0f, ResourceOp::RES_RD, "Signal to noise ratio", Units::NA, 18);
    Resource *communicationFailure = new Resource(false, ResourceOp::RES_RD, "Communication failure", Units::NA, 19);
    Resource *receivedSignalStrengthIndication = new Resource(0.0f, ResourceOp::RES_RD, "Received Signal Strength Indication", Units::DECIBEL_MILLIWATT, 20);
    Resource *imsi = new Resource(std::string(""), ResourceOp::RES_RD, "IMSI", Units::NA, 21);
    Resource *imei = new Resource(std::string(""), ResourceOp::RES_RD, "IMEI", Units::NA, 22);
    Resource *currentCommunicationOperator = new Resource(std::string(""), ResourceOp::RES_RD, "Current Communication Operator", Units::NA, 23);
    Resource *integratedCircuitCardIdentifier = new Resource(std::string(""), ResourceOp::RES_RD, "Integrated Circuit Card Identifier", Units::NA, 24);

    std::vector<Resource *> lpwanResources = {typeOfNetwork, ipv4Address, ipv6Address, networkAddress, secondaryNetworkAddress, macAddress, peerAddress, multicastGroupAddress, multicastGroupKey, dataRate, transmitPower, frequency, sessionTime, sessionDuration, meshNode, maximumRepeatTime, numberOfRepeats, signalToNoiseRatio, communicationFailure, receivedSignalStrengthIndication, imsi, imei, currentCommunicationOperator, integratedCircuitCardIdentifier};
    NodeObject *lpwanObject = new NodeObject(LPWAN_OBJECT_ID, 0, lpwanResources);

    // ================================== OBJECT DATA BRIDGE ==============================
    Resource *payload = new Resource(std::string(""), ResourceOp::RES_RDWR, "Payload", Units::NA, 1);
    Resource *hash = new Resource(std::string(""), ResourceOp::RES_RDWR, "Hash", Units::NA, 2);
    Resource *cumulatedDailyDataVolumeUp = new Resource(0, ResourceOp::RES_RD, "Cumulated daily data volume up", Units::BYTES, 3);
    Resource *cumulatedDailyDataVolumeDown = new Resource(0, ResourceOp::RES_RD, "Cumulated daily data volume down", Units::BYTES, 4);
    Resource *cumulatedDailyDataVolumeTotal = new Resource(0, ResourceOp::RES_RD, "Cumulated daily data volume total", Units::BYTES, 5);
    Resource *communicationError = new Resource(false, ResourceOp::RES_RD, "Communication error", Units::NA, 6);

    std::vector<Resource *> dataBridgeResources = {payload, hash, cumulatedDailyDataVolumeUp, cumulatedDailyDataVolumeDown, cumulatedDailyDataVolumeTotal, communicationError};
    NodeObject *dataBridgeObject = new NodeObject(DATA_BRIDGE_OBJECT_ID, 0 , dataBridgeResources);

    // ================================== OBJECT TIME SYNCHONISATION ==============================
    Resource *ntpServerAddress = new Resource(std::string(""), ResourceOp::RES_RDWR, "NTP server address", Units::NA, 1);
    Resource *backupNtpServerAddress = new Resource(std::string(""), ResourceOp::RES_RDWR, "Backup NTP server address", Units::NA, 2);
    Resource *ntpPeriod = new Resource(std::string(""), ResourceOp::RES_RDWR, "NTP period", Units::NA, 3);
    Resource *lastTimeSync = new Resource(0, ResourceOp::RES_RD, "Last time sync", Units::HOURS, 4);
    Resource *timeSyncError = new Resource(false, ResourceOp::RES_RD, "Time sync error", Units::NA, 5);

    std::vector<Resource *> timeSynchronisationResources = {ntpServerAddress, backupNtpServerAddress, ntpPeriod, lastTimeSync, timeSyncError};
    NodeObject *timeSynchronisationObject = new NodeObject(TIME_SYNCHRONISATION_OBJECT_ID, 0, timeSynchronisationResources);

    // ================================== OBJECT OUTDOOR LAMP CONTROLER ==============================
    Resource *command = new Resource(0, ResourceOp::RES_RDWR, "Command", Units::PERCENT, 1);
    Resource *commandInAction = new Resource(0, ResourceOp::RES_RD, "Command in action", Units::PERCENT, 2);
    Resource *dimmingLevel = new Resource(0, ResourceOp::RES_RD, "Dimming level", Units::PERCENT, 3);
    Resource *defaultDimmingLevel = new Resource(0, ResourceOp::RES_RDWR, "Default dimming level", Units::PERCENT, 4);
    Resource *lampFailure = new Resource(false, ResourceOp::RES_RD, "Lamp failure", Units::NA, 5);
    Resource *lampFailureReason = new Resource(0, ResourceOp::RES_RD, "Lamp failure reason", Units::NA, 6);
    Resource *controlGearFailure = new Resource(false, ResourceOp::RES_RD, "Control gear failure", Units::NA, 7);
    Resource *controlGearFailureReason = new Resource(0, ResourceOp::RES_RD, "Control gear failure reason", Units::NA, 8);
    Resource *relayFailure = new Resource(false, ResourceOp::RES_RD, "Relay failure", Units::NA, 9);
    Resource *dayBurner = new Resource(false, ResourceOp::RES_RD, "Day burner", Units::NA, 10);
    Resource *cyclingFailure = new Resource(false, ResourceOp::RES_RD, "Cycling failure", Units::NA, 11);
    Resource *controlGearCommunicationFailure = new Resource(false, ResourceOp::RES_RD, "Control gear communication failure", Units::NA, 12);
    Resource *schedulerID = new Resource(0, ResourceOp::RES_RDWR, "Scheduler ID", Units::NA, 13);
    Resource *invalidScheduler = new Resource(false, ResourceOp::RES_RD, "Invalid scheduler", Units::NA, 14);
    Resource *lampOperatingHours = new Resource(0, ResourceOp::RES_RD, "Lamp operating hours", Units::HOURS, 15);
    Resource *lampOperatingHoursReset = new Resource(0, ResourceOp::RES_E, "Lamp operating hours reset", Units::NA, 16);
    Resource *lampOnTimestamp = new Resource(0, ResourceOp::RES_RD, "Lamp ON timestamp", Units::DATE, 17);
    Resource *lampSwitchCounter = new Resource(0, ResourceOp::RES_RD, "Lamp switch counter", Units::NA, 18);
    Resource *lampSwitchCounterReset = new Resource(0, ResourceOp::RES_E, "Lamp switch counter reset", Units::NA, 19);
    Resource *controlGearStartCounter = new Resource(0, ResourceOp::RES_RD, "Control gear start counter", Units::NA, 20);
    Resource *controlGearTemperature = new Resource(0.0f, ResourceOp::RES_RD, "Control gear temperature", Units::CELCIUS_DEGREES, 21);
    Resource *controlGearThermalDerating = new Resource(false, ResourceOp::RES_RD, "Control gear thermal derating", Units::NA, 22);
    Resource *controlGearThermalDeratingCounter = new Resource(0, ResourceOp::RES_RD, "Control gear thermal derating counter", Units::NA, 23);
    Resource *controlGearThermalDeratingCounterReset = new Resource(0, ResourceOp::RES_E, "Control gear thermal derating counter reset", Units::NA, 24);
    Resource *controlGearThermalShutdown = new Resource(false, ResourceOp::RES_RD, "Control gear thermal shutdown", Units::NA, 25);
    Resource *controlGearThermalShutdownCounter = new Resource(0, ResourceOp::RES_RD, "Control gear thermal shutdown counter", Units::NA, 26);
    Resource *controlGearThermalShutdownCounterReset = new Resource(0, ResourceOp::RES_E, "Control gear thermal shutdown counter reset", Units::NA, 27);
    Resource *outputPort = new Resource(0, ResourceOp::RES_RDWR, "Output port", Units::NA, 28);
    Resource *standbyMode = new Resource(false, ResourceOp::RES_RDWR, "Standby mode", Units::NA, 29);
    Resource *constantLightOutput = new Resource(false, ResourceOp::RES_RDWR, "Constant light output", Units::NA, 30);
    Resource *cleaningFactorEnabled = new Resource(false, ResourceOp::RES_RDWR, "Cleaning factor enabled", Units::NA, 31);
    Resource *cleaningPeriod = new Resource(0, ResourceOp::RES_RDWR, "Cleaning period", Units::NA, 32);
    Resource *initialLampCleaningFactor = new Resource(0, ResourceOp::RES_RDWR, "Initial lamp cleaning factor", Units::PERCENT, 33);
    Resource *lampCleaningDate = new Resource(0, ResourceOp::RES_RDWR, "Lamp cleaning date", Units::DATE, 34);
    Resource *controlType = new Resource(0, ResourceOp::RES_RDWR, "Control type", Units::NA, 35);
    Resource *nominalLampWattage = new Resource(0, ResourceOp::RES_RDWR, "Nominal lamp wattage", Units::WATT, 36);
    Resource *minimumDimmingLevel = new Resource(0, ResourceOp::RES_RDWR, "Minimum dimming level", Units::PERCENT, 37);
    Resource *minimumLampWattage = new Resource(0, ResourceOp::RES_RDWR, "Minimum lamp wattage", Units::WATT, 38);
    Resource *lightColorTemperatureCommand = new Resource(std::string(""), ResourceOp::RES_RDWR, "Light color temperature command", Units::KELVIN, 39);
    Resource *actualLightColorTemperature = new Resource(std::string(""), ResourceOp::RES_RD, "Actual light color temperature", Units::KELVIN, 40);
    Resource *virtualPowerOutput = new Resource(0, ResourceOp::RES_RDWR, "Virtual power output", Units::PERCENT, 41);
    Resource *voltageAtMaxDimLevel = new Resource(0.0f, ResourceOp::RES_RDWR, "Voltage at max dim level", Units::VOLT, 42);
    Resource *voltageAtMinDimLevel = new Resource(0.0f, ResourceOp::RES_RDWR, "Voltage at min dim level", Units::VOLT, 43);
    Resource *lightSourceVoltage = new Resource(0.0f, ResourceOp::RES_RD, "Light source voltage", Units::VOLT, 44);
    Resource *lightSourceCurrent = new Resource(0.0f, ResourceOp::RES_RD, "Light source current", Units::AMPER, 45);
    Resource *lightSourceActivePower = new Resource(0.0f, ResourceOp::RES_RD, "Light source active power", Units::WATT, 46);
    Resource *lightSourceActiveEnergy = new Resource(0.0f, ResourceOp::RES_RD, "Light source active energy", Units::KILOWATT_HOUR, 47);

    std::vector<Resource *> outdoorLampControllerResources = {
        command, commandInAction, dimmingLevel, defaultDimmingLevel, lampFailure, lampFailureReason,
        controlGearFailure, controlGearFailureReason, relayFailure, dayBurner, cyclingFailure,
        controlGearCommunicationFailure, schedulerID, invalidScheduler, lampOperatingHours,
        lampOperatingHoursReset, lampOnTimestamp, lampSwitchCounter, lampSwitchCounterReset,
        controlGearStartCounter, controlGearTemperature, controlGearThermalDerating,
        controlGearThermalDeratingCounter, controlGearThermalDeratingCounterReset,
        controlGearThermalShutdown, controlGearThermalShutdownCounter,
        controlGearThermalShutdownCounterReset, outputPort, standbyMode, constantLightOutput,
        cleaningFactorEnabled, cleaningPeriod, initialLampCleaningFactor, lampCleaningDate,
        controlType, nominalLampWattage, minimumDimmingLevel, minimumLampWattage,
        lightColorTemperatureCommand, actualLightColorTemperature, virtualPowerOutput,
        voltageAtMaxDimLevel, voltageAtMinDimLevel, lightSourceVoltage, lightSourceCurrent,
        lightSourceActivePower, lightSourceActiveEnergy
    };
    NodeObject* outdoorLampControllerObject = new NodeObject(OUTDOOR_LAMP_CONTROLLER_OBJECT_ID, 0, outdoorLampControllerResources);

    // ================================== OBJECT LUMINAIR ASSETS ==============================
    Resource *assetGTIN = new Resource(std::string(""), ResourceOp::RES_RD, "Asset GTIN", Units::NA, 1);
    Resource *yearOfManufacture = new Resource(0, ResourceOp::RES_RDWR, "Year of manufacture", Units::NA, 2);
    Resource *weekOfManufacture = new Resource(0, ResourceOp::RES_RDWR, "Week of manufacture", Units::NA, 3);
    Resource *nominalLightOutput = new Resource(0, ResourceOp::RES_RDWR, "Nominal light output", Units::LUMEN, 4);
    Resource *lightDistributionType = new Resource(0, ResourceOp::RES_RDWR, "Light distribution type", Units::NA, 5);
    Resource *luminaireColor = new Resource(std::string(""), ResourceOp::RES_RDWR, "Luminaire color", Units::NA, 6);
    Resource *nominalInputPower = new Resource(0.0f, ResourceOp::RES_RD, "Nominal input power", Units::WATT, 7);
    Resource *powerAtMinimumDimLevel = new Resource(0.0f, ResourceOp::RES_RD, "Power at minimum dim level", Units::WATT, 8);
    Resource *nominalMaxACMainsVoltage = new Resource(0, ResourceOp::RES_RD, "Nominal max AC mains voltage", Units::VOLT, 9);
    Resource *nominalMinACMainsVoltage = new Resource(0, ResourceOp::RES_RD, "Nominal min AC mains voltage", Units::VOLT, 10);
    Resource *cri = new Resource(0, ResourceOp::RES_RD, "CRI", Units::NA, 11);
    Resource *cctValue = new Resource(0, ResourceOp::RES_RD, "CCT value", Units::KELVIN, 12);
    Resource *luminaireIdentification = new Resource(std::string(""), ResourceOp::RES_RD, "Luminaire identification", Units::NA, 13);
    Resource *luminaireIdentificationNumber = new Resource(std::string(""), ResourceOp::RES_RD, "Luminaire identification number", Units::NA, 14);

    std::vector<Resource *> luminaireAssestsResources = {
        assetGTIN, yearOfManufacture, weekOfManufacture, nominalLightOutput, lightDistributionType,
        luminaireColor, nominalInputPower, powerAtMinimumDimLevel, nominalMaxACMainsVoltage,
        nominalMinACMainsVoltage, cri, cctValue, luminaireIdentification, luminaireIdentificationNumber
    };
    NodeObject* luminairAssestObject = new NodeObject(LUMINAIR_ASSET_OBJECT_ID, 0, luminaireAssestsResources);

    std::vector<NodeObject *> *objects = new std::vector<NodeObject *>({securityObject, serverObject, deviceObject, deviceExtensionObject, batteryObject, lpwanObject, dataBridgeObject, timeSynchronisationObject, outdoorLampControllerObject, luminairAssestObject});

    NodeClient client = {objects};

    client.InitNetwork();
    client.StartClient();
    return 0;
}

#endif

#ifdef DEBUG

#include "mbed.h"
#include "NetworkInterface.h"
#include "EthernetInterface.h"
#include "UDPSocket.h"
#include "mesh_system.h"
#include "net_interface.h"
#include "ip6string.h"

#include "object_device.h"

#include "node_object.h"

#include <string>
#include <vector>

using namespace std::chrono;

// Network interface
NetworkInterface *eth;

static Thread lwm2m_main_thread(osPriorityNormal, OS_STACK_SIZE, nullptr, "lwm2m_main_thread");

void lwm2m_main_thread_task();

static Timeout lwm2m_main_thread_timeout_timer;

void lwm2m_main_thread_timeout_timer_cb()
{
    lwm2m_main_thread.flags_set(0x1);
}

extern "C"
{
#include "liblwm2m.h"
#include "connection.h"
#if defined(USE_DTLS)
#include "dtlsconnection.h"
#endif
#include "object_utils.h"

#define STR(x) #x

#define CLIENT_ENDPOINT_NAME "mbedM2M"
#define CLIENT_IDENTITY "mbedM2M"
#define CLIENT_KEYSTR "65875A0C3D4646A99BFC4D5967EE7DB3"
#define SERVER_DTLS_PORT "5684"
#define SERVER_PORT "5683"
#define CLIENT_LOCAL_PORT 0 // Let OS decide

// Connect to the Leshan test server as default: http://leshan.eclipse.org
#define M2M_SERVER_URL "2a01:111:f100:9001::1761:93fa" // LESHAN

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
#define PRV_BINDING_MODE "U"

#define PRV_OFFSET_MAXLEN 7 //+HH:MM\0 at max
#define PRV_TLV_BUFFER_SIZE 128

    extern lwm2m_object_t *get_object_device(void);
    extern void free_object_device(lwm2m_object_t *objectP);
    extern lwm2m_object_t *get_server_object(void);
    extern void free_server_object(lwm2m_object_t *object);
    lwm2m_object_t *get_security_object(int serverId, const char *serverUri, char *bsPskId, char *psk, uint16_t pskLen, bool isBootstrap);
    extern void free_security_object(lwm2m_object_t *objectP);
    extern char *get_server_uri(lwm2m_object_t *objectP, uint16_t secObjInstID);
    extern lwm2m_object_t *get_test_object(void);
    extern void free_test_object(lwm2m_object_t *object);
}

#define MAX_PACKET_SIZE 2048

int g_reboot = 0;

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

static void print_interface_addr(int id)
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

int init_network()
{
    int ret = 0;

    // try to connect and get ip via DHCP.
    printf("=====================\n");
    printf("obtaining ip address...\n");

    eth = EthInterface::get_default_instance();
    ret = eth->connect();

    if (ret != 0)
    {
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

void print_state(lwm2m_context_t *lwm2mH)
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

#define OBJ_COUNT 4

int main(int argc, char *argv[])
{
    init_network();

    lwm2m_object_t *objArray[OBJ_COUNT];

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

    // objArray[2] = get_object_device();
    // ObjectDevice* objectDevice = new ObjectDevice();
    // objArray[2] = objectDevice->GetObjectDevice();

    // Multiple instance not implemented
    // Time type not implemented
    // Exec void value not implemented
    // objlnk type not implemented (for extdevinfo)

    Resource manufacturer(std::string(PRV_MANUFACTURER), ResourceOp::RES_RD, PRV_MANUFACTURER, Units::NA);
    Resource model_number(std::string(PRV_MODEL_NUMBER), ResourceOp::RES_RD, PRV_MODEL_NUMBER, Units::NA);
    Resource serial_number(std::string(PRV_SERIAL_NUMBER), ResourceOp::RES_RD, PRV_SERIAL_NUMBER, Units::NA);
    Resource firmware_version(std::string(PRV_FIRMWARE_VERSION), ResourceOp::RES_RD, PRV_FIRMWARE_VERSION, Units::NA);
    Resource reboot(0, ResourceOp::RES_E);
    Resource factory_reset(0, ResourceOp::RES_E);
    Resource available_power_source(0, ResourceOp::RES_RD, "Available power source", Units::NA);
    Resource power_source_voltage(0, ResourceOp::RES_RD, "Power source voltage", Units::VOLT);
    Resource power_source_current(0, ResourceOp::RES_RD, "Power source current", Units::AMPER);
    Resource battery_level(PRV_BATTERY_LEVEL, ResourceOp::RES_RD, "Battery level", Units::PERCENT);
    Resource memory_free(PRV_MEMORY_FREE, ResourceOp::RES_RD, "Free memory", Units::BYTES);
    Resource error_code(PRV_ERROR_CODE, ResourceOp::RES_RD, "Error code", Units::NA);
    Resource reset_error_code(0, ResourceOp::RES_E);
    Resource current_time(0, ResourceOp::RES_RDWR, "Current time", Units::NA);
    Resource utc_offset(string("UTC+X"), ResourceOp::RES_RDWR, "UTC offset", Units::NA);
    Resource timezone(std::string(PRV_TIME_ZONE), ResourceOp::RES_RDWR, "Timezone", Units::NA);
    Resource supported_binding_mode(std::string(PRV_BINDING_MODE), ResourceOp::RES_RD, "Supported binding and mode", Units::NA);
    Resource device_type(string("light node"), ResourceOp::RES_RD, "Device type", Units::NA);
    Resource hardware_version(string("1.0"), ResourceOp::RES_RD, "Hardware version", Units::NA);
    Resource software_version(string("1.0"), ResourceOp::RES_RD, "Software version", Units::NA);
    Resource battery_status(0, ResourceOp::RES_RD, "Battery status", Units::NA);
    Resource memory_total(1000, ResourceOp::RES_RD, "Memory total", Units::NA);
    Resource extdevinfo(0, ResourceOp::RES_RD, "External device info", Units::NA);

    manufacturer.BindOnRead<std::string>([](string str)
                                         { std::cout << "Manufacturer read get : " << str << std::endl; });

    manufacturer.Read<std::string>();

    std::vector<Resource> resources = {manufacturer, model_number, serial_number, firmware_version, reboot, factory_reset, available_power_source, power_source_voltage, power_source_current, battery_level, memory_free, error_code, reset_error_code, current_time, utc_offset, timezone, supported_binding_mode, device_type, hardware_version, software_version, battery_status, memory_total, extdevinfo};

    NodeObject customObject(3, 0, resources);
    objArray[2] = customObject.Get();
    if (NULL == objArray[2])
    {
        fprintf(stderr, "Failed to create Device object\r\n");
        return -1;
    }

    // NodeObject testObject(3313, myCustomObject, types, resOp, names, units);
    // objArray[3] = testObject.Get();
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

    while (1)
    {
        ThisThread::sleep_for(1s);
    }

    return 0;
}

void lwm2m_main_thread_task()
{
    while (1)
    {
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
        if (result != 0)
        {
            fprintf(stderr, "lwm2m_step() failed: 0x%X\r\n", result);
        }
        ThisThread::flags_wait_any_for(0x1, seconds(tv.tv_sec));
    }
}

void lwm2m_handle_incoming_socket_data_cpp_wrap(ns_address_t *addr, uint8_t *buf, size_t len)
{
    printf("New packet arrived !\n");
    if (connectionlayer_handle_packet(data.connLayer, addr, buf, len) == -1)
    {
        /*
         * This packet comes from an unknown peer
         */
        fprintf(stderr, "received bytes ignored!\r\n");
    }

    lwm2m_main_thread_timeout_timer.detach();
    lwm2m_main_thread.flags_set(0x1);
}

extern "C" void lwm2m_handle_incoming_socket_data(ns_address_t *addr, uint8_t *buf, size_t len)
{
    lwm2m_handle_incoming_socket_data_cpp_wrap(addr, buf, len);
}

#endif