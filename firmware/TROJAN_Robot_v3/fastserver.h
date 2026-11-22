#pragma once
// The stock WebServer::handleClient() serves one connection at a time and waits up to 5 s for a
// connection that has not sent its request yet. Browsers open spare connections they never use, so
// one idle socket could hold up every drive command for 5 s (the robot then stops on the dead-man):
// the "random stalls". This is the same state machine with short waits.
#include <WebServer.h>

class FastServer : public WebServer {
 public:
  using WebServer::WebServer;
  static const uint32_t WAIT_REQUEST_MS = 400;   // stock: 5000
  static const uint32_t IO_TIMEOUT_MS = 1500;    // stock: 5000 (read/send stall on a bad link)

  void handle() {
    if (_currentStatus == HC_NONE) {
      _currentClient = _server.accept();
      if (!_currentClient) { delay(1); return; }
      _currentClient.setNoDelay(true);
      _currentStatus = HC_WAIT_READ;
      _statusChange = millis();
    }
    bool keep = false;
    if (_currentClient.connected() && _currentStatus == HC_WAIT_READ) {
      if (_currentClient.available()) {
        _currentClient.setTimeout(IO_TIMEOUT_MS);
        if (_parseRequest(_currentClient)) {
          _contentLength = CONTENT_LENGTH_NOT_SET;
          _responseCode = 0;
          _clearResponseHeaders();
          _handleRequest();
        }
      } else if (millis() - _statusChange <= WAIT_REQUEST_MS) {
        keep = true;
      }
    }
    if (!keep) {
      _currentClient = NetworkClient();
      _currentStatus = HC_NONE;
      _currentUpload.reset();
      _currentRaw.reset();
    }
  }
};
