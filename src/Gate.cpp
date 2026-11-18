/**
 * @file Gate.cpp
 * @brief Implementation of the Gate base class.
 */

#include "Gate.h"
#include <stdexcept>
#include <utility>

Gate::Gate(std::string id,
           std::vector<std::shared_ptr<Wire>> inputs,
           std::shared_ptr<Wire> output)
    : m_id(std::move(id))
    , m_inputs(std::move(inputs))
    , m_output(std::move(output)) {
    if (m_id.empty()) {
        throw std::invalid_argument("Gate ID cannot be empty.");
    }
    if (!m_output) {
        throw std::invalid_argument("Gate output wire pointer cannot be null for gate " + m_id);
    }
    for (std::size_t i = 0; i < m_inputs.size(); ++i) {
        if (!m_inputs[i]) {
            throw std::invalid_argument("Gate input wire pointer at index " + std::to_string(i) +
                                        " cannot be null for gate " + m_id);
        }
    }
}

bool Gate::evaluate() {
    const LogicState newState = compute();
    return m_output->setState(newState);
}

const std::string& Gate::getId() const noexcept {
    return m_id;
}

const std::vector<std::shared_ptr<Wire>>& Gate::getInputs() const noexcept {
    return m_inputs;
}

const std::shared_ptr<Wire>& Gate::getOutput() const noexcept {
    return m_output;
}
