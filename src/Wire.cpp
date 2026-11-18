/**
 * @file Wire.cpp
 * @brief Implementation of the Wire class.
 */

#include "Wire.h"
#include <utility>

Wire::Wire(std::string name)
    : m_name(std::move(name))
    , m_state(LogicState::UNKNOWN)
    , m_isPrimaryInput(false)
    , m_isPrimaryOutput(false)
    , m_hasDriver(false) {}

const std::string& Wire::getName() const noexcept {
    return m_name;
}

LogicState Wire::getState() const noexcept {
    return m_state;
}

bool Wire::setState(LogicState newState) noexcept {
    if (m_state != newState) {
        m_state = newState;
        return true;
    }
    return false;
}

bool Wire::isPrimaryInput() const noexcept {
    return m_isPrimaryInput;
}

void Wire::setPrimaryInput(bool isInput) noexcept {
    m_isPrimaryInput = isInput;
    if (isInput) {
        m_hasDriver = true;
    }
}

bool Wire::isPrimaryOutput() const noexcept {
    return m_isPrimaryOutput;
}

void Wire::setPrimaryOutput(bool isOutput) noexcept {
    m_isPrimaryOutput = isOutput;
}

bool Wire::hasDriver() const noexcept {
    return m_hasDriver;
}

void Wire::setDriver(bool driven) noexcept {
    m_hasDriver = driven;
}

void Wire::reset() noexcept {
    m_state = LogicState::UNKNOWN;
}
