/*********************                                                        */
/*! \file quant_util.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Andrew Reynolds, Morgan Deters, Tim King
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2017 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief quantifier util
 **/

#include "cvc4_private.h"

#ifndef __CVC4__THEORY__QUANT_UTIL_H
#define __CVC4__THEORY__QUANT_UTIL_H

#include <iostream>
#include <map>

#include "theory/theory.h"
#include "theory/uf/equality_engine.h"

namespace CVC4 {
namespace theory {

class QuantifiersEngine;

namespace quantifiers {
  class TermDb;
  class TermUtil;
}

class QuantifiersModule {
protected:
  QuantifiersEngine* d_quantEngine;
public:
  QuantifiersModule( QuantifiersEngine* qe ) : d_quantEngine( qe ){}
  virtual ~QuantifiersModule(){}
  //get quantifiers engine
  QuantifiersEngine* getQuantifiersEngine() { return d_quantEngine; }
  /** presolve */
  virtual void presolve() {}
  /* whether this module needs to check this round */
  virtual bool needsCheck( Theory::Effort e ) { return e>=Theory::EFFORT_LAST_CALL; }
  /* whether this module needs a model built */
  virtual unsigned needsModel( Theory::Effort e );
  /* reset at a round */
  virtual void reset_round( Theory::Effort e ){}
  /* Call during quantifier engine's check */
  virtual void check( Theory::Effort e, unsigned quant_e ) = 0;
  /* check was complete, return false if there is no way to answer "SAT", true if maybe can answer "SAT" */
  virtual bool checkComplete() { return true; }
  /* check was complete for quantified formula q (e.g. no lemmas implies a model) */
  virtual bool checkCompleteFor( Node q ) { return false; }
  /* Called for new quantified formulas */
  virtual void preRegisterQuantifier( Node q ) { }
  /* Called for new quantifiers after owners are finalized */
  virtual void registerQuantifier( Node q ) = 0;
  virtual void assertNode( Node n ) {}
  virtual void propagate( Theory::Effort level ){}
  virtual Node getNextDecisionRequest( unsigned& priority ) { return TNode::null(); }
  /** Identify this module (for debugging, dynamic configuration, etc..) */
  virtual std::string identify() const = 0;
public:
  eq::EqualityEngine * getEqualityEngine();
  bool areDisequal( TNode n1, TNode n2 );
  bool areEqual( TNode n1, TNode n2 );
  TNode getRepresentative( TNode n );
  quantifiers::TermDb * getTermDatabase();
  quantifiers::TermUtil * getTermUtil();
};/* class QuantifiersModule */

class QuantifiersUtil {
public:
  QuantifiersUtil(){}
  virtual ~QuantifiersUtil(){}
  /* reset at a round */
  virtual bool reset( Theory::Effort e ) = 0;
  /** Identify this module (for debugging, dynamic configuration, etc..) */
  virtual std::string identify() const = 0;
};

/** Arithmetic utilities regarding monomial sums.
 *
 * Note the following terminology:
 *
 *   We say Node c is a {monomial constant} (or m-constant) if either:
 *   (a) c is a constant Rational, or
 *   (b) c is null.
 *
 *   We say Node v is a {monomial variable} (or m-variable) if either:
 *   (a) v.getType().isReal() and v is not a constant, or
 *   (b) v is null.
 *
 *   For m-constant or m-variable t, we write [t] to denote 1 if t.isNull() and
 *   t otherwise.
 *
 *   A monomial m is a pair ( mvariable, mconstant ) of the form ( v, c ), which
 *   is interpreted as [c]*[v].
 *
 *   A {monmoial sum} msum is represented by a std::map< Node, Node > having
 *   key-value pairs of the form ( mvariable, mconstant ).
 *   It is interpreted as:
 *   [msum] = sum_{( v, c ) \in msum } [c]*[v]
 *   It is critical that this map is ordered so that operations like adding
 *   two monomial sums can be done efficiently. The ordering itself is not 
 *   important, and currently corresponds to the default ordering on Nodes.
 *
 * The following has utilities involving monmoial sums.
 *
 */
class QuantArith
{
public:
 /** get monomial
  *
  * If n = n[0]*n[1] where n[0] is constant and n[1] is not,
  * this function returns true, sets c to n[0] and v to n[1].
  */
 static bool getMonomial(Node n, Node& c, Node& v);

 /** get monomial
  *
  * If this function returns true, it adds the ( m-constant, m-variable )
  * pair corresponding to the monomial representation of n to the
  * monomial sum msum.
  *
  * This function returns false if the m-variable of n is already
  * present in n.
  */
 static bool getMonomial(Node n, std::map<Node, Node>& msum);

