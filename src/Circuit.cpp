/**
 * @file Circuit.cpp
 * @brief Implementation of the Circuit class.
 */

#include "Circuit.h"
#include <algorithm>
#include <functional>
#include <sstream>
#include <stdexcept>

std::shared_ptr<Wire> Circuit::addWire(const std::string& name) {
    auto it = m_wires.find(name);
    if (it != m_wires.end()) {
        return it->second;
    }
    auto wire = std::make_shared<Wire>(name);
    m_wires[name] = wire;
    return wire;
}

std::shared_ptr<Wire> Circuit::getWire(const std::string& name) const {
    auto it = m_wires.find(name);
    if (it == m_wires.end()) {
        throw SimulatorException("Wire '" + name + "' does not exist in the circuit.");
    }
    return it->second;
}

bool Circuit::hasWire(const std::string& name) const noexcept {
    return m_wires.find(name) != m_wires.end();
}

void Circuit::addGate(std::shared_ptr<Gate> gate) {
    if (!gate) {
        throw std::invalid_argument("Gate pointer cannot be null.");
    }
    for (const auto& existing : m_gates) {
        if (existing->getId() == gate->getId()) {
            throw SimulatorException("Duplicate gate ID '" + gate->getId() + "' in circuit.");
        }
    }
    m_gates.push_back(std::move(gate));
}

void Circuit::declareInput(const std::string& name) {
    auto wire = addWire(name);
    wire->setPrimaryInput(true);
    if (std::find(m_primaryInputs.begin(), m_primaryInputs.end(), name) == m_primaryInputs.end()) {
        m_primaryInputs.push_back(name);
    }
}

void Circuit::declareOutput(const std::string& name) {
    auto wire = addWire(name);
    wire->setPrimaryOutput(true);
    if (std::find(m_primaryOutputs.begin(), m_primaryOutputs.end(), name) == m_primaryOutputs.end()) {
        m_primaryOutputs.push_back(name);
    }
}

void Circuit::setInput(const std::string& name, LogicState state) {
    auto it = m_wires.find(name);
    if (it == m_wires.end() || !it->second->isPrimaryInput()) {
        throw IllegalSignalStateException("Cannot set input: '" + name + "' is not a primary input.");
    }
    it->second->setState(state);
}

void Circuit::resetAll() noexcept {
    for (auto& [name, wire] : m_wires) {
        (void)name;
        wire->reset();
    }
    m_lastPassCount = 0;
}

void Circuit::detectCycles() const {
    const std::size_t numGates = m_gates.size();
    if (numGates == 0) {
        return;
    }

    // Map output wire name to driving gate index
    std::map<std::string, std::size_t> wireProducer;
    for (std::size_t i = 0; i < numGates; ++i) {
        wireProducer[m_gates[i]->getOutput()->getName()] = i;
    }

    // Adjacency list: gate A -> gate B if gate A's output is an input to gate B
    std::vector<std::vector<std::size_t>> adj(numGates);
    for (std::size_t j = 0; j < numGates; ++j) {
        for (const auto& inWire : m_gates[j]->getInputs()) {
            auto it = wireProducer.find(inWire->getName());
            if (it != wireProducer.end()) {
                const std::size_t producer = it->second;
                adj[producer].push_back(j);
            }
        }
    }

    // Three-color DFS: 0 = WHITE (unvisited), 1 = GRAY (visiting), 2 = BLACK (visited)
    enum Color : int { WHITE = 0, GRAY = 1, BLACK = 2 };
    std::vector<Color> colors(numGates, WHITE);
    std::vector<std::size_t> path;

    std::function<void(std::size_t)> dfs = [&](std::size_t u) {
        colors[u] = GRAY;
        path.push_back(u);

        for (std::size_t v : adj[u]) {
            if (colors[v] == GRAY) {
                // Cycle detected: extract cycle path from path stack
                auto cycleStart = std::find(path.begin(), path.end(), v);
                std::ostringstream oss;
                for (auto it = cycleStart; it != path.end(); ++it) {
                    oss << m_gates[*it]->getId() << " -> ";
                }
                oss << m_gates[v]->getId();
                throw CircuitInstabilityException("Combinational feedback loop detected: " + oss.str());
            }
            if (colors[v] == WHITE) {
                dfs(v);
            }
        }

        path.pop_back();
        colors[u] = BLACK;
    };

    for (std::size_t i = 0; i < numGates; ++i) {
        if (colors[i] == WHITE) {
            dfs(i);
        }
    }
}

