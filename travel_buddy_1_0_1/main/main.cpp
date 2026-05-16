// #include "esp_netif_sntp.h"
// #include "nvs_flash.h"
// #include "bsp/esp-bsp.h"
// #include "bsp_board_extra.h"
// #include "app_music/lvgl_music.h"
// #include "app_msg/sys_msg_ui.h"
// #include "app_eaf/eaf_test.h"
// #include "wake_word_det/wake_word_drv.h"

// #define TAG   "main"


// #define DA380_I2C_ADDRESS   0x27


// // 初始化总线和设备
// static i2c_master_bus_handle_t bus_handle;
// static i2c_master_dev_handle_t dev_handle;

// esp_err_t da380_bus_init(void)
// {
//     i2c_device_config_t dev_cfg = {
//         .dev_addr_length = I2C_ADDR_BIT_LEN_7,
//         .device_address = DA380_I2C_ADDRESS,
//         .scl_speed_hz = 100000,
//     };
//     return i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle);
// }

// static esp_err_t da380_write_byte(uint8_t reg, uint8_t value)
// {
//     uint8_t buf[2] = {reg, value};
//     return i2c_master_transmit(dev_handle, buf, sizeof(buf), -1);
// }

// static esp_err_t da380_read_byte(uint8_t reg, uint8_t *value)
// {
//     return i2c_master_transmit_receive(dev_handle, &reg, 1, value, 1, -1);
// }

// esp_err_t da380_init(void)
// {
//     ESP_ERROR_CHECK(da380_write_byte(0x0F, 0x03));
//     ESP_ERROR_CHECK(da380_write_byte(0x10, 0x0F));
//     return da380_write_byte(0x11, 0x1F);
// }

// esp_err_t da380_read_accel(float *ax, float *ay, float *az)
// {
//     uint8_t xlo, xhi, ylo, yhi, zlo, zhi;
//     da380_read_byte(0x03, &xlo);
//     da380_read_byte(0x02, &xhi);
//     da380_read_byte(0x05, &ylo);
//     da380_read_byte(0x04, &yhi);
//     da380_read_byte(0x07, &zlo);
//     da380_read_byte(0x06, &zhi);

//     int16_t raw_x = ((int16_t)(xlo << 8 | xhi)) >> 2;
//     int16_t raw_y = ((int16_t)(ylo << 8 | yhi)) >> 2;
//     int16_t raw_z = ((int16_t)(zlo << 8 | zhi)) >> 2;

//     *ax = (raw_x / 512.0f) * 9.80665f;
//     *ay = (raw_y / 512.0f) * 9.80665f;
//     *az = (raw_z / 512.0f) * 9.80665f;
//     return ESP_OK;
// }

// void app_main(void)
// {

//     // esp_err_t ret = nvs_flash_init();
//     // if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
//     //     ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//     //     ESP_ERROR_CHECK(nvs_flash_erase());
//     //     ret = nvs_flash_init();
//     // }

//     // lv_display_t *disp = bsp_display_start();
//     // lv_indev_t *tp = bsp_display_get_input_dev();
//     // bsp_display_backlight_on();
//     // bsp_sdcard_mount();
    
//     // Audio_Play_Init();
//     // msg_driver_init();
//     // wake_word_drv_init();

//     // esp_err_t err = get_file_list_by_ext("/sdcard/music",".mp3",&mp3_files);
//     // if (err == ESP_OK) {
//     //     for (int i = 0; i < mp3_files.count; i++) {
//     //         printf("MP3[%d]: %s\n", i, mp3_files.list[i]);
//     //     }
//     // }

//     // bsp_display_lock(-1);

//     // main_screen = lv_obj_create(NULL);
//     // lv_obj_t * tv = lv_tileview_create(main_screen);

//     // lv_obj_t * tile1 = lv_tileview_add_tile(tv, 0, 0, LV_DIR_RIGHT);
//     // msg_ui_screen_init(main_screen,tile1);
    

//     // lv_obj_t * tile2 = lv_tileview_add_tile(tv, 1, 0, LV_DIR_LEFT | LV_DIR_RIGHT);
//     // init_music_ui_screen(tile2,&mp3_files);

//     // lv_obj_t * tile3 = lv_tileview_add_tile(tv, 2, 0, LV_DIR_LEFT);
//     // eaf_ui_screen_init(tile3);

//     // lv_screen_load(main_screen);

//     // bsp_display_unlock();
// }




#include "esp_log.h"
#include "at_modem.h"
static const char *TAG = "ML307_DEMO";

void TestHttp(std::unique_ptr<AtModem>& modem) {
    ESP_LOGI(TAG, "开始 HTTP 测试");

    // 创建 HTTP 客户端
    auto http = modem->CreateHttp(0);
    
    // 设置请求头
    http->SetHeader("User-Agent", "Xiaozhi/3.0.0");
    http->SetTimeout(10000);
    
    // 发送 GET 请求
    if (http->Open("GET", "https://httpbin.org/json")) {
        ESP_LOGI(TAG, "HTTP 状态码: %d", http->GetStatusCode());
        ESP_LOGI(TAG, "响应内容长度: %zu bytes", http->GetBodyLength());
        
        // 读取响应内容
        std::string response = http->ReadAll();
        ESP_LOGI(TAG, "响应内容: %s", response.c_str());
        
        http->Close();
    } else {
        ESP_LOGE(TAG, "HTTP 请求失败");
    }
    
    // unique_ptr 会自动释放内存，无需手动 delete
}




extern "C" void app_main(void) {
    // 自动检测并初始化模组
    auto modem = AtModem::Detect(GPIO_NUM_12, GPIO_NUM_13);
    
    if (!modem) {
        ESP_LOGE(TAG, "模组检测失败");
        return;
    }
    
    // 设置网络状态回调
    modem->OnNetworkStateChanged([](bool ready) {
        ESP_LOGI(TAG, "网络状态: %s", ready ? "已连接" : "已断开");
    });
    
    // 等待网络就绪
    NetworkStatus status = modem->WaitForNetworkReady(30000);
    if (status != NetworkStatus::Ready) {
        ESP_LOGE(TAG, "网络连接失败");
        return;
    }
    
    // 打印模组信息
    ESP_LOGI(TAG, "模组版本: %s", modem->GetModuleRevision().c_str());
    ESP_LOGI(TAG, "IMEI: %s", modem->GetImei().c_str());
    ESP_LOGI(TAG, "ICCID: %s", modem->GetIccid().c_str());
    ESP_LOGI(TAG, "运营商: %s", modem->GetCarrierName().c_str());
    ESP_LOGI(TAG, "信号强度: %d", modem->GetCsq());

    TestHttp(modem);
}
