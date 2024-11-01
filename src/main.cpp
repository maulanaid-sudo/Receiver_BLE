#include <Arduino.h>
#if defined ESP32
#include <WiFi.h>
#include <esp_wifi.h>
#endif //ESP32
#include <QuickEspNow.h>
#include <M5Unified.h>
#include <lvgl.h>
#include <esp_timer.h>
#include <vector>
#include <algorithm> // untuk std::distance
#include <lvgl_porting.h>
#include <receive.h>

#define EXAMPLE_LVGL_TICK_PERIOD_MS 2    /*!< LVGL tick period in ms */
static lv_indev_drv_t lv_indev;
SemaphoreHandle_t lvglMutex;


void setup() {
    Serial.begin(115200);
    lvglMutex = xSemaphoreCreateMutex();
    M5.begin();
    M5.Display.begin();
    M5.Display.setEpdMode(epd_mode_t::epd_fastest);
    lvgl_init();

    // Timer untuk lv_tick_inc
   /*
    xTaskCreate([](void*) {
        while (true) {
            lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
            vTaskDelay(EXAMPLE_LVGL_TICK_PERIOD_MS / portTICK_PERIOD_MS);
        }
    }, "lv_tick_task", 4096, nullptr, 1, nullptr);
*/
    WiFi.mode(WIFI_MODE_STA);
    WiFi.disconnect(false, true);

    quickEspNow.begin(1, 0, false);
    quickEspNow.onDataRcvd(dataReceived);
    
}

void loop() {
   /* if (xSemaphoreTake(lvglMutex, portMAX_DELAY)) {
        lv_task_handler();
        xSemaphoreGive(lvglMutex);
    }
    */
   lv_task_handler();   
   checkTimeout();  // Cek status timeout untuk pengirim
  //  handleBeep();
     vTaskDelay(1);
}
