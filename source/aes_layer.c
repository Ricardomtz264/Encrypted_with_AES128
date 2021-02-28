/*


*/


/*****************************************************************************/
/* Includes:                                                                 */
/*****************************************************************************/
#include <string.h> // CBC mode, for memset
#include "aes.h"
#include "aes_layer.h"

/*
void decrypt(uint8_t *data, uint8_t *len,uint8_t *newconn,uint8_t *buf)
{
	CRC_Type *base = CRC0;
	uint32_t checksum32;
	size_t rev_data_enc_len;
	uint8_t rev_data_enc[512] = {0};
	//uint8_t rev_crc[4] = {0};
	long rev_crc = 0x00;
	struct AES_ctx ctx;

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



	AES_init_ctx(&ctx, key);

	AES_ECB_decrypt(&ctx, rev_data_enc);
	//AES_CBC_decrypt_buffer(&ctx, rev_data_enc, rev_data_enc_len);

	PRINTF("\r\nDecryption by AES128: %d\r\n", rev_data_enc);
	for(int i=0; i<rev_data_enc_len; i++) {
		PRINTF("0x%02x,", rev_data_enc[i]);
	}

	PRINTF("\r\n\n");

}


void encrypt(void *arg)
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


accept_comm(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err;
  LWIP_UNUSED_ARG(arg);

  conn = netconn_new(NETCONN_TCP);
  netconn_bind(conn, IP_ADDR_ANY, 7);
  netconn_listen(conn);


	err = netconn_accept(conn, &newconn);
	PRINTF("accepted new connection %p\r\n", newconn);

	return newconn;
}


rev_comm(uint8_t *newconn)
{
	struct netbuf *buf;
	void *data;
	u16_t len;

	while ((err = netconn_recv(newconn, &buf)) == ERR_OK) {

	  do {
	       netbuf_data(buf, &data, &len);
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

	return newconn;
}


Server(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err;
  LWIP_UNUSED_ARG(arg);

  conn = netconn_new(NETCONN_TCP);
  netconn_bind(conn, IP_ADDR_ANY, 7);
  netconn_listen(conn);

  while (1) {

    err = netconn_accept(conn, &newconn);
    PRINTF("accepted new connection %p\r\n", newconn);

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
      netconn_close(newconn);
      netconn_delete(newconn);
    }
  }
}
*/

