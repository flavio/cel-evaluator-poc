#include "evaluate.h"
#include "proto_to_cel.h"

#include "absl/strings/str_format.h"

// CEL -- BEGIN
#include "eval/public/activation.h"
#include "eval/public/activation_bind_helper.h"
#include "eval/public/builtin_func_registrar.h"
#include "eval/public/cel_expr_builder_factory.h"
#include "eval/public/structs/cel_proto_wrapper.h"
#include "eval/public/containers/field_access.h"
#include "eval/public/containers/container_backed_list_impl.h"
#include "eval/public/containers/container_backed_map_impl.h"
#include "parser/parser.h"
// CEL -- END

#include "google/protobuf/util/json_util.h"

namespace cel_parser = google::api::expr::parser;
namespace cel_runtime = google::api::expr::runtime;

using namespace google;

EvaluationResult::EvaluationResult(std::string errorMsg)
    : mIsOk(false),
      mErrorMsg(errorMsg)
{
}

EvaluationResult::EvaluationResult(bool outcome)
    : mIsOk(true),
      mOutcome(outcome)
{
}

bool EvaluationResult::isOk()
{
  return mIsOk;
}

bool EvaluationResult::isConstraintSatisfied()
{
  return mOutcome;
}

std::string EvaluationResult::errorMsg()
{
  return mErrorMsg;
}

EvaluationResult
evaluate(const std::string &constraint, const std::string &request)
{
  // Parse the CEL constraint
  auto parse_status = cel_parser::Parse(constraint);
  if (!parse_status.ok())
  {
    std::string errorMsg = absl::StrFormat(
        "Cannot parse CEL constraint: %s",
        parse_status.status().ToString());
    return EvaluationResult(errorMsg);
  }

  // Obtain the CEL expression representing that constraint
  auto parsed_expr = parse_status.value();

  protobuf::Arena arena;
  cel_runtime::InterpreterOptions options;
  auto builder = cel_runtime::CreateCelExpressionBuilder(options);
  auto status =
      cel_runtime::RegisterBuiltinFunctions(builder->GetRegistry(), options);

  google::api::expr::v1alpha1::SourceInfo source_info;

  auto cel_expression_status = builder->CreateExpression(
      &parsed_expr.expr(), &source_info);
  if (!cel_expression_status.ok())
  {
    std::string errorMsg = absl::StrFormat(
        "expr_create compile error: %s",
        cel_expression_status.status().message());
    return EvaluationResult(errorMsg);
  }

  auto cel_expr = std::move(cel_expression_status.value());

  cel_runtime::Activation activation;

  protobuf::Value request_protobuf_msg;

  protobuf::util::JsonParseOptions json_parse_options;
  auto status_json_string_to_msg = protobuf::util::JsonStringToMessage(
      request, &request_protobuf_msg, json_parse_options);
  if (!status_json_string_to_msg.ok())
  {
    std::string errorMsg = absl::StrFormat(
        "json to protobuf::Value conversion failed: %s",
        status_json_string_to_msg.ToString());
    return EvaluationResult(errorMsg);
  }

  auto cel_converted_value = protobufValueToCelValue(request_protobuf_msg, &arena);

  activation.InsertValue("request", cel_converted_value);

  auto activation_status = cel_runtime::BindProtoToActivation(
      &request_protobuf_msg,
      &arena,
      &activation);
  if (!activation_status.ok())
  {
    std::string errorMsg = absl::StrFormat(
        "bind proto activation failed: %s",
        activation_status.ToString());
    return EvaluationResult(errorMsg);
  }

  auto eval_status = cel_expr->Evaluate(activation, &arena);
  if (!eval_status.ok())
  {
    std::string errorMsg = absl::StrFormat(
        "evaluation failed with an error: %s",
        eval_status.status().ToString());
    return EvaluationResult(errorMsg);
  }

  cel_runtime::CelValue result = eval_status.value();
  if (result.IsError())
  {
    std::string errorMsg = absl::StrFormat(
        "Evaluation error: %s",
        result.ErrorOrDie()->ToString());
    return EvaluationResult(errorMsg);
  }
  if (!result.IsBool())
  {
    std::string errorMsg = absl::StrFormat(
        "Evaluation didn't return a boolean value as expected: %s",
        result.DebugString());
    return EvaluationResult(errorMsg);
  }

  return EvaluationResult(result.BoolOrDie());
}