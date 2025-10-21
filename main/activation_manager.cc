#include "activation_manager.h"
#include "ota.h"
#include "settings.h"
#include "application.h"

#include <esp_log.h>
#include <cJSON.h>

#define TAG "ActivationManager"

ActivationManager::ActivationManager() : is_activated_(false) {
    // Load activation status from settings
    Settings settings("activation", false);
    is_activated_ = settings.GetBool("activated", false);
    last_activation_time_ = settings.GetString("last_activation_time", "");
    activation_message_ = settings.GetString("activation_message", "");

    ESP_LOGI(TAG, "ActivationManager initialized, activated: %s", is_activated_ ? "true" : "false");
}

ActivationManager::~ActivationManager() {
}

esp_err_t ActivationManager::ForceReActivate() {
    ESP_LOGI(TAG, "Force re-activation requested");

    // Reset local activation state first
    ResetActivationState();

    // Perform activation
    esp_err_t err = PerformActivation();
    if (err == ESP_OK) {
        is_activated_ = true;

        // Save activation status
        Settings settings("activation", true);
        settings.SetBool("activated", true);
        settings.SetString("last_activation_time", GetCurrentTimeString());
        settings.SetString("activation_message", "Re-activated successfully");

        ESP_LOGI(TAG, "Re-activation successful");
    } else {
        ESP_LOGE(TAG, "Re-activation failed: %s", esp_err_to_name(err));
    }

    return err;
}

bool ActivationManager::IsActivated() {
    return is_activated_;
}

std::string ActivationManager::GetActivationStatus() {
    cJSON *status = cJSON_CreateObject();
    cJSON_AddBoolToObject(status, "activated", is_activated_);
    cJSON_AddStringToObject(status, "last_activation_time", last_activation_time_.c_str());
    cJSON_AddStringToObject(status, "message", activation_message_.c_str());

    auto json_str = cJSON_PrintUnformatted(status);
    std::string result(json_str);
    cJSON_free(json_str);
    cJSON_Delete(status);

    return result;
}

void ActivationManager::ResetActivationState() {
    ESP_LOGI(TAG, "Resetting activation state");

    is_activated_ = false;
    last_activation_time_ = "";
    activation_message_ = "";

    // Clear settings
    Settings settings("activation", true);
    settings.EraseKey("activated");
    settings.EraseKey("last_activation_time");
    settings.EraseKey("activation_message");
}

esp_err_t ActivationManager::PerformActivation() {
    // Create OTA instance to perform activation
    Ota ota;

    // Force check version to get activation challenge
    if (!ota.CheckVersion()) {
        ESP_LOGE(TAG, "Failed to check version for activation");
        return ESP_FAIL;
    }

    // If no activation challenge, device might already be activated
    if (!ota.HasActivationChallenge() && !ota.HasActivationCode()) {
        ESP_LOGI(TAG, "No activation required - device already activated");
        return ESP_OK;
    }

    // Perform activation
    esp_err_t err = ota.Activate();
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Activation completed successfully");
    }

    return err;
}

std::string ActivationManager::GetCurrentTimeString() {
    time_t now;
    time(&now);
    char buffer[26];
    ctime_r(&now, buffer);
    // Remove newline character
    buffer[strlen(buffer) - 1] = '\0';
    return std::string(buffer);
}