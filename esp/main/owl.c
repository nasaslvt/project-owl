#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"
#include <esp_http_server.h>
#include <esp_camera.h>
#include <esp_timer.h>
#include <string.h>
#include <dirent.h>
#include<unistd.h>

#include <driver/sdmmc_host.h>
#include <driver/sdmmc_defs.h>
#include <sdmmc_cmd.h>
#include <esp_vfs_fat.h>

#define WIFI_SSID "owl"
#define WIFI_PASS "str1g1f0rm35"

#define WIFI_STATIC_IP_ADDR "192.168.8.10"
#define WIFI_STATIC_NETMASK_ADDR "255.255.255.0"
#define WIFI_STATIC_GW_ADDR "192.168.8.1"

#define CAM_PIN_PWDN 32
#define CAM_PIN_RESET -1 //software reset will be performed
#define CAM_PIN_XCLK 0
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static const char *TAG = "owl";

static camera_config_t camera_config = {
    .pin_pwdn  = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    .xclk_freq_hz = 10000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_QSXGA,

    .jpeg_quality = 10,
    .fb_count = 2,
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_LATEST
};

static esp_err_t init_camera() {

    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }

    sensor_t * s = esp_camera_sensor_get();
    s->set_vflip(s, 1);

    return ESP_OK;
}

static esp_err_t init_sdcard()
{
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 3,
    };
    sdmmc_card_t *card;

    ESP_LOGI(TAG, "Mounting SD card...");
    esp_err_t err = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to mount SD card VFAT filesystem. Error: %s", esp_err_to_name(err));
    }

    return ESP_OK;
}

static esp_err_t parse_get(httpd_req_t *req, char **obuf)
{
    char *buf = NULL;
    size_t buf_len = 0;

    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = (char *)malloc(buf_len);
        if (!buf) {
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            *obuf = buf;
            return ESP_OK;
        }
        free(buf);
    }
    return ESP_OK;
}
/*
esp_err_t capture_handler(httpd_req_t *req) {

    char *buf = NULL;
    char vflip[1];
    char value[32];

    sensor_t *s = esp_camera_sensor_get();

    if (parse_get(req, &buf) != ESP_OK) {
        return ESP_FAIL;
    }

    httpd_query_key_value(buf, "vflip", vflip, sizeof(vflip));
    httpd_query_key_value(buf, "filter", value, sizeof(value));

    if (vflip[0] == '1') {
        s->set_vflip(s, 0);
    } else {
        s->set_vflip(s, 1);
    }

    s->set_special_effect(s, 0);
    if (strcmp(value, "grayscale")) {
        s->set_special_effect(s, 2);
    }
    if (strcmp(value, "edge")) {
        s->set_special_effect(s, 6);
    }

    free(buf);

    ESP_LOGI(TAG, "Taking picture...");

    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    int64_t fr_start = esp_timer_get_time();

    fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    char *pic_name = malloc(17 + sizeof(int64_t));
    sprintf(pic_name, "/sdcard/pic_%lli.jpg", fr_start);

    FILE *file = fopen(pic_name, "w");
    if (file != NULL)
    {
        ESP_LOGI(TAG, "Fwrite %d", fwrite(fb->buf, 1, fb->len, file));
        ESP_LOGI(TAG, "File saved");
    }
    else
    {
        ESP_LOGE(TAG, "Could not open file =(");
        httpd_resp_send_500(req);
        res = ESP_FAIL;
    }
    fclose(file);
    free(pic_name);

    ESP_LOGI(TAG, "Picture taken! Its size was: %zu bytes", fb->len);
    esp_camera_fb_return(fb);

    int64_t fr_end = esp_timer_get_time();
    ESP_LOGI(TAG, "JPG: %zuKB %ums", fb->len, (uint32_t)((fr_end - fr_start)/1000));
    
    const char resp[] = "Picture Saved";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

    s->set_vflip(s, 1);
    s->set_special_effect(s, 0);

    return res;
}

esp_err_t file_handler(httpd_req_t *req) {

    char *buf = NULL;
    char value[32];

    if (parse_get(req, &buf) != ESP_OK) {
        return ESP_FAIL;
    }

    esp_err_t err = httpd_query_key_value(buf, "fname", value, sizeof(value));

    free(buf);

    if (err == ESP_ERR_NOT_FOUND) {
        return ESP_OK;
    }

    char *pic_name = malloc(13 + sizeof(value));
    sprintf(pic_name, "/sdcard/%s.jpg", value);

    FILE *file = fopen(pic_name, "r");
    if (file != NULL)
    {
        ESP_LOGI(TAG, "File opened");
    }
    else
    {
        ESP_LOGE(TAG, "Could not open file %s", pic_name);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(size);
    fread(buffer, 1, size, file);
    fclose(file);
    free(pic_name);

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_send(req, buffer, size);
    free(buffer);

    return ESP_OK;
}

esp_err_t list_handler(httpd_req_t *req) {

    char *buf = NULL;
    char value[32];

    if (parse_get(req, &buf) != ESP_OK) {
        return ESP_FAIL;
    }

    esp_err_t err = httpd_query_key_value(buf, "action", value, sizeof(value));

    free(buf);

    if (err == ESP_ERR_NOT_FOUND) {
        return ESP_OK;
    }

    if (strcmp(value, "list") == 0) {
        DIR *dir;
        struct dirent *ent;
        char *html_buffer = malloc(1024);
        char *html_buffer_ptr = html_buffer;

        if ((dir = opendir("/sdcard")) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                if (ent->d_type == DT_REG) {
                    html_buffer_ptr += sprintf(html_buffer_ptr, "<p>%s</p>", ent->d_name);
                }
            }
            closedir(dir);
        }

        httpd_resp_set_type(req, "text/html");
        httpd_resp_send(req, html_buffer, strlen(html_buffer));
        free(html_buffer);
    }
    else if (strcmp(value, "delete") == 0) {
        char *file_name = malloc(32);
        err = httpd_query_key_value(buf, "fname", file_name, 32);
        if (err == ESP_OK) {
            char *file_path = malloc(13 + strlen(file_name));
            sprintf(file_path, "/sdcard/%s.jpg", file_name);
            if (remove(file_path) == 0) {
                httpd_resp_sendstr(req, "File deleted successfully");
            }
            else {
                httpd_resp_send_500(req);
            }
            free(file_path);
        }
        else {
            httpd_resp_send_500(req);
        }
        free(file_name);
    }
    else {
        httpd_resp_send_500(req);
    }

    return ESP_OK;
}
*/

