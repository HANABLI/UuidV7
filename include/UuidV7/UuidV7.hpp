#pragma once

/**
 * @file Uuid-v7.hpp
 *
 * This module contains the declaration UUIDv7 - time-ordered,
 * RFC-4122 variant class.
 *
 * © copyright 2025 by Hatem Nabli
 */
#include <memory>
#include <random>
#include <array>
#include <chrono>
#include <iomanip>
#include <string>
#include <sstream>
#include <string_view>
#include <algorithm>
#include <stdexcept>

namespace UuidV7
{
    class UuidV7
    {
        // Types
    public:
        using bytes_type = std::array<std::uint8_t, 16>;

    public:
        bool operator==(const UuidV7& a) noexcept;
        bool operator!=(const UuidV7& a) noexcept;
        bool operator<(const UuidV7& a) noexcept;

        // Methods
    public:
        UuidV7() noexcept;
        explicit UuidV7(const bytes_type& b) noexcept;

        /**
         * This method generate a new variant type Uuid v7.
         */
        static UuidV7 Generate();

        /**
         * This method returns all-zero UUID.
         */
        static UuidV7 Nil() noexcept;

        /**
         * To string Method
         */
        std::string ToString() const;

        /**
         * This method parse the given string uuid.
         */
        static UuidV7 FromStroring(std::string_view s);

    private:
        struct Impl;

        std::shared_ptr<Impl> impl_;
    };

}  // namespace UuidV7
