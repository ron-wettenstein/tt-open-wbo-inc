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

float* MaxSATFormula::calculateBanzhafValues() {
  float* banzhafValues = new float[nVars()];
  for (int i = 0; i < nVars(); i++) {
    banzhafValues[i] = 0;
  }
  for (int i = 0; i < nSoft(); i++) {
    float weight = getSoftClause(i).weight;
    int clause_size = getSoftClause(i).clause.size();
    // printf("\n\n%u:%u:%u  \n", (1 << (clause_size - 1)), weight, clause_size);
    for (int j = 0; j < clause_size; j++) {
      // printf("%u:%f:%u:%u  ", var(getSoftClause(i).clause[j]), weight / (1 << (clause_size - 1)), weight, clause_size);

      // The sign is opposite to what makes sences...
      if (sign(getSoftClause(i).clause[j]) == false) {
        banzhafValues[var(getSoftClause(i).clause[j])] += weight / (1 << (clause_size - 1));
      } else {
        banzhafValues[var(getSoftClause(i).clause[j])] -= weight / (1 << (clause_size - 1));
      }
    }
  }
  return banzhafValues;
}


float* MaxSATFormula::calculateBanzhafValuesOnHardClauses(float hardClauseWeight) {
  float* banzhafValues = new float[nVars()];
  for (int i = 0; i < nVars(); i++) {
    banzhafValues[i] = 0;
  }
  for (int i = 0; i < nHard(); i++) {
    int clause_size = getHardClause(i).clause.size();
    // printf("\n\n%d:%d:%d  \n", (1 << (clause_size - 1)), hardClauseWeight, clause_size);
    for (int j = 0; j < clause_size; j++) {

      // The sign is opposite to what makes sences...
      if (sign(getHardClause(i).clause[j]) == false) {
        banzhafValues[var(getHardClause(i).clause[j])] += hardClauseWeight / (1 << (clause_size - 1));
      } else {
        banzhafValues[var(getHardClause(i).clause[j])] -= hardClauseWeight / (1 << (clause_size - 1));
      }
      // printf("%u:", var(getHardClause(i).clause[j]));
      // printf("%f:", hardClauseWeight / (1 << (clause_size - 1)));
      // printf("%d:", clause_size);
      // printf(" %f   ", banzhafValues[var(getHardClause(i).clause[j])]);
    }
  }
  return banzhafValues;
}


float* MaxSATFormula::calculateBanzhafValuesOnHardAndSoftClauses(float hardClauseWeight, bool sameRatio, bool preferSoft) {
  float* banzhafValues = calculateBanzhafValues();
  float* hardClausesBanzhafValues = calculateBanzhafValuesOnHardClauses(hardClauseWeight);
  if (sameRatio) {
    float sumSoftValue = 0;
    float sumHardValue = 0;
    int notZeroValues = 0;
    for (int i = 0; i < nVars(); i++) {
      sumHardValue += abs(hardClausesBanzhafValues[i]);
      sumSoftValue += abs(banzhafValues[i]);
      if (banzhafValues[i] != 0 || hardClausesBanzhafValues[i] != 0) {
        notZeroValues += 1;
      }
    }
    float meanSoftValue = sumSoftValue / notZeroValues;
    float meanHardValue = sumHardValue / notZeroValues;
    for (int i = 0; i < nVars(); i++) {
      banzhafValues[i] = (banzhafValues[i] / meanSoftValue + hardClausesBanzhafValues[i] / meanHardValue) * meanSoftValue;
    }

  } else {
    if (preferSoft) {
      for (int i = 0; i < nVars(); i++) {
        if (banzhafValues[i] == 0) {
          banzhafValues[i] = hardClausesBanzhafValues[i];
        }
      }
    } else {
      for (int i = 0; i < nVars(); i++) {
        banzhafValues[i] += hardClausesBanzhafValues[i];
      }
    }
  }
  return banzhafValues;
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
