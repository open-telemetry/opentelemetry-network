fn main() {
    // Capture the process args and forward to the C++ entrypoint.
    let code = kernel_collector_sys::run_with_args(std::env::args_os());
    // Propagate the exit code to the OS.
    std::process::exit(code);
}
