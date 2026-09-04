#pragma once
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include <atomic>
#include "esphome/components/display/display.h"
#include "esphome/components/touchscreen/touchscreen.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "JPEGDEC.h"
#include "protocol.h"
#include "remote_webview_config.h"

#include "esp_event.h"
#include "esp_websocket_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#if defined(CONFIG_IDF_TARGET_ESP32P4)
  #include "driver/jpeg_decode.h"
  #define REMOTE_WEBVIEW_HW_JPEG 1
#else
  #define REMOTE_WEBVIEW_HW_JPEG 0
#endif
#if defined(CONFIG_IDF_TARGET_ESP32P4)
  #include "esp_cache.h"
  #define REMOTE_WEBVIEW_HAS_CACHE_MSYNC 1
#else
  #define REMOTE_WEBVIEW_HAS_CACHE_MSYNC 0
#endif

namespace esphome {
namespace remote_webview {

class RemoteWebView : public Component {
 public:
  void set_display(display::Display *d) { display_ = d; }
  void set_touchscreen(touchscreen::Touchscreen *t) { touch_ = t; }
  void set_device_id(const std::string &s) { device_id_ = s; }
  void set_url(const std::string &s) { url_ = s; }
  void set_server(const std::string &s);
  void set_tile_size(int v) { tile_size_ = v; }
  void set_full_frame_tile_count(int v) { full_frame_tile_count_ = v; }
  void set_full_frame_area_threshold(float v) { full_frame_area_threshold_ = v; }
  void set_full_frame_every(int v) { full_frame_every_ = v; }
  void set_every_nth_frame(int v) { every_nth_frame_ = v; }
  void set_min_frame_interval(int v) { min_frame_interval_ = v; }
  void set_jpeg_quality(int v) { jpeg_quality_ = v; }
  void set_max_bytes_per_msg(int v) { max_bytes_per_msg_ = v; }
  void set_big_endian(bool v) { rgb565_big_endian_ = v; }
  void set_rotation(int v) { rotation_ = v; }
  void set_chroma_subsampling(const std::string &s) { chroma_ = s; }      // "444" | "420"
  void set_screencast_format(const std::string &s) { screencast_format_ = s; }  // "png" | "jpeg"
  void set_screencast_quality(int v) { screencast_quality_ = v; }
  void set_reduced_motion(bool v) { reduced_motion_ = v ? 1 : 0; }
  void set_screencast_mode(const std::string &s) { screencast_mode_ = s; }  // "stream" | "ondemand"
  // Lossless RLE565 for flat rects: server uses it when the rect compresses to
  // at most this fraction of its raw size (0 = JPEG only). Needs server 1.1.22+.
  void set_rle_max_ratio(float v) { rle_max_ratio_ = v; }
  void set_lossless_max_ratio(float v) { lossless_max_ratio_ = v; }
  void set_deflate_level(int v) { deflate_level_ = v; }
  // ESP32-P4 hardware JPEG decode (default on). The hardware applies a fixed
  // limited-range YUV->RGB expansion; the server (1.1.24+) pre-compensates
  // tiles of >= hw_jpeg_min_pixels for it (URL param hwj), so colours match
  // the software-decoded small tiles exactly.
  void set_hw_jpeg(bool v) { hw_jpeg_enabled_ = v; }
  // Frames the server may have in flight to this panel (server 1.1.25+). 1 on
  // weak WiFi halves queueing latency; 2 (server default) pipelines on good links.
  void set_max_inflight(int v) { max_inflight_ = v; }
  void disable_touch(bool disable);
  bool open_url(const std::string &s, bool force = false);
  // Ask the server to reload the current page and push a full frame.
  bool refresh();
  bool is_connected() const;

  void add_on_connect_callback(std::function<void()> &&callback);
  void add_on_disconnect_callback(std::function<void()> &&callback);

