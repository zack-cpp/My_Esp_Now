#include <My_Esp_Now.h>

NOW_TRANSMITTER_SIMPLE_NETWORK::NOW_TRANSMITTER_SIMPLE_NETWORK(uint8_t wifiChannel, now_network_encryption_t encryption) {
  this->_wifiChannel = wifiChannel;
  this->_encryption = encryption;
}

/**
 * @brief initialize ESP Now without encryption
 * @param callback_receive callback function for receiving data
 * @param callback_send callback function for sending data
 * @return - ESP_OK (0x00): success
 * @return - 0x01: encryption enabled in the constructor. this function only for
 * unencrypted ESP Now
 * @return - 0x02: failed to initialize ESP Now
 * @return - 0x04: failed to register receive callback function
 * @return - 0x08: failed to register send callback function
 * @return - another error code is addition by errors above
 */
uint8_t NOW_TRANSMITTER_SIMPLE_NETWORK::begin(esp_now_recv_cb_t callback_receive, esp_now_send_cb_t callback_send) {
  uint8_t checkNum = 0;
  uint8_t errCode = 0;

  if (this->_encryption == ENCRYPTION_TRUE) {
    errCode |= (0x01 << checkNum);
    return errCode;
  }
  checkNum++;

  WiFi.mode(WIFI_STA);
  ESP_ERROR_CHECK(esp_wifi_set_channel(this->_wifiChannel, WIFI_SECOND_CHAN_NONE));
  if (esp_now_init() != ESP_OK) {
    errCode |= (0x01 << checkNum);
  }
  checkNum++;

  if (esp_now_register_recv_cb(esp_now_recv_cb_t(callback_receive)) != ESP_OK) {
    errCode |= (0x01 << checkNum);
  }
  checkNum++;

  if (esp_now_register_send_cb(callback_send) != ESP_OK) {
    errCode |= (0x01 << checkNum);
  }
  checkNum++;

  return errCode;
}

/**
 * @brief initialize ESP Now with encryption
 * @param callback_receive callback function for receiving data
 * @param callback_send callback function for sending data
 * @param pmkKey encryption primary key
 * @return - ESP_OK (0x00): success
 * @return - 0x01: encryption enabled in the constructor. this function only for
 * encrypted ESP Now
 * @return - 0x02: failed to initialize ESP Now
 * @return - 0x04: failed to set primary key
 * @return - 0x08: failed to register receive callback function
 * @return - 0x10: failed to register send callback function
 * @return - another error code is addition by errors above
 */
uint8_t NOW_TRANSMITTER_SIMPLE_NETWORK::begin(esp_now_recv_cb_t callback_receive, esp_now_send_cb_t callback_send, String pmkKey) {
  uint8_t checkNum = 0;
  uint8_t errCode = 0;

  if (this->_encryption == ENCRYPTION_FALSE) {
    errCode |= (0x01 << checkNum);
    return errCode;
  }
  checkNum++;

  WiFi.mode(WIFI_STA);
  ESP_ERROR_CHECK(esp_wifi_set_channel(this->_wifiChannel, WIFI_SECOND_CHAN_NONE));
  if (esp_now_init() != ESP_OK) {
    errCode |= (0x01 << checkNum);
  }
  checkNum++;

  if (esp_now_set_pmk((uint8_t *)pmkKey.c_str()) != ESP_OK) {
    errCode |= (0x01 << checkNum);
  }
  checkNum++;

  if (esp_now_register_recv_cb(callback_receive) != ESP_OK) {
    errCode |= (0x01 << checkNum);
  }
  checkNum++;

  if (esp_now_register_send_cb(callback_send) != ESP_OK) {
    errCode |= (0x01 << checkNum);
  }
  checkNum++;

  return errCode;
}

/**
 * @brief register receiver MAC address
 * @param receiverAddr receiver MAC address
 * @param delPrevPeer delete previous peer if given MAC address already registered
 * @return - ESP_OK (0x00): success
 * @return - 0x01: encryption set to true. this function only for unencrypted ESP Now
 * @return - 0x02: MAC address already registered
 * @return - 0x04: failed to register peer
 * @return - another error code is addition by errors above
 */