esp_err_t camera_get_handler(httpd_req_t *req) {
    
    sensor_t * s = esp_camera_sensor_get();
    camera_sensor_info_t * info = esp_camera_sensor_get_info(&s->id);

    char json_response[256];
    sprintf(json_response, "{\n\t\"name\": \"%s\"\n}", info->name);

    httpd_resp_send(req, json_response, strlen(json_response));
    return ESP_OK;
}

static const httpd_uri_t camera_get_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = camera_get_handler,
    .user_ctx  = NULL
};

esp_err_t camera_stream_get_handler(httpd_req_t *req)
{
    sensor_t * s = esp_camera_sensor_get();
    s->set_framesize(s, FRAMESIZE_HD);

    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len;
    uint8_t * _jpg_buf;
    char * part_buf[64];
    static int64_t last_frame = 0;
    if(!last_frame) {
        last_frame = esp_timer_get_time();
    }

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if(res != ESP_OK){
        return res;
    }

    while(true){
        fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            res = ESP_FAIL;
            break;
        }
        if(fb->format != PIXFORMAT_JPEG){
            bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
            if(!jpeg_converted){
                ESP_LOGE(TAG, "JPEG compression failed");
                esp_camera_fb_return(fb);
                res = ESP_FAIL;
            }
        } else {
            _jpg_buf_len = fb->len;
            _jpg_buf = fb->buf;
        }

        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if(res == ESP_OK){
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);

            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if(fb->format != PIXFORMAT_JPEG){
            free(_jpg_buf);
        }
        esp_camera_fb_return(fb);
        if(res != ESP_OK){
            break;
        }
        int64_t fr_end = esp_timer_get_time();
        int64_t frame_time = fr_end - last_frame;
        last_frame = fr_end;
        frame_time /= 1000;
        ESP_LOGI(TAG, "MJPG: %uKB %ums (%.1ffps)",
            (uint32_t)(_jpg_buf_len/1024),
            (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time);
    }

    last_frame = 0;
    return res;
}

