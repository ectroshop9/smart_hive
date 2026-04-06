#include "display.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "lvgl.h"

// تعريف الـ Mutex المطلوب
SemaphoreHandle_t lvgl_mutex = xSemaphoreCreateRecursiveMutex();

extern "C" {
    void display_create_ui() { /* UI Logic */ }
    
    // التصحيح: استلام مؤشر لهيكل البيانات بدلاً من أرقام منفصلة
    void display_update_hive(const hive_data_t* data) { 
        if (data) {
            // هنا سيتم تحديث LVGL لاحقاً بالبيانات من data->temperature و data->humidity
        }
    }
    
    void display_update_ai_status(const char* status) { /* Status update */ }
    void display_timer_handler() { lv_timer_handler(); }
    void display_set_selected_hive(int id) { /* Selection logic */ }
}

esp_lcd_panel_t* display_init() {
    esp_lcd_panel_io_spi_config_t io_config = {};
    io_config.cs_gpio_num = 10;
    io_config.dc_gpio_num = 11;
    io_config.spi_mode = 0;
    io_config.pclk_hz = 40 * 1000 * 1000;
    io_config.trans_queue_depth = 10;
    io_config.lcd_cmd_bits = 8;
    io_config.lcd_param_bits = 8;

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle);

    esp_lcd_panel_dev_config_t panel_config = {};
    panel_config.reset_gpio_num = 12;
    panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
    panel_config.bits_per_pixel = 16;

    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle);

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_disp_on_off(panel_handle, true);

    return (esp_lcd_panel_t*)panel_handle;
}
