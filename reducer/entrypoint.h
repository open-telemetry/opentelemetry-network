#pragma once
namespace reducer_cfg {
// forward declaration
struct ReducerConfig;

int otn_reducer_main_with_config(ReducerConfig const &cfg);

// Logging initialization and level control (called from Rust CLI)
void otn_init_logging(bool log_console, bool no_log_file);
// Level code mapping: 0=trace,1=debug,2=info,3=warning,4=error,5=critical
void otn_set_log_level(int level_code);
} // namespace reducer_cfg