void Circuit::validate() const {
    // 1. Check multiple drivers
    std::map<std::string, std::string> firstDriver;

    // Primary inputs are drivers
    for (const auto& inputName : m_primaryInputs) {
        firstDriver[inputName] = "primary input '" + inputName + "'";
    }

    // Gate outputs are drivers
    for (const auto& gate : m_gates) {
        const std::string& outName = gate->getOutput()->getName();
        auto it = firstDriver.find(outName);
        if (it != firstDriver.end()) {
            throw MultipleDriverException("Wire '" + outName + "' has multiple drivers (driven by " +
                                           it->second + " and gate '" + gate->getId() + "').");
        }
        firstDriver[outName] = "gate '" + gate->getId() + "'";
    }

    // 2. Check unconnected gate inputs
    for (const auto& gate : m_gates) {
        for (const auto& inWire : gate->getInputs()) {
            if (firstDriver.find(inWire->getName()) == firstDriver.end()) {
                throw UnconnectedPinException("Input wire '" + inWire->getName() + "' of gate '" +
                                               gate->getId() + "' is floating (has no driver).");
            }
        }
    }

    // 3. Check declared outputs
    for (const auto& outName : m_primaryOutputs) {
        if (firstDriver.find(outName) == firstDriver.end()) {
            throw UnconnectedPinException("Primary output wire '" + outName +
                                           "' has no driving signal.");
        }
    }

    // 4. Check for combinational feedback loops
    detectCycles();
}

std::size_t Circuit::simulate(bool verbose, std::ostream& out) {
    if (m_gates.empty()) {
        m_lastPassCount = 0;
        return 0;
    }

    const std::size_t maxPasses = m_gates.size() + 2;
    std::size_t passes = 0;

    while (true) {
        passes++;
        bool anyChanged = false;

        if (verbose) {
            out << "[Pass " << passes << "]\n";
        }

        for (const auto& gate : m_gates) {
            const LogicState oldState = gate->getOutput()->getState();
            const bool changed = gate->evaluate();
            const LogicState newState = gate->getOutput()->getState();

            if (changed) {
                anyChanged = true;
                if (verbose) {
                    out << "  Gate '" << gate->getId() << "' (" << gate->getTypeName()
                        << ") updated wire '" << gate->getOutput()->getName() << "': "
                        << oldState << " -> " << newState << "\n";
                }
            }
        }

        if (!anyChanged) {
            if (verbose) {
                out << "Circuit converged in " << passes << " pass(es).\n";
            }
            m_lastPassCount = passes;
            return passes;
        }

        if (passes >= maxPasses) {
            throw CircuitInstabilityException(
                "Circuit did not converge after " + std::to_string(passes) +
                " passes (exceeded safety bound of " + std::to_string(maxPasses) + ").");
        }
    }
}

std::size_t Circuit::getLastPassCount() const noexcept {
    return m_lastPassCount;
}

std::vector<std::pair<std::string, LogicState>> Circuit::getOutputs() const {
    std::vector<std::pair<std::string, LogicState>> outputs;
    outputs.reserve(m_primaryOutputs.size());
    for (const auto& name : m_primaryOutputs) {
        auto it = m_wires.find(name);
        if (it != m_wires.end()) {
            outputs.emplace_back(name, it->second->getState());
        } else {
            outputs.emplace_back(name, LogicState::UNKNOWN);
        }
    }
    return outputs;
}

const std::vector<std::string>& Circuit::getPrimaryInputs() const noexcept {
    return m_primaryInputs;
}

const std::vector<std::string>& Circuit::getPrimaryOutputs() const noexcept {
    return m_primaryOutputs;
}

const std::map<std::string, std::shared_ptr<Wire>>& Circuit::getWires() const noexcept {
    return m_wires;
}

const std::vector<std::shared_ptr<Gate>>& Circuit::getGates() const noexcept {
    return m_gates;
}
