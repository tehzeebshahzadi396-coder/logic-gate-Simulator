/**
 * @file NetlistParser.cpp
 * @brief Implementation of NetlistParser class.
 */

#include "NetlistParser.h"
#include "Exceptions.h"
#include "Gates.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <set>
#include <sstream>
#include <vector>

bool NetlistParser::isValidIdentifier(const std::string& ident) noexcept {
    if (ident.empty()) {
        return false;
    }
    const unsigned char first = static_cast<unsigned char>(ident[0]);
    if (!std::isalpha(first) && first != '_') {
        return false;
    }
    for (char c : ident) {
        const unsigned char uc = static_cast<unsigned char>(c);
        if (!std::isalnum(uc) && uc != '_') {
            return false;
        }
    }
    return true;
}

std::string NetlistParser::toUpper(std::string str) {
    for (char& c : str) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return str;
}

std::string NetlistParser::trim(const std::string& str) {
    const auto first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    const auto last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

Circuit NetlistParser::parseFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw NetlistParseException(path, 0, "Could not open netlist file: '" + path + "'");
    }
    return parseStream(file, path);
}

Circuit NetlistParser::parseStream(std::istream& in, const std::string& sourceName) {
    Circuit circuit;
    std::set<std::string> declaredWires;
    std::set<std::string> declaredGates;

    std::string rawLine;
    std::size_t lineNumber = 0;

    while (std::getline(in, rawLine)) {
        lineNumber++;

        // Strip comments introduced by '#'
        const auto commentPos = rawLine.find('#');
        std::string lineContent = (commentPos != std::string::npos)
                                      ? rawLine.substr(0, commentPos)
                                      : rawLine;

        lineContent = trim(lineContent);
        if (lineContent.empty()) {
            continue;
        }

        // Tokenize space-separated elements
        std::istringstream iss(lineContent);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }

        if (tokens.empty()) {
            continue;
        }

        const std::string command = toUpper(tokens[0]);

        if (command == "INPUT") {
            if (tokens.size() < 2) {
                throw NetlistParseException(sourceName, lineNumber,
                                           "Missing identifier(s) in INPUT declaration.");
            }
            for (std::size_t i = 1; i < tokens.size(); ++i) {
                const auto& name = tokens[i];
                if (!isValidIdentifier(name)) {
                    throw NetlistParseException(sourceName, lineNumber,
                                               "Invalid identifier '" + name + "' in INPUT declaration.");
                }
                if (declaredWires.find(name) != declaredWires.end()) {
                    throw NetlistParseException(sourceName, lineNumber,
                                               "Duplicate wire declaration '" + name + "'.");
                }
                declaredWires.insert(name);
                circuit.declareInput(name);
            }
        } else if (command == "OUTPUT") {
            if (tokens.size() < 2) {
                throw NetlistParseException(sourceName, lineNumber,
                                           "Missing identifier(s) in OUTPUT declaration.");
            }
            for (std::size_t i = 1; i < tokens.size(); ++i) {
                const auto& name = tokens[i];
                if (!isValidIdentifier(name)) {
                    throw NetlistParseException(sourceName, lineNumber,
                                               "Invalid identifier '" + name + "' in OUTPUT declaration.");
                }
                if (declaredWires.find(name) != declaredWires.end()) {
                    throw NetlistParseException(sourceName, lineNumber,
                                               "Duplicate wire declaration '" + name + "'.");
                }
                declaredWires.insert(name);
                circuit.declareOutput(name);
            }
        } else if (command == "WIRE") {
            if (tokens.size() < 2) {
                throw NetlistParseException(sourceName, lineNumber,
                                           "Missing identifier(s) in WIRE declaration.");
            }
            for (std::size_t i = 1; i < tokens.size(); ++i) {
                const auto& name = tokens[i];
                if (!isValidIdentifier(name)) {
                    throw NetlistParseException(sourceName, lineNumber,
                                               "Invalid identifier '" + name + "' in WIRE declaration.");
                }
                if (declaredWires.find(name) != declaredWires.end()) {
                    throw NetlistParseException(sourceName, lineNumber,
                                               "Duplicate wire declaration '" + name + "'.");
                }
                declaredWires.insert(name);
                circuit.addWire(name);
            }
        } else if (command == "GATE") {
            // Expected: GATE <TYPE> <ID> <IN1> [<IN2> ...] <OUT>
            if (tokens.size() < 5) {
                throw NetlistParseException(
                    sourceName, lineNumber,
                    "Incomplete GATE statement. Expected: GATE <TYPE> <ID> <IN1> [<IN2>...] <OUT>");
            }

            const std::string& gateType = tokens[1];
            const std::string& gateId = tokens[2];

            if (!isValidIdentifier(gateId)) {
                throw NetlistParseException(sourceName, lineNumber,
                                           "Invalid gate identifier '" + gateId + "'.");
            }
            if (declaredGates.find(gateId) != declaredGates.end()) {
                throw NetlistParseException(sourceName, lineNumber,
                                           "Duplicate gate ID '" + gateId + "'.");
            }
            declaredGates.insert(gateId);

            if (!GateFactory::isKnownType(gateType)) {
                throw UnknownGateTypeException(sourceName, lineNumber, gateType);
            }

            const std::size_t numInputs = tokens.size() - 4;
            const std::string upperType = toUpper(gateType);

            if (upperType == "NOT") {
                if (numInputs != 1) {
                    throw NetlistParseException(
                        sourceName, lineNumber,
                        "NOT gate requires exactly 1 input, but got " + std::to_string(numInputs));
                }
            } else {
                if (numInputs < 2) {
                    throw NetlistParseException(
                        sourceName, lineNumber,
                        upperType + " gate requires at least 2 inputs, but got " +
                            std::to_string(numInputs));
                }
            }

            std::vector<std::shared_ptr<Wire>> inputWires;
            inputWires.reserve(numInputs);
            for (std::size_t i = 3; i < tokens.size() - 1; ++i) {
                const auto& inWireName = tokens[i];
                if (!isValidIdentifier(inWireName)) {
                    throw NetlistParseException(
                        sourceName, lineNumber,
                        "Invalid input wire identifier '" + inWireName + "' in GATE '" + gateId + "'.");
                }
                if (declaredWires.find(inWireName) == declaredWires.end()) {
                    throw NetlistParseException(
                        sourceName, lineNumber,
                        "Use of undeclared wire '" + inWireName + "' in GATE '" + gateId + "'.");
                }
                inputWires.push_back(circuit.getWire(inWireName));
            }

            const auto& outWireName = tokens.back();
            if (!isValidIdentifier(outWireName)) {
                throw NetlistParseException(
                    sourceName, lineNumber,
                    "Invalid output wire identifier '" + outWireName + "' in GATE '" + gateId + "'.");
            }
            if (declaredWires.find(outWireName) == declaredWires.end()) {
                throw NetlistParseException(
                    sourceName, lineNumber,
                    "Use of undeclared wire '" + outWireName + "' in GATE '" + gateId + "'.");
            }
            auto outWire = circuit.getWire(outWireName);

            auto gate = GateFactory::create(gateType, gateId, std::move(inputWires), std::move(outWire));
            circuit.addGate(std::move(gate));
        } else {
            throw NetlistParseException(sourceName, lineNumber,
                                       "Unknown statement/keyword '" + tokens[0] + "'.");
        }
    }

    if (circuit.getPrimaryInputs().empty()) {
        throw NetlistParseException(sourceName, lineNumber > 0 ? lineNumber : 1,
                                   "No INPUT declared in netlist file.");
    }
    if (circuit.getPrimaryOutputs().empty()) {
        throw NetlistParseException(sourceName, lineNumber > 0 ? lineNumber : 1,
                                   "No OUTPUT declared in netlist file.");
    }

    // Perform full topological and electrical rule checks
    circuit.validate();

    return circuit;
}
