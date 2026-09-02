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

// Connection liveness. The esp_websocket_client sends a WS PING every
// ws_ping_interval_sec and drops the connection (triggering its own auto
// reconnect) if no PONG arrives within ws_pingpong_timeout_sec. TCP keepalive
// catches half-open sockets that never see any traffic at all.
inline constexpr int ws_ping_interval_sec     = 5;
inline constexpr int ws_pingpong_timeout_sec  = 15;
inline constexpr int ws_reconnect_timeout_ms  = 2000;
inline constexpr int ws_network_timeout_ms    = 10000;
inline constexpr int ws_tcp_keepalive_idle_s  = 5;
inline constexpr int ws_tcp_keepalive_intvl_s = 5;
inline constexpr int ws_tcp_keepalive_count   = 3;
// Supervisor backstop: if the client is still disconnected after this long
// (its own auto reconnect should have fixed it within a few seconds),
// stop+start the client from the supervisor task to recover from a dead
// client task.
inline constexpr uint64_t ws_supervisor_restart_after_us = 30ULL * 1000 * 1000;

inline constexpr int send_queue_depth = 32;
inline constexpr int send_task_stack  = 4096;

// Log a per-frame latency breakdown (rx / decode / ack) every N frames; 0 disables.
inline constexpr uint32_t latency_log_every = 50;

inline constexpr bool coalesce_moves = true;
inline constexpr uint32_t move_rate_hz = 60;

} // namespace cfg
} // namespace remote_webview
} // namespace esphome