  // Optional diagnostics exposed to Home Assistant (published from loop()).
  void set_connected_sensor(binary_sensor::BinarySensor *s) { connected_sensor_ = s; }
  void set_fps_sensor(sensor::Sensor *s) { fps_sensor_ = s; }
  void set_frame_time_sensor(sensor::Sensor *s) { frame_time_sensor_ = s; }
  void set_reconnects_sensor(sensor::Sensor *s) { reconnects_sensor_ = s; }

  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::LATE; }

 private:
  struct WsMsg {
    uint8_t *buf{nullptr};
    size_t   len{0};
    void    *client{nullptr}; // opaque esp_websocket_client_handle_t
  };
  struct WsReasm {
    uint8_t *buf{nullptr};
    size_t total{0}, filled{0};
  };
  struct SendMsg {
    uint8_t  buf[16];       // inline: touch(8), stats(10), keepalive(2)
    uint8_t *heap{nullptr}; // heap-alloc for open_url; freed after send
    size_t   len{0};
  };

  static constexpr bool     kCoalesceMoves  = cfg::coalesce_moves;
  static constexpr uint32_t kMoveRateHz     = cfg::move_rate_hz;
  static constexpr uint32_t kMoveIntervalUs = (kMoveRateHz ? (1000000u / kMoveRateHz) : 0);

  static RemoteWebView *self_;
  display::Display *display_{nullptr};
  touchscreen::Touchscreen *touch_ = nullptr;
  class RemoteWebViewTouchListener *touch_listener_ = nullptr;
  int display_width_{0};
  int display_height_{0};
  std::string url_;
  std::string server_host_;
  std::string device_id_;
  int server_port_{0};
  int tile_size_{-1};
  int full_frame_tile_count_{-1};
  float full_frame_area_threshold_{-1.0f};
  int full_frame_every_{-1};
  int every_nth_frame_{-1};
  int min_frame_interval_{-1};
  int jpeg_quality_{-1};
  int max_bytes_per_msg_{-1};
  bool rgb565_big_endian_{true};
  int rotation_{0};
  bool touch_disabled_{false};
  std::string chroma_;
  std::string screencast_format_;
  int screencast_quality_{-1};
  int reduced_motion_{-1};
  std::string screencast_mode_;
  float rle_max_ratio_{-1.0f};
  float lossless_max_ratio_{cfg::lossless_max_ratio_default};
  int deflate_level_{cfg::deflate_level_default};
  bool hw_jpeg_enabled_{true};
  int max_inflight_{-1};
  uint16_t *rle_buf_{nullptr};
  uint8_t *lz_buf_{nullptr};      // full-screen RGB565 output buffer (PSRAM)
  void *lz_dec_{nullptr};         // tinfl_decompressor (~11 KB, internal RAM)

#if REMOTE_WEBVIEW_HW_JPEG
  jpeg_decoder_handle_t hw_dec_{nullptr};
  uint8_t *hw_decode_input_buf_{nullptr};
  uint8_t *hw_decode_output_buf_{nullptr};
  size_t hw_decode_input_size_{0};
  size_t hw_decode_output_size_{0};
#endif

  uint64_t last_move_us_{0};
  uint64_t last_keepalive_us_{0};
  uint64_t disconnected_since_us_{0};  // supervisor task only

  // Set from the WS event handler (websocket task), consumed in loop().
  std::atomic<bool> connect_pending_{false};
  std::atomic<bool> disconnect_pending_{false};
  std::atomic<bool> was_connected_{false};  // so failed reconnect attempts don't re-fire on_disconnect
  CallbackManager<void()> on_connect_callback_{};
  CallbackManager<void()> on_disconnect_callback_{};

