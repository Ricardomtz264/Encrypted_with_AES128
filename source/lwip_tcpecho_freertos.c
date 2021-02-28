/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "lwip/opt.h"

#if LWIP_NETCONN

#include "tcpecho.h"
#include "lwip/netifapi.h"
#include "lwip/tcpip.h"
#include "netif/ethernet.h"
#include "enet_ethernetif.h"

#include "board.h"

#include "fsl_device_registers.h"
#include "pin_mux.h"
#include "clock_config.h"

#include "aes.h"
#include "fsl_crc.h"

#include "aes_layer.c"
#include "aes_layer.h"
#include "aes_layer_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* MAC address configuration. */
#define configMAC_ADDR                     \
    {                                      \
        0x02, 0x12, 0x13, 0x10, 0x15, 0x11 \
    }

/* Address of PHY interface. */
#define EXAMPLE_PHY_ADDRESS BOARD_ENET0_PHY_ADDRESS

/* System clock name. */
#define EXAMPLE_CLOCK_NAME kCLOCK_CoreSysClk


#ifndef EXAMPLE_NETIF_INIT_FN
/*! @brief Network interface initialization function. */
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init
#endif /* EXAMPLE_NETIF_INIT_FN */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void test_task(void *arg);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{
    static struct netif netif;
#if defined(FSL_FEATURE_SOC_LPC_ENET_COUNT) && (FSL_FEATURE_SOC_LPC_ENET_COUNT > 0)
    static mem_range_t non_dma_memory[] = NON_DMA_MEMORY_ARRAY;
#endif /* FSL_FEATURE_SOC_LPC_ENET_COUNT */
    ip4_addr_t netif_ipaddr, netif_netmask, netif_gw;
    ethernetif_config_t enet_config = {
        .phyAddress = EXAMPLE_PHY_ADDRESS,
        .clockName  = EXAMPLE_CLOCK_NAME,
        .macAddress = configMAC_ADDR,
#if defined(FSL_FEATURE_SOC_LPC_ENET_COUNT) && (FSL_FEATURE_SOC_LPC_ENET_COUNT > 0)
        .non_dma_memory = non_dma_memory,
#endif /* FSL_FEATURE_SOC_LPC_ENET_COUNT */
    };

    SYSMPU_Type *base = SYSMPU;
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    /* Disable SYSMPU. */
    base->CESR &= ~SYSMPU_CESR_VLD_MASK;

    IP4_ADDR(&netif_ipaddr, configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3);
    IP4_ADDR(&netif_netmask, configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3);
    IP4_ADDR(&netif_gw, configGW_ADDR0, configGW_ADDR1, configGW_ADDR2, configGW_ADDR3);

    tcpip_init(NULL, NULL);

    netifapi_netif_add(&netif, &netif_ipaddr, &netif_netmask, &netif_gw, &enet_config, EXAMPLE_NETIF_INIT_FN,
                       tcpip_input);
    netifapi_netif_set_default(&netif);
    netifapi_netif_set_up(&netif);

    PRINTF("\r\n************************************************\r\n");
    PRINTF(" TCP Echo example\r\n");
    PRINTF("************************************************\r\n");
    PRINTF(" IPv4 Address     : %u.%u.%u.%u\r\n", ((u8_t *)&netif_ipaddr)[0], ((u8_t *)&netif_ipaddr)[1],
           ((u8_t *)&netif_ipaddr)[2], ((u8_t *)&netif_ipaddr)[3]);
    PRINTF(" IPv4 Subnet mask : %u.%u.%u.%u\r\n", ((u8_t *)&netif_netmask)[0], ((u8_t *)&netif_netmask)[1],
           ((u8_t *)&netif_netmask)[2], ((u8_t *)&netif_netmask)[3]);
    PRINTF(" IPv4 Gateway     : %u.%u.%u.%u\r\n", ((u8_t *)&netif_gw)[0], ((u8_t *)&netif_gw)[1],
           ((u8_t *)&netif_gw)[2], ((u8_t *)&netif_gw)[3]);
    PRINTF("************************************************\r\n");

    //tcpecho_init();

    sys_thread_new("test_task", test_task, NULL, 1024, 3);

    vTaskStartScheduler();

    /* Will not get here unless a task calls vTaskEndScheduler ()*/
    return 0;
}
#endif

