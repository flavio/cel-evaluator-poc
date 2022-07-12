#ifndef __PROTO_TO_CEL_H__
#define __PROTO_TO_CEL_H__

#include "eval/public/cel_value.h"
#include "google/protobuf/struct.pb.h"

// Attempts to convert a Protobuf Struct into a Cel Value
google::api::expr::runtime::CelValue protobufStructToCelValue(
    google::protobuf::Struct struct_value,
    google::protobuf::Arena *arena);

// Attempts to convert a Protobuf List into a Cel Value
google::api::expr::runtime::CelValue protobufListToCelValue(
    google::protobuf::ListValue list_value,
    google::protobuf::Arena *arena);

// Attempts to convert a Protobuf Value into a Cel Value
google::api::expr::runtime::CelValue protobufValueToCelValue(
    google::protobuf::Value proto_value,
    google::protobuf::Arena *arena);

#endif