 /** get monomial sum for real-valued term n
  *
  * If this function returns true, it sets msum to a monmoial sum such that
  *   [msum] is equivalent to n
  *
  * This function may return false if n is not a sum of monomials
  * whose variables are pairwise unique.
  * If term n is in rewritten form, this function should always return true.
  */
 static bool getMonomialSum(Node n, std::map<Node, Node>& msum);

 /** get monmoial sum literal for literal lit
  *
  * If this function returns true, it sets msum to a monmoial sum such that
  *   [msum] <k> 0  is equivalent to lit[0] <k> lit[1]
  * where k is the Kind of lit, one of { EQUAL, GEQ }.
  *
  * This function may return false if either side of lit is not a sum
  * of monomials whose variables are pairwise unique on that side.
  * If literal lit is in rewritten form, this function should always return
  * true.
  */
 static bool getMonomialSumLit(Node lit, std::map<Node, Node>& msum);

 /** make node for monomial sum
  *
  * Make the Node corresponding to the interpretation of msum, [msum], where:
  *   [msum] = sum_{( v, c ) \in msum } [c]*[v]
  */
 static Node mkNode(std::map<Node, Node>& msum);

 /** make coefficent term
  *
  * Input coeff is a m-constant.
  * Returns the term t if coeff.isNull() or coeff*t otherwise.
  */
 static Node mkCoeffTerm(Node coeff, Node t);

 /** isolate variable v in constraint ([msum] <k> 0)
  *
  * If this function returns a value ret where ret != 0, then
  * veq_c is set to m-constant, and val is set to a term such that:
  *    If ret=1, then ([veq_c] * v <k> val) is equivalent to [msum] <k> 0.
  *   If ret=-1, then (val <k> [veq_c] * v) is equivalent to [msum] <k> 0.
  *   If veq_c is non-null, then it is a positive constant Rational.
  * The returned value of veq_c is only non-null if v has integer type.
  *
  * This function returns 0 indicating a failure if msum does not contain
  * a (non-zero) monomial having mvariable v.
  */
 static int isolate(
     Node v, std::map<Node, Node>& msum, Node& veq_c, Node& val, Kind k);

 /** isolate variable v in constraint ([msum] <k> 0)
  *
  * If this function returns a value ret where ret != 0, then veq
  * is set to a literal that is equivalent to ([msum] <k> 0), and:
  *    If ret=1, then veq is of the form ( v <k> val) if veq_c.isNull(),
  *                   or ([veq_c] * v <k> val) if !veq_c.isNull().
  *   If ret=-1, then veq is of the form ( val <k> v) if veq_c.isNull(),
  *                   or (val <k> [veq_c] * v) if !veq_c.isNull().
  * If doCoeff = false or v does not have Integer type, then veq_c is null.
  *
  * This function returns 0 indiciating a failure if msum does not contain
  * a (non-zero) monomial having variable v, or if veq_c must be non-null
  * for an integer constraint and doCoeff is false.
  */
 static int isolate(Node v,
                    std::map<Node, Node>& msum,
                    Node& veq,
                    Kind k,
                    bool doCoeff = false);

 /** solve equality lit for variable
  *
  * If return value ret is non-null, then:
  *    v = ret is equivalent to lit.
  *
  * This function may return false if lit does not contain v,
  * or if lit is an integer equality with a coefficent on v,
  * e.g. 3*v = 7.
  */
 static Node solveEqualityFor(Node lit, Node v);

 /** decompose real-valued term n
 *
 * If this function returns true, then
 *   ([coeff]*v + rem) is equivalent to n
 * where coeff is non-zero m-constant.
 *
 * This function will return false if n is not a monomial sum containing
 * a monomial with factor v.
 */
 static bool decompose(Node n, Node v, Node& coeff, Node& rem);

 /** return the rewritten form of (UMINUS t) */
 static Node negate(Node t);

 /** return the rewritten form of (PLUS t (CONST_RATIONAL i)) */
 static Node offset(Node t, int i);

