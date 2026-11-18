/**
 * @file Gates.cpp
 * @brief Implementation of concrete logic gates and GateFactory registry.
 */

#include "Gates.h"
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <utility>

// ============================================================================
// ANDGate
// ============================================================================

ANDGate::ANDGate(std::string id,
                 std::vector<std::shared_ptr<Wire>> inputs,
                 std::shared_ptr<Wire> output)
    : Gate(std::move(id), std::move(inputs), std::move(output)) {
    if (m_inputs.size() < minInputs()) {
        throw std::invalid_argument("AND gate '" + m_id + "' requires at least " +
                                    std::to_string(minInputs()) + " inputs, but got " +
                                    std::to_string(m_inputs.size()));
    }
}

LogicState ANDGate::compute() const {
    LogicState result = LogicState::HIGH;
    for (const auto& inWire : m_inputs) {
        result = logicAnd(result, inWire->getState());
    }
    return result;
}

std::string ANDGate::getTypeName() const {
    return "AND";
}

// ============================================================================
// ORGate
// ============================================================================

ORGate::ORGate(std::string id,
               std::vector<std::shared_ptr<Wire>> inputs,
               std::shared_ptr<Wire> output)
    : Gate(std::move(id), std::move(inputs), std::move(output)) {
    if (m_inputs.size() < minInputs()) {
        throw std::invalid_argument("OR gate '" + m_id + "' requires at least " +
                                    std::to_string(minInputs()) + " inputs, but got " +
                                    std::to_string(m_inputs.size()));
    }
}

LogicState ORGate::compute() const {
    LogicState result = LogicState::LOW;
    for (const auto& inWire : m_inputs) {
        result = logicOr(result, inWire->getState());
    }
    return result;
}

std::string ORGate::getTypeName() const {
    return "OR";
}

// ============================================================================
// NOTGate
// ============================================================================

NOTGate::NOTGate(std::string id,
                 std::vector<std::shared_ptr<Wire>> inputs,
                 std::shared_ptr<Wire> output)
    : Gate(std::move(id), std::move(inputs), std::move(output)) {
    if (m_inputs.size() != 1) {
        throw std::invalid_argument("NOT gate '" + m_id + "' requires exactly 1 input, but got " +
                                    std::to_string(m_inputs.size()));
    }
}

LogicState NOTGate::compute() const {
    return logicNot(m_inputs[0]->getState());
}

std::string NOTGate::getTypeName() const {
    return "NOT";
}

// ============================================================================
// NANDGate
// ============================================================================

NANDGate::NANDGate(std::string id,
                   std::vector<std::shared_ptr<Wire>> inputs,
                   std::shared_ptr<Wire> output)
    : Gate(std::move(id), std::move(inputs), std::move(output)) {
    if (m_inputs.size() < minInputs()) {
        throw std::invalid_argument("NAND gate '" + m_id + "' requires at least " +
                                    std::to_string(minInputs()) + " inputs, but got " +
                                    std::to_string(m_inputs.size()));
    }
}

LogicState NANDGate::compute() const {
    LogicState result = LogicState::HIGH;
    for (const auto& inWire : m_inputs) {
        result = logicAnd(result, inWire->getState());
    }
    return logicNot(result);
}

std::string NANDGate::getTypeName() const {
    return "NAND";
}

// ============================================================================
// NORGate
// ============================================================================

NORGate::NORGate(std::string id,
                 std::vector<std::shared_ptr<Wire>> inputs,
                 std::shared_ptr<Wire> output)
    : Gate(std::move(id), std::move(inputs), std::move(output)) {
    if (m_inputs.size() < minInputs()) {
        throw std::invalid_argument("NOR gate '" + m_id + "' requires at least " +
                                    std::to_string(minInputs()) + " inputs, but got " +
                                    std::to_string(m_inputs.size()));
    }
}

LogicState NORGate::compute() const {
    LogicState result = LogicState::LOW;
    for (const auto& inWire : m_inputs) {
        result = logicOr(result, inWire->getState());
    }
    return logicNot(result);
}

std::string NORGate::getTypeName() const {
    return "NOR";
}

// ============================================================================
// XORGate
// ============================================================================

