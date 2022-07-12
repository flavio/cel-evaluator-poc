load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_binary(
    name = "evaluator",
    srcs = ["main.cc"],
    deps = [
      "@com_google_absl//absl/flags:flag",
      "@com_google_absl//absl/flags:parse",
      ":evaluate",
    ],
)

cc_library(
  name = "proto_to_cel",
  srcs = ["proto_to_cel.cc"],
  hdrs = ["proto_to_cel.h"],
  deps = [
    "@com_google_absl//absl/strings:str_format",
    "@com_google_cel_cpp//eval/public:cel_expr_builder_factory",
  ],
)

cc_library(
    name = "evaluate",
    srcs = ["evaluate.cc"],
    hdrs = ["evaluate.h"],
    deps = [
      "@com_google_absl//absl/strings:str_format",
      "@com_google_cel_cpp//eval/public:activation",
      "@com_google_cel_cpp//eval/public:activation_bind_helper",
      "@com_google_cel_cpp//eval/public:builtin_func_registrar",
      "@com_google_cel_cpp//eval/public/structs:cel_proto_wrapper",
      "@com_google_cel_cpp//eval/public/containers:field_access",
      "@com_google_cel_cpp//parser",
      ":proto_to_cel",
    ],
)
