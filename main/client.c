#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_task_wdt.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/esp_debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"

#include "utils.h"
#include "network.h"
#include "client.h"



static const char *TAG = "webclient";

static const char *REQUEST = "POST " WEB_URL " HTTP/1.0\r\n"
    "Host: "WEB_SERVER"\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "Content-Type: application/x-www-form-urlencoded\r\n"
    "Content-Length: %d\r\n"
    "\r\n"
    "%s\r\n";

static const char *RESPONSE_OK = "HTTP/1.0 200";


/* Root cert for forms.google.com, taken from server_root_cert.pem

   The PEM file was extracted from the output of this command:
   openssl s_client -showcerts -connect forms.google.com:443 </dev/null

   The CA root cert is the last cert given in the chain of certs.

   To embed it in the app binary, the PEM file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/
extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");
extern const uint8_t server_root_cert_pem_end[]   asm("_binary_server_root_cert_pem_end");

//simply double the template on the assumption that that will be enough for sprintf-processed string
#define WEB_POSTDATA_SIZE_REFERENCE WEB_POSTDATA_TEMPLATE WEB_POSTDATA_TEMPLATE

static TaskHandle_t xHandle = NULL;
static client_config_t config;

// the double string will be an initial value that defines its size and will be overwritten
static char body[] = WEB_POSTDATA_SIZE_REFERENCE; //assume that this should be enough


static void get_request_body(char * buf) {
//    char temp[10];

	ESP_LOGD(TAG, "Size of body array is %d", sizeof(body));

    config.callback_gen_body((char *)&body);

    sprintf(buf, REQUEST , strlen(body), body);
    ESP_LOGD(TAG, "strlen body: %d, request: %d", strlen(body), strlen(buf));
}


 void https_post_task(void *pvParameters) {
    char request[1024];
    char buf[512];
    int ret, flags, len;

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_x509_crt cacert;
    mbedtls_ssl_config conf;
    mbedtls_net_context server_fd;

    mbedtls_ssl_init(&ssl);
    mbedtls_x509_crt_init(&cacert);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    ESP_LOGI(TAG, "Seeding the random number generator");

    mbedtls_ssl_config_init(&conf);

    mbedtls_entropy_init(&entropy);
    if((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                    NULL, 0)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ctr_drbg_seed returned %d", ret);
        abort();
    }

    ESP_LOGI(TAG, "Loading the CA root certificate...");

    ret = mbedtls_x509_crt_parse(&cacert, server_root_cert_pem_start,
                                 server_root_cert_pem_end-server_root_cert_pem_start);

    if(ret < 0)
    {
        ESP_LOGE(TAG, "mbedtls_x509_crt_parse returned -0x%x\n\n", -ret);
        abort();
    }

    ESP_LOGI(TAG, "Setting hostname for TLS session...");

     /* Hostname set here should match CN in server certificate */
    if((ret = mbedtls_ssl_set_hostname(&ssl, WEB_SERVER)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ssl_set_hostname returned -0x%x", -ret);
        abort();
    }

    ESP_LOGI(TAG, "Setting up the SSL/TLS structure...");

    if((ret = mbedtls_ssl_config_defaults(&conf,
                                          MBEDTLS_SSL_IS_CLIENT,
                                          MBEDTLS_SSL_TRANSPORT_STREAM,
                                          MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ssl_config_defaults returned %d", ret);
        goto exit;
    }
    /* MBEDTLS_SSL_VERIFY_OPTIONAL is bad for security, in this example it will print
       a warning if CA verification fails but it will continue to connect.

       You should consider using MBEDTLS_SSL_VERIFY_REQUIRED in your own code.
    */
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
#ifdef CONFIG_MBEDTLS_DEBUG
    mbedtls_esp_enable_debug_log(&conf, 4);
#endif

    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ssl_setup returned -0x%x\n\n", -ret);
        goto exit;
    }

    ESP_LOGI(TAG, "Initial Submit Delay: %d s", INITIAL_DELAY / 1000);
    delay(INITIAL_DELAY );


    while(1) {
        ESP_LOGI(TAG, "While begins");
        wifi_exclusive_start(TAG);

        /* Wait for the callback to set the CONNECTED_BIT in the
           event group.
        */
        wifi_await_connection();

        ESP_LOGI(TAG, "Ensured connection to AP");

        mbedtls_net_init(&server_fd);

        ESP_LOGI(TAG, "Connecting to %s:%s%s", WEB_SERVER, WEB_PORT, WEB_URL);

        if ((ret = mbedtls_net_connect(&server_fd, WEB_SERVER,
                                      WEB_PORT, MBEDTLS_NET_PROTO_TCP)) != 0)
        {
            ESP_LOGE(TAG, "mbedtls_net_connect returned -%x", -ret);
            goto exit;
        }

        ESP_LOGD(TAG, "Connected.");

        mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

        ESP_LOGD(TAG, "Performing the SSL/TLS handshake...");

        while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
        {
            if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                ESP_LOGE(TAG, "mbedtls_ssl_handshake returned -0x%x", -ret);
                goto exit;
            }
        }

        if (WEB_VERIFY_SSL) {
            ESP_LOGD(TAG, "Verifying peer X.509 certificate...");

            if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
            {
                /* In real life, we probably want to close connection if ret != 0 */
                ESP_LOGW(TAG, "Failed to verify peer certificate!");
                bzero(buf, sizeof(buf));
                mbedtls_x509_crt_verify_info(buf, sizeof(buf), "  ! ", flags);
                ESP_LOGW(TAG, "verification info: %s", buf);
                goto exit;
            }
            else {
                ESP_LOGI(TAG, "Certificate verified.");
            }
        }

        ESP_LOGD(TAG, "Cipher suite is %s", mbedtls_ssl_get_ciphersuite(&ssl));


        get_request_body(request);

        ESP_LOGI(TAG, "Writing HTTP request which is %d bytes", strlen(request));
        ESP_LOGD(TAG, "REQUEST:\n%s", request);

        size_t written_bytes = 0;
        do {
            ret = mbedtls_ssl_write(&ssl,
                                    (const unsigned char *)request + written_bytes,
                                    strlen(request) - written_bytes);
            if (ret >= 0) {
                ESP_LOGD(TAG, "%d bytes written", ret);
                written_bytes += ret;
            } else if (ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != MBEDTLS_ERR_SSL_WANT_READ) {
                ESP_LOGE(TAG, "mbedtls_ssl_write returned -0x%x", -ret);
                goto exit;
            }
        } while(written_bytes < strlen(request));

        ESP_LOGD(TAG, "Reading HTTP response...");

        do
        {
            len = sizeof(buf) - 1;
            bzero(buf, sizeof(buf));
            ret = mbedtls_ssl_read(&ssl, (unsigned char *)buf, len);

            if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
                continue;

            if(ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
                ret = 0;
                break;
            }

            if(ret < 0)
            {
                ESP_LOGE(TAG, "mbedtls_ssl_read returned -0x%x", -ret);
                break;
            }

            if(ret == 0)
            {
                ESP_LOGI(TAG, "connection closed");
                break;
            }

            len = ret;

            //let's confirm that it's a 200 and break
            if (strncmp(buf, RESPONSE_OK, strlen(RESPONSE_OK)) == 0) {
                ESP_LOGI(TAG, "Response was OK in the first %d chars, skipping the rest of the output", len);
                ESP_LOGD(TAG, "RESPONSE:\n%s", buf);
                ret = 0;
                break;
            }

            ESP_LOGI(TAG, "Dumping response because it was not OK");
            ESP_LOGI(TAG, "%d bytes read", len);
            /* Print response directly to stdout as it is read */
            for(int i = 0; i < len; i++) {
                putchar(buf[i]);
            }
        } while(1);

        mbedtls_ssl_close_notify(&ssl);

        esp_task_wdt_reset(); //feed the watchdog

//        post_request_hook(main_data);
        config.callback_post_request();

    exit:
		wifi_exclusive_end(TAG);
        mbedtls_ssl_session_reset(&ssl);
        mbedtls_net_free(&server_fd);



        if(ret != 0)
        {
            mbedtls_strerror(ret, buf, 100);
            ESP_LOGE(TAG, "Last error was: -0x%x - %s", -ret, buf);
        }

        putchar('\n'); // JSON output doesn't have a newline at end

        static int request_count;
        ESP_LOGI(TAG, "Completed %d requests", ++request_count);
        ESP_LOGD(TAG, "Free Heap Memory: %u", xPortGetFreeHeapSize());

//        delay(SUBMIT_FREQUENCY * 60000 );

        xTaskNotifyWait( 0x00,      /* Don't clear any notification bits on entry. */
			ULONG_MAX, /* Reset the notification value to 0 on exit. */
			NULL, /* Notified value pass out in
							  ulNotifiedValue. */
			SUBMIT_FREQUENCY * 1000 / portTICK_RATE_MS);  /* Blocking delay. */


        ESP_LOGI(TAG, "Starting again!");

    }
}

/**
 * Notifies the wait in the request loop to resume now and
 * proceed to the next request.
 */
void client_force_request_now() {
	ESP_LOGI(TAG, "Forcing request now, waking up from wait");
	xTaskNotify( xHandle, 0, eNoAction );
}


void start_client(void (*callback_gen_body)(char *), void (*callback_post_request)()) {
	stop_client();

	config.callback_gen_body = callback_gen_body;
	config.callback_post_request = callback_post_request;

    xTaskCreate(&https_post_task, "https_post_task", 8192, NULL, 15, &xHandle);

    ESP_ERROR_CHECK(esp_task_wdt_add(xHandle));
}

void stop_client() {
	if (xHandle != NULL) {
        esp_task_wdt_delete(xHandle);
		vTaskDelete(xHandle);
		xHandle = NULL;
	}
}