XORGate::XORGate(std::string id,
                 std::vector<std::shared_ptr<Wire>> inputs,
                 std::shared_ptr<Wire> output)
    : Gate(std::move(id), std::move(inputs), std::move(output)) {
    if (m_inputs.size() < minInputs()) {
        throw std::invalid_argument("XOR gate '" + m_id + "' requires at least " +
                                    std::to_string(minInputs()) + " inputs, but got " +
                                    std::to_string(m_inputs.size()));
    }
}

LogicState XORGate::compute() const {
    LogicState result = m_inputs[0]->getState();
    for (std::size_t i = 1; i < m_inputs.size(); ++i) {
        result = logicXor(result, m_inputs[i]->getState());
    }
    return result;
}

std::string XORGate::getTypeName() const {
    return "XOR";
}

// ============================================================================
// XNORGate
// ============================================================================

XNORGate::XNORGate(std::string id,
                   std::vector<std::shared_ptr<Wire>> inputs,
                   std::shared_ptr<Wire> output)
    : Gate(std::move(id), std::move(inputs), std::move(output)) {
    if (m_inputs.size() < minInputs()) {
        throw std::invalid_argument("XNOR gate '" + m_id + "' requires at least " +
                                    std::to_string(minInputs()) + " inputs, but got " +
                                    std::to_string(m_inputs.size()));
    }
}

LogicState XNORGate::compute() const {
    LogicState result = m_inputs[0]->getState();
    for (std::size_t i = 1; i < m_inputs.size(); ++i) {
        result = logicXor(result, m_inputs[i]->getState());
    }
    return logicNot(result);
}

std::string XNORGate::getTypeName() const {
    return "XNOR";
}

// ============================================================================
// GateFactory
// ============================================================================

std::string GateFactory::toUpper(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return s;
}

std::map<std::string, GateFactory::CreatorFunc>& GateFactory::getRegistry() {
    static std::map<std::string, CreatorFunc> registry = []() {
        std::map<std::string, CreatorFunc> reg;
        reg["AND"] = [](const std::string& id,
                        std::vector<std::shared_ptr<Wire>> inputs,
                        std::shared_ptr<Wire> output) {
            return std::make_shared<ANDGate>(id, std::move(inputs), std::move(output));
        };
        reg["OR"] = [](const std::string& id,
                       std::vector<std::shared_ptr<Wire>> inputs,
                       std::shared_ptr<Wire> output) {
            return std::make_shared<ORGate>(id, std::move(inputs), std::move(output));
        };
        reg["NOT"] = [](const std::string& id,
                        std::vector<std::shared_ptr<Wire>> inputs,
                        std::shared_ptr<Wire> output) {
            return std::make_shared<NOTGate>(id, std::move(inputs), std::move(output));
        };
        reg["NAND"] = [](const std::string& id,
                         std::vector<std::shared_ptr<Wire>> inputs,
                         std::shared_ptr<Wire> output) {
            return std::make_shared<NANDGate>(id, std::move(inputs), std::move(output));
        };
        reg["NOR"] = [](const std::string& id,
                        std::vector<std::shared_ptr<Wire>> inputs,
                        std::shared_ptr<Wire> output) {
            return std::make_shared<NORGate>(id, std::move(inputs), std::move(output));
        };
        reg["XOR"] = [](const std::string& id,
                        std::vector<std::shared_ptr<Wire>> inputs,
                        std::shared_ptr<Wire> output) {
            return std::make_shared<XORGate>(id, std::move(inputs), std::move(output));
        };
        reg["XNOR"] = [](const std::string& id,
                         std::vector<std::shared_ptr<Wire>> inputs,
                         std::shared_ptr<Wire> output) {
            return std::make_shared<XNORGate>(id, std::move(inputs), std::move(output));
        };
        return reg;
    }();
    return registry;
}

std::shared_ptr<Gate> GateFactory::create(
    const std::string& type,
    const std::string& id,
    std::vector<std::shared_ptr<Wire>> inputs,
    std::shared_ptr<Wire> output) {
    const std::string upperType = toUpper(type);
    auto& reg = getRegistry();
    auto it = reg.find(upperType);
    if (it == reg.end()) {
        throw UnknownGateTypeException(type);
    }
    return it->second(id, std::move(inputs), std::move(output));
}

void GateFactory::registerGateType(const std::string& type, CreatorFunc creator) {
    getRegistry()[toUpper(type)] = std::move(creator);
}

bool GateFactory::isKnownType(const std::string& type) {
    auto& reg = getRegistry();
    return reg.find(toUpper(type)) != reg.end();
}
