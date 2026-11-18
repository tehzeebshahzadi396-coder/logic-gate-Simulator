#pragma once

/**
 * @file Exceptions.h
 * @brief Custom exception hierarchy for the Logic Gate Simulator.
 */

#include <stdexcept>
#include <string>
#include <utility>

/**
 * @brief Base class for all simulator-specific exceptions.
 * Demonstrates custom exception handling and inheritance hierarchy.
 */
class SimulatorException : public std::runtime_error {
public:
    /**
     * @brief Constructs a SimulatorException with an error message.
     * @param message Descriptive error message.
     */
    explicit SimulatorException(const std::string& message)
        : std::runtime_error(message) {}
};

/**
 * @brief Exception thrown during netlist parsing, reporting filename and line number.
 */
class NetlistParseException : public SimulatorException {
public:
    /**
     * @brief Constructs a NetlistParseException with location details.
     * @param fileName Path or name of the netlist file.
     * @param lineNumber Line number (1-indexed) where the error occurred.
     * @param message Error description.
     */
    NetlistParseException(std::string fileName, std::size_t lineNumber, const std::string& message)
        : SimulatorException(formatMessage(fileName, lineNumber, message))
        , m_fileName(std::move(fileName))
        , m_lineNumber(lineNumber)
        , m_rawMessage(message) {}

    /**
     * @brief Gets the filename where the parse error occurred.
     * @return Filename string.
     */
    [[nodiscard]] const std::string& getFileName() const noexcept {
        return m_fileName;
    }

    /**
     * @brief Gets the line number of the parse error.
     * @return 1-indexed line number.
     */
    [[nodiscard]] std::size_t getLineNumber() const noexcept {
        return m_lineNumber;
    }

    /**
     * @brief Gets the raw error message without location prefix.
     * @return Raw message string.
     */
    [[nodiscard]] const std::string& getRawMessage() const noexcept {
        return m_rawMessage;
    }

private:
    std::string m_fileName;
    std::size_t m_lineNumber;
    std::string m_rawMessage;

    static std::string formatMessage(const std::string& file, std::size_t line, const std::string& msg) {
        return file + ":" + std::to_string(line) + ": " + msg;
    }
};

/**
 * @brief Exception thrown when an unrecognized gate type is encountered.
 */
class UnknownGateTypeException : public NetlistParseException {
public:
    /**
     * @brief Constructs an UnknownGateTypeException with full line info.
     * @param fileName File name.
     * @param lineNumber Line number.
     * @param gateType Unrecognized gate type name.
     */
    UnknownGateTypeException(const std::string& fileName, std::size_t lineNumber, const std::string& gateType)
        : NetlistParseException(fileName, lineNumber, "Unknown gate type '" + gateType + "'")
        , m_gateType(gateType) {}

    /**
     * @brief Constructs an UnknownGateTypeException without file/line context.
     * @param gateType Unrecognized gate type name.
     */
    explicit UnknownGateTypeException(const std::string& gateType)
        : NetlistParseException("unknown", 0, "Unknown gate type '" + gateType + "'")
        , m_gateType(gateType) {}

    /**
     * @brief Gets the invalid gate type name.
     * @return Gate type string.
     */
    [[nodiscard]] const std::string& getGateType() const noexcept {
        return m_gateType;
    }

private:
    std::string m_gateType;
};

/**
 * @brief Exception thrown when a circuit has combinational loops or fails to converge.
 */
class CircuitInstabilityException : public SimulatorException {
public:
    /**
     * @brief Constructs a CircuitInstabilityException with details.
     * @param message Instability reason (e.g. cycle path or pass limit exceeded).
     */
    explicit CircuitInstabilityException(const std::string& message)
        : SimulatorException(message) {}
};

/**
 * @brief Exception thrown when a gate pin or output has no driving signal.
 */
class UnconnectedPinException : public SimulatorException {
public:
    /**
     * @brief Constructs an UnconnectedPinException.
     * @param message Description of the unconnected pin.
     */
    explicit UnconnectedPinException(const std::string& message)
        : SimulatorException(message) {}
};

/**
 * @brief Exception thrown when a single wire is driven by more than one source.
 */
class MultipleDriverException : public SimulatorException {
public:
    /**
     * @brief Constructs a MultipleDriverException.
     * @param message Description of the conflicted wire.
     */
    explicit MultipleDriverException(const std::string& message)
        : SimulatorException(message) {}
};

/**
 * @brief Exception thrown on invalid signal manipulation (e.g., driving a non-primary input).
 */
class IllegalSignalStateException : public SimulatorException {
public:
    /**
     * @brief Constructs an IllegalSignalStateException.
     * @param message Description of the illegal state transition.
     */
    explicit IllegalSignalStateException(const std::string& message)
        : SimulatorException(message) {}
};
