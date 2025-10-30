fn main() {
    let code = k8s_relay_sys::run_with_args(std::env::args_os());
    std::process::exit(code);
}
