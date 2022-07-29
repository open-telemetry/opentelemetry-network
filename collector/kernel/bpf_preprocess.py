#!/usr/bin/python3
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


import os
import sys
from pcpp.preprocessor import Preprocessor, OutputDirective, Action
import argparse

#
# Pass-through preprocessor to improve BCC's parsing
#

class PassThruPreprocessor(Preprocessor):
    def __init__(self,lexer=None):
        super(PassThruPreprocessor, self).__init__(lexer)
        self.passthrough = False
        self.current_file_line_id = 0
        self.file_line_table = []

    def write_debug_info(self, debugfile):
        lineid = 0
        debugfile.write("int g_bpf_debug_line_info[] = {\n")
        for x in self.file_line_table:   
            debugfile.write("  {0},\n".format(x["line"]))
            lineid += 1
        debugfile.write("};\n")
        
        lineid = 0
        debugfile.write("const char *g_bpf_debug_file_info[] = {\n")
        for x in self.file_line_table:            
            debugfile.write("  \"{0}\",\n".format(x["file"]))
            lineid += 1
        debugfile.write("};\n")
    

    def token(self):
        """Method to return individual tokens, overriding custom macros"""
        tok = super(PassThruPreprocessor, self).token()
        if tok and tok.value=="__FILELINEID__":
            tok.value=str(self.current_file_line_id)
            self.file_line_table.append({ "file": os.path.basename(self.macros['__FILE__'].value[0].value.strip("\"")), "line": tok.lineno })
            self.current_file_line_id += 1
        return tok

    def on_include_not_found(self, is_malformed, is_system_include, curdir, includepath):
        # If includes are not found, complain
        sys.stderr.write("Unable to find include file: "+str(includepath)+"\n")
        sys.exit(1)
    def on_unknown_macro_in_defined_expr(self, tok):
        # Pass through as expanded as possible, unexpanded without complaining if not possible
        return None
    def on_unknown_macro_in_expr(self, tok):
        # Pass through as expanded as possible, unexpanded without complaining if not possible
        return None
    def on_directive_handle(self, directive, toks, ifpassthru, precedingtoks):
        if directive.value=="pragma":
            if len(toks)>=1 and toks[0].type=="CPP_ID" and toks[0].value=="passthrough":
                if len(toks)==3 and toks[1].type=="CPP_WS" and toks[2].type=="CPP_ID" and toks[2].value=="on":
                    # Turn on passthrough
                    self.passthrough = True
                    raise OutputDirective(Action.IgnoreAndRemove)
                elif len(toks)==3 and toks[1].type=="CPP_WS" and toks[2].type=="CPP_ID" and toks[2].value=="off":
                    # Turn on passthrough
                    self.passthrough = False
                    raise OutputDirective(Action.IgnoreAndRemove)
                sys.stderr.write("Invalid passthrough pragma\n")
                sys.exit(1)
                
        if self.passthrough:
            # Pass through without execution EVERYTHING if we have used pragma passthrough
            raise OutputDirective(Action.IgnoreAndPassThrough)

        if directive.value=="define":
            # Process and ALSO pass through as well
            return None
        if directive.value=="include":
            if toks[0].type=="CPP_STRING":
                # Process #include "" normally
                return True
            else:
                # Always pass through #include<>
                raise OutputDirective(Action.IgnoreAndPassThrough)

        # Attempt to process all other directives
        return True
    def on_directive_unknown(self, directive, toks, ifpassthru, precedingtoks):
        raise OutputDirective(Action.IgnoreAndPassThrough)   


#
# Parse the BPF
#

parser = argparse.ArgumentParser(description='Preprocess BPF')
parser.add_argument("infile", type=str, help="input file")
parser.add_argument("outfile", type=str, help="output file")
parser.add_argument("debugfile", type=str, help="output debug information file")
parser.add_argument("-I", dest="includedirs", metavar="includedir", default=[], type=str, action="append", help="include directory")
parser.add_argument("-D", dest="definitions", metavar="define", default=[], type=str, action="append", help="definitions")
args = parser.parse_args()

pp = PassThruPreprocessor()

if args.infile == "-":
    infile = sys.stdin
else:
    infile = open(args.infile, "rt")

if args.outfile == "-":
    outfile = sys.stdout
else:
    outfile = open(args.outfile, "wt")

if args.debugfile == "-":
    print("Can't write debug information to stdout")
    sys.exit(1)
else:
    debugfile = open(args.debugfile, "wt")

for includedir in args.includedirs:
    pp.add_path(includedir)
    
extra_defs = ""
for definition in args.definitions:
    d = definition.split("=",1)
    if len(d)==1:
        extra_defs += "#define {}\n".format(d[0])
    elif len(d)==2:
        extra_defs += "#define {} {}\n".format(d[0], d[1])

all_source = extra_defs + infile.read()

pp.parse(all_source, infile.name)
pp.write(outfile)

pp.write_debug_info(debugfile)
