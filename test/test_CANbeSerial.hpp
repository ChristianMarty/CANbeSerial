#ifdef TEST_RUN

#include "source/CANbeSerial.h"
#include "source/cobs_u8.h"
#include "source/softCRC.h"
#include "_test_helper.hpp"

inline uint8_t makeFrame(uint8_t *destination, const uint8_t *source, uint8_t sourceLength)
{
    destination[0] = 0;
    uint8_t i = 0;
    for(; i<sourceLength; i++) {
        destination[i+2] = source[i];
    }

    uint16_t crc = crc16(source, sourceLength);
    destination[i+2] = crc>>8;
    i++;
    destination[i+2] = crc;
    i++;

    return cobs_encode(&destination[1], &destination[2], i)+1;
}

cbs_data_t txDataFrame;
inline void clearTxFrame()
{
    txDataFrame.identifier = 0;
    txDataFrame.flags.byte = 0;
    for(int i = 0; i<sizeof(txDataFrame.data); i++) {
        txDataFrame.data[i] = 0;
    }
}

// callbacks from CANbeSerial
void cbs_handleDataFrame(cbs_t *cbs, cbs_data_t *data)
{
    txDataFrame = *data;
}

void cds_handleConfigurationChange(cbs_t *cbs)
{

}

TEST_CASE( "DLC" ) {
    SECTION( "Valid Codes" ) {
        REQUIRE(cbs_dlcToLength(0) == 0);
        REQUIRE(cbs_dlcToLength(1) == 1);
        REQUIRE(cbs_dlcToLength(2) == 2);
        REQUIRE(cbs_dlcToLength(3) == 3);
        REQUIRE(cbs_dlcToLength(4) == 4);
        REQUIRE(cbs_dlcToLength(5) == 5);
        REQUIRE(cbs_dlcToLength(6) == 6);
        REQUIRE(cbs_dlcToLength(7) == 7);
        REQUIRE(cbs_dlcToLength(8) == 8);
        REQUIRE(cbs_dlcToLength(9) == 12);
        REQUIRE(cbs_dlcToLength(10) == 16);
        REQUIRE(cbs_dlcToLength(11) == 20);
        REQUIRE(cbs_dlcToLength(12) == 24);
        REQUIRE(cbs_dlcToLength(13) == 32);
        REQUIRE(cbs_dlcToLength(14) == 48);
        REQUIRE(cbs_dlcToLength(15) == 64);
    }

    SECTION( "Invalid Codes" ) {
        REQUIRE(cbs_dlcToLength(16) == -1);
        REQUIRE(cbs_dlcToLength(34) == -1);
        REQUIRE(cbs_dlcToLength(184) == -1);
        REQUIRE(cbs_dlcToLength(255) == -1);
    }
}

TEST_CASE( "CANbySerial Version" ) {

    static cbs_t cbs;
    cbs_init(&cbs);

    SECTION( "Get version" ) {

        const uint8_t sourceData[] = {cbs_protocolVersionRequest};

        uint8_t data[256];
        uint8_t dataSize = makeFrame(&data[0], &sourceData[0], sizeof(sourceData));

        cbs_onSerialDataReceived(&cbs,&data[0],dataSize);

        REQUIRE(cbs.txIndex == 7);
        REQUIRE(cbs.txData[2] == cbs_protocolVersion);
        REQUIRE(cbs.txData[3] == CBS_PROTOCOL_VERSION);
    }
}

TEST_CASE( "CANbySerial Device Information" ) {

    static cbs_t cbs;
    cbs_init(&cbs);

    SECTION( "Get Information" ) {

        const uint8_t sourceData[] = {cbs_deviceInformationRequest};

        uint8_t data[256];
        uint8_t dataSize = makeFrame(&data[0], &sourceData[0], sizeof(sourceData));

        cbs_onSerialDataReceived(&cbs,&data[0],dataSize);

        char testData[] = CBS_DEVICE_INFORMATION;
        REQUIRE(cbs.txIndex == sizeof(CBS_DEVICE_INFORMATION)+3);
        REQUIRE(cbs.txData[2] == cbs_deviceInformation);
        REQUIRE(match_string((char*)&cbs.txData[3], &testData[1], cbs.txIndex-7));
    }
}

TEST_CASE( "CANbySerial send frame on bus" ) {

    SECTION( "Test Frame 1" ) {

        static cbs_t cbs;
        cbs_init(&cbs);
        clearTxFrame();

        const uint8_t sourceData[] = {cbs_data,0x01,0x02,0x03,0x04,0x23,0x11, 0x22};

        uint8_t data[256];
        uint8_t dataSize = makeFrame(&data[0], &sourceData[0], sizeof(sourceData));

        cbs_onSerialDataReceived(&cbs,&data[0],dataSize);

        REQUIRE(txDataFrame.identifier == 0x01020304);
        REQUIRE(txDataFrame.flags.bits.dlc == 2);
        REQUIRE(txDataFrame.flags.bits.extended == 1);
        REQUIRE(txDataFrame.flags.bits.fd == 1);
        REQUIRE(txDataFrame.data[0] == 0x11);
        REQUIRE(txDataFrame.data[1] == 0x22);
    }

    SECTION( "Test Frame 2" ) {

        static cbs_t cbs;
        cbs_init(&cbs);
        clearTxFrame();

        const uint8_t sourceData[] = {cbs_data,0x01,0x02,0x03,0x04,0x20,0x11, 0x22};

        uint8_t data[256];
        uint8_t dataSize = makeFrame(&data[0], &sourceData[0], sizeof(sourceData));

        cbs_onSerialDataReceived(&cbs,&data[0],dataSize);

        REQUIRE(txDataFrame.identifier == 0x01020304);
        REQUIRE(txDataFrame.flags.bits.dlc == 2);
        REQUIRE(txDataFrame.flags.bits.extended == 0);
        REQUIRE(txDataFrame.flags.bits.fd == 0);
        REQUIRE(txDataFrame.data[0] == 0x11);
        REQUIRE(txDataFrame.data[1] == 0x22);
    }
}

#endif