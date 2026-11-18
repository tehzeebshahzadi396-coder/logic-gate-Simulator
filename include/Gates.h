#pragma once

/**
 * @file Gates.h
 * @brief Concrete logic gate declarations and GateFactory registry.
 * 
 * OOP Concepts Demonstrated: INHERITANCE, POLYMORPHISM, FACTORY PATTERN
 * Concrete gates inherit from Gate and override compute() and getTypeName().
 * GateFactory maintains an extensible registry mapping gate type strings to
 * constructor lambdas.
 */

#include "Gate.h"
#include "Exceptions.h"
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

/**
 * @brief AND logic gate supporting 2 or more inputs.
 */
class ANDGate final : public Gate {
public:
    ANDGate(std::string id,
            std::vector<std::shared_ptr<Wire>> inputs,
            std::shared_ptr<Wire> output);

    [[nodiscard]] LogicState compute() const override;
    [[nodiscard]] std::string getTypeName() const override;
};

/**
 * @brief OR logic gate supporting 2 or more inputs.
 */
class ORGate final : public Gate {
public:
    ORGate(std::string id,
           std::vector<std::shared_ptr<Wire>> inputs,
           std::shared_ptr<Wire> output);

    [[nodiscard]] LogicState compute() const override;
    [[nodiscard]] std::string getTypeName() const override;
};

/**
 * @brief NOT logic gate (inverter) requiring exactly 1 input.
 */
class NOTGate final : public Gate {
public:
    NOTGate(std::string id,
            std::vector<std::shared_ptr<Wire>> inputs,
            std::shared_ptr<Wire> output);

    [[nodiscard]] LogicState compute() const override;
    [[nodiscard]] std::string getTypeName() const override;
    [[nodiscard]] std::size_t minInputs() const override { return 1; }
    [[nodiscard]] std::size_t maxInputs() const override { return 1; }
};

/**
 * @brief NAND logic gate supporting 2 or more inputs.
 */
class NANDGate final : public Gate {
public:
    NANDGate(std::string id,
             std::vector<std::shared_ptr<Wire>> inputs,
             std::shared_ptr<Wire> output);

    [[nodiscard]] LogicState compute() const override;
    [[nodiscard]] std::string getTypeName() const override;
};

/**
 * @brief NOR logic gate supporting 2 or more inputs.
 */
class NORGate final : public Gate {
public:
    NORGate(std::string id,
            std::vector<std::shared_ptr<Wire>> inputs,
            std::shared_ptr<Wire> output);

    [[nodiscard]] LogicState compute() const override;
    [[nodiscard]] std::string getTypeName() const override;
};

/**
 * @brief XOR logic gate (odd parity) supporting 2 or more inputs.
 */
class XORGate final : public Gate {
public:
    XORGate(std::string id,
            std::vector<std::shared_ptr<Wire>> inputs,
            std::shared_ptr<Wire> output);

    [[nodiscard]] LogicState compute() const override;
    [[nodiscard]] std::string getTypeName() const override;
};

/**
 * @brief XNOR logic gate (even parity) supporting 2 or more inputs.
 */
class XNORGate final : public Gate {
public:
    XNORGate(std::string id,
             std::vector<std::shared_ptr<Wire>> inputs,
             std::shared_ptr<Wire> output);

    [[nodiscard]] LogicState compute() const override;
    [[nodiscard]] std::string getTypeName() const override;
};

/**
 * @brief Factory class for creating Gate instances via an extensible type registry.
 */
class GateFactory {
public:
    using CreatorFunc = std::function<std::shared_ptr<Gate>(
        const std::string& id,
        std::vector<std::shared_ptr<Wire>> inputs,
        std::shared_ptr<Wire> output)>;

    /**
     * @brief Creates a gate of the specified type.
     * @param type Gate type string (case-insensitive: "AND", "OR", etc.).
     * @param id Gate identifier.
     * @param inputs Vector of input wires.
     * @param output Output wire.
     * @return Shared pointer to newly created Gate.
     * @throws UnknownGateTypeException If type is not registered.
     */
    static std::shared_ptr<Gate> create(
        const std::string& type,
        const std::string& id,
        std::vector<std::shared_ptr<Wire>> inputs,
        std::shared_ptr<Wire> output);

    /**
     * @brief Registers a new gate type constructor.
     * @param type Case-insensitive gate type name.
     * @param creator Function returning a shared pointer to a Gate.
     */
    static void registerGateType(const std::string& type, CreatorFunc creator);

    /**
     * @brief Checks if a gate type name is registered.
     * @param type Case-insensitive gate type name.
     * @return true if known, false otherwise.
     */
    static bool isKnownType(const std::string& type);

private:
    static std::map<std::string, CreatorFunc>& getRegistry();
    static std::string toUpper(std::string s);
};
