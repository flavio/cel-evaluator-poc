#ifndef __EVALUATE_H__
#define __EVALUATE_H__

#include <string>

class EvaluationResult
{
public:
  EvaluationResult(std::string errorMsg);
  EvaluationResult(bool outcome);

  bool isConstraintSatisfied();
  bool isOk();
  std::string errorMsg();

private:
  bool mIsOk;
  bool mOutcome;
  std::string mErrorMsg;
};

EvaluationResult
evaluate(const std::string &constraint, const std::string &request);

#endif