uint8_t NOW_TRANSMITTER_SIMPLE_NETWORK::register_peer(const uint8_t *receiverAddr, bool delPrevPeer) {
  uint8_t checkNum = 0;
  uint8_t errCode = 0;

  if (this->_encryption == ENCRYPTION_TRUE) {
    errCode |= (0x01 << checkNum);
    return errCode;
  }
  checkNum++;

  /* check if MAC already registered */
  if (esp_now_is_peer_exist(receiverAddr)) {
    if (delPrevPeer) {
      esp_now_del_peer(receiverAddr);
    } else {
      errCode |= (0x01 << checkNum);
      return errCode;
    }
  }
  checkNum++;

  /* peer setup */
  memset(&peerInfo, 0, sizeof(esp_now_peer_info_t));
  peerInfo.channel = _wifiChannel;
  peerInfo.encrypt = false;
  memcpy(&peerInfo.peer_addr, receiverAddr, sizeof(uint8_t[6]));

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    errCode |= (0x01 << checkNum);
  }
  checkNum++;

  /* save MAC address to internal variable upon success */
  if (errCode == ESP_OK) {
    memcpy(this->_receiverAddr, receiverAddr, sizeof(receiverAddr));
  }

  return errCode;
}

/**
 * @brief register receiver MAC address
 * @param receiverAddr receiver MAC address
 * @param delPrevPeer delete previous peer if given MAC address already registered
 * @return - ESP_OK (0x00): success
 * @return - 0x01: encryption set to true. this function only for encrypted ESP Now
 * @return - 0x02: MAC address already registered
 * @return - 0x04: failed to register peer
 * @return - another error code is addition by errors above
 */
uint8_t NOW_TRANSMITTER_SIMPLE_NETWORK::register_peer(const uint8_t *receiverAddr, String lmkKey, bool delPrevPeer) {
  uint8_t checkNum = 0;
  uint8_t errCode = 0;

  if (this->_encryption == ENCRYPTION_FALSE) {
    errCode |= (0x01 << checkNum);
    return errCode;
  }
  checkNum++;

  /* check if MAC already registered */
  if (esp_now_is_peer_exist(receiverAddr)) {
    if (delPrevPeer) {
      esp_now_del_peer(receiverAddr);
    } else {
      errCode |= (0x01 << checkNum);
      return errCode;
    }
  }
  checkNum++;

  /* peer setup */
  memset(&peerInfo, 0, sizeof(esp_now_peer_info_t));
  peerInfo.channel = _wifiChannel;
  for (uint8_t i = 0; i < lmkKey.length(); i++) {
    peerInfo.lmk[i] = uint8_t(lmkKey[i]);
  }
  peerInfo.encrypt = true;
  memcpy(&peerInfo.peer_addr, receiverAddr, sizeof(uint8_t[6]));

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    errCode |= (0x01 << checkNum);
  }
  checkNum++;

  /* save MAC address to internal variable upon success */
  if (errCode == ESP_OK) {
    memcpy(this->_receiverAddr, receiverAddr, sizeof(receiverAddr));
  }

  return errCode;
}

uint8_t NOW_TRANSMITTER_SIMPLE_NETWORK::send(const char *receiverAddr, const uint8_t *data, size_t len) {
  uint8_t mac[6];
  str_to_mac(receiverAddr, ':', mac);
  return this->send(mac, data, len);
}

/**
 * @brief     Send ESPNOW data
 *
 * @attention 1. If peer_addr is not NULL, send data to the peer whose MAC
 * address matches peer_addr
 * @attention 2. If peer_addr is NULL, send data to all of the peers that are
 * added to the peer list
 * @attention 3. The maximum length of data must be less than
 * ESP_NOW_MAX_DATA_LEN
 * @attention 4. The buffer pointed to by data argument does not need to be
 * valid after esp_now_send returns
 *
 * @param     peer_addr  peer MAC address
 * @param     data  data to send
 * @param     len  length of data
 *
 * @return
 *          - ESP_OK : succeed
 *          - ESP_ERR_ESPNOW_NOT_INIT : ESPNOW is not initialized
 *          - ESP_ERR_ESPNOW_ARG : invalid argument
 *          - ESP_ERR_ESPNOW_INTERNAL : internal error
 *          - ESP_ERR_ESPNOW_NO_MEM : out of memory, when this happens, you can
 * delay a while before sending the next data
 *          - ESP_ERR_ESPNOW_NOT_FOUND : peer is not found
 *          - ESP_ERR_ESPNOW_IF : current WiFi interface doesn't match that of
 * peer
 */
