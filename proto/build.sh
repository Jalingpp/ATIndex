#!/bin/bash

# 执行 protoc 命令生成 Go 代码和 gRPC 相关的代码
/root/pkg/protoc/bin/protoc --go_out=. --go-grpc_out=. client_ma.proto
/root/pkg/protoc/bin/protoc --go_out=. --go-grpc_out=. ma_sn.proto