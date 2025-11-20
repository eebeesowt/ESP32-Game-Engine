#include "Drivers/AudioDriver.h"
#include "driver/i2s.h"
#include "esp_log.h"

static const char* TAG = "AudioDriver";

#define I2S_NUM         (I2S_NUM_0)
#define SAMPLE_RATE     (44100)

AudioDriver::AudioDriver() : _initialized(false) {
}

AudioDriver::~AudioDriver() {
    if (_initialized) {
        i2s_driver_uninstall(I2S_NUM);
    }
}

bool AudioDriver::init() {
    if (_initialized) return true;

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    
    i2s_pin_config_t pin_config = {
        .bck_io_num = PIN_I2S_BCLK,
        .ws_io_num = PIN_I2S_LRCK,
        .data_out_num = PIN_I2S_DOUT,
        .data_in_num = PIN_I2S_DIN
    };

    esp_err_t err = i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install I2S driver: %d", err);
        return false;
    }

    err = i2s_set_pin(I2S_NUM, &pin_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set I2S pins: %d", err);
        return false;
    }
    
    ESP_LOGI(TAG, "I2S Audio initialized");
    _initialized = true;
    return true;
}

void AudioDriver::playSample(const int16_t* data, size_t length) {
    if (!_initialized) return;
    size_t bytes_written;
    i2s_write(I2S_NUM, data, length * sizeof(int16_t), &bytes_written, portMAX_DELAY);
}

size_t AudioDriver::recordSample(int16_t* buffer, size_t length) {
    if (!_initialized) return 0;
    size_t bytes_read;
    i2s_read(I2S_NUM, buffer, length * sizeof(int16_t), &bytes_read, portMAX_DELAY);
    return bytes_read / sizeof(int16_t);
}

void AudioDriver::setVolume(uint8_t volume) {
    // Software volume or codec command (if I2C codec exists)
    // T-Embed usually uses MAX98357A (I2S amp) which has no volume control via I2C.
    // Volume must be handled in software (scaling samples).
}
