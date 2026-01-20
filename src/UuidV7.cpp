#include "UuidV7/UuidV7.hpp"

#include <iomanip>
#include <random>

namespace UuidV7
{
    struct UuidV7::Impl
    {
        bytes_type bytes_;

        static std::mt19937_64::result_type seed_engine() {
            std::random_device rd;
            std::seed_seq seq{rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd()};
            return std::mt19937_64(seq)();
        }

        static std::mt19937_64& random_engine() {
            // Thread-local PRNG, seeded once per thread from std::random_device
            static thread_local std::mt19937_64 eng{Impl::seed_engine()};
            return eng;
        }

        // Parsing: returns false on error (no exceptions)
        static bool try_parse(std::string_view s, UuidV7& out) {
            // Expected format: 8-4-4-4-12 = 36 characters
            if (s.size() != 36)
                return false;

            auto is_hex = [](char c)
            { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); };

            // Check hyphen positions
            if (s[8] != '-' || s[13] != '-' || s[18] != '-' || s[23] != '-')
                return false;

            // Check all other chars are hex
            for (std::size_t i = 0; i < s.size(); ++i)
            {
                if (i == 8 || i == 13 || i == 18 || i == 23)
                    continue;
                if (!is_hex(s[i]))
                    return false;
            }

            bytes_type b{};
            std::size_t bi = 0;  // byte index
            std::size_t i = 0;   // char index

            auto hex_to_val = [](char c) -> std::uint8_t
            {
                if (c >= '0' && c <= '9')
                    return static_cast<std::uint8_t>(c - '0');
                if (c >= 'a' && c <= 'f')
                    return static_cast<std::uint8_t>(10 + (c - 'a'));
                return static_cast<std::uint8_t>(10 + (c - 'A'));
            };

            while (i < s.size() && bi < b.size())
            {
                if (s[i] == '-')
                {
                    ++i;
                    continue;
                }
                std::uint8_t hi = hex_to_val(s[i++]);
                std::uint8_t lo = hex_to_val(s[i++]);
                b[bi++] = static_cast<std::uint8_t>((hi << 4) | lo);
            }

            if (bi != b.size())
                return false;

