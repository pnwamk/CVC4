/*********************                                                        */
/*! \file solver_state.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Andrew Reynolds
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2019 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief The solver state of the theory of strings
 **/

#include "cvc4_private.h"

#ifndef CVC4__THEORY__STRINGS__SOLVER_STATE_H
#define CVC4__THEORY__STRINGS__SOLVER_STATE_H

#include <map>

#include "context/cdo.h"
#include "context/context.h"
#include "expr/node.h"
#include "theory/uf/equality_engine.h"
#include "theory/valuation.h"

namespace CVC4 {
namespace theory {
namespace strings {

/**
 * SAT-context-dependent information about an equivalence class. This
 * information is updated eagerly as assertions are processed by the theory of
 * strings at standard effort.
 */
class EqcInfo
{
 public:
  EqcInfo(context::Context* c);
  ~EqcInfo() {}
  /** add prefix constant
   *
   * This informs this equivalence class info that a term t in its
   * equivalence class has a constant prefix (if isSuf=true) or suffix
   * (if isSuf=false). The constant c (if non-null) is the value of that
   * constant, if it has been computed already.
   *
   * If this method returns a non-null node ret, then ret is a conjunction
   * corresponding to a conflict that holds in the current context.
   */
  Node addEndpointConst(Node t, Node c, bool isSuf);
  /**
   * If non-null, this is a term x from this eq class such that str.len( x )
   * occurs as a term in this SAT context.
   */
  context::CDO<Node> d_lengthTerm;
  /**
   * If non-null, this is a term x from this eq class such that str.code( x )
   * occurs as a term in this SAT context.
   */
  context::CDO<Node> d_codeTerm;
  context::CDO<unsigned> d_cardinalityLemK;
  context::CDO<Node> d_normalizedLength;
  /**
   * A node that explains the longest constant prefix known of this
   * equivalence class. This can either be:
   * (1) A term from this equivalence class, including a constant "ABC" or
   * concatenation term (str.++ "ABC" ...), or
   * (2) A membership of the form (str.in.re x R) where x is in this
   * equivalence class and R is a regular expression of the form
   * (str.to.re "ABC") or (re.++ (str.to.re "ABC") ...).
   */
  context::CDO<Node> d_prefixC;
  /** same as above, for suffix. */
  context::CDO<Node> d_suffixC;
};

/**
 * Solver state for strings.
 *
 * The purpose of this class is track and provide query functions for the state
 * of the assertions for the theory of strings. This includes:
 * (1) Equality queries via the equality engine,
 * (2) Whether the set of assertions is in conflict.
 * (3) Equivalence class information as in the class above.
 */
class SolverState
{
 public:
  SolverState(context::Context* c, eq::EqualityEngine& ee, Valuation& v);
  ~SolverState();
  //-------------------------------------- equality information
  /**
   * Get the representative of t in the equality engine of this class, or t
   * itself if it is not registered as a term.
   */
  Node getRepresentative(Node t) const;
  /** Is t registered as a term in the equality engine of this class? */
  bool hasTerm(Node a) const;
  /**
   * Are a and b equal according to the equality engine of this class? Also
   * returns true if a and b are identical.
   */
  bool areEqual(Node a, Node b) const;
  /**
   * Are a and b disequal according to the equality engine of this class? Also
   * returns true if the representative of a and b are distinct constants.
   */
  bool areDisequal(Node a, Node b) const;
  /** get equality engine */
  eq::EqualityEngine* getEqualityEngine() const;
  //-------------------------------------- end equality information
  //------------------------------------------ conflicts
  /**
   * Set that the current state of the solver is in conflict. This should be
   * called immediately after a call to conflict(...) on the output channel of
   * the theory of strings.
   */
  void setConflict();
  /** Are we currently in conflict? */
  bool isInConflict() const;
  /** set pending conflict
   *
   * If conf is non-null, this is called when conf is a conjunction of literals
   * that hold in the current context that are unsatisfiable. It is set as the
   * "pending conflict" to be processed as a conflict lemma on the output
   * channel of this class. It is not sent out immediately since it may require
   * explanation from the equality engine, and may be called at any time, e.g.
   * during a merge operation, when the equality engine is not in a state to
   * provide explanations.
   */
  void setPendingConflictWhen(Node conf);
  /** get the pending conflict, or null if none exist */
  Node getPendingConflict() const;
  //------------------------------------------ end conflicts
  /** get length with explanation
   *
   * If possible, this returns an arithmetic term that exists in the current
   * context that is equal to the length of te, or otherwise returns the
   * length of t. It adds to exp literals that hold in the current context that
   * explain why that term is equal to the length of t. For example, if
   * we have assertions:
   *   len( x ) = 5 ^ z = x ^ x = y,
   * then getLengthExp( z, exp, y ) returns len( x ) and adds { z = x } to
   * exp. On the other hand, getLengthExp( z, exp, x ) returns len( x ) and
   * adds nothing to exp.
   */
  Node getLengthExp(Node t, std::vector<Node>& exp, Node te);
  /** shorthand for getLengthExp(t, exp, t) */
  Node getLength(Node t, std::vector<Node>& exp);
  /**
   * Get the above information for equivalence class eqc. If doMake is true,
   * we construct a new information class if one does not exist. The term eqc
   * should currently be a representative of the equality engine of this class.
   */
  EqcInfo* getOrMakeEqcInfo(Node eqc, bool doMake = true);

  /** add endpoints to eqc info
   *
   * This method is called when term t is the explanation for why equivalence
   * class eqc may have a constant endpoint due to a concatentation term concat.
   * For example, we may call this method on:
   *   t := (str.++ x y), concat := (str.++ x y), eqc
   * for some eqc that is currently equal to t. Another example is:
   *   t := (str.in.re z (re.++ r s)), concat := (re.++ r s), eqc
   * for some eqc that is currently equal to z.
   */
  void addEndpointsToEqcInfo(Node t, Node concat, Node eqc);
  /** Entailment check
   *
   * This calls entailmentCheck on the Valuation object of theory of strings.
   */
  std::pair<bool, Node> entailmentCheck(TheoryOfMode mode, TNode lit);
  /** Separate by length
   *
   * Separate the string representatives in argument n into a partition cols
   * whose collections have equal length. The i^th vector in cols has length
   * lts[i] for all elements in col.
   */
  void separateByLength(const std::vector<Node>& n,
                        std::vector<std::vector<Node> >& cols,
                        std::vector<Node>& lts);

 private:
  /** Pointer to the SAT context object used by the theory of strings. */
  context::Context* d_context;
  /** Reference to equality engine of the theory of strings. */
  eq::EqualityEngine& d_ee;
  /** Reference to the valuation of the theory of strings */
  Valuation& d_valuation;
  /** Are we in conflict? */
  context::CDO<bool> d_conflict;
  /** The pending conflict if one exists */
  context::CDO<Node> d_pendingConflict;
  /** Map from representatives to their equivalence class information */
  std::map<Node, EqcInfo*> d_eqcInfo;
}; /* class TheoryStrings */

}  // namespace strings
}  // namespace theory
}  // namespace CVC4

#endif /* CVC4__THEORY__STRINGS__THEORY_STRINGS_H */