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
    val pathPrefix = pkg + "/"
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
    # Build as both rlib and staticlib
    # - rlib: lets Rust binaries depend on this crate via Cargo and expose
    #         the #[no_mangle] extern "C" encoder symbols during Rust linking.
    # - staticlib: keeps support for direct C/C++ linking where needed.
    crate-type = ["rlib", "staticlib"]

    [dependencies]
    render_parser = { workspace = true }
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

    // Modules use the standard Rust module system; files live under src/
    #[allow(dead_code)]
    pub mod wire_messages;
    #[allow(dead_code)]
    pub mod parsed_message;
    pub mod encoder;
    #[allow(dead_code)]
    pub mod hash;
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
    # Aggregator remains a staticlib for C++ consumers; Rust binaries
    # link per-app crates directly via rlib dependencies.
    crate-type = ["staticlib"]

    [dependencies]
    «FOR a : apps»encoder_«pkg»_«a.name» = { path = "«a.name»" }
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
