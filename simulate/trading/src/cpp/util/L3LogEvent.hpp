/*
 * SPDX-FileCopyrightText: 2025 Rayleigh Research <to@rayleigh.re>
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include "Order.hpp"
#include "Trade.hpp"
#include "Cancellation.hpp"

#include <cstdint>
#include <variant>

//-------------------------------------------------------------------------

namespace taosim
{

//-------------------------------------------------------------------------

struct L3LogEvent
{
    std::variant<
        OrderWithLogContext,
        TradeWithLogContext,
        CancellationWithLogContext> item;
    uint32_t id;
};

//-------------------------------------------------------------------------

}  // namespace taosim

//-------------------------------------------------------------------------