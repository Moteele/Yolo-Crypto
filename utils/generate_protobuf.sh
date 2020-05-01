#!/bin/bash
# generates respective .h and .cpp files
# usage: generate_protobuf.sh paht/to/file.proto

proto=$1
# ignore everything from 'doc' subfolder
if [[ ${proto} == *"doc"* ]]; then
	return 0
fi

path=${proto%/*.proto}

protoc --proto_path=$path --cpp_out=$path $proto &&
for cc in `ls ${path}/*.cc`; do mv $cc ${cc%.cc}.cpp; done ||
echo "${proto}: Generation from .proto file failed!"
