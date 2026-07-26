#include <WiFi.h>
#include "Arduino.h"
// As bibliotecas "soc" são necessárias para desabilitar o recurso 
// de Brownout do ESP32-CAM
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// Substituir pelas credenciais da rede Wi-Fi
const char* ssid = "SUBSTITUA_PELO_SSID_DA_REDE_WIFI"
const char* pswd = "SUBSTITUA_PELA_SENHA_DA_REDE_WIFI"

void startCameraServer();
// void setupLedFlash(int pin);

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();

    camera_config_t config
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_UXGA;
    // config.pixel_format = PIXFORMAT_JPEG; // para streaming
    config.pixel_format = PIXFORMAT_RGB565; // para reconhecimento facial
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    // config.jpeg_quality = 12;
    config.fb_count = 1;


    /*
        Se o IC PSRAM estiver presente, inicializa com resolucão UXGA
        e maior qualidade de JPEG para um maior frame buffer pre-alocado
    */
    if (config.pixel_format == PIXFORMAT_JPEG){
        if(psramFound()){
            config.jpeg_quality = 10;
            config.fb_count = 2;
            config.grab_mode = CAMERA_GRAB_LATEST;
        } else {
            // Limita o frame size se o PSRAM não estiver presente
            config.frame_size = FRAMESIZE_SVGA;
            config.fb_location = CAMERA_FB_IN_DRAM;
        }
    } else {
        // Melhor opção para detecção facial
        config.frame_size = FRAMESIZE_240X240;
        #if CONFIG_IDF_TARGET_ESP32S3
            config.fb_count = 2;
        #endif
    }

    
    #if defined(CAMERA_MODEL_ESP_EYE)
        pinMode(13, INPUT_PULLUP);
        pinMode(14, INPUT_PULLUP);
    #endif


    // Inicializa a câmera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Inicialização da câmera falhou com erro 0x%x", err);
        return;
    }


    // Sensores estão inicialmente virados verticalmente e com cores saturadas
    sensor_t *s = esp_camera_sensor_get();
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1); // vira de volta
        s->set_brightness(s, 1); // aumenta um pouquinho o brilho
        s->set_saturation(s, -2); // diminui saturação
    }
    // Diminui o frame size no começo para ter maior frame rate inicial
    if (config.pixel_format == PIXFORMAT_JPEG){
        s->set_framesize(s, FRAMESIZE_QVGA);
    }

    #if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
    #endif

    #if defined(CAMERA_MODEL_ESP32S3_EYE)
    s->set_vflip(s, 1);
    #endif

    // Configura o LED Flash se o LED Pin estiver definido em camera_pins.h
    #if defined(LED_GPIO_NUM)
        setupLedFlash(LED_GPIO_NUM);
    #endif



    WiFi.begin(ssid, pswd);
    WiFi.setSleep(false);
    
    while (WiFi.status() !=  WL_CONNECTED){
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("Wi-Fi conectado.");

    Serial.printf("Câmera Pronta! Use 'http://%s' para se conectar.\n", WiFi.localIP());
}

void loop() {
    delay(10000);
}