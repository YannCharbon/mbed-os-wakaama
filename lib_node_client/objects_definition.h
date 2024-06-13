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
#define PRV_BINDING_MODE "U"
#define PRV_OFFSET_MAXLEN 7 //+HH:MM\0 at max
#define PRV_TLV_BUFFER_SIZE 128
#define PRV_SHORT_SERVER_ID 123
#define PRV_LIFETIME 600
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

std::vector<NodeObject *> *initializeObjects()
{
    // ================================== OBJECT SERVER ==============================
    Resource *shortServerId = new Resource(PRV_SHORT_SERVER_ID, ResourceOp::RES_RD, "Short server id", Units::NA, 0);
    Resource *lifetime = new Resource(PRV_LIFETIME, ResourceOp::RES_RDWR, "Lifetime", Units::SECONDS, 1);
    Resource *defaultMinimumPeriod = new Resource(0, ResourceOp::RES_RDWR, "Default minimum period", Units::SECONDS, 2);
    Resource *defaultMaximumPeriod = new Resource(0, ResourceOp::RES_RDWR, "Default maximum period", Units::SECONDS, 3);
    Resource *disable = new Resource(0, ResourceOp::RES_E, "Disable", Units::NA, 4);
    Resource *disableTimeout = new Resource(0, ResourceOp::RES_RDWR, "Disable timeout", Units::SECONDS, 5);
    Resource *notificationStoringWhenDisabledOrOffline = new Resource(PRV_STORING, ResourceOp::RES_RDWR, "Notification storing when disabled or offline", Units::NA, 6);
    Resource *binding = new Resource(std::string(PRV_BINDING), ResourceOp::RES_RDWR, "Binding", Units::NA, 7);
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
    Resource *utcOffset = new Resource(std::string("UTC+X"), ResourceOp::RES_RDWR, "UTC offset", Units::NA, 14);
    Resource *timezone = new Resource(std::string(PRV_TIME_ZONE), ResourceOp::RES_RDWR, "Timezone", Units::NA, 15);
    Resource *supportedBindingMode = new Resource(std::string(PRV_BINDING_MODE), ResourceOp::RES_RD, "Supported binding and mode", Units::NA, 16);
    Resource *deviceType = new Resource(std::string("light node"), ResourceOp::RES_RD, "Device type", Units::NA, 17);
    Resource *hardwareVersion = new Resource(std::string("1.0"), ResourceOp::RES_RD, "Hardware version", Units::NA, 18);
    Resource *softwareVersion = new Resource(std::string("1.0"), ResourceOp::RES_RD, "Software version", Units::NA, 19);
    Resource *batteryStatus = new Resource(0, ResourceOp::RES_RD, "Battery status", Units::NA, 20);
    Resource *memoryTotal = new Resource(1000, ResourceOp::RES_RD, "Memory total", Units::NA, 21);
    Resource *extdevinfo = new Resource(0, ResourceOp::RES_RD, "External device info", Units::NA, 22);

    manufacturer->BindOnRead<std::string>([](std::string str)
                                          { std::cout << "Manufacturer read get : " << str << std::endl; });

    std::vector<Resource *> deviceResources = {manufacturer, modelNumber, serialNumber, firmwareVersion, reboot, factoryReset, availablePowerSource, powerSourceVoltage, powerSourceCurrent, serverBatteryLevel, memoryFree, errorCode, resetErrorCode, currentTime, utcOffset, timezone, supportedBindingMode, deviceType, hardwareVersion, softwareVersion, batteryStatus, memoryTotal, extdevinfo};
    NodeObject *deviceObject = new NodeObject(DEVICE_OBJECT_ID, 0, deviceResources);

    // ================================== OBJECT DEVICE EXTENSION ==============================
    Resource *gtinModelNumber = new Resource(std::string(""), ResourceOp::RES_RD, "GTIN model number", Units::NA, 1);
    Resource *manufacturerIdentifier = new Resource(std::string(""), ResourceOp::RES_RD, "Manufacturer identifier", Units::NA, 2);
    Resource *userGivenName = new Resource(std::string(""), ResourceOp::RES_RDWR, "User-given name", Units::NA, 3);
    Resource *assetIdentifier = new Resource(std::string(""), ResourceOp::RES_RDWR, "Asset identifier", Units::NA, 4);
    Resource *installationDate = new Resource(0, ResourceOp::RES_RDWR, "Installation date", Units::DATE, 5);
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
    Resource *ipv4Address = new Resource(std::string("192.168.1.0"), ResourceOp::RES_RDWR, "IPv4 address", Units::NA, 2);
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
    Resource *sessionTime = new Resource(0, ResourceOp::RES_RDWR, "Session time", Units::NA, 13);
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
    NodeObject *dataBridgeObject = new NodeObject(DATA_BRIDGE_OBJECT_ID, 0, dataBridgeResources);

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

    controlGearThermalDeratingCounterReset->BindOnExec<int>([](int a)
                                                            { std::cout << "Control gear thermal derating counter reset value : " << a << std::endl; });

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
        lightSourceActivePower, lightSourceActiveEnergy};
    NodeObject *outdoorLampControllerObject = new NodeObject(OUTDOOR_LAMP_CONTROLLER_OBJECT_ID, 0, outdoorLampControllerResources);

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
        nominalMinACMainsVoltage, cri, cctValue, luminaireIdentification, luminaireIdentificationNumber};
    NodeObject *luminaireAssetObject = new NodeObject(LUMINAIR_ASSET_OBJECT_ID, 0, luminaireAssestsResources);

    return new std::vector<NodeObject *>({serverObject, deviceObject, deviceExtensionObject, batteryObject, lpwanObject, dataBridgeObject, timeSynchronisationObject, outdoorLampControllerObject, luminaireAssetObject});
}

#endif