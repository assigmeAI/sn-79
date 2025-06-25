/*
 * SPDX-FileCopyrightText: 2025 Rayleigh Research <to@rayleigh.re>
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include "common.hpp"
#include "Fees.hpp"

//-------------------------------------------------------------------------

namespace taosim
{

struct FeeLogEvent
{
    BookId bookId;
    AgentId restingAgentId;
    AgentId aggressingAgentId;
    exchange::Fees fees;
    decimal_t price;
    decimal_t volume;
};

}  // namespace taosim

//-------------------------------------------------------------------------