 /** debug print for a monmoial sum, prints to Trace(c) */
 static void debugPrintMonomialSum(std::map<Node, Node>& msum, const char* c);
};


class QuantRelevance
{
private:
  /** for computing relevance */
  bool d_computeRel;
  /** map from quantifiers to symbols they contain */
  std::map< Node, std::vector< Node > > d_syms;
  /** map from symbols to quantifiers */
  std::map< Node, std::vector< Node > > d_syms_quants;
  /** relevance for quantifiers and symbols */
  std::map< Node, int > d_relevance;
  /** compute symbols */
  void computeSymbols( Node n, std::vector< Node >& syms );
public:
  QuantRelevance( bool cr ) : d_computeRel( cr ){}
  ~QuantRelevance(){}
  /** register quantifier */
  void registerQuantifier( Node f );
  /** set relevance */
  void setRelevance( Node s, int r );
  /** get relevance */
  int getRelevance( Node s ) { return d_relevance.find( s )==d_relevance.end() ? -1 : d_relevance[s]; }
  /** get number of quantifiers for symbol s */
  int getNumQuantifiersForSymbol( Node s ) { return (int)d_syms_quants[s].size(); }
};

class QuantPhaseReq
{
private:
  /** helper functions compute phase requirements */
  void computePhaseReqs( Node n, bool polarity, std::map< Node, int >& phaseReqs );
public:
  QuantPhaseReq(){}
  QuantPhaseReq( Node n, bool computeEq = false );
  ~QuantPhaseReq(){}
  void initialize( Node n, bool computeEq );
  /** is phase required */
  bool isPhaseReq( Node lit ) { return d_phase_reqs.find( lit )!=d_phase_reqs.end(); }
  /** get phase requirement */
  bool getPhaseReq( Node lit ) { return d_phase_reqs.find( lit )==d_phase_reqs.end() ? false : d_phase_reqs[ lit ]; }
  /** phase requirements for each quantifier for each instantiation literal */
  std::map< Node, bool > d_phase_reqs;
  std::map< Node, bool > d_phase_reqs_equality;
  std::map< Node, Node > d_phase_reqs_equality_term;

  static void getPolarity( Node n, int child, bool hasPol, bool pol, bool& newHasPol, bool& newPol );
  static void getEntailPolarity( Node n, int child, bool hasPol, bool pol, bool& newHasPol, bool& newPol );
};


class EqualityQuery : public QuantifiersUtil {
public:
  EqualityQuery(){}
  virtual ~EqualityQuery(){};
  /** extends engine */
  virtual bool extendsEngine() { return false; }
  /** contains term */
  virtual bool hasTerm( Node a ) = 0;
  /** get the representative of the equivalence class of a */
  virtual Node getRepresentative( Node a ) = 0;
  /** returns true if a and b are equal in the current context */
  virtual bool areEqual( Node a, Node b ) = 0;
  /** returns true is a and b are disequal in the current context */
  virtual bool areDisequal( Node a, Node b ) = 0;
  /** get the equality engine associated with this query */
  virtual eq::EqualityEngine* getEngine() = 0;
  /** get the equivalence class of a */
  virtual void getEquivalenceClass( Node a, std::vector< Node >& eqc ) = 0;
  /** get the term that exists in EE that is congruent to f with args (f is returned by TermDb::getMatchOperator(...) */
  virtual TNode getCongruentTerm( Node f, std::vector< TNode >& args ) = 0;
};/* class EqualityQuery */

class QuantEPR
{
private:
  void registerNode( Node n, std::map< int, std::map< Node, bool > >& visited, bool beneathQuant, bool hasPol, bool pol );
  /** non-epr */
  std::map< TypeNode, bool > d_non_epr;
  /** axioms for epr */
  std::map< TypeNode, Node > d_epr_axiom;
public:
  QuantEPR(){}
  virtual ~QuantEPR(){}
  /** constants per type */
  std::map< TypeNode, std::vector< Node > > d_consts;
  /* reset */
  //bool reset( Theory::Effort e ) {}
  /** identify */
  //std::string identify() const { return "QuantEPR"; }
  /** register assertion */
  void registerAssertion( Node assertion );
  /** finish init */
  void finishInit();
  /** is EPR */
  bool isEPR( TypeNode tn ) const { return d_non_epr.find( tn )==d_non_epr.end(); }
  /** is EPR constant */
  bool isEPRConstant( TypeNode tn, Node k ); 
  /** add EPR constant */
  void addEPRConstant( TypeNode tn, Node k ); 
  /** get EPR axiom */
  Node mkEPRAxiom( TypeNode tn );
  /** has EPR axiom */
  bool hasEPRAxiom( TypeNode tn ) const { return d_epr_axiom.find( tn )!=d_epr_axiom.end(); }
};

class TermRecBuild {
private:
  std::vector< Node > d_term;
  std::vector< std::vector< Node > > d_children;
  std::vector< Kind > d_kind;
  std::vector< bool > d_has_op;
  std::vector< unsigned > d_pos;
  void addTerm( Node n );
public:
  TermRecBuild(){}
  void init( Node n );
  void push( unsigned p );
  void pop();
  void replaceChild( unsigned i, Node n );
  Node getChild( unsigned i );
  Node build( unsigned p=0 );
};

}
}

#endif
