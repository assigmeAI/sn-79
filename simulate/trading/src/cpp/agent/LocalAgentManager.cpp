/*
 * SPDX-FileCopyrightText: 2025 Rayleigh Research <to@rayleigh.re>
 * SPDX-License-Identifier: MIT
 */
#include "LocalAgentManager.hpp"

#include "ALGOTraderAgent.hpp"
#include "InitializationAgent.hpp"
#include "Simulation.hpp"
#include "StylizedTraderAgent.hpp"
#include "HighFrequencyTraderAgent.hpp"
#include "RandomTraderAgent.hpp"

#include <cassert>
#include <string_view>

//-------------------------------------------------------------------------

LocalAgentManager::LocalAgentManager(Simulation* simulation) noexcept
    : m_simulation{simulation}
{}

//-------------------------------------------------------------------------

void LocalAgentManager::createAgentsInstanced(
    pugi::xml_node node, std::function<void(pugi::xml_node)> creationCallback)
{
    std::map<std::string, uint32_t> baseNamesToCounts;

    for (pugi::xml_node child : node.children()) {
        std::string_view name = child.name();

        if (name == "MultiBookExchangeAgent") {
            createAgentInstanced<MultiBookExchangeAgent>(child);
        }
        else if (name == "DistributedProxyAgent") {
            createAgentInstanced<DistributedProxyAgent>(child);
        }
        else if (name == "StylizedTraderAgent") {
            createAgentInstanced<StylizedTraderAgent>(child);
        }
        else if (name == "HighFrequencyTraderAgent") {
            createAgentInstanced<HighFrequencyTraderAgent>(child);
        }
        else if (name == "InitializationAgent") {
            createAgentInstanced<InitializationAgent>(child);
        }
        else if (name == "TradeLogAgent") {
            createAgentInstanced<TradeLogAgent>(child);
        }
        else if (name == "ALGOTraderAgent") {
            // createAgentInstanced <taosim::agent::ALGOTraderAgent>(child);
        }
        else if (name == "RandomTraderAgent") {
            createAgentInstanced<taosim::agent::RandomTraderAgent>(child);
        }
        else {
            createAgentInstanced<PythonAgent>(child);
        }

        const char* agentBaseName = child.attribute("name").as_string();
        auto it = baseNamesToCounts.find(agentBaseName);
        if (it != baseNamesToCounts.end()) {
            throw std::invalid_argument{fmt::format(
                "{}: {} 'name' attribute '{}' already in use",
                std::source_location::current().function_name(), name, agentBaseName)};
        }
        baseNamesToCounts.insert(
            {agentBaseName, child.attribute("instanceCount").as_uint(1)});

        creationCallback(child);
    }

    std::sort(
        m_agents.begin(), m_agents.end(), [](const auto& lhs, const auto& rhs) {
            return lhs->name() < rhs->name();
        });

    m_roster = std::make_unique<LocalAgentRoster>(std::move(baseNamesToCounts));
}

//-------------------------------------------------------------------------

template<std::derived_from<Agent> T>
void LocalAgentManager::createAgentInstanced(pugi::xml_node node)
{
    const uint32_t instanceCount = node.attribute("instanceCount").as_uint(1);
    const std::string baseName = node.attribute("name").as_string();

    for (uint32_t instanceId = 0; instanceId < instanceCount; ++instanceId) {
        node.attribute("name").set_value(fmt::format("{}_{}", baseName, instanceId).c_str());
        m_agents.push_back([this, node] {
            auto agent = std::make_unique<T>(m_simulation);
            agent->configure(node);
            return agent;
        }());
    }

    node.attribute("name").set_value(baseName.c_str());
}

//-------------------------------------------------------------------------

template<>
void LocalAgentManager::createAgentInstanced<DistributedProxyAgent>(pugi::xml_node node)
{
    m_agents.push_back([this, node] {
        auto agent = std::make_unique<DistributedProxyAgent>(m_simulation);
        agent->configure(node);
        return agent;
    }());
}

//-------------------------------------------------------------------------

template<>
void LocalAgentManager::createAgentInstanced<MultiBookExchangeAgent>(pugi::xml_node node)
{
    m_agents.push_back([this, node] {
        auto agent = std::make_unique<MultiBookExchangeAgent>(m_simulation);
        agent->configure(node);
        m_simulation->m_exchange = agent.get();
        return agent;
    }());
}

//-------------------------------------------------------------------------

template<>
void LocalAgentManager::createAgentInstanced<TradeLogAgent>(pugi::xml_node node)
{
    m_agents.push_back([this, node] {
        auto agent = std::make_unique<TradeLogAgent>(m_simulation);
        agent->configure(node);
        return agent;
    }());
}

//-------------------------------------------------------------------------

template<>
void LocalAgentManager::createAgentInstanced<PythonAgent>(pugi::xml_node node)
{
    const fs::path pySourcePath = node.attribute("file").as_string(
        fmt::format("{}.py", node.name()).c_str());

    if (!fs::exists(pySourcePath)) {
        throw std::invalid_argument(fmt::format(
            "{}: File '{}' missing",
            std::source_location::current().function_name(),
            pySourcePath.c_str()));
    }

    const fs::path pyFilename =
        std::string_view{pySourcePath.stem().c_str()} != node.name() ? pySourcePath : "";
    const uint32_t instanceCount = node.attribute("instanceCount").as_uint(1);
    const std::string baseName = node.attribute("name").as_string();

    for (uint32_t instanceId = 0; instanceId < instanceCount; ++instanceId) {
        m_agents.push_back([&] {
            node.attribute("name").set_value(fmt::format("{}_{}", baseName, instanceId).c_str());
            auto agent = std::make_unique<PythonAgent>(m_simulation, node.name(), pyFilename);
            agent->configure(node);
            return agent;
        }());
    }

    node.attribute("name").set_value(baseName.c_str());
}

//-------------------------------------------------------------------------
