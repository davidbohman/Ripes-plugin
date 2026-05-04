#pragma once
#include "VSRTL/core/vsrtl_component.h"
#include "processors/RISC-V/riscv.h"

namespace vsrtl {
namespace core {
using namespace Ripes;

enum class BranchPredictionPolicy { AlwaysNotTaken, AlwaysTaken };

template <unsigned XLEN>
class BranchPredictor : public Component {
public:
  BranchPredictionPolicy m_policy = BranchPredictionPolicy::AlwaysNotTaken;

  BranchPredictor(const std::string &name, SimComponent *parent)
      : Component(name, parent) {

    // Should we speculatively take the branch in ID?
    predict_taken << [this] {
      if (m_policy == BranchPredictionPolicy::AlwaysTaken)
        return do_br.uValue() == 1 ? 1u : 0u;
      return 0u; // AlwaysNotTaken: never speculatively jump
    };

    // Was our prediction wrong?
    // AlwaysTaken:    wrong if branch resolved as NOT taken
    // AlwaysNotTaken: wrong if branch resolved as taken
    mispredict << [this] {
      const bool branchTaken = branch_taken.uValue() == 1;
      const bool isBranch = is_branch.uValue() == 1;
      if (!isBranch) return 0u;
      if (m_policy == BranchPredictionPolicy::AlwaysTaken)
        return branchTaken ? 0u : 1u;
      else
        return branchTaken ? 1u : 0u;
    };
  }

  INPUTPORT(do_br, 1);        // Is this a branch instruction? (from control)
  INPUTPORT(branch_taken, 1); // Did the branch actually resolve as taken? (from br_and)
  INPUTPORT(is_branch, 1);    // Is there a branch in EX stage?

  OUTPUTPORT(predict_taken, 1); // Drive PC speculatively in ID
  OUTPUTPORT(mispredict, 1);    // Trigger flush on misprediction
};

} // namespace core
} // namespace vsrtl