#!/usr/bin/env python3
# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0


#
# Standalone debugging code for tcp-processor subtree of bpf_src
# To use, sudo python tcp-processor.py
#

from bcc import BPF #, DEBUG_PREPROCESSOR
import io, os, sys
from pcpp.preprocessor import Preprocessor, OutputDirective, Action
import ctypes as ct

if "--debug" in sys.argv:
    import ptvsd
    print("Waiting for debugger attach")
    ptvsd.enable_attach(address=('localhost', 5678), redirect_output=True)
    ptvsd.wait_for_attach()
    breakpoint()

#
# Wrapper for BCC that prints useful diagnostics
#

class BPFWrapper:

    def __init__(self, bpf):
        self._bpf = bpf

    def attach_kprobe(self, event=b"", event_off=0, fn_name=b"", event_re=b""):
        print("attach_kprobe: event={} fn_name={}".format(event, fn_name))
        try:
            self._bpf.attach_kprobe(event,event_off,fn_name,event_re)
        except:
            print("   failed for {}".format(event))
            return
        print("   succeeded for {}".format(event))
        
    def attach_kprobe_all(self, events, event_off=0, fn_name=b"", event_re=b""):
        print("attach_kprobe_all: events={} fn_name={}".format(str(events), fn_name))
        for event in events:
            try:
                self._bpf.attach_kprobe(event,event_off,fn_name,event_re)
            except:
                print("   failed for {}".format(event))
                continue
            print("   succeeded for {}".format(event))
                
    def attach_kretprobe(self, event=b"", fn_name=b"", event_re=b"", maxactive=0):
        print("attach_kretprobe: event={} fn_name={}".format(event, fn_name))
        try:
            self._bpf.attach_kretprobe(event,fn_name,event_re,maxactive)
        except:
            print("   failed for {}".format(event))
            return
        print("   succeeded for {}".format(event))
        
    def attach_kretprobe_all(self, events, fn_name=b"", event_re=b"", maxactive=0):
        print("attach_kretprobe_all: events={} fn_name={}".format(str(events), fn_name))
        for event in events:
            try:
                self._bpf.attach_kretprobe(event,fn_name,event_re, maxactive)
            except:
                print("   failed for {}".format(event))
                continue
            print("   succeeded for {}".format(event))                    


    def __getattr__(self, name):
        if name in self.__dict__:
            return self.__dict__[name]
        if name in self.__class__.__dict__:
            return self.__class__.__dict__[name]
        return getattr(self._bpf, name)
    
    def __getitem__(self, key):
        return self._bpf[key]

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
            debugfile.write("  {0},\n".format(x["file"]))
            lineid += 1
        debugfile.write("};\n")
    
    def token(self):
        """Method to return individual tokens, overriding custom macros"""
        tok = super(PassThruPreprocessor, self).token()
        if tok and tok.value=="__FILELINEID__":
            tok.value=str(self.current_file_line_id)
            self.file_line_table.append({ "file": self.macros['__FILE__'].value[0].value, "line": tok.lineno })
            self.current_file_line_id += 1
        return tok

    def on_include_not_found(self, is_system_include, curdir, includepath):
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

pp = PassThruPreprocessor()
instr = "#define DEBUG_LOG 1\n#define ENABLE_TCP_DATA_STREAM 1\n#define _PROCESSING_BPF 1\n#define STANDALONE_TCP_PROCESSOR 1\n" + open("./bpf_tcp_processor.c","rt").read()
outfile = io.StringIO()
pp.add_path("../../../../src/")
pp.add_path("../../../../")
pp.parse(instr, source = "./bpf_tcp_processor.c")
pp.write(outfile)
preprocessed_bpf_text = outfile.getvalue()
# print("preproc: "+preprocessed_bpf_text)

print("debug info:")
pp.write_debug_info(sys.stdout)

b = BPFWrapper(BPF(text=preprocessed_bpf_text))

# Set up tail calls (mirroring bpf_handler.cc)

tail_calls = b.get_table("tail_calls")
#tail_calls[ct.c_int(int(pp.macros["TAIL_CALL_ON_UDP_SEND_SKB__2"].value[0].value))] = b.load_func("on_udp_send_skb__2", BPF.KPROBE)
#tail_calls[ct.c_int(int(pp.macros["TAIL_CALL_ON_UDP_V6_SEND_SKB__2"].value[0].value))] = b.load_func("on_udp_v6_send_skb__2", BPF.KPROBE)
#tail_calls[ct.c_int(int(pp.macros["TAIL_CALL_ON_IP_SEND_SKB__2"].value[0].value))] = b.load_func("on_ip_send_skb__2", BPF.KPROBE)
#tail_calls[ct.c_int(int(pp.macros["TAIL_CALL_ON_IP6_SEND_SKB__2"].value[0].value))] = b.load_func("on_ip6_send_skb__2", BPF.KPROBE)
#tail_calls[ct.c_int(int(pp.macros["TAIL_CALL_HANDLE_RECEIVE_UDP_SKB"].value[0].value))] = b.load_func("handle_receive_udp_skb", BPF.KPROBE)
#tail_calls[ct.c_int(int(pp.macros["TAIL_CALL_HANDLE_RECEIVE_UDP_SKB__2"].value[0].value))] = b.load_func("handle_receive_udp_skb__2", BPF.KPROBE)
tail_calls[ct.c_int(int(pp.macros["TAIL_CALL_CONTINUE_TCP_SENDMSG"].value[0].value))] = b.load_func("continue_tcp_sendmsg", BPF.KPROBE)
tail_calls[ct.c_int(int(pp.macros["TAIL_CALL_CONTINUE_TCP_RECVMSG"].value[0].value))] = b.load_func("continue_tcp_recvmsg", BPF.KPROBE)