/*!
 * @brief Init for CRC-32.
 * @details Init CRC peripheral module for CRC-32 protocol.
 *          width=32 poly=0x04c11db7 init=0xffffffff refin=true refout=true xorout=0xffffffff check=0xcbf43926
 *          name="CRC-32"
 *          http://reveng.sourceforge.net/crc-catalogue/
 */


void test_task(void *arg)
{
	aescrc_Enc();
	Server();
	//server_accept();
	//recived_data();
	//aescrc_Dec();

	PRINTF("AES and CRC test task\r\n");
}
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


//**********************************************
//
//   from file aes_layer.c
//
//**********************************************

server_accept(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err;
  LWIP_UNUSED_ARG(arg);

  /* Create a new connection identifier. */
  /* Bind connection to well known port number 7. */
#if LWIP_IPV6
  conn = netconn_new(NETCONN_TCP_IPV6);
  netconn_bind(conn, IP6_ADDR_ANY, 7);
#else /* LWIP_IPV6 */
  conn = netconn_new(NETCONN_TCP);
  netconn_bind(conn, IP_ADDR_ANY, 7);
#endif /* LWIP_IPV6 */
  LWIP_ERROR("tcpecho: invalid conn", (conn != NULL), return;);

  /* Tell connection to go into listening mode. */
  netconn_listen(conn);
  err = netconn_accept(conn, &newconn);
  PRINTF("accepted new connection %p\r\n", newconn);

}

recived_data(void *arg)
{
	struct netconn *conn, *newconn;
	err_t err;
	struct netbuf *buf;
	void *data;
	u16_t len;
	PRINTF("accepted new connection %p\r\n", newconn);
	while ((err = netconn_recv(newconn, &buf)) == ERR_OK) {
		PRINTF("\n Recived \r\n");

		netbuf_data(buf, &data, &len);
		err = netconn_write(newconn, data, len, NETCONN_COPY);

		if (err != ERR_OK) {
			printf("tcpecho: netconn_write: error \"%s\"\n", lwip_strerr(err));
		}
    }
}


void aescrc_Enc(void *arg)
{

	struct AES_ctx ctx;
	size_t test_string_len, padded_len;
	uint8_t padded_msg[512] = {0};

	// CRC data
	CRC_Type *base = CRC0;
	uint32_t checksum32;

	for(int i=0; i<strlen(test_string); i++) {
		PRINTF("0x%02x,", test_string[i]);
	}
	PRINTF("\r\n");

	PRINTF("\nDecryption by AES128\r\n\n");
	// Init the AES context structure
	AES_init_ctx_iv(&ctx, key, iv);

	// To encrypt an array its lenght must be a multiple of 16 so we add zeros
	test_string_len = strlen(test_string);
	padded_len = test_string_len + (16 - (test_string_len%16) );
	memcpy(padded_msg, test_string, test_string_len);

	AES_CBC_encrypt_buffer(&ctx, padded_msg, padded_len);

	PRINTF("Encrypted Message: %d : %d :   ", padded_len, test_string_len);
	for(int i=0; i<padded_len; i++) {
		PRINTF("0x%02x,", padded_msg[i]);
	}
	PRINTF("\r\n");


	PRINTF("\nTesting CRC32\r\n\n");

    InitCrc32(base, 0xFFFFFFFFU);
    CRC_WriteData(base, (uint8_t *)&padded_msg[0], padded_len);
    checksum32 = CRC_Get32bitResult(base);

    PRINTF("CRC-32: 0x%08x\r\n", checksum32);

}