  binary_sensor::BinarySensor *connected_sensor_{nullptr};
  sensor::Sensor *fps_sensor_{nullptr};
  sensor::Sensor *frame_time_sensor_{nullptr};
  sensor::Sensor *reconnects_sensor_{nullptr};
  // Written by the decode task, read by loop().
  std::atomic<uint32_t> stat_frames_{0};
  std::atomic<uint32_t> stat_frame_time_ms_{0};
  std::atomic<uint32_t> connect_count_{0};
  uint32_t last_stats_publish_ms_{0};
  // Latency breakdown (diagnostic only; benign cross-task races).
  uint32_t lat_fid_{0xffffffffu};   // frame currently being timed
  uint64_t lat_rx_first_us_{0};    // first fragment of the frame seen by the WS task
  uint64_t lat_rx_last_us_{0};     // last packet of the frame fully reassembled
  uint64_t lat_decoded_us_{0};     // last tile drawn, ack enqueued
  uint32_t lat_bytes_{0};
  bool connected_published_{false};
  bool connected_state_{false};
  
  uint64_t frame_start_us_ = 0;
  uint32_t frame_id_{0xffffffffu};
  uint16_t frame_tiles_{0};
  size_t   frame_bytes_{0};
  uint32_t frame_stats_time_{0};
  uint32_t frame_stats_count_{0};
  size_t   frame_stats_bytes_{0};

  QueueHandle_t     q_decode_{nullptr};
  QueueHandle_t     q_send_{nullptr};
  TaskHandle_t      t_ws_{nullptr};
  TaskHandle_t      t_decode_{nullptr};
  TaskHandle_t      t_send_{nullptr};

  esp_websocket_client_handle_t ws_client_{nullptr};  // set by event handler, read by send task

  void start_ws_task_();
  void start_decode_task_();
  void start_send_task_();
  static void ws_task_tramp_(void *arg);
  static void decode_task_tramp_(void *arg);
  static void send_task_tramp_(void *arg);
  bool ws_enqueue_send_(const uint8_t *data, size_t len, uint8_t *heap_buf, TickType_t wait);

  static void ws_event_handler_(void *handler_arg, esp_event_base_t base, int32_t event_id, void *event_data);
  static void reasm_reset_(WsReasm &r);

  void process_packet_(void *client, const uint8_t *data, size_t len);
  void process_frame_packet_(const uint8_t *data, size_t len);
  void process_frame_stats_packet_(const uint8_t *data, size_t len);
  bool decode_jpeg_tile_to_lcd_(int16_t dst_x, int16_t dst_y, uint16_t w, uint16_t h, const uint8_t *data, size_t len);
  // Set while a hardware-intended (pre-compensated) tile falls back to the
  // software decoder: the draw callback then applies the hardware's expansion.
  bool sw_expand_{false};
  bool decode_jpeg_tile_software_(int16_t dst_x, int16_t dst_y, const uint8_t *data, size_t len);
  bool draw_rle_tile_(int16_t dst_x, int16_t dst_y, uint16_t w, uint16_t h, const uint8_t *data, size_t len);
  bool draw_deflate_tile_(int16_t dst_x, int16_t dst_y, uint16_t w, uint16_t h, const uint8_t *data, size_t len);

  static int jpeg_draw_cb_s_(JPEGDRAW *p);
  int jpeg_draw_cb_(JPEGDRAW *p);
  JPEGDEC jd_;

  bool ws_send_touch_event_(proto::TouchType type, int x, int y, uint8_t pid);
  bool ws_send_keepalive_();
  bool ws_send_frame_ack_(uint32_t frame_id);
  bool ws_send_open_url_(const char *url, uint16_t flags);

  std::string resolve_device_id_() const;
  std::string build_ws_uri_() const;
  static void append_q_int_(std::string &s, const char *k, int v);
  static void append_q_float_(std::string &s, const char *k, float v);
  static void append_q_str_(std::string &s, const char *k, const char *v);

  friend class RemoteWebViewTouchListener;
};

class RemoteWebViewTouchListener : public touchscreen::TouchListener {
 public:
  explicit RemoteWebViewTouchListener(RemoteWebView *p) : parent_(p) {}
  void touch(touchscreen::TouchPoint tp) override;
  void update(const touchscreen::TouchPoints_t &pts) override;
  void release() override;
 private:
  RemoteWebView *parent_{nullptr};
};

}  // namespace remote_webview
}  // namespace esphome
