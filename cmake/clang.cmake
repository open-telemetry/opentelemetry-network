# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

include_guard()

find_package(CLANG REQUIRED CONFIG NAMES Clang)
message(STATUS "Found Clang ${CLANG_VERSION}")
message(STATUS "Using ClangConfig.cmake in: ${CLANG_CONFIG}")

option(ENABLE_LLVM_SHARED "Enable linking LLVM as a shared library" OFF)

if(NOT ENABLE_LLVM_SHARED)
  #
  # Overwrite libclang's INTERFACE_LINK_LIBRARIES property to link with static LLVM libraries.
  #
  set_target_properties(clangBasic PROPERTIES
    INTERFACE_LINK_LIBRARIES "LLVMCore;LLVMMC;LLVMSupport"
  )
  set_target_properties(clangLex PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangBasic;LLVMSupport"
  )
  set_target_properties(clangParse PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangAST;clangBasic;clangLex;clangSema;LLVMMC;LLVMMCParser;LLVMSupport"
  )
  set_target_properties(clangAST PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangBasic;clangLex;LLVMBinaryFormat;LLVMSupport"
  )
  set_target_properties(clangDynamicASTMatchers PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangAST;clangASTMatchers;clangBasic;LLVMSupport"
  )
  set_target_properties(clangASTMatchers PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangAST;clangBasic;LLVMSupport"
  )
  set_target_properties(clangCrossTU PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangAST;clangBasic;clangFrontend;clangIndex;LLVMSupport"
  )
  set_target_properties(clangSema PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangAST;clangAnalysis;clangBasic;clangEdit;clangLex;LLVMSupport"
  )
  set_target_properties(clangCodeGen PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangAnalysis;clangAST;clangBasic;clangFrontend;clangLex;clangSerialization;LLVMAnalysis;LLVMBitReader;LLVMBitWriter;LLVMCore;LLVMCoroutines;LLVMCoverage;LLVMipo;LLVMIRReader;LLVMAggressiveInstCombine;LLVMInstCombine;LLVMInstrumentation;LLVMLTO;LLVMLinker;LLVMMC;LLVMObjCARCOpts;LLVMObject;LLVMPasses;LLVMProfileData;LLVMScalarOpts;LLVMSupport;LLVMTarget;LLVMTransformUtils"
  )
  set_target_properties(clangAnalysis PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangAST;clangASTMatchers;clangBasic;clangLex;LLVMSupport"
  )
  set_target_properties(clangEdit PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangAST;clangBasic;clangLex;LLVMSupport"
  )
  set_target_properties(clangRewrite PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangBasic;clangLex;LLVMSupport"
  )
  set_target_properties(clangARCMigrate PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangAST;clangAnalysis;clangBasic;clangEdit;clangFrontend;clangLex;clangRewrite;clangSema;clangSerialization;clangStaticAnalyzerCheckers;clangStaticAnalyzerCore;LLVMSupport"
  )
  set_target_properties(clangDriver PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangBasic;LLVMBinaryFormat;LLVMOption;LLVMSupport"
  )
  set_target_properties(clangSerialization PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangAST;clangBasic;clangLex;clangSema;LLVMBitReader;LLVMSupport"
  )
  set_target_properties(clangRewriteFrontend PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangAST;clangBasic;clangEdit;clangFrontend;clangLex;clangRewrite;clangSerialization;LLVMSupport"
  )
  set_target_properties(clangFrontend PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangAST;clangBasic;clangDriver;clangEdit;clangLex;clangParse;clangSema;clangSerialization;LLVMBitReader;LLVMOption;LLVMProfileData;LLVMSupport"
  )
  set_target_properties(clangFrontendTool PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangBasic;clangCodeGen;clangDriver;clangFrontend;clangRewriteFrontend;clangARCMigrate;clangStaticAnalyzerFrontend;LLVMOption;LLVMSupport"
  )
  set_target_properties(clangToolingCore PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangAST;clangBasic;clangLex;clangRewrite;LLVMSupport"
  )
  set_target_properties(clangToolingInclusions PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangBasic;clangLex;clangRewrite;clangToolingCore;LLVMSupport"
  )
  set_target_properties(clangToolingASTDiff PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangBasic;clangAST;clangLex;LLVMSupport"
  )
  set_target_properties(clangTooling PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangAST;clangASTMatchers;clangBasic;clangDriver;clangFormat;clangFrontend;clangLex;clangRewrite;clangSerialization;clangToolingCore;LLVMOption;LLVMSupport"
  )
  set_target_properties(clangIndex PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangAST;clangBasic;clangFormat;clangFrontend;clangLex;clangRewrite;clangSerialization;clangToolingCore;LLVMCore;LLVMSupport"
  )
  set_target_properties(clangStaticAnalyzerCore PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangAST;clangASTMatchers;clangAnalysis;clangBasic;clangCrossTU;clangLex;clangRewrite;LLVMSupport"
  )
  set_target_properties(clangStaticAnalyzerCheckers PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangAST;clangASTMatchers;clangAnalysis;clangBasic;clangLex;clangStaticAnalyzerCore;LLVMSupport"
  )
  set_target_properties(clangStaticAnalyzerFrontend PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangAST;clangAnalysis;clangBasic;clangCrossTU;clangFrontend;clangLex;clangStaticAnalyzerCheckers;clangStaticAnalyzerCore;LLVMSupport"
  )
  set_target_properties(clangFormat PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangBasic;clangLex;clangToolingCore;clangToolingInclusions;LLVMSupport"
  )
  set_target_properties(clangHandleCXX PROPERTIES
    INTERFACE_LINK_LIBRARIES "clangBasic;clangCodeGen;clangFrontend;clangLex;clangSerialization;clangTooling;LLVMBPFCodeGen;LLVMBPFAsmParser;LLVMBPFAsmPrinter;LLVMBPFDesc;LLVMBPFDisassembler;LLVMBPFInfo;LLVMX86CodeGen;LLVMX86AsmParser;LLVMX86AsmPrinter;LLVMX86Desc;LLVMX86Disassembler;LLVMX86Info;LLVMX86Utils;LLVMSupport"
  )
  set_target_properties(clangHandleLLVM PROPERTIES
    INTERFACE_LINK_LIBRARIES "LLVMAnalysis;LLVMCodeGen;LLVMCore;LLVMExecutionEngine;LLVMipo;LLVMIRReader;LLVMMC;LLVMMCJIT;LLVMObject;LLVMRuntimeDyld;LLVMSelectionDAG;LLVMSupport;LLVMTarget;LLVMTransformUtils;LLVMX86CodeGen;LLVMX86AsmParser;LLVMX86AsmPrinter;LLVMX86Desc;LLVMX86Disassembler;LLVMX86Info;LLVMX86Utils"
  )
endif(NOT ENABLE_LLVM_SHARED)