uint8_t NOW_TRANSMITTER_SIMPLE_NETWORK::send(const uint8_t *receiverAddr, const uint8_t *data, size_t len) {
  return esp_now_send(receiverAddr, (uint8_t *)data, len);
}

/**
 * @brief convert MAC address to string format
 * @param ar MAC address
 * @return MAC address with string format
*/
String NOW_TRANSMITTER_SIMPLE_NETWORK::mac_to_str(const uint8_t ar[]) {
  String s;
  for (uint8_t i = 0; i < 6; ++i) {
    char buf[3];
    sprintf(buf, "%02X", ar[i]);
    s += buf;
    if (i < 5) s += ':';
  }
  return s;
}

/**
 * @brief convert string to MAC address
 * @param sourceBuffer string input
 * @param sep separator. ':' by default
 * @param targetBuffer MAC address buffer
 * @return none
*/
void NOW_TRANSMITTER_SIMPLE_NETWORK::str_to_mac(const char *sourceBuffer, char sep, byte *targetBuffer) {
  for (int i = 0; i < 6; i++) {
    targetBuffer[i] = strtoul(sourceBuffer, NULL, 16);
    sourceBuffer = strchr(sourceBuffer, sep);
    if (sourceBuffer == NULL || *sourceBuffer == '\0') {
      break;
    }
    sourceBuffer++;
  }
}

uint8_t NOW_TRANSMITTER_SIMPLE_NETWORK::get_wifi_channel(void) {
  return _wifiChannel;
}

/**
 * @brief get receiver MAC address
 * @param none
 * @return MAC address string
*/
String NOW_TRANSMITTER_SIMPLE_NETWORK::get_receiver(void) {
  return mac_to_str(this->_receiverAddr);
}

void NOW_TRANSMITTER_SIMPLE_NETWORK::set_wifi_channel(uint8_t channel) {
  this->_wifiChannel = channel;
}

void NOW_TRANSMITTER_SIMPLE_NETWORK::set_encrpytion(now_network_encryption_t encryption) {
  this->_encryption = encryption;
}

NOW_TRANSMITTER_ADVANCE_NETWORK::NOW_TRANSMITTER_ADVANCE_NETWORK(uint8_t wifiChannel, now_network_encryption_t encryption): NOW_TRANSMITTER_SIMPLE_NETWORK(wifiChannel, encryption) {
  this->set_wifi_channel(wifiChannel);
  this->set_encrpytion(encryption);
}

/**
 * @brief perform "ping" to broadcast address
 * @param none
 * @return - ESP_OK (0x00): success
 * @return - 0x01: failed to add peer
 * @return - 0x02: failed to send message
 * @return - 0x04: failed to delete broadcasr peer
 * @return - another error code is addition by errors above
*/
uint8_t NOW_TRANSMITTER_ADVANCE_NETWORK::broadcast_ping_receiver(uint8_t index) {
  uint8_t checkNum = 0;
  uint8_t errCode = 0;

  uint8_t broadcastAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  memset(&peerInfo, 0, sizeof(esp_now_peer_info_t));
  peerInfo.channel = this->get_wifi_channel();
  /* broadcast doesn't support encryption */
  peerInfo.encrypt = false;
  memcpy(&peerInfo.peer_addr, broadcastAddr, sizeof(uint8_t[6]));
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    errCode |= (0x01 << checkNum);
  }
  checkNum++;

  espnow_command_t command = {.cmd = "ping", .param = "init"};
  if (esp_now_send(broadcastAddr, (uint8_t *)&command, sizeof(command)) != ESP_OK) {
    errCode |= (0x01 << checkNum);
  }
  checkNum++;

  /* delete broadcast peer after usage */
  if (esp_now_del_peer(broadcastAddr) != ESP_OK) {
    errCode |= (0x01 << checkNum);
  }
  checkNum++;

  return errCode;
}

int8_t NOW_TRANSMITTER_ADVANCE_NETWORK::ping_return_handle(const uint8_t *mac, const uint8_t *incomingData, const char *lmkKey) {
  espnow_command_t espnow_cmd_message;
  memcpy(&espnow_cmd_message, incomingData, sizeof(espnow_cmd_message));
  if (register_peer(mac, lmkKey) != ESP_OK) {
    return ESP_FAIL;
  }
  return ESP_OK;
}