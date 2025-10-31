pub fn run() {
    use std::env;

    // Rebuild if any of the link env vars change
    println!("cargo:rerun-if-env-changed=OTN_LINK_SEARCH");
    println!("cargo:rerun-if-env-changed=OTN_LINK_LIBS");
    println!("cargo:rerun-if-env-changed=OTN_LINK_ARGS");

    if let Ok(search) = env::var("OTN_LINK_SEARCH") {
        for p in search.split(':').filter(|s| !s.is_empty()) {
            println!("cargo:rustc-link-search=native={}", p);
        }
    }

    if let Ok(libs) = env::var("OTN_LINK_LIBS") {
        for spec in libs.split(';').filter(|s| !s.is_empty()) {
            // spec format: kind=name (e.g., static=agentlib or dylib=stdc++)
            if let Some((kind, name)) = spec.split_once('=') {
                println!("cargo:rustc-link-lib={}={}", kind, name);
            } else {
                // default to "dylib" if kind omitted (not expected)
                println!("cargo:rustc-link-lib={}", spec);
            }
        }
    }

    if let Ok(args) = env::var("OTN_LINK_ARGS") {
        for arg in args.split(';').filter(|s| !s.is_empty()) {
            println!("cargo:rustc-link-arg={}", arg);
        }
    }
}

