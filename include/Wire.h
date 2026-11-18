#pragma once

/**
 * @file Wire.h
 * @brief Represents an electrical connection carrying a digital logic signal.
 * 
 * OOP Concept Demonstrated: ENCAPSULATION
 * All state data members are strictly private. Access and mutation are controlled
 * via public member functions. State mutation returns a boolean dirty-flag to support
 * convergence tracking in the simulation engine.
 */

#include "LogicState.h"
#include <string>

class Wire {
public:
    /**
     * @brief Constructs a Wire with a unique identifier.
     * @param name Unique wire identifier name.
     */
    explicit Wire(std::string name);

    /**
     * @brief Gets the identifier name of the wire.
     * @return Const reference to wire name string.
     */
    [[nodiscard]] const std::string& getName() const noexcept;

    /**
     * @brief Gets the current logic state of the wire.
     * @return Current LogicState.
     */
    [[nodiscard]] LogicState getState() const noexcept;

    /**
     * @brief Updates the logic state of the wire.
     * @param newState The new state to assign.
     * @return true if the state actually changed, false if unchanged.
     */
    bool setState(LogicState newState) noexcept;

    /**
     * @brief Checks if this wire is designated as a circuit primary input.
     * @return true if primary input, false otherwise.
     */
    [[nodiscard]] bool isPrimaryInput() const noexcept;

    /**
     * @brief Sets whether this wire is a primary input.
     * @param isInput true to designate as primary input.
     */
    void setPrimaryInput(bool isInput) noexcept;

    /**
     * @brief Checks if this wire is designated as a circuit primary output.
     * @return true if primary output, false otherwise.
     */
    [[nodiscard]] bool isPrimaryOutput() const noexcept;

    /**
     * @brief Sets whether this wire is a primary output.
     * @param isOutput true to designate as primary output.
     */
    void setPrimaryOutput(bool isOutput) noexcept;

    /**
     * @brief Checks if this wire has an active signal driver (input or gate output).
     * @return true if driven, false if floating.
     */
    [[nodiscard]] bool hasDriver() const noexcept;

    /**
     * @brief Marks this wire as driven by an input or gate output.
     * @param driven Flag indicating driving status (default true).
     */
    void setDriver(bool driven = true) noexcept;

    /**
     * @brief Resets the wire state to UNKNOWN.
     */
    void reset() noexcept;

private:
    std::string m_name;
    LogicState m_state{LogicState::UNKNOWN};
    bool m_isPrimaryInput{false};
    bool m_isPrimaryOutput{false};
    bool m_hasDriver{false};
};