static const httpd_uri_t camera_stream_get_uri = {
    .uri       = "/stream",
    .method    = HTTP_GET,
    .handler   = camera_stream_get_handler,
    .user_ctx  = NULL
};

esp_err_t camera_record_get_handler(httpd_req_t *req)
{
    sensor_t * s = esp_camera_sensor_get();
    s->set_framesize(s, FRAMESIZE_SVGA);

    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    int64_t fr_start;

    char *vid_name = malloc(17 + sizeof(int64_t));
    sprintf(vid_name, "/sdcard/vid_%lli.jpg", esp_timer_get_time());
    FILE *file = fopen(vid_name, "w");

    while(true) {
        fb = esp_camera_fb_get();
        if (fb) {
            fr_start = esp_timer_get_time();
            fwrite(fb->buf, 1, fb->len, file);
            esp_camera_fb_return(fb);
            ESP_LOGI(TAG, "JPG: %zuKB %ums", fb->len/1024, (uint32_t)((esp_timer_get_time() - fr_start)/1000));
        }
    }
    free(vid_name);
    fclose(file);
}

static const httpd_uri_t camera_record_get_uri = {
    .uri       = "/record",
    .method    = HTTP_GET,
    .handler   = camera_record_get_handler,
    .user_ctx  = NULL
};


static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 8080;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    esp_err_t res = httpd_start(&server, &config);
    if (res == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &camera_get_uri);
        httpd_register_uri_handler(server, &camera_record_get_uri);
        httpd_register_uri_handler(server, &camera_stream_get_uri);
        return server;
    }
    ESP_LOGI(TAG, "Error starting server: %s", esp_err_to_name(res));
    return NULL;
}

static esp_err_t stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        if (stop_webserver(*server) == ESP_OK) {
            *server = NULL;
        } else {
            ESP_LOGE(TAG, "Failed to stop http server");
        }
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

static void set_static_ip(esp_netif_t *netif)
{
    if (esp_netif_dhcpc_stop(netif) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop dhcp client");
        return;
    }
    esp_netif_ip_info_t ip;
    memset(&ip, 0 , sizeof(esp_netif_ip_info_t));
    ip.ip.addr = ipaddr_addr(WIFI_STATIC_IP_ADDR);
    ip.netmask.addr = ipaddr_addr(WIFI_STATIC_NETMASK_ADDR);
    ip.gw.addr = ipaddr_addr(WIFI_STATIC_GW_ADDR);
    if (esp_netif_set_ip_info(netif, &ip) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set ip info");
        return;
    }
    ESP_LOGD(TAG, "Success to set static ip: %s, netmask: %s, gw: %s", WIFI_STATIC_IP_ADDR, WIFI_STATIC_NETMASK_ADDR, WIFI_STATIC_GW_ADDR);
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_START) {
            esp_err_t err = esp_wifi_connect();
            ESP_LOGI(TAG, "esp_wifi_connect: %s", esp_err_to_name(err));
        }
        else if (event_id == WIFI_EVENT_STA_CONNECTED) {
            set_static_ip(arg);
        }
        else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
            esp_err_t err = esp_wifi_connect();
            ESP_LOGI(TAG, "esp_wifi_connect: %s", esp_err_to_name(err));
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    }
}

static void init_wifi() {

    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        sta_netif,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        sta_netif,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
          //.threshold.authmode = WIFI_AUTH_WEP,
        },
    };
    ESP_LOGI(TAG, "Connecting to: %s", WIFI_SSID);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "init_wifi finished.");
}

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    //Initialize the underlying TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());

    //Create default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    init_wifi();
    init_sdcard();
    init_camera();

    static httpd_handle_t server;

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));

    /* Start the server for the first time */
    server = start_webserver();
}