/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/Firebase-ESP32
 *
 * Copyright (c) 2023 mobizt
 *
 */

// This example shows how to update firmware file OTA via data stored in RTDB.

#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#elif defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#include <FirebaseESP8266.h>
#endif

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "API_KEY"

/* 3. Define the RTDB URL */
#define DATABASE_URL "URL" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool taskCompleted = false;

void setup()
{

    Serial.begin(115200);
    Serial.println();
    Serial.println();

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    // For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

#if defined(ESP8266)
    // required for large file data, increase Rx size as needed.
    fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 512 /* Tx buffer size in bytes from 512 - 16384 */);
#endif

    // Or use legacy authenticate method
    // config.database_url = DATABASE_URL;
    // config.signer.tokens.legacy_token = "<database secret>";

    // To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);
}

// The Firebase download callback function
void rtdbDownloadCallback(RTDB_DownloadStatusInfo info)
{
    if (info.status == fb_esp_rtdb_download_status_init)
    {
        Serial.printf("Downloading firmware file %s (%d)\n", info.remotePath.c_str(), info.size);
    }
    else if (info.status == fb_esp_rtdb_download_status_download)
    {
        Serial.printf("Downloaded %d%s\n", (int)info.progress, "%");
    }
    else if (info.status == fb_esp_rtdb_download_status_complete)
    {
        Serial.println("Update firmware completed.");
        Serial.println();
        Serial.println("Restarting...\n\n");
        delay(2000);
        ESP.restart();
    }
    else if (info.status == fb_esp_rtdb_download_status_error)
    {
        Serial.printf("Download failed, %s\n", info.errorMsg.c_str());
    }
}

// The Firebase upload callback function
void rtdbUploadCallback(RTDB_UploadStatusInfo info)
{
    if (info.status == fb_esp_rtdb_upload_status_init)
    {
        Serial.printf("Uploading firmware file %s (%d) to %s\n", info.localFileName.c_str(), info.size, info.remotePath.c_str());
    }
    else if (info.status == fb_esp_rtdb_upload_status_upload)
    {
        Serial.printf("Uploaded %d%s\n", (int)info.progress, "%");
    }
    else if (info.status == fb_esp_rtdb_upload_status_complete)
    {
        Serial.println("Upload completed\n");
    }
    else if (info.status == fb_esp_rtdb_upload_status_error)
    {
        Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
    }
}

void loop()
{

    // Firebase.ready() should be called repeatedly to handle authentication tasks.

    if (Firebase.ready() && !taskCompleted)
    {
        taskCompleted = true;

        // Assume you use the following code to upload the firmware file stored on SD card to RTDB at path test/firmware/bin

        /*
        Serial.println("\nUpload firmware to database...\n");
        if (!Firebase.setFile(fbdo, StorageType::FLASH, "test/firmware/bin", "<firmware.bin>", rtdbUploadCallback))
            Serial.println(fbdo.errorReason());
        */

        Serial.println("\nDownload firmware file...\n");

        // In ESP8266, this function will allocate 16k+ memory for internal SSL client.
        if (!Firebase.downloadOTA(fbdo, F("test/firmware/bin"), rtdbDownloadCallback /* callback function */))
            Serial.println(fbdo.errorReason());
    }
}