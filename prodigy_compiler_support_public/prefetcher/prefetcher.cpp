/*

BSD 3-Clause License

Copyright (c) 2021, Kuba Kaszyk and Chris Vasiladiotis
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// LLVM
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

// standard
#include <fstream>
#include <string>
#include <vector>

// project
#include "prefetcher.hpp"
#include "util.hpp"

// Register Pass
#include "llvm/IR/LegacyPassManager.h"

// Clone Function
#include "llvm/Transforms/Utils/Cloning.h"

#include "llvm/Support/CommandLine.h"

#define DEBUG 1
#define MAX_STACK_COUNT 2

llvm::cl::opt<std::string>
    FunctionWhiteListFile("func-wl-file", llvm::cl::Hidden,
                          llvm::cl::desc("function whitelist file"));

namespace {
bool getAllocationSizeCalc(llvm::Value &I, std::set<llvm::Value *> &vals,
                           int stack_count = 0) {
    bool ret = false;

    // Limit recursion depth to prevent stack overflow
    if (stack_count >= 200) {
        return false;
    }

    if (llvm::Instruction *Instr = llvm::dyn_cast<llvm::Instruction>(&I)) {
        for (int i = 0; i < Instr->getNumOperands(); ++i) {
            
            if (auto *user = llvm::dyn_cast<llvm::Instruction>(Instr->getOperand(i))) {
                
                // Modern LLVM way to check for a Call instruction
                if (auto *Call = llvm::dyn_cast<llvm::CallInst>(user)) {
                    
                    // CRITICAL FIX: Ensure the called function is not null (not an indirect call)
                    if (llvm::Function *F = Call->getCalledFunction()) {
                        
                        // Check if it's our specific intrinsic. 
                        // Using StringRef (==) is safe and fast here.
                        if (F->getName() == "llvm.umul.with.overflow.i64") {
                            ret = true;
                            vals.insert(user);
                            return true; // Fast exit
                        }
                    }
                }

                // Pass stack_count + 1 to correctly measure depth!
                ret |= getAllocationSizeCalc(*user, vals, stack_count + 1);
            }
        }
    }

    return ret;
}

void identifyNewA(llvm::Function &F,
                  llvm::SmallVectorImpl<myAllocCallInfo> &allocInfos) {
  for (llvm::BasicBlock &BB : F) {
    for (llvm::Instruction &I : BB) {
      auto *CB = llvm::dyn_cast<llvm::CallBase>(&I);
      if (!CB) {
        continue;
      }
      llvm::Value *called = CB->getCalledOperand()->stripPointerCasts();

      if (llvm::Function *f = llvm::dyn_cast<llvm::Function>(called)) {
        if (f->getName().equals("_Znam")) {
          std::set<llvm::Value *> vals;
          getAllocationSizeCalc(*(CB->getArgOperand(0)), vals);
          if (vals.size() > 0) {
            for (auto v : vals) {
              auto *sizeCB = llvm::dyn_cast<llvm::CallBase>(v);
              myAllocCallInfo allocInfo;
              allocInfo.allocInst = &I;
              allocInfo.inputArguments.insert(allocInfo.inputArguments.end(),
                                              CB->getArgOperand(0));
              allocInfo.inputArguments.insert(allocInfo.inputArguments.end(),
                                              sizeCB->getArgOperand(1));
              allocInfos.push_back(allocInfo);
            }
          } else {
          }
        }
      }
    }
  }
}

bool isTargetGEPusedInLoad(llvm::Instruction *I) {
  for (auto &u : I->uses()) {
    auto *user = llvm::dyn_cast<llvm::Instruction>(u.getUser());

    if (user->getOpcode() == Instruction::Load) {
      return true;
    }
  }

  return false;
}

llvm::Instruction *findGEPToSameBasePtr(llvm::Function &F,
                                        llvm::Instruction &firstI) {
  for (llvm::BasicBlock &BB : F) {
    for (llvm::Instruction &I : BB) {
      if (I.getOpcode() == llvm::Instruction::GetElementPtr &&
          firstI.getOperand(0) == I.getOperand(0) && &firstI != &I) {
        return &I;
      }
    }
  }
  return nullptr;
}

bool areUsedInComparisonOp(Instruction *I, Instruction *I2) {
  llvm::SmallVector<llvm::Instruction *, 8> I_loads;
  llvm::SmallVector<llvm::Instruction *, 8> I2_loads;

  for (auto &u : I->uses()) {
    auto *user = llvm::dyn_cast<llvm::Instruction>(u.getUser());
    if (dyn_cast<llvm::Instruction>(user)->getOpcode() ==
        llvm::Instruction::Load) {
      I_loads.push_back(dyn_cast<llvm::Instruction>(user));
    }
  }

  for (auto &u : I2->uses()) {
    auto *user = llvm::dyn_cast<llvm::Instruction>(u.getUser());
    if (dyn_cast<llvm::Instruction>(user)->getOpcode() ==
        llvm::Instruction::Load) {
      I2_loads.push_back(dyn_cast<llvm::Instruction>(user));
    }
  }

  for (auto l1 : I_loads) {
    for (auto l2 : I2_loads) {
      for (auto &l1_u : l1->uses()) {
        auto *user = llvm::dyn_cast<llvm::Instruction>(l1_u.getUser());
        if (dyn_cast<llvm::Instruction>(user)->getOpcode() ==
            llvm::Instruction::ICmp) {
          for (auto &l2_u : l2->uses()) {
            auto *user_2 = llvm::dyn_cast<llvm::Instruction>(l2_u.getUser());
            if (user == user_2) {
              return true;
            }
          }
        }
      }
    }
  }

  return false;
}

void findSourceGEPCandidates(
    Function &F, llvm::SmallVectorImpl<llvm::Instruction *> &source_geps) {
  for (llvm::BasicBlock &BB : F) {
    for (llvm::Instruction &I : BB) {
      if (I.getOpcode() == Instruction::GetElementPtr) {
        source_geps.push_back(&I);
      }
    }
  }
}

void getLoadsUsingSourceGEP(llvm::Instruction *I, 
                            llvm::SmallVectorImpl<llvm::Instruction*> &loads, 
                            int depth = 0) {
    // 1. Safe depth check right at the top
    if (depth >= 20) {
        return;
    }

    // 2. Modern LLVM iteration directly over users
    for (llvm::User *U : I->users()) {
        // 3. Safe cast: ensure the user is actually an Instruction
        if (auto *UserInst = llvm::dyn_cast<llvm::Instruction>(U)) {
            
            // 4. Idiomatic type checking
            if (auto *Load = llvm::dyn_cast<llvm::LoadInst>(UserInst)) {
                loads.push_back(Load);
                // We do NOT return here. We want to find ALL loads in the chain.
            } 
            else if (!llvm::isa<llvm::GetElementPtrInst>(UserInst) && 
                     !llvm::isa<llvm::StoreInst>(UserInst)) {
                
                // 5. Pass depth + 1 (NOT ++depth) to avoid mutating the current frame
                getLoadsUsingSourceGEP(UserInst, loads, depth + 1);
            }
        }
    }
}

void getGEPsUsingLoad(llvm::Instruction *I, 
                      llvm::SmallVectorImpl<llvm::Instruction*> &target_geps, 
                      int depth = 0) {
    // 1. Safe depth check at the top
    if (depth >= 5) {
        return;
    }

    for (llvm::User *U : I->users()) {
        if (auto *UserInst = llvm::dyn_cast<llvm::Instruction>(U)) {
            
            if (auto *GEP = llvm::dyn_cast<llvm::GetElementPtrInst>(UserInst)) {
                // Ensure the GEP has at least 2 operands to prevent out-of-bounds access.
                // Operand(1) is the first index.
                if (GEP->getNumOperands() > 1 && GEP->getOperand(1) == I) {
                    target_geps.push_back(GEP);
                }
            }

            // Recurse for ALL instruction types (which matches your original logic)
            // Passing depth + 1 safely limits the tree depth
            getGEPsUsingLoad(UserInst, target_geps, depth + 1);
        }
    }
}

bool RIfindLoadUsingGEP(llvm::Instruction *src,
                        std::vector<llvm::Instruction *> &targets,
                        int stack_count = 0) {
  bool ret = false;
  for (auto &u : src->uses()) {
    auto *user = llvm::dyn_cast<llvm::Instruction>(u.getUser());

    if (user->getOpcode() == llvm::Instruction::Load) {
      targets.push_back(user);
      return true;
    }

    if (stack_count < MAX_STACK_COUNT) {
      ret |= RIfindLoadUsingGEP(user, targets, ++stack_count);
    }
  }
  return ret;
}

void identifyCorrectRangedIndirection(
    Function &F, llvm::SmallVectorImpl<GEPDepInfo> &riInfos) {
  for (llvm::BasicBlock &BB : F) {
    for (llvm::Instruction &I : BB) {
      if (I.getOpcode() == llvm::Instruction::GetElementPtr) {
        llvm::Instruction *otherGEP = findGEPToSameBasePtr(F, I);
        if (otherGEP) {
          GEPDepInfo gepdepinfo;
          if (areUsedInComparisonOp(&I, otherGEP)) {
            std::vector<llvm::Instruction *> targets;
            bool found_load = RIfindLoadUsingGEP(&I, targets);
            if (found_load) {
              gepdepinfo.source = I.getOperand(0);
              gepdepinfo.target = targets.at(0);
              riInfos.push_back(gepdepinfo);
            }
          }
        }
      }
    }
  }
}

void identifyCorrectGEPDependence(Function &F,
                                  llvm::SmallVectorImpl<GEPDepInfo> &gepInfos) {

  llvm::SmallVector<llvm::Instruction *, 8> source_geps;
  findSourceGEPCandidates(F, source_geps);

  for (auto I : source_geps) {
    llvm::SmallVector<llvm::Instruction *, 8> loads;
    getLoadsUsingSourceGEP(I, loads);

    for (auto ld : loads) {
      llvm::SmallVector<llvm::Instruction *, 8> target_geps;
      getGEPsUsingLoad(ld, target_geps);
      for (auto target_gep : target_geps) {
        if (isTargetGEPusedInLoad(target_gep)) {
          GEPDepInfo g;
          g.source = I->getOperand(0);
          g.source_use = ld;
          g.funcSource = I->getParent()->getParent();
          g.target = target_gep->getOperand(0);
          g.funcTarget = target_gep->getParent()->getParent();
          gepInfos.push_back(g);
          // If the source GEP comes from a PHI node, we use the result of the
          // phi node as the source edge, and insert the registration call
          // immediately after the phi nodes
          if (dyn_cast<llvm::Instruction>(ld->getOperand(0))->getOpcode() ==
              llvm::Instruction::PHI) {
            g.phi_node = dyn_cast<llvm::Instruction>(ld->getOperand(0));
            g.phi = true;
          }
#if DEBUG == 1
          errs() << "Identify source: " << *g.source << "\n";
          errs() << "Identify target: " << *g.target << "\n\n";
#endif
        }
      }
    }
  }
}

void removeDuplicates(std::set<GEPDepInfo> &svInfos,
                      std::set<GEPDepInfo> &riInfos) {
  llvm::SmallVector<GEPDepInfo, 8> duplicates;
  for (auto g : riInfos) {
    std::set<GEPDepInfo>::iterator duplicate =
        std::find(svInfos.begin(), svInfos.end(), g);
    if (duplicate != svInfos.end()) {
      svInfos.erase(duplicate);
    }
  }
}

} // namespace

void PrefetcherPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<MemorySSAWrapperPass>();
  AU.addRequired<DependenceAnalysisWrapperPass>();
  AU.setPreservesAll();
}

bool in(llvm::SmallVectorImpl<std::string> &C, std::string E) {
  for (std::string entry : C) {
    if (E.find(entry) != std::string::npos) {
      return true;
    }
  }
  return false;
};

bool PrefetcherPass::runOnFunction(llvm::Function &F) {
  llvm::SmallVector<std::string, 32> FunctionWhiteList;

  if (FunctionWhiteListFile.getPosition()) {
    std::ifstream wlFile{FunctionWhiteListFile};

    std::string funcName;
    while (wlFile >> funcName) {
      FunctionWhiteList.push_back(funcName);
    }
  }

  if (F.isDeclaration()) {
    return false;
  }

  Result->allocs.clear();
  Result->geps.clear();
  Result->ri_geps.clear();
  auto &TLI = getAnalysis<llvm::TargetLibraryInfoWrapperPass>().getTLI(F);

  identifyNewA(F, Result->allocs);

  if (FunctionWhiteListFile.getPosition() &&
      !in(FunctionWhiteList, F.getName().str())) {
    llvm::errs() << "skipping func: " << F.getName()
                 << " reason: not in whitelist\n";
    ;
    return false;
  }

  identifyCorrectGEPDependence(F, Result->geps);
  identifyCorrectRangedIndirection(F, Result->ri_geps);

  return false;
}
char PrefetcherPass::ID = 0;

static llvm::RegisterPass<PrefetcherPass> X("prefetcher", "Prefetcher Pass",
                                            false, false);
