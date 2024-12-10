/*!
 * \author Ruben Martins - ruben@sat.inesc-id.pt
 *
 * @section LICENSE
 *
 * Open-WBO, Copyright (c) 2013-2015, Ruben Martins, Vasco Manquinho, Ines Lynce
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <iostream>
#include <cstdlib>
#include <vector>


#include "MaxSATFormula.h"

using namespace openwbo;

MaxSATFormula *MaxSATFormula::copyMaxSATFormula() {
  assert(format == _FORMAT_MAXSAT_);

  MaxSATFormula *copymx = new MaxSATFormula();
  copymx->setInitialVars(nVars());

  for (int i = 0; i < nVars(); i++)
    copymx->newVar();

  for (int i = 0; i < nSoft(); i++)
    copymx->addSoftClause(getSoftClause(i).weight, getSoftClause(i).clause);

  for (int i = 0; i < nHard(); i++)
    copymx->addHardClause(getHardClause(i).clause);

  copymx->setProblemType(getProblemType());
  copymx->updateSumWeights(getSumWeights());
  copymx->setMaximumWeight(getMaximumWeight());
  copymx->setHardWeight(getHardWeight());

  return copymx;
}


void MaxSATFormula::addClauseSimpleValues(std::vector<double> &simpleValues, vec<Lit> &clause, float weight) {
    for (int j = 0; j < clause.size(); j++) {
      if (sign(clause[j]) == false) {
        simpleValues[var(clause[j])] += weight;
      } else {
        simpleValues[var(clause[j])] -= weight;
      }
    }
}

std::vector<double> MaxSATFormula::calculateSimpleValues() {
  // initialed the values and fill them with zeros
  std::vector<double> values(nVars(), 0); 
  for (int i = 0; i < nSoft(); i++) {
    addClauseSimpleValues(values, getSoftClause(i).clause, getSoftClause(i).weight);
  }
  return values;
}


std::vector<double> MaxSATFormula::calculateSimpleValuesOnHardClauses(double hardClauseWeight) {
    std::vector<double> values(nVars(), 0); 
    for (int i = 0; i < nHard(); i++) {
      addClauseSimpleValues(values, getHardClause(i).clause, hardClauseWeight);
    }
    return values;
}


std::vector<double> MaxSATFormula::calculateSimpleValuesOnHardAndSoftClauses(double hardClauseWeight, bool sameRatio, bool preferSoft) {
  if (nHard() == 0) {
    hardClauseWeight = 0;
    sameRatio = false;
    preferSoft = false;
  }

  std::vector<double> values;
  values = calculateSimpleValues();
  if (hardClauseWeight == 0) {
    return values;
  }
  if (sameRatio) {
    float weightsSum = 0;
    for (int i = 0; i < nSoft(); i++) {
      weightsSum += getSoftClause(i).weight;
    }
    hardClauseWeight = (weightsSum * 1.0) / nHard();
    printf("Hard clause weight %f = %f / %u \n", hardClauseWeight, weightsSum, nHard());
  }
  std::vector<double> hardClausesValues;
  hardClausesValues = calculateSimpleValuesOnHardClauses(hardClauseWeight);
  if (preferSoft) {
    for (int i = 0; i < nVars(); i++) {
      if (values[i] == 0) {
        values[i] = hardClausesValues[i];
      }
    }
  } else {
    for (int i = 0; i < nVars(); i++) {
      values[i] += hardClausesValues[i];
    }
  }
  
  return values;
}






void MaxSATFormula::addClauseBanzhafValues(std::vector<double> &banzhafValues, vec<Lit> &clause, float weight) {
    int clause_size = clause.size();
    int clause_size_for_banzhaf = clause_size;
    if (clause_size > 64) {
      clause_size_for_banzhaf = 64;
    }
    double banzhafFactor = ((uint64_t) 1 << (clause_size_for_banzhaf - 1));
    // printf("\n\n%u:%u:%u  \n", (1 << (clause_size - 1)), weight, clause_size);
    for (int j = 0; j < clause_size; j++) {
      // printf("%u:%f:%u:%u  ", var(getSoftClause(i).clause[j]), weight / (1 << (clause_size - 1)), weight, clause_size);
      // The sign is opposite to what makes sences... (false means the lit is possitive)
      if (sign(clause[j]) == false) {
        banzhafValues[var(clause[j])] += weight / banzhafFactor;
      } else {
        banzhafValues[var(clause[j])] -= weight / banzhafFactor;
      }
    }
}

// Calculate Banzhaf values on the soft clauses using the simplified formula:
// value_i = sum over all clauses: [ clause_weight/2^(clause_len - 1) if x_i is not negated else -clause_weight/2^(clause_len - 1) ]
std::vector<double> MaxSATFormula::calculateBanzhafValues() {
  // initialed the values and fill them with zeros
  std::vector<double> banzhafValues(nVars(), 0); 
  for (int i = 0; i < nSoft(); i++) {
    addClauseBanzhafValues(banzhafValues, getSoftClause(i).clause, getSoftClause(i).weight);
  }
  return banzhafValues;
}


// Calculate Banzhaf values on the hard clauses, give hardClauseWeight as the weight of all hard clauses. Similar to calculateBanzhafValues()
std::vector<double> MaxSATFormula::calculateBanzhafValuesOnHardClauses(double hardClauseWeight) {
    std::vector<double> banzhafValues(nVars(), 0); 
    for (int i = 0; i < nHard(); i++) {
      addClauseBanzhafValues(banzhafValues, getHardClause(i).clause, hardClauseWeight);
    }
    return banzhafValues;
}

// Calculate and merge the Banzhaf/Shapley values of the soft and the hard clauses.
// hardClauseWeight - give a constant weight to all the hard clauses.
// sameRatio - Give a hardClauseWeight so that the weight sum on the soft and the hard clauses will be the same.
// preferSoft - Will use the Banzhaf/Shapley values of the hard clauses only when the value on the soft clauses is zero.
std::vector<double> MaxSATFormula::calculateBanzhafOrShpaleyValuesOnHardAndSoftClauses(bool banzhaf, double hardClauseWeight, bool sameRatio, bool preferSoft) {
  if (nHard() == 0) {
    hardClauseWeight = 0;
    sameRatio = false;
    preferSoft = false;
  }

  std::vector<double> values;
  if (banzhaf) {
    values = calculateBanzhafValues();
  } else {
    values = calculateShapleyValues();
  }
  if (hardClauseWeight == 0) {
    return values;
  }
  if (sameRatio) {
    float weightsSum = 0;
    for (int i = 0; i < nSoft(); i++) {
      weightsSum += getSoftClause(i).weight;
    }
    hardClauseWeight = (weightsSum * 1.0) / nHard();
    printf("Hard clause weight %f = %f / %u \n", hardClauseWeight, weightsSum, nHard());
  }
  std::vector<double> hardClausesValues;
  if (banzhaf) {
    hardClausesValues = calculateBanzhafValuesOnHardClauses(hardClauseWeight);
  } else {
    hardClausesValues = calculateShapleyValuesOnHardClauses(hardClauseWeight);
  }
  if (preferSoft) {
    for (int i = 0; i < nVars(); i++) {
      if (values[i] == 0) {
        values[i] = hardClausesValues[i];
      }
    }
  } else {
    for (int i = 0; i < nVars(); i++) {
      values[i] += hardClausesValues[i];
    }
  }
  
  return values;
}

double MaxSATFormula::factorial(const int n)
{
    // returns n!
    double f = 1;
    for (int i=1; i<=n; ++i)
        f *= i;
    return f;
}

double MaxSATFormula::shapleyWeight(const int k, const int n) {
  // todo use unordered map between k+n and the values.
  // this is equvalent to 1 / |S+|*(|S| choose |S+|)    where n = |S| and k=|S+|
  if (n>160) {
    // treat overflow (define min), TODO better solution
    return 0;
  }
  return (1.0 * factorial(k-1) * factorial(n-k)) / factorial(n);
}

// Calculate Shapley values on one given clause and add them to the shapleyValues array:
// value_i = sum over all clauses: [ clause_weight * shapleyWeight(|S+|, |S|) if x_i is not negated else -clause_weight * shapleyWeight(|S-|, |S|) ]
// where |S| is the clause length, |S+| is number of possitive literals and |S-| is number of negative literals.
void MaxSATFormula::addClauseShapleyValues(std::vector<double> &shapleyValues, vec<Lit> &clause, float weight) {
    int clause_size = clause.size();

    // Find |S+| and |S-| (the number of possitive and negative literals in the clause)
    double shapleyPositiveWeight = 0;
    double shapleyNegativeWeight = 0;
    int sPlus = 0;
    int sMinus = 0;
    for (int j = 0; j < clause_size; j++) {
      // sign(clause[j]) is true if the literal is negated
      if (sign(clause[j])) {
        sMinus++;
      } else {
        sPlus++;
      }
    }

    for (int j = 0; j < clause_size; j++) {
      // printf("%u:%f:%u:%u  ", var(getSoftClause(i).clause[j]), weight / (1 << (clause_size - 1)), weight, clause_size);

      // sign(clause[j]) is true if the literal is negated 
      if (sign(clause[j])) {
        if (shapleyNegativeWeight == 0) {
          shapleyNegativeWeight = weight * shapleyWeight(sMinus, clause_size);
        }
        shapleyValues[var(clause[j])] -= shapleyNegativeWeight;
      } else {
        if (shapleyPositiveWeight == 0) {
          shapleyPositiveWeight = weight * shapleyWeight(sPlus, clause_size);
        }
        shapleyValues[var(clause[j])] += shapleyPositiveWeight;
      }
    }
}

// Calculate Shapley values on the soft clauses
std::vector<double> MaxSATFormula::calculateShapleyValues() {
  std::vector<double> shapleyValues(nVars(), 0); 
  for (int i = 0; i < nSoft(); i++) {
    addClauseShapleyValues(shapleyValues, getSoftClause(i).clause, getSoftClause(i).weight);
  }
  return shapleyValues;
}

// Calculate Shapley values on the hard clauses, give hardClauseWeight as the weight of all hard clauses. Similar to calculateShapleyValues()
std::vector<double> MaxSATFormula::calculateShapleyValuesOnHardClauses(double hardClauseWeight) {
  std::vector<double> shapleyValues(nVars(), 0); 
  for (int i = 0; i < nHard(); i++) {
    addClauseShapleyValues(shapleyValues, getHardClause(i).clause, hardClauseWeight);
  }
  return shapleyValues;
}

// Adds a new hard clause to the hard clause database.
void MaxSATFormula::addHardClause(vec<Lit> &lits) {
  total_lit_count += lits.size();
  hard_clauses.push();
  vec<Lit> copy_lits;
  lits.copyTo(copy_lits);
  new (&hard_clauses[hard_clauses.size() - 1]) Hard(copy_lits);
  n_hard++;
}

// Adds a new soft clause to the hard clause database.
void MaxSATFormula::addSoftClause(uint64_t weight, vec<Lit> &lits) {
  total_lit_count += lits.size();
  soft_clauses.push();
  vec<Lit> vars;
  Lit assump = lit_Undef;
  vec<Lit> copy_lits;
  lits.copyTo(copy_lits);

  new (&soft_clauses[soft_clauses.size() - 1])
      Soft(copy_lits, weight, assump, vars);
  n_soft++;
}

// Adds a new soft clause to the hard clause database with predefined relaxation
// variables.
void MaxSATFormula::addSoftClause(uint64_t weight, vec<Lit> &lits,
                                  vec<Lit> &vars) {
    total_lit_count += lits.size();
  soft_clauses.push();
  Lit assump = lit_Undef;
  vec<Lit> copy_lits;
  lits.copyTo(copy_lits);

  new (&soft_clauses[soft_clauses.size() - 1])
      Soft(copy_lits, weight, assump, vars);
  n_soft++;
}

int MaxSATFormula::nInitialVars() {
  return n_initial_vars;
} // Returns the number of variables in the working MaxSAT formula.

void MaxSATFormula::setInitialVars(int vars) { n_initial_vars = vars; }

int MaxSATFormula::nVars() {
  return n_vars;
} // Returns the number of variables in the working MaxSAT formula.

int MaxSATFormula::nSoft() {
  return n_soft;
} // Returns the number of soft clauses in the working MaxSAT formula.

int MaxSATFormula::nHard() {
  return n_hard;
} // Returns the number of hard clauses in the working MaxSAT formula.

uint64_t MaxSATFormula::nTotalLitCount() {
  return total_lit_count;
}

void MaxSATFormula::newVar() {
  n_vars++;
} // Increases the number of variables in the working MaxSAT formula.

// Makes a new literal to be used in the working MaxSAT formula.
Lit MaxSATFormula::newLiteral(bool sign) {
  Lit p = mkLit(nVars(), sign);
  newVar();
  return p;
}

void MaxSATFormula::setProblemType(int type) {
  problem_type = type;
} // Sets the problem type.

int MaxSATFormula::getProblemType() {
  return problem_type; // Return the problem type.
}

// 'ubCost' is initialized to the sum of weights of the soft clauses.
void MaxSATFormula::updateSumWeights(uint64_t weight) {
  if (weight != hard_weight)
    sum_soft_weight += weight;
}

// The initial 'currentWeight' corresponds to the maximum weight of the soft
// clauses.
void MaxSATFormula::setMaximumWeight(uint64_t weight) {
  if (weight > max_soft_weight && weight != hard_weight)
    max_soft_weight = weight;
}

uint64_t MaxSATFormula::getMaximumWeight() { return max_soft_weight; }

void MaxSATFormula::setHardWeight(uint64_t weight) {
  hard_weight = weight;
} // Sets the weight of hard clauses.

Soft &MaxSATFormula::getSoftClause(int pos) {
  assert(pos < nSoft());
  return soft_clauses[pos];
}

Hard &MaxSATFormula::getHardClause(int pos) {
  assert(pos < nHard());
  return hard_clauses[pos];
}

void MaxSATFormula::addPBConstraint(PB *p) {

  // Add constraint to formula data structure.
  if (p->isClause()) {
    addHardClause(p->_lits);
  } else if (p->isCardinality()) {
    if (!p->_sign) {
      p->changeSign();
    }
    cardinality_constraints.push(new Card(p->_lits, p->_rhs));

  } else {
    if (!p->_sign) {
      p->changeSign();
    }

    pb_constraints.push(new PB(p->_lits, p->_coeffs, p->_rhs, p->_sign));
  }
}

int MaxSATFormula::newVarName(char *varName) {
  int id = varID(varName);
  if (id == var_Undef) {
    id = nVars();
    newVar();
    std::string s(varName);
    std::pair<std::string, int> nv(s, id);
    std::pair<int, std::string> ni(id, s);
    _nameToIndex.insert(nv);
    _indexToName.insert(ni);
  }
  return id;
}

int MaxSATFormula::varID(char *varName) {
  std::string s(varName);

  nameMap::const_iterator iter = _nameToIndex.find(s);
  if (iter != _nameToIndex.end()) {
    return iter->second;
  }
  return var_Undef;
}

void MaxSATFormula::convertPBtoMaxSAT() {
  assert(objective_function != NULL);
  vec<Lit> unit_soft(1);

  // Convert objective function to soft clauses
  for (int i = 0; i < objective_function->_lits.size(); i++) {
    assert(objective_function->_coeffs[i] > 0);
    unit_soft[0] = ~objective_function->_lits[i];
    addSoftClause(objective_function->_coeffs[i], unit_soft);

    // Updates the maximum weight of soft clauses.
    setMaximumWeight(objective_function->_coeffs[i]);
    // Updates the sum of the weights of soft clauses.
    updateSumWeights(objective_function->_coeffs[i]);
  }

  if (getMaximumWeight() == 1)
    setProblemType(_UNWEIGHTED_);
  else
    setProblemType(_WEIGHTED_);
}
