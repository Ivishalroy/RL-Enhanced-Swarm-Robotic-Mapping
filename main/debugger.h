// debugger.h
// Centralized debug message sender for ESP32 car interface

#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <WebSocketsServer.h>
#include <ArduinoJson.h>

class Debugger {
  public:
    Debugger(WebSocketsServer* socket) : _socket(socket) {}

    void sendDebug(const String& msg) {
      if (_socket) {
        DynamicJsonDocument doc(128);
        doc["key"] = "debug";
        doc["msg"] = msg;
        String out;
        serializeJson(doc, out);
        _socket->broadcastTXT(out);
      }
    }

  private:
    WebSocketsServer* _socket;
};

#endif // DEBUGGER_H