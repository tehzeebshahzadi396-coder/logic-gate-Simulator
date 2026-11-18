#pragma once

/**
 * @file Gate.h
 * @brief Abstract base class for all digital logic gates.
 * 
 * OOP Concept Demonstrated: ABSTRACTION & TEMPLATE METHOD PATTERN
 * Gate provides an abstract interface for digital logic gates.
 * The evaluate() method embodies the Template Method pattern: it enforces
 * the simulation lifecycle algorithm (computing the next state and mutating
 * the output wire while tracking changes), delegating the pure functional
 * logic evaluation to the compute() pure virtual method.
 */

#include "Wire.h"
#include <limits>
#include <memory>
#include <string>
#include <vector>

class Gate {
public:
    /**
     * @brief Constructs a Gate with an identifier, input wires, and an output wire.
     * @param id Unique identifier for the gate.
     * @param inputs Vector of shared pointers to input wires.
     * @param output Shared pointer to the output wire.
     * @throws std::invalid_argument If id is empty or any wire pointer is null.
     */
    Gate(std::string id,
         std::vector<std::shared_ptr<Wire>> inputs,
         std::shared_ptr<Wire> output);

    /**
     * @brief Virtual destructor for polymorphic cleanup.
     */
    virtual ~Gate() = default;

    // Prohibit copying to enforce clean ownership via smart pointers
    Gate(const Gate&) = delete;
    Gate& operator=(const Gate&) = delete;
    Gate(Gate&&) noexcept = default;
    Gate& operator=(Gate&&) noexcept = default;

    /**
     * @brief Computes the output logic state based purely on current input wire states.
     * Pure virtual function representing the customization point for concrete gates.
     * Does not cause side effects.
     * @return Resulting LogicState.
     */
    [[nodiscard]] virtual LogicState compute() const = 0;

    /**
     * @brief Evaluates the gate: computes output and updates the output wire.
     * Template Method pattern: calls compute() and updates output wire state.
     * @return true if the output wire's state changed, false otherwise.
     */
    bool evaluate();

    /**
     * @brief Gets the type name of the gate (e.g. "AND", "OR", "NOT").
     * @return Gate type string.
     */
    [[nodiscard]] virtual std::string getTypeName() const = 0;

    /**
     * @brief Returns the minimum number of inputs required by this gate type.
     * @return Minimum inputs (default 2).
     */
    [[nodiscard]] virtual std::size_t minInputs() const {
        return 2;
    }

    /**
     * @brief Returns the maximum number of inputs permitted by this gate type.
     * @return Maximum inputs (default unlimited).
     */
    [[nodiscard]] virtual std::size_t maxInputs() const {
        return std::numeric_limits<std::size_t>::max();
    }

    /**
     * @brief Gets the gate identifier.
     * @return Const reference to gate ID string.
     */
    [[nodiscard]] const std::string& getId() const noexcept;

    /**
     * @brief Gets the list of input wires.
     * @return Const reference to vector of shared pointers to input wires.
     */
    [[nodiscard]] const std::vector<std::shared_ptr<Wire>>& getInputs() const noexcept;

    /**
     * @brief Gets the output wire.
     * @return Const reference to shared pointer to output wire.
     */
    [[nodiscard]] const std::shared_ptr<Wire>& getOutput() const noexcept;

protected:
    std::string m_id;
    std::vector<std::shared_ptr<Wire>> m_inputs;
    std::shared_ptr<Wire> m_output;
};
