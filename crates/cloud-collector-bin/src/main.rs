fn main() {
    let code = cloud_collector_sys::run_with_args(std::env::args_os());
    std::process::exit(code);
}
