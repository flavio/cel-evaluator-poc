#include <iostream>
#include <fstream>
#include <sstream>

// CLI FLAGS - BEGIN

#include "absl/flags/parse.h"
#include "absl/flags/flag.h"
#include "absl/flags/usage.h"

ABSL_FLAG(std::string, constraint, "", "The Cel constraint to be evaluated");
ABSL_FLAG(std::string, constraint_file, "", "Read constraint from file");
ABSL_FLAG(std::string, request, "", "The JSON-encoded request to be used");
ABSL_FLAG(std::string, request_file, "", "Read request JSON object from file");

// CLI FLAGS - END

const char *USAGE = R"USAGE(evaluate a Cel constraint against user provided input.

The input is an arbitrary JSON object.

Usage:

  evaluator --constraint '<Cel constraint>' --request '{ <JSON object> }'

It's also possible to read both the constraint and the request from file.
This is done with the `--constraint_file` and `--request_file` flags.
)USAGE";

#include "evaluate.h"

std::string readFileOrDie(std::string filename)
{
  std::ifstream file(filename);
  if (!file.is_open())
  {
    std::cerr << "Cannot read file " << filename << std::endl;
    exit(1);
  }

  return std::string(
      (std::istreambuf_iterator<char>(file)),
      std::istreambuf_iterator<char>());
}

int main(int argc, char **argv)
{
  absl::SetProgramUsageMessage(USAGE);
  absl::ParseCommandLine(argc, argv);

  std::string constraint = absl::GetFlag(FLAGS_constraint);
  auto constraint_file = absl::GetFlag(FLAGS_constraint_file);
  if (constraint_file.empty() && constraint.empty())
  {
    std::cerr << "No constraint to evaluate, use either the `--constraint` or the `--constraint_file` flag" << std::endl;
    return 1;
  }
  else if (!constraint_file.empty() && !constraint.empty())
  {
    std::cerr << "The `--constraint` and the `--constraint_file` flags cannot be used at the same time" << std::endl;
    return 1;
  }
  else if (!constraint_file.empty())
  {
    constraint = readFileOrDie(constraint_file);
  }

  std::string request = absl::GetFlag(FLAGS_request);
  auto request_file = absl::GetFlag(FLAGS_request_file);
  if (request_file.empty() && request.empty())
  {
    std::cerr << "No request to evaluate, use either the `--request` or the `--request_file` flag" << std::endl;
    return 1;
  }
  else if (!request_file.empty() && !request.empty())
  {
    std::cerr << "The `--request` and the `--request_file` flags cannot be used at the same time" << std::endl;
    return 1;
  }
  else if (!request_file.empty())
  {
    request = readFileOrDie(request_file);
  }

  auto eval_res = evaluate(constraint, request);
  if (eval_res.isOk())
  {
    std::cout << "The constraint has "
              << (eval_res.isConstraintSatisfied() ? "" : "not ")
              << "been satisfied" << std::endl;
    return 0;
  }

  std::cerr << "Evaluation could not be done because of an error: "
            << eval_res.errorMsg()
            << std::endl;

  return 1;
}
