#ifndef MY_ESP_NOW_H
#define MY_ESP_NOW_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

typedef enum { ENCRYPTION_TRUE, ENCRYPTION_FALSE } now_network_encryption_t;

class NOW_TRANSMITTER_SIMPLE_NETWORK {
 public:
  NOW_TRANSMITTER_SIMPLE_NETWORK(uint8_t wifiChannel, now_network_encryption_t encryption);

  uint8_t begin(esp_now_recv_cb_t callback_receive, esp_now_send_cb_t callback_send);
  uint8_t begin(esp_now_recv_cb_t callback_receive, esp_now_send_cb_t callback_send, String pmkKey);
  uint8_t register_peer(const uint8_t *receiverAddr, bool delPrevPeer = false);
  uint8_t register_peer(const uint8_t *receiverAddr, String lmkKey, bool delPrevPeer = false);
  uint8_t send(const char *receiverAddr, const uint8_t *data, size_t len);
  uint8_t send(const uint8_t *receiverAddr, const uint8_t *data, size_t len);
  String mac_to_str(const uint8_t ar[]);
  void str_to_mac(const char *sourceBuffer, char sep, byte *targetBuffer);

 protected:
  esp_now_peer_info_t peerInfo;
  uint8_t get_wifi_channel(void);
  String get_receiver(void);
  void set_wifi_channel(uint8_t channel);
  void set_encrpytion(now_network_encryption_t encryption);

 private:
  uint8_t _wifiChannel;
  now_network_encryption_t _encryption;
  uint8_t _receiverAddr[6];
};

class NOW_TRANSMITTER_ADVANCE_NETWORK : public NOW_TRANSMITTER_SIMPLE_NETWORK {
 public:
  NOW_TRANSMITTER_ADVANCE_NETWORK(uint8_t wifiChannel, now_network_encryption_t encryption);
  uint8_t broadcast_ping_receiver(uint8_t index);
  int8_t ping_return_handle(const uint8_t *mac, const uint8_t *incomingData, const char *lmkKey);

 private:
  typedef struct {
    String cmd;
    String param;
  }espnow_command_t;
};

#endif