# OTLP-TO-PROM
This tool runs a small, synchronous GRPC server that implements the OTLP protocol.  It then outputs to stdout in prom format.  It will also output to a file (metrics.txt by default).

## Command line parameters
|Parameter|Default|Purpose|
|---|---|---|
|--listen-port|4317|the port to listen for OTLP messages|
|--out-file|metrics.txt|the file to write output|

