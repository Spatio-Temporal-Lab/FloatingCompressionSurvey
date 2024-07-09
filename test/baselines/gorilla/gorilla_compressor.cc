#include "gorilla_compressor.h"

GorillaCompressor::GorillaCompressor(int capacity) {
    output_bit_stream_ = std::make_unique<OutputBitStream>(2 * capacity * sizeof(double));
}

void GorillaCompressor::addValue(double v) {
    uint64_t raw_binary = Double::DoubleToLongBits(v);
    if (first_) {
        first_ = false;
        compress_size_in_bits_ += output_bit_stream_->WriteLong(raw_binary, 64);
    } else {
        uint64_t xored_value = pr_value_ ^ raw_binary;
        if (xored_value == 0) {
            compress_size_in_bits_ += output_bit_stream_->WriteBit(false);
        } else {
            compress_size_in_bits_ += output_bit_stream_->WriteBit(true);
            int lead = __builtin_clzll(xored_value);
            if (lead >= 32) lead = 31;
            int trail = __builtin_ctzll(xored_value);
            if (lead >= pr_lead_ && trail >= pr_trail_) {
                compress_size_in_bits_ += output_bit_stream_->WriteBit(false);
                int significant_bits = 64 - pr_lead_ - pr_trail_;
                compress_size_in_bits_ += output_bit_stream_->WriteLong(
                        xored_value >> pr_trail_, significant_bits);
            } else {
                compress_size_in_bits_ += output_bit_stream_->WriteBit(true);
                compress_size_in_bits_ += output_bit_stream_->WriteInt(lead, 5);
                int significant_bits = 64 - lead - trail;
                if (significant_bits == 64) {
                    compress_size_in_bits_ += output_bit_stream_->WriteInt(0, 6);
                } else {
                    compress_size_in_bits_ += output_bit_stream_->WriteInt(
                            significant_bits, 6);
                }
                compress_size_in_bits_ += output_bit_stream_->WriteLong(
                        xored_value >> trail, significant_bits);
                pr_lead_ = lead;
                pr_trail_ = trail;
            }
        }
    }

    pr_value_ = raw_binary;
}

void GorillaCompressor::close() {
    addValue(std::numeric_limits<double>::quiet_NaN());
    output_bit_stream_->Flush();
}

Array<uint8_t> GorillaCompressor::get_compress_pack() {
    compress_pack_ = output_bit_stream_->GetBuffer(std::ceil
            (compress_size_in_bits_ / 8.0));
    return compress_pack_;
}

long GorillaCompressor::get_compress_size_in_bits() {
    return compress_size_in_bits_;
}

