#include "GLSounds.h"
#include "GLUtils.h"
#include "GLBufferReader.h"
#include "GLDataToWave.h"
#include <stdint.h>
#include <cstring>
#include <memory>
#include <vector>

uint32_t read32LittleEndian(const uint8_t *data)
{
    uint32_t val = 0;
    val |= (uint32_t)data[0];
    val |= (uint32_t)data[1] << 8;
    val |= (uint32_t)data[2] << 16;
    val |= (uint32_t)data[3] << 24;
    return val;
}

uint16_t read16LittleEndian(const uint8_t *data)
{
    uint16_t val = 0;
    val |= (uint16_t)data[0];
    val |= (uint16_t)data[1] << 8;
    return val;
}

bool dataToWave(const uint8_t *data, unsigned dataLen, GL::WaveData &output)
{
    GLBufferReader reader(data, dataLen);
    size_t sampleDataLen = 0;
    char *sampleData = nullptr;

    // Check header
    const uint8_t fourccRiff[4] = { 'R', 'I', 'F', 'F' };
    const uint8_t fourccWave[4] = { 'W', 'A', 'V', 'E' };
    uint8_t header[12];
    if (reader.read(header, sizeof(header)) != sizeof(header) ||
        std::memcmp(header, fourccRiff, sizeof(fourccRiff)) != 0 ||
        std::memcmp(header + 8, fourccWave, sizeof(fourccWave)) != 0) {
        return false;
    }

    // Read chunks
    const uint8_t chunkFmt[4] = { 'f', 'm', 't', ' ' };
    const uint8_t chunkData[4] = { 'd', 'a', 't', 'a' };
    uint8_t chunkHeader[8];
    while (reader.read(chunkHeader, sizeof(chunkHeader)) == sizeof(chunkHeader)) {
        uint32_t chunkLen = read32LittleEndian(chunkHeader + 4);
        size_t chunkOffset = reader.offset();
        if (std::memcmp(chunkHeader, chunkFmt, 4) == 0) {
            uint8_t commData[16];
            if (chunkLen != sizeof(commData) || reader.read(commData, sizeof(commData)) != sizeof(commData)) {
                return false;
            }
            int format = read16LittleEndian(commData);
            output.numChannels = read16LittleEndian(commData + 2);
            output.sampleRate = read32LittleEndian(commData + 4);
            output.sampleSize = read16LittleEndian(commData + 14);
            if (format != 1 || output.numChannels != 1 || output.sampleSize != 8) {
                // This code only supports 8-bit mono.
                return false;
            }
        } else if (std::memcmp(chunkHeader, chunkData, 4) == 0) {
            output.numSampleFrames = chunkLen;
            sampleDataLen = (output.sampleSize / 8) * output.numSampleFrames * output.numChannels;
            sampleData = new char[chunkLen];
            if (reader.read((uint8_t*)sampleData, chunkLen) != chunkLen) {
                return false;
            }
        }
        if (!reader.seek(chunkOffset + chunkLen)) {
            break;
        }
    }

    if (output.numChannels == 0 || output.numSampleFrames == 0 || output.sampleSize == 0 || output.sampleRate == 0 || sampleData == NULL) {
        return false;
    }

    output.data = sampleData;
    output.dataLen = sampleDataLen;

    return true;
}
