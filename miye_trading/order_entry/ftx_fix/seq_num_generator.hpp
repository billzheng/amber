#pragma once
#include <cassert>
#include <stdint.h>

namespace miye::trading::fix
{

class SequenceNumGenerator
{
  public:
    explicit SequenceNumGenerator(uint32_t startSeq = 0) : seqNum_(startSeq) {}

  public:
    uint32_t getNextSeqNum() noexcept
    {
        assert(seqNum_ < 999'999'999);
        seqNum_++;
        return seqNum_;
    }
    uint32_t getSeqNum() const noexcept { return seqNum_; }
    void setSeqNum(uint32_t seqNum) noexcept
    {
        // std::cout << "reset sequence from: " << seqNum_ << " to : " << seqNum << std::endl;

        seqNum_ = seqNum;
    }

  private:
    uint32_t seqNum_;
};

} // namespace miye::trading::fix
