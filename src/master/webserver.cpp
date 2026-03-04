/**
 * @file webserver.cpp
 * @brief WEB-сервер на AsyncWebServer, раздача из LittleFS, API для режимов и калибровки.
 */

#include "webserver.h"
#include "control.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

static AsyncWebServer* s_server = nullptr;

static const char* modeToStr(LightMode m) {
    switch (m) {
        case LightMode::HALT:       return "HALT";
        case LightMode::GO:         return "GO";
        case LightMode::STOP:      return "STOP";
        case LightMode::LEFT:      return "LEFT";
        case LightMode::RIGHT:     return "RIGHT";
        case LightMode::GO_LEFT:   return "GO_LEFT";
        case LightMode::GO_RIGHT:  return "GO_RIGHT";
        case LightMode::STOP_LEFT: return "STOP_LEFT";
        case LightMode::STOP_RIGHT: return "STOP_RIGHT";
        case LightMode::SOS:       return "SOS";
        case LightMode::TEST:      return "TEST";
        default: return "?";
    }
}

void webserverStart() {
    if (s_server) return;
    s_server = new AsyncWebServer(80);
    s_server->serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    s_server->on("/api/status", HTTP_GET, [](AsyncWebServerRequest* req) {
        MotionParams p;
        LightMode mode;
        bool fsReady, logging;
        controlGetStatus(&p, &mode, &fsReady, &logging);
        StaticJsonDocument<256> doc;
        doc["ls"] = (int)(p.ls * 100 + 0.5f) / 100.0;
        doc["accel"] = (int)(p.accel * 100 + 0.5f) / 100.0;
        doc["cnt"] = p.cnt;
        doc["mode"] = modeToStr(mode);
        doc["fsReady"] = fsReady;
        doc["logging"] = logging;
        String buf;
        serializeJson(doc, buf);
        req->send(200, "application/json", buf);
    });
    s_server->on("/api/calibrate", HTTP_POST, [](AsyncWebServerRequest* req) {
        controlTriggerCalibrate();
        req->send(200, "text/plain", "OK");
    });
    s_server->on("/api/mode", HTTP_POST, [](AsyncWebServerRequest* req) {
        if (req->hasParam("m", true)) {
            String m = req->getParam("m", true)->value();
            if (m == "GO")   controlSetManualMode(LightMode::GO);
            else if (m == "STOP") controlSetManualMode(LightMode::STOP);
            else if (m == "LEFT") controlSetManualMode(LightMode::LEFT);
            else if (m == "RIGHT") controlSetManualMode(LightMode::RIGHT);
            else if (m == "SOS") controlSetManualMode(LightMode::SOS);
        }
        req->send(200, "text/plain", "OK");
    });
    s_server->on("/api/test", HTTP_POST, [](AsyncWebServerRequest* req) {
        controlSetManualMode(LightMode::TEST);
        req->send(200, "text/plain", "OK");
    });
    s_server->on("/api/logging", HTTP_POST, [](AsyncWebServerRequest* req) {
        if (req->hasParam("en", true))
            controlSetLogging(req->getParam("en", true)->value().toInt() != 0);
        req->send(200, "text/plain", "OK");
    });
    s_server->begin();
}

void webserverStop() {
    if (s_server) {
        s_server->end();
        delete s_server;
        s_server = nullptr;
    }
}
