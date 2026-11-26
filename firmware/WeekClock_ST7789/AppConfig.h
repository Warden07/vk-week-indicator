#pragma once
// No Wi-Fi credentials or GitHub access tokens belong in this file.
constexpr unsigned FW_BUILD = 2;
constexpr char FW_VERSION[] = "1.0.1";
constexpr char TIME_ZONE[] = "MSK-3";
constexpr int TFT_CS = D8; // GPIO15 must stay LOW during boot. See HARDWARE.md.
constexpr int TFT_DC = D2;
constexpr int TFT_RST = D0;
constexpr int TFT_ROTATION = 0;
constexpr unsigned CONNECT_WAIT_MS = 60000;
constexpr unsigned PORTAL_TIME_MS = 300000;
constexpr unsigned UPDATE_INTERVAL_MS = 21600000; // six hours
// Public raw files; no access token. Keep each release in its own directory.
constexpr char UPDATE_BASE[] = "https://raw.githubusercontent.com/Warden07/vk-week-indicator/main/firmware/ota/";