void aescrc_Dec(uint8_t *data, uint8_t *len,uint8_t *newconn,uint8_t *buf)
{
	CRC_Type *base = CRC0;
	uint32_t checksum32;
	size_t rev_data_enc_len;
	uint8_t rev_data_enc[512] = {0};
	//uint8_t rev_crc[4] = {0};
	long rev_crc = 0x00;
	struct AES_ctx ctx;

	//************************************
	//       Read Data Received
	//************************************
	PRINTF(" \r\n\n");


	//Copy only the data
	memcpy(rev_data_enc, data, len-4);
	rev_data_enc_len = strlen(rev_data_enc);

	// Get CRC in the message
    for (int count_crc = 0; count_crc < 4; count_crc++) {
    	for (int i = len; i >= rev_data_enc_len; i--){
    		//rev_crc[count_crc] = data[i];
    		rev_crc = rev_crc << 8;
    		rev_crc |= data[i];
    		count_crc++;
    	}
    }
    PRINTF("Received CRC-32: 0x%08x\r\n", rev_crc);

    // Print only the data
	PRINTF("Received Data Encrypted Message: %d\r\n");
	for(int i=0; i<rev_data_enc_len; i++) {
		PRINTF("0x%02x,", rev_data_enc[i]);
	}

	PRINTF("\r\n\n");

	//************************************
	//       Evaluation CRC
	//************************************

    InitCrc32(base, 0xFFFFFFFFU);
    CRC_WriteData(base, (uint8_t *)&rev_data_enc[0], rev_data_enc_len);
    checksum32 = CRC_Get32bitResult(base);
    PRINTF("Recalculated CRC-32: 0x%08x\r\n", checksum32);
    if (rev_crc == checksum32){
    	PRINTF("The Checksum is correct\r\n");
    	//netconn_send(newconn, buf);
    }
    else{
    	PRINTF("The both Checksums are NOT matching \r\n");
    }


	//************************************
	//       Obtetion Data
	//************************************

	AES_init_ctx(&ctx, key);

	AES_ECB_decrypt(&ctx, rev_data_enc);
	//AES_CBC_decrypt_buffer(&ctx, rev_data_enc, rev_data_enc_len);

	PRINTF("\r\nDecryption by AES128: %d\r\n", rev_data_enc);
	for(int i=0; i<rev_data_enc_len; i++) {
		PRINTF("0x%02x,", rev_data_enc[i]);
	}

	PRINTF("\r\n\n");

}



Server(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err;
  LWIP_UNUSED_ARG(arg);

  conn = netconn_new(NETCONN_TCP);
  netconn_bind(conn, IP_ADDR_ANY, 7);
  //LWIP_ERROR("tcpecho: invalid conn", (conn != NULL), return;);

  /* Tell connection to go into listening mode. */
  netconn_listen(conn);

  while (1) {

    /* Grab new connection. */
    err = netconn_accept(conn, &newconn);
    PRINTF("accepted new connection %p\r\n", newconn);

    /* Process the new connection. */
    if (err == ERR_OK) {
      struct netbuf *buf;
      void *data;
      u16_t len;

      while ((err = netconn_recv(newconn, &buf)) == ERR_OK) {

        do {
             netbuf_data(buf, &data, &len);
             //netconn_send(newconn, &buf);
             aescrc_Dec(data, len, newconn, &buf);
             err = netconn_write(newconn, data, len, NETCONN_COPY);
#if 0
            if (err != ERR_OK) {
              printf("tcpecho: netconn_write: error \"%s\"\n", lwip_strerr(err));
            }
#endif
        } while (netbuf_next(buf) >= 0);
        netbuf_delete(buf);
      }
      /*printf("Got EOF, looping\n");*/
      /* Close connection and discard connection identifier. */
      netconn_close(newconn);
      netconn_delete(newconn);
    }
  }
}