            out = UuidV7(b);
            return true;
        }
    };

    UuidV7::UuidV7() noexcept : impl_(new Impl) {}
    UuidV7::UuidV7(const bytes_type& b) noexcept : impl_(new Impl) { impl_->bytes_ = b; }

    bool UuidV7::operator==(const UuidV7& other) noexcept {
        return impl_->bytes_ == other.impl_->bytes_;
    }

    bool UuidV7::operator!=(const UuidV7& other) noexcept { return !(*this == other); }

    bool UuidV7::operator<(const UuidV7& other) noexcept {
        return std::lexicographical_compare(impl_->bytes_.begin(), impl_->bytes_.end(),
                                            other.impl_->bytes_.begin(), other.impl_->bytes_.end());
    }

    UuidV7 UuidV7::Generate() {
        bytes_type b{};

        // Get the curent time in milliseconds
        using clock = std::chrono::system_clock;
        const auto now = clock::now();
        const auto ms_since_epoch =
            std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

        std::uint64_t unix_ms = static_cast<std::uint64_t>(ms_since_epoch);

        // Monotonic sub-millisecond counter (12 bits)
        // We encode timestamp as: (unix_ms << 12) | (sequence & 0x0FFF)

        static thread_local std::uint64_t last_unix_ms = 0;
        static thread_local std::uint16_t seq = 0;

        if (unix_ms > last_unix_ms)
        {
            last_unix_ms = unix_ms;
            seq = 0;
        } else if (unix_ms == last_unix_ms)
        {
            ++seq;  // wrap naturally on 16 bits; we only use low 12 bits
        } else
        {
            // Clock went backwards; we accept it and restart sequence
            last_unix_ms = unix_ms;
            seq = 0;
        }

        const std::uint64_t timestamp = (unix_ms << 12) | (seq & 0x0FFF);

        // --- 3. Split into UUID fields (per UUIDv7 layout) ---
        // According to UUIDv7:
        //   - time_low:              32 bits  (high bits of timestamp)
        //   - time_mid:              16 bits
        //   - time_hi_and_version:   16 bits (12 bits timestamp + 4 bits version = 0b0111)
        //
        // timestamp is at most ~53 bits (41 bits ms + 12 bits sub-ms),
        // comfortably fitting into the 60 bit space reserved for time in v7.

        const std::uint32_t time_low = static_cast<std::uint32_t>(timestamp >> 28);
        const std::uint16_t time_mid = static_cast<std::uint16_t>((timestamp >> 12) & 0xFFFFu);
        std::uint16_t time_hi_and_ver = static_cast<std::uint16_t>(timestamp & 0x0FFFu);

        // Set version (4 bits: 0b0111 for version 7)
        time_hi_and_ver |= static_cast<std::uint16_t>(0x7000u);

        // --- 4. Generate remaining random bits (62 bits total) ---
        auto& rng = Impl::random_engine();
        std::uint64_t r1 = rng();
        std::uint64_t r2 = rng();

        // clock_seq: 14 random bits
        std::uint16_t clock_seq = static_cast<std::uint16_t>(r1 & 0x3FFFu);

        std::uint8_t clock_seq_hi = static_cast<std::uint8_t>((clock_seq >> 8) & 0x3Fu);
        std::uint8_t clock_seq_lo = static_cast<std::uint8_t>(clock_seq & 0xFFu);

        // Set RFC-4122 variant: 10xxxxxx
        clock_seq_hi |= 0x80u;  // set bit7=1, bit6=0

        // node: 48 random bits
        std::uint64_t node = r2 & 0x0000FFFFFFFFFFFFull;

        // --- 5. Write everything into bytes[] in network order (big-endian) ---
        bytes_type out{};

        // time_low (32 bits)
        out[0] = static_cast<std::uint8_t>((time_low >> 24) & 0xFFu);
        out[1] = static_cast<std::uint8_t>((time_low >> 16) & 0xFFu);
        out[2] = static_cast<std::uint8_t>((time_low >> 8) & 0xFFu);
        out[3] = static_cast<std::uint8_t>(time_low & 0xFFu);

        // time_mid (16 bits)
        out[4] = static_cast<std::uint8_t>((time_mid >> 8) & 0xFFu);
        out[5] = static_cast<std::uint8_t>(time_mid & 0xFFu);

        // time_hi_and_version (16 bits)
        out[6] = static_cast<std::uint8_t>((time_hi_and_ver >> 8) & 0xFFu);
        out[7] = static_cast<std::uint8_t>(time_hi_and_ver & 0xFFu);

        // clock_seq_hi_and_reserved + clock_seq_low
        out[8] = clock_seq_hi;
        out[9] = clock_seq_lo;

        // node (48 bits)
        out[10] = static_cast<std::uint8_t>((node >> 40) & 0xFFu);
        out[11] = static_cast<std::uint8_t>((node >> 32) & 0xFFu);
        out[12] = static_cast<std::uint8_t>((node >> 24) & 0xFFu);
        out[13] = static_cast<std::uint8_t>((node >> 16) & 0xFFu);
        out[14] = static_cast<std::uint8_t>((node >> 8) & 0xFFu);
        out[15] = static_cast<std::uint8_t>(node & 0xFFu);

        return UuidV7(out);
    }

    UuidV7 UuidV7::Nil() noexcept { return UuidV7{}; }

    std::string UuidV7::ToString() const {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');

        for (std::size_t i = 0; i < impl_->bytes_.size(); ++i)
        {
            oss << std::setw(2) << static_cast<unsigned>(impl_->bytes_[i]);
            if (i == 3 || i == 5 || i == 7 || i == 9)
            { oss << '-'; }
        }
        return oss.str();
    }

    UuidV7 UuidV7::FromString(std::string_view s) {
        UuidV7 id;

        if (!Impl::try_parse(s, id))
            throw std::invalid_argument("invalid UUIDv7 string");

        return id;
    }

}  // namespace UuidV7