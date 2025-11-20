#include "Drivers/SDCardDriver.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdspi_host.h"
#include "esp_log.h"

static const char* TAG = "SDCardDriver";

SDCardDriver::SDCardDriver() : _isMounted(false), _mountPoint("/sdcard") {
}

SDCardDriver::~SDCardDriver() {
    unmount();
}

bool SDCardDriver::init() {
    if (_isMounted) return true;

    ESP_LOGI(TAG, "Initializing SD Card...");

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    
    sdmmc_card_t* card;
    
    // Note: T-Embed likely uses SPI for SD card.
    // If it shares SPI with the display, we need to be careful about bus initialization.
    // LGFX initializes SPI2_HOST.
    
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI3_HOST; // Use SPI3 to avoid conflict with Display on SPI2
    
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_SD_MOSI,
        .miso_io_num = PIN_SD_MISO,
        .sclk_io_num = PIN_SD_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    
    // Initialize the SPI bus
    esp_err_t ret = spi_bus_initialize(SPI3_HOST, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus. Error: %s", esp_err_to_name(ret));
        return false;
    }
    
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_SD_CS;
    slot_config.host_id = SPI3_HOST;

    ret = esp_vfs_fat_sdspi_mount(_mountPoint, &host, &slot_config, &mount_config, &card);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount filesystem. Error: %s", esp_err_to_name(ret));
        // If mount failed, we might want to deinit bus, but for now just return
        return false;
    }
    
    _isMounted = true;
    sdmmc_card_print_info(stdout, card);
    
    return true;
}

void SDCardDriver::unmount() {
    if (_isMounted) {
        esp_vfs_fat_sdcard_unmount(_mountPoint, NULL);
        _isMounted = false;
    }
}

void SDCardDriver::printCardInfo() {
    if (_isMounted) {
        // ...
    }
}
