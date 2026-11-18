#pragma once

/**
 * @file Circuit.h
 * @brief Circuit container managing wires, gates, validation, and multi-pass simulation.
 * 
 * OOP Concept Demonstrated: COMPOSITION
 * Circuit owns collections of Wire and Gate objects via smart pointers.
 * It manages the graph topology, performs 3-color DFS cycle detection,
 * validates driving constraints, and simulates signal propagation to convergence.
 */

#include "Exceptions.h"
#include "Gate.h"
#include "LogicState.h"
#include "Wire.h"
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class Circuit {
public:
    /**
     * @brief Constructs an empty circuit.
     */
    Circuit() = default;

    /**
     * @brief Destructor.
     */
    ~Circuit() = default;

    // Disallow copying to enforce sole ownership of circuit graph resources
    Circuit(const Circuit&) = delete;
    Circuit& operator=(const Circuit&) = delete;

    // Allow move semantics
    Circuit(Circuit&&) noexcept = default;
    Circuit& operator=(Circuit&&) noexcept = default;

    /**
     * @brief Adds or retrieves a wire by name.
     * @param name Unique wire identifier.
     * @return Shared pointer to the wire.
     */
    std::shared_ptr<Wire> addWire(const std::string& name);

    /**
     * @brief Retrieves a wire by name.
     * @param name Wire name.
     * @return Shared pointer to the wire.
     * @throws SimulatorException If wire does not exist.
     */
    [[nodiscard]] std::shared_ptr<Wire> getWire(const std::string& name) const;

    /**
     * @brief Checks if a wire exists in the circuit.
     * @param name Wire name.
     * @return true if wire exists, false otherwise.
     */
    [[nodiscard]] bool hasWire(const std::string& name) const noexcept;

    /**
     * @brief Adds a gate to the circuit.
     * @param gate Shared pointer to the gate.
     * @throws std::invalid_argument If gate pointer is null.
     */
    void addGate(std::shared_ptr<Gate> gate);

    /**
     * @brief Declares a primary input wire for the circuit.
     * @param name Name of the input wire.
     */
    void declareInput(const std::string& name);

    /**
     * @brief Declares a primary output wire for the circuit.
     * @param name Name of the output wire.
     */
    void declareOutput(const std::string& name);

    /**
     * @brief Sets the logic state of a primary input wire.
     * @param name Name of the primary input wire.
     * @param state New logic state.
     * @throws IllegalSignalStateException If the wire is not a primary input.
     */
    void setInput(const std::string& name, LogicState state);

    /**
     * @brief Resets all wires in the circuit to UNKNOWN state.
     */
    void resetAll() noexcept;

    /**
     * @brief Validates circuit topological constraints:
     * - Every gate input has a driver (primary input or gate output).
     * - No wire has multiple driving sources.
     * - Every primary output has a driver.
     * - No combinational feedback loops exist (via 3-color DFS).
     * @throws UnconnectedPinException On floating inputs/outputs.
     * @throws MultipleDriverException On bus contention.
     * @throws CircuitInstabilityException On combinational cycles.
     */
    void validate() const;

    /**
     * @brief Executes multi-pass signal propagation until the circuit stabilizes.
     * @param verbose If true, prints each pass and gate output changes.
     * @param out Output stream for verbose messages.
     * @return Number of passes required to achieve stability.
     * @throws CircuitInstabilityException If circuit does not converge within max passes.
     */
    std::size_t simulate(bool verbose = false, std::ostream& out = std::cout);

    /**
     * @brief Gets the number of passes taken by the last simulation run.
     * @return Pass count.
     */
    [[nodiscard]] std::size_t getLastPassCount() const noexcept;

    /**
     * @brief Retrieves current logic states of all declared primary outputs.
     * @return Vector of (name, LogicState) pairs in declared order.
     */
    [[nodiscard]] std::vector<std::pair<std::string, LogicState>> getOutputs() const;

    /**
     * @brief Gets the list of declared primary input names.
     * @return Const reference to vector of input names.
     */
    [[nodiscard]] const std::vector<std::string>& getPrimaryInputs() const noexcept;

    /**
     * @brief Gets the list of declared primary output names.
     * @return Const reference to vector of output names.
     */
    [[nodiscard]] const std::vector<std::string>& getPrimaryOutputs() const noexcept;

    /**
     * @brief Gets all wires in the circuit.
     * @return Const reference to map of name -> Wire shared_ptr.
     */
    [[nodiscard]] const std::map<std::string, std::shared_ptr<Wire>>& getWires() const noexcept;

    /**
     * @brief Gets all gates in the circuit.
     * @return Const reference to vector of Gate shared_ptr.
     */
    [[nodiscard]] const std::vector<std::shared_ptr<Gate>>& getGates() const noexcept;

private:
    std::map<std::string, std::shared_ptr<Wire>> m_wires;
    std::vector<std::shared_ptr<Gate>> m_gates;
    std::vector<std::string> m_primaryInputs;
    std::vector<std::string> m_primaryOutputs;
    std::size_t m_lastPassCount{0};

    void detectCycles() const;
};
