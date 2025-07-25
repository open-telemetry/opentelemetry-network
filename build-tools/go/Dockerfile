# Copyright The OpenTelemetry Authors
# SPDX-License-Identifier: Apache-2.0

# Golang: grpc, grpc-gateway

ARG base_IMAGE_TAG
FROM $base_IMAGE_TAG AS build

# protoc-gen-go
RUN /usr/local/go/bin/go install google.golang.org/protobuf/cmd/protoc-gen-go@v1.34.2
RUN /usr/local/go/bin/go install google.golang.org/grpc/cmd/protoc-gen-go-grpc@v1.5.1


# protoc-gen-grpc-gateway and protoc-gen-swagger
# use v1 branch of grpc-gateway
RUN mkdir -p $HOME/go/src/github.com/grpc-ecosystem && \
  cd $HOME/go/src/github.com/grpc-ecosystem && \
  git clone https://github.com/grpc-ecosystem/grpc-gateway && \
  cd grpc-gateway && git checkout v1 && \
  /usr/local/go/bin/go install github.com/grpc-ecosystem/grpc-gateway/protoc-gen-grpc-gateway && \
  /usr/local/go/bin/go install github.com/grpc-ecosystem/grpc-gateway/protoc-gen-swagger

# staticheck linter
RUN /usr/local/go/bin/go install honnef.co/go/tools/cmd/staticcheck@2023.1

# Runtime stage - copy only necessary artifacts
FROM $base_IMAGE_TAG
COPY --from=build $HOME/go/bin $HOME/go/bin
COPY --from=build $HOME/go/src $HOME/go/src
COPY --from=build $HOME/go/pkg $HOME/go/pkg
