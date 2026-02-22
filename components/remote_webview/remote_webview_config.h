#pragma once

namespace esphome {
namespace remote_webview {
namespace cfg {

inline constexpr int decode_task_stack = 32 * 1024;
inline constexpr int ws_task_stack = 8 * 1024;
inline constexpr int ws_task_prio = 5;
inline constexpr int decode_queue_depth = 12;

inline constexpr size_t ws_max_message_bytes = 600 * 1024;  // must exceed server MAX_BYTES_PER_MESSAGE (default 524288)
inline constexpr size_t ws_buffer_size = 64 * 1024;         // transport read buffer; larger = fewer fragment events
inline constexpr size_t ws_keepalive_interval_us = 60 * 1000 * 1000;

inline constexpr int send_queue_depth = 32;
inline constexpr int send_task_stack  = 4096;

inline constexpr bool coalesce_moves = true;
inline constexpr uint32_t move_rate_hz = 60;

} // namespace cfg
} // namespace remote_webview
} // namespace esphome
