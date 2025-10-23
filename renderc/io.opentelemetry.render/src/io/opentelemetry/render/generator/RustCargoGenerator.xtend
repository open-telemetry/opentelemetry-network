// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

package io.opentelemetry.render.generator

import org.eclipse.xtext.generator.IFileSystemAccess2

import io.opentelemetry.render.render.App
import static io.opentelemetry.render.generator.AppGenerator.outputPath

import static extension io.opentelemetry.render.extensions.AppExtensions.*

/**
 * Emits per-app Cargo crate metadata into the generated folder so CMake can
 * run `cargo build` directly from there.
 * Files:
 *  - Cargo.toml with unique package name (encoder_<pkg>_<app>) and rlib output
 *  - src/lib.rs that includes generated wire_messages.rs and encoder.rs
 */
class RustCargoGenerator {

  def void doGenerateApp(App app, IFileSystemAccess2 fsa) {
    fsa.generateFile(outputPath(app, "Cargo.toml"), appCargoToml(app))
    fsa.generateFile(outputPath(app, "src/lib.rs"), appLibRs(app))
  }

  /**
   * Emit a per-package aggregator crate that depends on all per-app crates.
   * This allows linking a single Rust staticlib into C++ binaries that need
   * multiple apps, avoiding duplicate Rust runtime symbols.
   *
   * Layout under the generated output root:
   *   <pkg>/rust-agg/Cargo.toml
   *   <pkg>/rust-agg/src/lib.rs
   */
  def void doGeneratePackage(Iterable<App> apps, IFileSystemAccess2 fsa) {
    if (apps === null || apps.empty) return
    val pkg = apps.head.pkg.name
    val pathPrefix = pkg + "/rust-agg/"
    fsa.generateFile(pathPrefix + "Cargo.toml", packageCargoToml(pkg, apps))
    fsa.generateFile(pathPrefix + "src/lib.rs", packageLibRs(pkg, apps))
  }

  private static def appCargoToml(App app) {
    val crate = 'encoder_' + app.pkg.name + '_' + app.name
    '''
    [package]
    name = "«crate»"
    version = "0.1.0"
    edition = "2021"

    [lib]
    # Build as an rlib so it can be used as a dependency by the
    # per-package aggregator crate. The aggregator produces the staticlib
    # that C++ links against.
    crate-type = ["rlib"]

    [profile.release]
    opt-level = 3
    lto = true
    codegen-units = 1
    '''
  }

  private static def appLibRs(App app) {
    '''
    #![allow(non_camel_case_types)]
    #![allow(non_snake_case)]
    #![allow(unused_variables)]

    use core::ffi::c_char;

    #[repr(C)]
    pub struct JbBlob {
        pub buf: *const c_char,
        pub len: u16,
    }

    // Include generated modules from the crate directory
    #[allow(dead_code)]
    pub mod wire_messages {
        include!(concat!(env!("CARGO_MANIFEST_DIR"), "/wire_messages.rs"));
    }

    pub mod encoder {
        include!(concat!(env!("CARGO_MANIFEST_DIR"), "/encoder.rs"));
    }
    '''
  }

  private static def packageCargoToml(String pkg, Iterable<App> apps) {
    val crate = 'encoder_' + pkg + '_all'
    '''
    [package]
    name = "«crate»"
    version = "0.1.0"
    edition = "2021"

    [lib]
    crate-type = ["staticlib"]

    [profile.release]
    opt-level = 3
    lto = true
    codegen-units = 1

    [dependencies]
    «FOR a : apps»encoder_«pkg»_«a.name» = { path = "../«a.name»" }
    «ENDFOR»
    '''
  }

  private static def packageLibRs(String pkg, Iterable<App> apps) {
    val sb = new StringBuilder
    sb.append('''
    #![allow(dead_code)]
    #![allow(unused_imports)]
    #![allow(unused_extern_crates)]
    ''')
    for (a : apps) {
      sb.append('''
      extern crate encoder_«pkg»_«a.name» as _crate_«a.name»;
      ''')
    }
    // Provide a stable symbol so the archive is never empty.
    sb.append('''
    #[no_mangle]
    pub extern "C" fn __encoder_«pkg»_bundle_marker() {}
    ''')
    sb.toString
  }
}
