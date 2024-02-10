#include "CANbeSerial.h"
#include "softCRC.h"

#define ERROR_FRAME_SIZE 7

void cbs_init(cbs_t *cbs)
{
    cobs_decodeStreamStart(&cbs->cobsDecoder);
}

void cbs_handler(cbs_t *cbs)
{

}

void cbs_sendData(cbs_t *cbs, cbs_data_t *data)
{
    uint8_t length = _cbs_dlcToLength(data->flags.bits.dlc);
    uint8_t txData[72] = {
            cbs_data,
            data->identifier >> 24,
            data->identifier >> 16,
            data->identifier >> 8,
            data->identifier,
            data->flags.byte};

    for(uint8_t i = 0; i<length; i++) {
        txData[i + 6] = data->data[i];
    }
    _cbs_send(cbs,&txData[0], length+8);
}

void cbs_onSerialDataReceived(cbs_t *cbs, uint8_t *data, uint8_t dataLength)
{
    for(uint8_t i= 0; i<dataLength; i++) {

        uint8_t rxFrameSize = cobs_decodeStream(&cbs->cobsDecoder, data[i], &cbs->rxData[0]);
        if(rxFrameSize == 0) continue;
        if(crc16(&cbs->rxData[0],rxFrameSize) != 0x00) {
            _cbs_errorFrame(cbs,cbs_error_crc);
            continue;
        }
        _cbs_handleFrame(cbs,&cbs->rxData[0],rxFrameSize);
    }
}

void _cbs_handleFrame(cbs_t *cbs, uint8_t *data, uint8_t dataLength)
{
    cbs_payloadId_t payloadId = data[0];

    switch (payloadId) {
        case cbs_data: _cbs_handleDataFrame(cbs, &data[1], dataLength-1); break;

        case cbs_protocolVersion: _cbs_errorFrame(cbs, cbs_error_unsupportedPayloadId); break; //no-op
        case cbs_protocolVersionRequest: _cbs_sendProtocolVersion(cbs); break;

        case cbs_configurationState: _cbs_errorFrame(cbs, cbs_error_unsupportedPayloadId); break; //no-op
        case cbs_configurationStateRequest: _cbs_sendConfigurationState(cbs); break;
        case cbs_configurationStateCommand: _cbs_setConfigurationState(cbs, &data[1], dataLength-1); break;

        case cbs_deviceInformation: _cbs_errorFrame(cbs, cbs_error_unsupportedPayloadId); break; //no-op
        case cbs_deviceInformationRequest:  _cbs_sendDeviceInformation(cbs); break;

        default: _cbs_errorFrame(cbs, cbs_error_unknownPayloadId); break;
    }
}

void _cbs_send(cbs_t *cbs, uint8_t *data, uint8_t dataLength)
{
    if(cbs->txIndex+dataLength+3> TX_BUFFER_SIZE-ERROR_FRAME_SIZE){ // always reserve space for one error frame
        _cbs_errorFrame(cbs, cbs_errer_txBufferFull);
        return;
    }
    _cbs_encode(cbs, data, dataLength);
}

void _cbs_errorFrame(cbs_t *cbs, cbs_error_t error)
{
    uint8_t txData[] = {cbs_error,error,0,0}; // Byte 2 and 3 are reserved for CRC
    _cbs_encode(cbs,&txData[0], sizeof(txData));
}

void _cbs_encode(cbs_t *cbs, uint8_t *data, uint8_t dataLength)
{
    uint16_t crc = crc16(data, dataLength-2);
    data[dataLength-2] = crc>>8;
    data[dataLength-1] = crc;

    cbs->txData[cbs->txIndex] = 0;
    cbs->txIndex++;

    cbs->txIndex += cobs_encode(&cbs->txData[cbs->txIndex], data, dataLength);
}

void _cbs_handleDataFrame(cbs_t *cbs, const uint8_t *data, uint8_t dataLength)
{
    uint32_t identifier;
    identifier  = data[0]<<24;
    identifier |= data[1]<<16;
    identifier |= data[2]<<8;
    identifier |= data[3];

    cbs_data_t canData;
    canData.identifier = identifier;
    canData.flags.byte = data[4];

    uint8_t length = _cbs_dlcToLength(canData.flags.bits.dlc);
    for(uint8_t i = 0; i<length; i++) {
        canData.data[i] = data[i+4];
    }

    cbs_handleDataFrame(cbs, &canData);
}

void _cbs_sendProtocolVersion(cbs_t *cbs)
{
    uint8_t txData[] = {cbs_protocolVersion,CBS_PROTOCOL_VERSION,0,0}; // Byte 2 and 3 are reserved for CRC
    _cbs_send(cbs,&txData[0], sizeof(txData));
}

void _cbs_sendConfigurationState(cbs_t *cbs)
{
    uint8_t txData[] = {
            cbs_configurationState,
            cbs->configuration.baudrate,
            cbs->configuration.fdBaudrate,
            0,0}; // Last two bytes are reserved for CRC
    _cbs_send(cbs,&txData[0], sizeof(txData));
}
void _cbs_setConfigurationState(cbs_t *cbs, const uint8_t *data, uint8_t dataLength)
{
    if(dataLength != sizeof(cbs_configuration_t)){
        _cbs_errorFrame(cbs, cbs_error_configurationStateCommand_size);
        return;
    }
    cbs->configuration.baudrate = data[0];
    cbs->configuration.fdBaudrate = data[1];
}

void _cbs_sendDeviceInformation(cbs_t *cbs)
{
    uint8_t txData[] = {CBS_DEVICE_INFORMATION};
    txData[0] = cbs_deviceInformation;
    _cbs_send(cbs,&txData[0], sizeof(txData));
}

int8_t _cbs_dlcToLength(uint8_t dlc)
{
    static int8_t dlcToLength[] = {0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64};
    if(dlc>=sizeof(dlcToLength)) return -1;
    return dlcToLength[dlc];
}
