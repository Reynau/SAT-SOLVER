#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <vector>
using namespace std;

#define UNDEF -1
#define TRUE 1
#define FALSE 0

#define DIVIDER 2


uint numVars;
uint numClauses;
vector<vector<int> > clauses;
vector<int> model;
vector<int> modelStack;
uint indexOfNextLitToPropagate;
uint decisionLevel;

vector<int> points;

vector< vector<int> > posClauses;
vector< vector<int> > negClauses;

void readClauses( ){
  // Skip comments
  char c = cin.get();
  while (c == 'c') {
    while (c != '\n') c = cin.get();
    c = cin.get();
  }
  // Read "cnf numVars numClauses"
  string aux;
  cin >> aux >> numVars >> numClauses;
  clauses.resize(numClauses);
  points.resize(numVars + 1, 0);
  posClauses.resize(numVars + 1);
  negClauses.resize(numVars + 1);
  // Read clauses
  for (uint i = 0; i < numClauses; ++i) {
    int lit;
    while (cin >> lit and lit != 0) {
      clauses[i].push_back(lit);
      if (lit > 0) posClauses[lit].push_back(i);
      else negClauses[-lit].push_back(i);
    }
  }
}

int currentValueInModel(int lit){
  if (lit >= 0) return model[lit];
  else {
    if (model[-lit] == UNDEF) return UNDEF;
    else return 1 - model[-lit];
  }
}

void setLiteralToTrue(int lit){
  modelStack.push_back(lit);
  if (lit > 0) model[lit] = TRUE;
  else model[-lit] = FALSE;
}

bool propagateGivesConflict ( ) {
  while ( indexOfNextLitToPropagate < modelStack.size() ) {
    int lit = modelStack[indexOfNextLitToPropagate];
    uint size = lit > 0 ? negClauses[lit].size() : posClauses[-lit].size();
    for (uint k = 0; k < size; ++k) {
      int i = lit > 0 ? negClauses[lit][k] : posClauses[-lit][k]; // Position of the clause on clauses array
      bool someLitTrue = false;
      int numUndefs = 0;
      int lastLitUndef = 0;
      for (uint j = 0; !someLitTrue && j < clauses[i].size(); ++j) {
        int val = currentValueInModel(clauses[i][j]);
      	if (val == TRUE) someLitTrue = true;
      	else if (val == UNDEF) {
          ++numUndefs;
          lastLitUndef = clauses[i][j];
        }
      }
      if (!someLitTrue && numUndefs == 0) {
        for (uint k = 0; k < clauses[i].size(); ++k){
          int val = clauses[i][k];
          ++points[abs(val)];
        }
        return true; // conflict! all lits false
      }
      else if (not someLitTrue and numUndefs == 1) setLiteralToTrue(lastLitUndef);
    }
    ++indexOfNextLitToPropagate;
  }
  return false;
}

void backtrack(){
  uint i = modelStack.size() -1;
  int lit = 0;
  while (modelStack[i] != 0){ // 0 is the DL mark
    lit = modelStack[i];
    model[abs(lit)] = UNDEF;
    modelStack.pop_back();
    --i;
  }
  // at this point, lit is the last decision
  modelStack.pop_back(); // remove the DL mark
  --decisionLevel;
  indexOfNextLitToPropagate = modelStack.size();
  setLiteralToTrue(-lit);  // reverse last decision
}

void dividePoints () {
  for (uint i = 0; i < points.size(); ++i) {
    points[i] /= DIVIDER;
  }
}

// Heuristic for finding the next decision literal:
int getNextDecisionLiteral(){
  uint i, bigger = 0;
  for (i = 1; i <= numVars; ++i) {
    if(model[i] == UNDEF) {
      bigger = i;
      break;
    }
  }
  for (i; i <= numVars; ++i) // stupid heuristic:
    if (model[i] == UNDEF && points[i] > points[bigger])
      bigger = i;  // returns first UNDEF var, positively
  if (points[bigger] > numVars) dividePoints();
  return bigger; // reurns 0 when all literals are defined
}

void checkmodel(){
  for (int i = 0; i < numClauses; ++i){
    bool someTrue = false;
    for (int j = 0; not someTrue and j < clauses[i].size(); ++j)
      someTrue = (currentValueInModel(clauses[i][j]) == TRUE);
    if (not someTrue) {
      cout << "Error in model, clause is not satisfied:";
      for (int j = 0; j < clauses[i].size(); ++j) cout << clauses[i][j] << " ";
      cout << endl;
      exit(1);
    }
  }
}

int main(){
  readClauses(); // reads numVars, numClauses and clauses
  model.resize(numVars+1,UNDEF);
  indexOfNextLitToPropagate = 0;
  decisionLevel = 0;

  // Take care of initial unit clauses, if any
  for (uint i = 0; i < numClauses; ++i)
    if (clauses[i].size() == 1) {
      int lit = clauses[i][0];
      int val = currentValueInModel(lit);
      if (val == FALSE) {cout << "UNSATISFIABLE" << endl; return 10;}
      else if (val == UNDEF) setLiteralToTrue(lit);
    }

  // DPLL algorithm
  while (true) {
    while ( propagateGivesConflict() ) {
      if ( decisionLevel == 0) { cout << "UNSATISFIABLE" << endl; return 10; }
      backtrack();
    }
    int decisionLit = getNextDecisionLiteral();
    if (decisionLit == 0) { checkmodel(); cout << "SATISFIABLE" << endl; return 20; }
    // start new decision level:
    modelStack.push_back(0);  // push mark indicating new DL
    ++indexOfNextLitToPropagate;
    ++decisionLevel;
    setLiteralToTrue(decisionLit);    // now push decisionLit on top of the mark
  }
}
