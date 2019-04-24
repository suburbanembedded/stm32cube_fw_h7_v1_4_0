/*
 *  Copyright (C) 2018, STMicroelectronics, all right reserved.
 *  Copyright (C) 2006-2018, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "main.h"
#include "cmsis_os.h"

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_time       time
#define mbedtls_time_t     time_t 
#define mbedtls_fprintf    fprintf
#define mbedtls_printf     printf
#endif

#include <stdlib.h>
#include <string.h>

#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"

#if defined(MBEDTLS_SSL_CACHE_C)
#include "mbedtls/ssl_cache.h"
#endif


static mbedtls_net_context listen_fd, client_fd;
static uint8_t buf[1024];
static const char *pers = "ssl_server";
mbedtls_entropy_context entropy;
mbedtls_ctr_drbg_context ctr_drbg;
mbedtls_ssl_context ssl;
mbedtls_ssl_config conf;
mbedtls_x509_crt srvcert;
mbedtls_pk_context pkey;

#if defined(MBEDTLS_SSL_CACHE_C)
  mbedtls_ssl_cache_context cache;
#endif

void SSL_Server(void const *argument)
{
  int ret, len;
  UNUSED(argument);
  
  
#ifdef MBEDTLS_MEMORY_BUFFER_ALLOC_C
  mbedtls_memory_buffer_alloc_init(memory_buf, sizeof(memory_buf));
#endif
  mbedtls_net_init( &listen_fd );
  mbedtls_net_init( &client_fd );
  mbedtls_ssl_init( &ssl );
  mbedtls_ssl_config_init( &conf );
#if defined(MBEDTLS_SSL_CACHE_C)
  mbedtls_ssl_cache_init( &cache );
#endif
  mbedtls_x509_crt_init( &srvcert );
  mbedtls_pk_init( &pkey );
  mbedtls_entropy_init( &entropy );
  mbedtls_ctr_drbg_init( &ctr_drbg );

  /*
   * 1. Load the certificates and private RSA key
   */
  mbedtls_printf( "\n  . Loading the server cert. and key..." );

  /*
   * This demonstration program uses embedded test certificates.
   * Using mbedtls_x509_crt_parse_file() to read the server and CA certificates
   * resuires the implmentation of the File I/O API using the FatFs, that is 
   * not implemented yet.
   */

  ret = mbedtls_x509_crt_parse( &srvcert, (const unsigned char *) mbedtls_test_srv_crt, mbedtls_test_srv_crt_len );
  if(ret != 0)
  {
    mbedtls_printf( " failed\n  !  mbedtls_x509_crt_parse returned %d\n\n", ret );
    goto exit;
  }

  ret = mbedtls_x509_crt_parse(&srvcert, (const unsigned char *) mbedtls_test_cas_pem, mbedtls_test_cas_pem_len);
  if( ret != 0 )
  {
    mbedtls_printf(" failed\n  !  mbedtls_x509_crt_parse returned %d\n\n", ret);
    goto exit;
  }

  ret =  mbedtls_pk_parse_key(&pkey, (const unsigned char *) mbedtls_test_srv_key, mbedtls_test_srv_key_len, NULL, 0);

  if( ret != 0 )
  {
    mbedtls_printf(" failed\n  !  mbedtls_pk_parse_key returned %d\n\n", ret);
    goto exit;
  }

  mbedtls_printf( " ok\n" );

  /*
   * 2. Setup the listening TCP socket
   */
  mbedtls_printf( "  . Bind on https://localhost:%s/ ...", SERVER_PORT );

  if((ret = mbedtls_net_bind(&listen_fd, NULL, SERVER_PORT , MBEDTLS_NET_PROTO_TCP )) != 0)
  {
    mbedtls_printf( " failed\n  ! mbedtls_net_bind returned %d\n\n", ret );
    goto exit;
  }

  mbedtls_printf( " ok\n" );

  /*
   * 3. Seed the RNG
   */
  mbedtls_printf( "  . Seeding the random number generator..." );

  if((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *) pers, strlen( (char *)pers))) != 0)
  {
    mbedtls_printf( " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret );
    goto exit;
  }

  mbedtls_printf( " ok\n" );

  /*
   * 4. Setup stuff
   */
  mbedtls_printf( "  . Setting up the SSL data...." );

  if((ret = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
  {
    mbedtls_printf( " failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret );
    goto exit;
  }

  mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

#if defined(MBEDTLS_SSL_CACHE_C)
  mbedtls_ssl_conf_session_cache(&conf, &cache, mbedtls_ssl_cache_get, mbedtls_ssl_cache_set);
#endif

  mbedtls_ssl_conf_ca_chain(&conf, srvcert.next, NULL);
  if((ret = mbedtls_ssl_conf_own_cert(&conf, &srvcert, &pkey)) != 0)
  {
    mbedtls_printf( " failed\n  ! mbedtls_ssl_conf_own_cert returned %d\n\n", ret );
    goto exit;
  }

  if((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
  {
    mbedtls_printf( " failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret );
    goto exit;
  }

  mbedtls_printf( " ok\n" );

reset:
#ifdef MBEDTLS_ERROR_C
  if(ret != 0)
  {
    uint8_t error_buf[100];
    mbedtls_strerror( ret, (char *)error_buf, 100 );
    mbedtls_printf("Last error was: %d - %s\n\n", ret, error_buf );
  }
#endif
  
  mbedtls_net_free(&client_fd);

  mbedtls_ssl_session_reset(&ssl);

  /*
   * 5. Wait until a client connects
   */
  mbedtls_printf( "  . Waiting for a remote connection ...\n" );

  if((ret = mbedtls_net_accept(&listen_fd, &client_fd, NULL, 0, NULL)) != 0)
  {
    mbedtls_printf(" failed\n  ! mbedtls_net_accept returned %d\n\n", ret);
    goto exit;
  }

  mbedtls_ssl_set_bio(&ssl, &client_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

  mbedtls_printf(" ok\n");

  /*
   * 6. Handshake
   */
  mbedtls_printf("  . Performing the SSL/TLS handshake...");

  while((ret = mbedtls_ssl_handshake(&ssl)) != 0)
  {
    if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
    {
      mbedtls_printf(" failed\n  ! mbedtls_ssl_handshake returned %d\n\n", ret);
      goto reset;
    }
  }

  mbedtls_printf(" ok\n");

  /*
   * 7. Read the HTTP Request
   */
  mbedtls_printf("  < Read from client:");
  do
  {
    len = sizeof(buf) - 1;
    memset(buf, 0, sizeof(buf));
    BSP_LED_Off(LED_GREEN);
    BSP_LED_Off(LED_RED);
    ret = mbedtls_ssl_read(&ssl, buf, len);

    if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
    {
      continue;
    }
    if(ret <= 0)
    {
      switch(ret)
      {
        case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
          mbedtls_printf(" connection was closed gracefully\n");
          HAL_Delay(100);
          BSP_LED_On(LED_RED);
          break;

        case MBEDTLS_ERR_NET_CONN_RESET:
          mbedtls_printf(" connection was reset by peer\n");
          HAL_Delay(100);
          BSP_LED_On(LED_RED);
          break;

        default:
          mbedtls_printf(" mbedtls_ssl_read returned -0x%x\n", -ret);
          HAL_Delay(100);
          BSP_LED_On(LED_RED);
          break;
      }
      
      HAL_Delay(100);
      BSP_LED_On(LED_GREEN);
      break;
    }

    len = ret;
    mbedtls_printf(" %d bytes read\n\n%s", len, (char *) buf);

    if(ret > 0)
    {
      break;
    }
  } while(1);

  /*
   * 8. Write the 200 Response
   */
  mbedtls_printf( "  > Write to client:" );
  len = sprintf((char *) buf, HTTP_RESPONSE, mbedtls_ssl_get_ciphersuite(&ssl));

  while((ret = mbedtls_ssl_write(&ssl, buf, len)) <= 0)
  {
    if(ret == MBEDTLS_ERR_NET_CONN_RESET)
    {
      mbedtls_printf(" failed\n  ! peer closed the connection\n\n");
      goto reset;
    }

    if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
    {
      mbedtls_printf( " failed\n  ! mbedtls_ssl_write returned %d\n\n", ret );
      goto exit;
    }
  }

  len = ret;
  mbedtls_printf(" %d bytes written\n\n%s\n", len, (char *) buf);

  mbedtls_printf("  . Closing the connection...");

  while((ret = mbedtls_ssl_close_notify(&ssl)) < 0)
  {
    if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
    {
      mbedtls_printf( " failed\n  ! mbedtls_ssl_close_notify returned %d\n\n", ret );
      goto reset;
    }
  }

  if (ret == 0)
  {
    BSP_LED_On(LED_GREEN);
    mbedtls_printf( " ok\n" );
  }
  
  ret = 0;
  goto reset;

exit:
  BSP_LED_Off(LED_GREEN);
  BSP_LED_On(LED_RED);

  mbedtls_net_free( &client_fd );
  mbedtls_net_free( &listen_fd );

  mbedtls_x509_crt_free( &srvcert );
  mbedtls_pk_free( &pkey );
  mbedtls_ssl_free( &ssl );
  mbedtls_ssl_config_free( &conf );
#if defined(MBEDTLS_SSL_CACHE_C)
  mbedtls_ssl_cache_free( &cache );
#endif
  mbedtls_ctr_drbg_free( &ctr_drbg );
  mbedtls_entropy_free( &entropy );

}
