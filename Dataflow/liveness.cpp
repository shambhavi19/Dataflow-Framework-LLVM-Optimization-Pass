// ECE/CS 5544 S22 Assignment 2: available.cpp
// Group: Shambhavi Kuthe, Rohit Mehta

////////////////////////////////////////////////////////////////////////////////

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include "dataflow.h"

using namespace llvm;
using namespace std;

//genSet = useSet; killset = defSet; For this pass

namespace{
    
    class Analysis : public Dataflow{

        public:
            Analysis (int size, Domain domain, enum MeetOp mo, enum Direction dir, BitVector bv, BitVector iv) : Dataflow(size, domain, mo, dir, bv, iv) {}
            
            std::map<Value*, int> value_ptr_index_map;   //To convert value pointer to integer
            std::map<int, Value*> index_value_ptr_map;  //To convert integer to value pointer
            BitVector phiSet;

            BBAttributes transferFn(BitVector in, BasicBlock* current)
            {      		
                BBAttributes output;
                BitVector nullSet((int)domain.size(), false);
                BitVector gen = nullSet;
                BitVector kill = nullSet;
                phiSet = nullSet;
                int vectorIdx = 0;
		        int index = 0;
			
                for(auto *v : domain)
                {
                    value_ptr_index_map[(Value*)v] = index;
                    index_value_ptr_map[index] = (Value*)v;
                    index++;
                }


                for (BasicBlock::reverse_iterator i = current->rbegin(), e = current->rend(); i!=e; ++i)
                {
                    Instruction * I = &*i;
                    //Phi Set for Phi nodes
                    if (PHINode *phi = dyn_cast<PHINode> (I)) {
                        for(int z = 0; z < (int)phi->getNumIncomingValues();++z)
                        {
                            Value* phiVal = phi->getIncomingValue(z);
                            if(std::find(domain.begin(),domain.end(),phiVal) != domain.end())
                            {
                                int j = domainIndex(phiVal);
                                if(j != -1)
                                {
                                    phiSet.set(j);
                                }
                            }
                        }
                    } 
                    //Gen Set for non-phi nodes
                    else{

                        for (auto op = I->op_begin(); op != I->op_end(); ++op)
                        {
                            Value* val = *op;
                            if (isa<Instruction>(val) || isa<Argument>(val)) {
                                int i = domainIndex((void*)val);
                                if(i != -1)
                                {
                                    gen.set(i);
                                }
                            }
                        }
                    }

                    //KILL Set & Repeated Gen Set
                    //Finds and kills instruction in the domain if present
                    for(auto vec : domain)
                    {
                        if((((Value*)vec)->getName()).equals(I->getName()))
                        {
                            //If instruction is found set the kill set
                            kill.set(value_ptr_index_map[(Value*)vec]);

                            //If instruction is repeated, remove it from the gen set
                            if(gen[value_ptr_index_map[(Value*)vec]])
                            {
                                gen.reset(value_ptr_index_map[(Value*)vec]);
                            }
                        }

                    }

                }

                output.genSet = gen;
                output.killSet = kill;
                output.tfOut = kill;
                output.tfOut.flip();
                output.tfOut &= in;
                output.tfOut |= gen;
		        phiSet.flip();

                return output;
            }

    };

    class Liveness : public FunctionPass
    {

        public:
            static char ID;

            Liveness() : FunctionPass(ID) {}

        //Function to print the dataflow analysis results
        void displayResultsDFA(std::map<BasicBlock*, BBAttributes*> BBAttrsMap_print, std::vector<string> domain)
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

            std::vector<string> bb_gen_set_exp;
            //Domain bb_gen_set_exp;
            outs() << "use[BB]: ";
            for (int j = 0; j < (int)temp_bb_attr->genSet.size(); ++j)
            {
                if (temp_bb_attr->genSet[j])
                bb_gen_set_exp.push_back(domain[j]);
            }
            printString (&bb_gen_set_exp);

            std::vector<string> bb_kill_set_exp;
            outs() << "def[BB]: ";
            for (int j = 0; j < temp_bb_attr->killSet.size(); ++j)
            {
                if (temp_bb_attr->killSet[j])
                bb_kill_set_exp.push_back(domain[j]);
            }
            printString (&bb_kill_set_exp);

            std::vector<string> bb_in_set_exp;
            outs() << "IN[BB]: ";
            for (int j = 0; j < temp_bb_attr->input.size(); ++j)
            {
                if (temp_bb_attr->input[j])
                    bb_in_set_exp.push_back(domain[j]);
            }
            printString (&bb_in_set_exp);

            std::vector<string> bb_out_set_exp;
            outs() << "OUT[BB]: ";
            for (int j = 0; j < temp_bb_attr->output.size(); ++j)
            {
                if (temp_bb_attr->output[j])
                    bb_out_set_exp.push_back(domain[j]);
            }
            printString (&bb_out_set_exp);
            outs() << "\n";
            }
        }
    

        virtual bool runOnFunction(Function& F)
        {
            outs() << "Liveness DFA\n";
            outs() << "Function name: " << F.getName() << "\n";
            Domain domain;
            std::vector<std::string> domainS;

            for (auto I = inst_begin(F); I != inst_end(F); ++I) {
                if (Instruction* inst = dyn_cast<Instruction> (&*I)) {

                    for(User::op_iterator OI = inst->op_begin(); OI != inst->op_end(); ++OI){		  
                        Value *val = *OI;

                        if (isa<Instruction> (val) || isa<Argument> (val)) {
                            if (std::find(domain.begin(), domain.end(), val) == domain.end()) {
                                domain.push_back(val);
                                domainS.push_back((std::string)(val->getName()));
                            }
                        }
                    }
                }
            }

            outs() << "Variables Domain set: \n";
	        printString(&domainS);

            //Initializing boundary and init vectors
            BitVector boundary(domain.size(), false);
            BitVector init(domain.size(), false);
            //Pass set of expressions(domain), size of domain, meet operator, direction, boundary and initial conditions to dataflow framework 
            //Liveness : Meet operator = Union, Direction = Backward
            Analysis analysis(domain.size(), domain, UNION, BACKWARD, boundary, init);
            //Run dataflow analysis and store result 
	        DFAResult liveness_result = analysis.dataflowAnalysis(F);
            //Display analysis results
	        displayResultsDFA(liveness_result.result, domainS);

            // Did not modify the incoming Function.
            return false;
        }
        
    };

    char Liveness::ID = 3;
    RegisterPass<Liveness> X("liveness", "ECE/CS 5544 Liveness");
};

