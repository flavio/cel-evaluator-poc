#include "proto_to_cel.h"

#include "eval/public/containers/container_backed_list_impl.h"
#include "eval/public/containers/container_backed_map_impl.h"

#include "absl/strings/str_format.h"

namespace cel_runtime = google::api::expr::runtime;
using namespace google;

cel_runtime::CelValue protobufValueToCelValue(
    protobuf::Value proto_value,
    protobuf::Arena *arena)
{
  if (proto_value.has_null_value())
  {
    return cel_runtime::CelValue::CreateNull();
  }
  if (proto_value.has_number_value())
  {
    return cel_runtime::CelValue::CreateDouble(proto_value.number_value());
  }
  if (proto_value.has_string_value())
  {
    // TODO: is this leaking? is CelValue going to handle this free op?
    auto str = new std::string(proto_value.string_value());
    return cel_runtime::CelValue::CreateString(str);
  }
  if (proto_value.has_bool_value())
  {
    return cel_runtime::CelValue::CreateBool(proto_value.bool_value());
  }
  if (proto_value.has_struct_value())
  {
    return protobufStructToCelValue(proto_value.struct_value(), arena);
  }
  if (proto_value.has_list_value())
  {
    return protobufListToCelValue(proto_value.list_value(), arena);
  }
  return cel_runtime::CreateErrorValue(
      arena,
      "type not handled by conversion");
}

cel_runtime::CelValue protobufStructToCelValue(
    protobuf::Struct struct_value,
    protobuf::Arena *arena)
{
  // TODO: is CelValue going to handle the deallocation of this object?
  auto cel_map = new cel_runtime::CelMapBuilder();

  for (auto &[key, value] : struct_value.fields())
  {
    // TODO: is this leaking? is CelValue going to handle this free op?
    auto str = new std::string(key);
    auto key_value = cel_runtime::CelValue::CreateString(str);
    auto cel_value = protobufValueToCelValue(value, arena);
    if (cel_value.IsError())
    {
      std::string errorMsg = absl::StrFormat(
          "Cannot convert value of key '%s' to CelValue: %s",
          key, cel_value.DebugString());
      return cel_runtime::CreateErrorValue(
          arena,
          errorMsg);
    }
    else
    {
      auto add_status = cel_map->Add(key_value, cel_value);
      if (!add_status.ok())
      {
        std::string errorMsg = absl::StrFormat(
            "Cannot add key '%s' to CelMap: %s",
            key, add_status.ToString());
        return cel_runtime::CreateErrorValue(
            arena,
            errorMsg);
      }
    }
  }
  return cel_runtime::CelValue::CreateMap(cel_map);
}

cel_runtime::CelValue protobufListToCelValue(
    protobuf::ListValue list_value,
    protobuf::Arena *arena)
{
  std::vector<cel_runtime::CelValue> cel_values;
  for (auto value : list_value.values())
  {
    auto cel_value = protobufValueToCelValue(value, arena);
    if (cel_value.IsError())
    {
      std::string errorMsg = absl::StrFormat(
          "Cannot convert value found inside of protobuf list: %s",
          cel_value.DebugString());
      return cel_runtime::CreateErrorValue(
          arena,
          errorMsg);
    }
    else
    {
      cel_values.push_back(cel_value);
    }
  }

  // TODO: is CelValue going to handle the deallocation of this object?
  auto cel_list = new cel_runtime::ContainerBackedListImpl(cel_values);

  return cel_runtime::CelValue::CreateList(cel_list);
}