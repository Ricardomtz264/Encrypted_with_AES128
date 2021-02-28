//**********************************************
//
//   from file aes_layer.h
//
//**********************************************
#include <string.h> // CBC mode, for memset
#include "board.h"
#include "fsl_crc.h"
/*
static void InitCrc32(CRC_Type *base, uint32_t seed)
{
    crc_config_t config;

    config.polynomial         = 0x04C11DB7U;
    config.seed               = seed;
    config.reflectIn          = true;
    config.reflectOut         = true;
    config.complementChecksum = true;
    config.crcBits            = kCrcBits32;
    config.crcResult          = kCrcFinalChecksum;

    CRC_Init(base, &config);
}
*/
