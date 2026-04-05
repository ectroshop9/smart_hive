#include "hive_manager.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>
#include <atomic>

namespace HiveManager {

static const char* TAG = "HIVE_MANAGER";
static hive_data_t hives[MAX_HIVES];
static int current_hive_count = 0;
static std::atomic<int> selected_hive_idx{0};

class NVSStorage {
private:
    nvs_handle_t handle_;
    static const char* TAG;

    uint32_t calculateChecksum(const hive_data_t* data, uint32_t count) const {
        uint32_t sum = 0;
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);
        for (uint32_t i = 0; i < count * sizeof(hive_data_t); i++) {
            sum += bytes[i];
        }
        return sum;
    }

    bool validateHeader(const nvs_header_t* header) const {
        if (header->magic_number != NVS_MAGIC_NUMBER) return false;
        if (header->version != NVS_VERSION) return false;
        return true;
    }

public:
    NVSStorage() : handle_(0) {}

    esp_err_t init() {
        esp_err_t err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            err = nvs_flash_init();
        }
        if (err != ESP_OK) return err;
        return nvs_open("hive_master", NVS_READWRITE, &handle_);
    }

    esp_err_t saveAll(const hive_data_t* hives_data, uint32_t hive_count) {
        if (handle_ == 0 || hive_count > MAX_HIVES) return ESP_ERR_INVALID_STATE;

        nvs_header_t header;
        header.magic_number = NVS_MAGIC_NUMBER;
        header.version = NVS_VERSION;
        header.hive_count = hive_count;
        header.timestamp = (uint32_t)(esp_timer_get_time() / 1000000ULL);
        header.checksum = calculateChecksum(hives_data, hive_count);
        memset(header.reserved, 0, sizeof(header.reserved));

        size_t total_size = sizeof(nvs_header_t) + (hive_count * sizeof(hive_data_t));
        uint8_t* buffer = (uint8_t*)malloc(total_size);
        if (!buffer) return ESP_ERR_NO_MEM;

        memcpy(buffer, &header, sizeof(nvs_header_t));
        memcpy(buffer + sizeof(nvs_header_t), hives_data, hive_count * sizeof(hive_data_t));

        esp_err_t err = nvs_set_blob(handle_, "hives_data", buffer, total_size);
        if (err == ESP_OK) err = nvs_commit(handle_);

        free(buffer);
        return err;
    }

    esp_err_t loadAll(hive_data_t* hives_out, uint32_t* hive_count_out, uint32_t max_hives) {
        if (handle_ == 0) return ESP_ERR_INVALID_STATE;

        size_t required_size = 0;
        esp_err_t err = nvs_get_blob(handle_, "hives_data", nullptr, &required_size);
        if (err != ESP_OK) {
            *hive_count_out = 0;
            return ESP_OK;
        }

        uint8_t* buffer = (uint8_t*)malloc(required_size);
        if (!buffer) return ESP_ERR_NO_MEM;

        err = nvs_get_blob(handle_, "hives_data", buffer, &required_size);
        if (err != ESP_OK) {
            free(buffer);
            return err;
        }

        nvs_header_t* header = (nvs_header_t*)buffer;
        if (!validateHeader(header)) {
            free(buffer);
            return ESP_ERR_INVALID_VERSION;
        }

        uint32_t stored_count = (header->hive_count > max_hives) ? max_hives : header->hive_count;
        memcpy(hives_out, buffer + sizeof(nvs_header_t), stored_count * sizeof(hive_data_t));

        *hive_count_out = stored_count;
        free(buffer);
        return ESP_OK;
    }

    esp_err_t saveBackup(const hive_data_t* hives_data, uint32_t hive_count) {
        return nvs_set_blob(handle_, "hives_backup", hives_data, hive_count * sizeof(hive_data_t));
    }

    ~NVSStorage() { if (handle_ != 0) nvs_close(handle_); }
};

const char* NVSStorage::TAG = "NVS_STORAGE";
static NVSStorage nvs_storage;

void init(void) {
    nvs_storage.init();
    loadAll();
}

void saveAll(void) {
    nvs_storage.saveAll(hives, (uint32_t)current_hive_count);
    nvs_storage.saveBackup(hives, (uint32_t)current_hive_count);
}

void loadAll(void) {
    uint32_t loaded_count = 0;
    esp_err_t err = nvs_storage.loadAll(hives, &loaded_count, MAX_HIVES);
    if (err == ESP_OK && loaded_count > 0) {
        current_hive_count = (int)loaded_count;
        ESP_LOGI(TAG, "Loaded %d hives from NVS", (int)current_hive_count);
    } else {
        addTestData();
    }
}

int getCount(void) {
    return current_hive_count;
}

std::optional<hive_data_t> getHive(int index) {
    if (index < 0 || index >= current_hive_count) {
        return std::nullopt;
    }
    return hives[index];
}

void updateHive(const hive_data_t& data) {
    for (int i = 0; i < current_hive_count; i++) {
        if (hives[i].id == data.id) {
            hives[i] = data;
            return;
        }
    }
    if (current_hive_count < MAX_HIVES) {
        hives[current_hive_count] = data;
        current_hive_count++;
        ESP_LOGI(TAG, "New hive %d added", (int)data.id);
    }
}

void addTestData(void) {
    const int test_count = 50;
    for (int i = 0; i < test_count && i < MAX_HIVES; i++) {
        hives[i].id = (int16_t)(i + 1);
        hive_set_weight(&hives[i], 20.0f + (i % 15) * 1.5f);
        hive_set_temp_1(&hives[i], 34.0f + (i % 8) * 0.5f);
        hive_set_temp_2(&hives[i], 34.5f + (i % 8) * 0.5f);
        hive_set_temp_3(&hives[i], 35.0f + (i % 8) * 0.5f);
        hive_set_hum(&hives[i], 40.0f + (i % 20));
        hives[i].battery = 70 + (i % 30);
        hives[i].gas = 5 + (i % 25);
        hives[i].uv = 2 + (i % 5);
        hives[i].motion_entrance = (i % 3 == 0);
        hives[i].motion_inside = (i % 4 == 0);
        hives[i].sound = 30 + (i % 50);
        hives[i].vibration = 10 + (i % 40);
    }
    current_hive_count = test_count;
    ESP_LOGI(TAG, "Generated %d test hives", (int)current_hive_count);
}

void setSelected(int idx) {
    selected_hive_idx.store(idx);
}

int getSelected(void) {
    return selected_hive_idx.load();
}

} // namespace HiveManager