#
# Attach probes
#

# Create/destroy
b.attach_kprobe(event="tcp_init_sock", fn_name="handle_kprobe__tcp_init_sock")
b.attach_kprobe(event="security_sk_free", fn_name="handle_kprobe__security_sk_free")

# Accept
b.attach_kprobe(event="inet_csk_accept", fn_name="handle_kprobe__inet_csk_accept")
b.attach_kretprobe(event="inet_csk_accept", fn_name="handle_kretprobe__inet_csk_accept")

# Send
b.attach_kprobe(event="tcp_sendmsg", fn_name="handle_kprobe__tcp_sendmsg")
b.attach_kretprobe(event="tcp_sendmsg", fn_name="handle_kretprobe__tcp_sendmsg")

# Receive
b.attach_kprobe(event="tcp_recvmsg", fn_name="handle_kprobe__tcp_recvmsg")
b.attach_kretprobe(event="tcp_recvmsg", fn_name="handle_kretprobe__tcp_recvmsg")

#
# Print trace output
#

class TCPEventHTTPResponse(ct.Structure):
    _fields_ = [
        ("code", ct.c_ushort),
        ("__pad0", ct.c_uint8*6),
        ("latency", ct.c_ulonglong)
    ]

class TCPEventTCPData(ct.Structure):
    _fields_ = [
        ("length", ct.c_uint),
        ("streamtype", ct.c_uint8),
        ("is_server", ct.c_uint8),
        ("__pad0", ct.c_uint16),
        ("offset", ct.c_ulonglong)
    ]

class TCPEventData(ct.Union):
    _fields_ = [ 
        ("http_response", TCPEventHTTPResponse),
        ("tcp_data", TCPEventTCPData),
        ("__pad0", ct.c_ulonglong),
        ("__pad1", ct.c_ulonglong)
        ]

class TCPEvent(ct.Structure):
    _anonymous_ = ("u",)
    _fields_ = [
        ("type", ct.c_uint),
        ("pid", ct.c_uint),
        ("ts", ct.c_ulonglong),
        ("sk", ct.c_ulonglong),
        ("u", TCPEventData)
        ]

class TCPDataHeader(ct.Structure):
    _pack_ = 1
    _fields_ = [
        ("length", ct.c_ulonglong),
    ]

class TCPDataMessage(ct.Structure):
    _pack_ = 1
    _fields_ = [
        ("hdr", TCPDataHeader),              
        ("data", ct.c_ubyte * 256)
    ]

print("sizeof(TCPDataHeader) = {}".format(ct.sizeof(TCPDataHeader)))
print("sizeof(TCPDataMessage) = {}".format(ct.sizeof(TCPDataMessage)))

def print_tcp_event(cpu, data, size):
    assert size >= ct.sizeof(TCPEvent)
    event = ct.cast(data, ct.POINTER(TCPEvent)).contents

    if event.type == 0:
        print(">>> TCP_EVENT_TYPE_HTTP_RESPONSE(pid=%u, ts=%u, sk=0x%X, code=%u, latency=%u)" % (event.pid, event.ts, event.sk, event.http_response.code, event.http_response.latency))
    elif event.type == 1:
        print(">>> TCP_EVENT_TYPE_TCP_DATA(pid=%u, ts=%u, sk=0x%X, length=%u, streamtype=%u, is_server=%u, offset=%u)" % (event.pid, event.ts, event.sk, event.tcp_data.length, event.tcp_data.streamtype, event.tcp_data.is_server, event.tcp_data.offset))
    else:
        print(">>> UNKNOWN TCP EVENT")


def process_data_channel(cpu, data, size):
    assert size >= ct.sizeof(TCPDataHeader)

    header = ct.cast(data, ct.POINTER(TCPDataHeader)).contents
    datalen = header.length

    out = "### DATA(size: {}, datalen: {}): ".format(size, datalen)
    print(out)

    assert size >= ct.sizeof(TCPDataHeader) + datalen

    message = ct.cast(data, ct.POINTER(TCPDataMessage)).contents
    out = ct.cast(message.data, ct.c_char_p).value[0:datalen]
    print(out)
        
b[b"tcp_events"].open_perf_buffer(print_tcp_event)
b[b"data_channel"].open_perf_buffer(process_data_channel)

while 1:
    try:
        had_message = False
        fields = b.trace_fields(True)
        if fields:
            (task, pid, cpu, flags, ts, msg) = fields
            if msg:
                had_message=True
                print(msg.decode('latin-1'))
        
        if not had_message:
            # Prefer to print messages instead of polling to speed through them
            b.perf_buffer_poll(10)

    except ValueError:
        continue 

