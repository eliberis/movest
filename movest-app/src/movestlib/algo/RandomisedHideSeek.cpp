//
// Created by el398 on 10/12/15.
//

#include <algorithm>
#include <iostream>
#include "RandomisedHideSeek.h"

extern "C" {
#include <rscode/ecc.h>
#define BLOCKSIZE 255
}

RandomisedHideSeek::RandomisedHideSeek(RandomisedHideSeek::AlgOptions *algOptions) {
    if(algOptions == nullptr) {
        this->opt = AlgOptions{0, 0};
    } else {
        this->opt = *algOptions;
    }
}

void RandomisedHideSeek::initAsEncoder(movest_params *params) {
    Algorithm::initAsEncoder(params);
    if(!(flags & MOVEST_DUMMY_PASS)) {
        initialize_ecc();

        fileSize = opt.fileSize != 0? opt.fileSize : (uint)datafile.remainingData();
        dataSize = eccDataInflation(fileSize);

        initialiseMapping(dataSize);

        // Fill the data buffer with blocks of file data & parity bytes
        data.resize(dataSize);
        unsigned char fileData[BLOCKSIZE - NPAR];
        uint currentPos = 0, fileRead = 0;
        while(fileRead < fileSize && currentPos < dataSize) {
            uint toRead = std::min(fileSize - fileRead, (uint)BLOCKSIZE - NPAR);
            int read = (int)datafile.read(&fileData[0], toRead);
            encode_data(fileData, read, &data[currentPos]);
            fileRead += read;
            currentPos += read + NPAR;
        }
    }
}

void RandomisedHideSeek::initAsDecoder(movest_params *params) {
    Algorithm::initAsDecoder(params);
    if(!(flags & MOVEST_DUMMY_PASS)) {
        initialize_ecc();
        fileSize = opt.fileSize;
        dataSize = eccDataInflation(fileSize);

        initialiseMapping(dataSize);

        data.resize(dataSize, 0);
    }
}

void RandomisedHideSeek::initialiseMapping(uint dataSize) {
    // Build a mapping from a data bit to the particular MV
    uint64_t capacity = opt.byteCapacity;

    assert(encoder || dataSize <= capacity);

    std::vector<uint8_t> seedData = this->deriveBytes(SEED_SIZE, "MovestRandSeed");
    std::seed_seq seed(seedData.begin(), seedData.end());
    std::default_random_engine rng(seed);

    ulong bitCapacity = ((ulong) capacity) * 8;
    std::uniform_int_distribution<ulong> dist(0, bitCapacity-1);
    ulong bitDataSize = ((ulong) dataSize) * 8;
    bitToMvMapping.resize(bitDataSize);

    std::vector<bool> used(bitCapacity, false);

    for(ulong i = 0; i < bitDataSize; ++i) {
        ulong mvNum = dist(rng);
        while(used[mvNum]) {
            ++mvNum;
            if(mvNum >= bitCapacity) mvNum = 0;
        }
        used[mvNum] = true;
        bitToMvMapping[i] = Pair { i,  mvNum };
    }

    // Sort the mapping in increasing order of MVs, for sequential embedding
    std::sort(bitToMvMapping.begin(), bitToMvMapping.end());
}

void RandomisedHideSeek::processMvComponentEmbed(int16_t *mv) {
    if(flags & MOVEST_DUMMY_PASS) {
        bitsProcessed++;
    } else {
        if(index >= 8*dataSize) return;
        if(bitsProcessed == bitToMvMapping[index].mv) {
            // We found a MV that's next on a list to be modified.
            ulong dataBit = bitToMvMapping[index].bit;
            int bit = data[dataBit / 8] >> (dataBit % 8);

            bool success = HideSeek::embedIntoMvComponent(mv, bit);
            if(success) index++;
        }
        bitsProcessed++;
    }
}

void RandomisedHideSeek::processMvComponentExtract(int16_t mv) {
    if(index >= 8*dataSize) return;
    if (bitsProcessed == bitToMvMapping[index].mv) {
        // We found a MV that was next on a list to be modified.
        int bit = 0;
        bool success = HideSeek::extractFromMvComponent(mv, &bit);
        if(success) {
            ulong dataBit = bitToMvMapping[index].bit;
            data[dataBit / 8] |= (bit & 1) << (dataBit % 8);
            index++;
        }
    }

    bitsProcessed++;
}

movest_result RandomisedHideSeek::finalise() {
    if(!(flags & MOVEST_DUMMY_PASS)) {
        if(!encoder) {
            uint currentPos = 0;
            while(currentPos < dataSize) {
                uint blockSize = std::min((uint)BLOCKSIZE, dataSize - currentPos);
                decode_data(&data[currentPos], blockSize);
                if (check_syndrome() != 0) {
                    correct_errors_erasures(&data[currentPos], blockSize, 0, NULL);
                }
                datafile.write(&data[currentPos], blockSize - NPAR);
                currentPos += blockSize;
            }
            datafile.close();
        }
    }

    return movest_result {
            uint(bitsProcessed / 8), 0
    };
}

unsigned int RandomisedHideSeek::computeEmbeddingSize(unsigned int dataSize) {
    uint fileSize = Algorithm::computeEmbeddingSize(dataSize);
    return eccDataInflation(fileSize);
}

unsigned int RandomisedHideSeek::eccDataInflation(unsigned int size) {
    // Total size of embedded data:
    // size + NPAR parity bytes for every (BLOCKSIZE - NPAR) bytes of the file
    uint blocks = (size / (BLOCKSIZE - NPAR)) + (size % (BLOCKSIZE - NPAR) != 0);
    return blocks * NPAR + size;
}
