// ECE/CS 5544 S22 Assignment 2: reaching.cpp
// Group: Shambhavi Kuthe, Rohit Mehta


////////////////////////////////////////////////////////////////////////////////

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"

#include "dataflow.h"
#include "reaching.h"

using namespace llvm;
using namespace std;

  static char ID;

  Reaching::Reaching() : FunctionPass(ID) { }

  void Reaching::getAnalysisUsage(AnalysisUsage& AU) const {
    AU.setPreservesAll();
  }

  bool Reaching::runOnFunction(Function &F)
  {
    outs() << "Reaching DFA\n";
    outs() << "Function name: " << F.getName() << "\n";

    Domain domain;
    for (BasicBlock &BB : F)
    {
      for (Instruction &I : BB)
      {
          if (isa<BinaryOperator>(I) || isa<PHINode>(I) || isa<Argument>(I))
          {
              domain.push_back(&I);
          }
      }
    }

    outs() << "DOMAIN set: \n";
    printSetInstr (&domain);

    //Initializing boundary and init vectors
    BitVector boundary(domain.size(), false);
    BitVector init(domain.size(), false);
    //Pass set of expressions(domain), size of domain, meet operator, direction, boundary and initial conditions to dataflow framework 
    //Available expressions : Meet operator = Union, Direction = Forward
    Analysis *analysis = new Analysis(domain.size(), domain, UNION, FORWARD, boundary, init);
    //Run dataflow analysis and store result 
    reaching_result = analysis->dataflowAnalysis(F);
    //Display analysis results
    displayResultsDFA(reaching_result.result, domain);

    return false;

  }

  Reaching::Analysis::Analysis (int size, Domain domain, enum MeetOp mo, enum Direction dir, BitVector bv, BitVector iv) : Dataflow (size, domain, mo, dir, bv, iv) {}

  //Transfer function: Reaching Definitions. Function takes current Basic block and input vector as parameters
  //OUT[BB] = fn(IN[BB])
  //fn = genSet U (IN[BB] - killSet)
  BBAttributes Reaching::Analysis::transferFn(BitVector in, BasicBlock* current)
  {
    BBAttributes output;

    StringRef assigned_var;
    BitVector nullSet((int) domain.size(), false);
    BitVector gen = nullSet;
    BitVector kill = nullSet;

    for (Instruction &I : *current)
    {
      // Gen set
      if (isa<BinaryOperator>(I) || isa<PHINode>(I) || isa<Argument>(I))
      {
        gen.set(domainIndex(&I));
      }

      // Kill set
      // Nothing to kill in SSA form
    }

    output.genSet = gen;
    output.killSet = kill;

    output.tfOut = in;
    output.tfOut |= gen;

    return output;
  }

  //Function to print the dataflow analysis results
  void Reaching::displayResultsDFA(std::map<BasicBlock*, BBAttributes*> BBAttrsMap_print, Domain domain)
  {
    std::map<BasicBlock*, BBAttributes*>::iterator i;

    for (i = BBAttrsMap_print.begin(); i != BBAttrsMap_print.end(); ++i)
    {
      struct BBAttributes *temp_bb_attr = BBAttrsMap_print[i->first];

      if (temp_bb_attr->BB->hasName())
        outs() << "BB Name: " << temp_bb_attr->BB->getName() << "\n";
      else
      {
        outs() << "BB Name: ";
        temp_bb_attr->BB->printAsOperand(outs(), false);
        outs() << "\n";
      }

      Domain bb_gen_set_exp;
      outs() << "gen[BB]: ";
      for (int j = 0; j < temp_bb_attr->genSet.size(); ++j)
      {
        if (temp_bb_attr->genSet[j])
            bb_gen_set_exp.push_back(domain[j]);
      }
      printSetInstr (&bb_gen_set_exp);

      Domain bb_kill_set_exp;
      outs() << "kill[BB]: ";
      for (int j = 0; j < temp_bb_attr->killSet.size(); ++j)
      {
        if (temp_bb_attr->killSet[j])
            bb_kill_set_exp.push_back(domain[j]);
      }
      printSetInstr (&bb_kill_set_exp);

      Domain bb_in_set_exp;
      outs() << "IN[BB]: ";
      for (int j = 0; j < temp_bb_attr->input.size(); ++j)
      {
        if (temp_bb_attr->input[j])
            bb_in_set_exp.push_back(domain[j]);
      }
      printSetInstr (&bb_in_set_exp);

      Domain bb_out_set_exp;
      outs() << "OUT[BB]: ";
      for (int j = 0; j < temp_bb_attr->output.size(); ++j)
      {
        if (temp_bb_attr->output[j])
            bb_out_set_exp.push_back(domain[j]);
      }
      printSetInstr (&bb_out_set_exp);
      outs() << "\n";
    }
  }

  char Reaching::ID = 4;
  RegisterPass<Reaching> X("reaching", "ECE/CS 5544 Reaching");
