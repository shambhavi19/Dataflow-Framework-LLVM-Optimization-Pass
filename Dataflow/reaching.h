#ifndef __REACHING_H__
#define __REACHING_H__

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "dataflow.h"

using namespace llvm;
using namespace std;

class Reaching : public FunctionPass 
{
      private:
    	DFAResult reaching_result;

      void displayResultsDFA(std::map<BasicBlock*, BBAttributes*> BBAttrsMap_print, Domain domain);

      public:
        static char ID;
    	Domain domain;
        Reaching();
        virtual bool runOnFunction(Function &F);

        virtual void getAnalysisUsage(AnalysisUsage &AU) const;

        std::map<BasicBlock*,BBAttributes*> *getReachingResult();

        class Analysis : public Dataflow 
        {
        public:
        	Analysis (int size, Domain domain, enum MeetOp mo, enum Direction dir, BitVector bv, BitVector iv);
        	BBAttributes transferFn (BitVector in, BasicBlock* current);
        };
       
};
#endif
