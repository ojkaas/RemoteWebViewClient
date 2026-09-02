#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "remote_webview.h"

namespace esphome {
namespace remote_webview {

// Fired once per WebSocket connect (runs in the ESPHome main loop).
class OnConnectTrigger : public Trigger<> {
 public:
  explicit OnConnectTrigger(RemoteWebView *parent) {
    parent->add_on_connect_callback([this]() { this->trigger(); });
  }
};

// Fired once per WebSocket disconnect/close (runs in the ESPHome main loop).
class OnDisconnectTrigger : public Trigger<> {
 public:
  explicit OnDisconnectTrigger(RemoteWebView *parent) {
    parent->add_on_disconnect_callback([this]() { this->trigger(); });
  }
};

// remote_webview.refresh: force the server to reload the current page and
// push a fresh full frame, even when the URL did not change.
template<typename... Ts> class RefreshAction : public Action<Ts...> {
 public:
  explicit RefreshAction(RemoteWebView *parent) : parent_(parent) {}
  void play(Ts... x) override { this->parent_->refresh(); }

 protected:
  RemoteWebView *parent_;
};

}  // namespace remote_webview
}  // namespace esphome
