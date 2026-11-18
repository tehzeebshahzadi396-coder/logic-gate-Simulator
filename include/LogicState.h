#pragma once

/**
 * @file LogicState.h
 * @brief Three-valued (Kleene) logic states and boolean algebra helper functions.
 */

#include <cstdint>
#include <iostream>
#include <string>

/**
 * @brief Represents digital signal levels according to three-valued Kleene logic.
 */
enum class LogicState : std::uint8_t {
    LOW = 0,
    HIGH = 1,
    UNKNOWN = 2
};

/**
 * @brief Converts a LogicState to a single-character representation ('0', '1', or 'X').
 * @param state The logic state to convert.
 * @return '0' for LOW, '1' for HIGH, 'X' for UNKNOWN.
 */
[[nodiscard]] constexpr char toChar(LogicState state) noexcept {
    switch (state) {
        case LogicState::LOW:     return '0';
        case LogicState::HIGH:    return '1';
        case LogicState::UNKNOWN: return 'X';
    }
    return 'X';
}

/**
 * @brief Converts a LogicState to a string representation ("0", "1", or "X").
 * @param state The logic state to convert.
 * @return String representing the logic state.
 */
[[nodiscard]] inline std::string toString(LogicState state) {
    return std::string(1, toChar(state));
}

/**
 * @brief Converts a standard C++ boolean to a LogicState.
 * @param value Boolean true or false.
 * @return LogicState::HIGH if true, LogicState::LOW if false.
 */
[[nodiscard]] constexpr LogicState fromBool(bool value) noexcept {
    return value ? LogicState::HIGH : LogicState::LOW;
}

/**
 * @brief Computes the Kleene NOT of a logic state.
 * NOT LOW = HIGH, NOT HIGH = LOW, NOT UNKNOWN = UNKNOWN.
 * @param state Input state.
 * @return Inverted logic state.
 */
[[nodiscard]] constexpr LogicState logicNot(LogicState state) noexcept {
    switch (state) {
        case LogicState::LOW:     return LogicState::HIGH;
        case LogicState::HIGH:    return LogicState::LOW;
        case LogicState::UNKNOWN: return LogicState::UNKNOWN;
    }
    return LogicState::UNKNOWN;
}

/**
 * @brief Computes the Kleene AND of two logic states.
 * LOW dominates (LOW AND X = LOW), HIGH AND X = X.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting logic state.
 */
[[nodiscard]] constexpr LogicState logicAnd(LogicState a, LogicState b) noexcept {
    if (a == LogicState::LOW || b == LogicState::LOW) {
        return LogicState::LOW;
    }
    if (a == LogicState::UNKNOWN || b == LogicState::UNKNOWN) {
        return LogicState::UNKNOWN;
    }
    return LogicState::HIGH;
}

/**
 * @brief Computes the Kleene OR of two logic states.
 * HIGH dominates (HIGH OR X = HIGH), LOW OR X = X.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting logic state.
 */
[[nodiscard]] constexpr LogicState logicOr(LogicState a, LogicState b) noexcept {
    if (a == LogicState::HIGH || b == LogicState::HIGH) {
        return LogicState::HIGH;
    }
    if (a == LogicState::UNKNOWN || b == LogicState::UNKNOWN) {
        return LogicState::UNKNOWN;
    }
    return LogicState::LOW;
}

/**
 * @brief Computes the Kleene XOR of two logic states.
 * Any UNKNOWN input results in UNKNOWN.
 * @param a First operand.
 * @param b Second operand.
 * @return Resulting logic state.
 */
[[nodiscard]] constexpr LogicState logicXor(LogicState a, LogicState b) noexcept {
    if (a == LogicState::UNKNOWN || b == LogicState::UNKNOWN) {
        return LogicState::UNKNOWN;
    }
    return (a != b) ? LogicState::HIGH : LogicState::LOW;
}

/**
 * @brief Stream insertion operator overload for LogicState.
 * @param os Output stream.
 * @param state Logic state to output.
 * @return Reference to the output stream.
 */
inline std::ostream& operator<<(std::ostream& os, LogicState state) {
    os << toChar(state);
    return os;
}
