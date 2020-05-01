/*********************                                                        */
/*! \file proof_checker.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Andrew Reynolds
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2019 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief Boolean proof checker utility
 **/

#include "cvc4_private.h"

#ifndef CVC4__THEORY__BOOLEANS__PROOF_CHECKER_H
#define CVC4__THEORY__BOOLEANS__PROOF_CHECKER_H

#include "expr/node.h"
#include "expr/proof_checker.h"
#include "expr/proof_node.h"

namespace CVC4 {
namespace theory {
namespace booleans {

/** A checker for boolean reasoning in proofs */
class BoolProofRuleChecker : public ProofRuleChecker
{
 public:
  BoolProofRuleChecker() {}
  ~BoolProofRuleChecker() {}
protected:
  /** Return the conclusion of the given proof step, or null if it is invalid */
  Node checkInternal(PfRule id,
             const std::vector<Node>& children,
             const std::vector<Node>& args) override;
};

}  // namespace booleans
}  // namespace theory
}  // namespace CVC4

#endif /* CVC4__THEORY__BOOLEANS__PROOF_CHECKER_H */