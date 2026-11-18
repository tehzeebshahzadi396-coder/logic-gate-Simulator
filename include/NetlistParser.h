#pragma once

/**
 * @file NetlistParser.h
 * @brief Parser and semantic validator for circuit netlist files.
 */

#include "Circuit.h"
#include <iosfwd>
#include <string>

class NetlistParser {
public:
    /**
     * @brief Parses a netlist file from the given filesystem path.
     * @param path Path to .netlist file.
     * @return Validated Circuit instance.
     * @throws NetlistParseException If file cannot be read, syntax is invalid, or semantics fail.
     * @throws SimulatorException On validation errors (cycles, driver conflicts).
     */
    static Circuit parseFile(const std::string& path);

    /**
     * @brief Parses a netlist from an input stream.
     * @param in Input stream.
     * @param sourceName Source identifier for error reporting (e.g. filename).
     * @return Validated Circuit instance.
     * @throws NetlistParseException On syntax or semantic error.
     * @throws SimulatorException On validation errors.
     */
    static Circuit parseStream(std::istream& in, const std::string& sourceName);

private:
    static bool isValidIdentifier(const std::string& ident) noexcept;
    static std::string toUpper(std::string str);
    static std::string trim(const std::string& str);
};
