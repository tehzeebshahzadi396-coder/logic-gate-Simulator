/**
 * @file TruthTableGenerator.cpp
 * @brief Implementation of TruthTableGenerator class.
 */

#include "TruthTableGenerator.h"
#include "Exceptions.h"
#include <fstream>
#include <iomanip>
#include <iostream>

TruthTableGenerator::TruthTableGenerator(Circuit& circuit)
    : m_circuit(circuit) {}

std::vector<TruthTableGenerator::TableRow> TruthTableGenerator::computeAllRows() {
    const auto& inputNames = m_circuit.getPrimaryInputs();
    const std::size_t numInputs = inputNames.size();

    if (numInputs > 20) {
        throw SimulatorException(
            "Circuit has " + std::to_string(numInputs) +
            " inputs. Truth table generation is limited to at most 20 inputs (2^20 = 1,048,576 combinations).");
    }

    const std::uint64_t totalCombinations = 1ULL << numInputs;
    std::vector<TableRow> rows;
    rows.reserve(static_cast<std::size_t>(totalCombinations));

    for (std::uint64_t i = 0; i < totalCombinations; ++i) {
        TableRow row;
        row.inputs.reserve(numInputs);

        m_circuit.resetAll();

        // Enumerate in standard binary counting order: first declared input is MSB
        for (std::size_t j = 0; j < numInputs; ++j) {
            const std::size_t bitShift = numInputs - 1 - j;
            const bool bitVal = ((i >> bitShift) & 1ULL) != 0;
            const LogicState state = fromBool(bitVal);
            row.inputs.push_back(state);
            m_circuit.setInput(inputNames[j], state);
        }

        m_circuit.simulate(false);

        const auto outputPairs = m_circuit.getOutputs();
        row.outputs.reserve(outputPairs.size());
        for (const auto& [name, state] : outputPairs) {
            (void)name;
            row.outputs.push_back(state);
        }

        rows.push_back(std::move(row));
    }

    return rows;
}

void TruthTableGenerator::printTable(std::ostream& out) {
    const auto& inNames = m_circuit.getPrimaryInputs();
    const auto& outNames = m_circuit.getPrimaryOutputs();

    if (inNames.empty() || outNames.empty()) {
        throw SimulatorException("Circuit must have both primary inputs and outputs to generate a truth table.");
    }

    const auto rows = computeAllRows();

    // Compute column widths (excluding outer borders, with 1 space on either side: width + 2)
    std::vector<std::size_t> inWidths;
    inWidths.reserve(inNames.size());
    for (const auto& name : inNames) {
        inWidths.push_back(std::max<std::size_t>(1, name.length()));
    }

    std::vector<std::size_t> outWidths;
    outWidths.reserve(outNames.size());
    for (const auto& name : outNames) {
        outWidths.push_back(std::max<std::size_t>(1, name.length()));
    }

    // Lambda to format centered content in a column with 1-space padding on each side
    auto formatCell = [](const std::string& text, std::size_t colWidth) {
        const std::size_t totalWidth = colWidth + 2;
        const std::size_t textLen = text.length();
        if (textLen >= totalWidth) {
            return text;
        }
        const std::size_t totalPadding = totalWidth - textLen;
        const std::size_t leftPadding = totalPadding / 2;
        const std::size_t rightPadding = totalPadding - leftPadding;
        return std::string(leftPadding, ' ') + text + std::string(rightPadding, ' ');
    };

    // Header row
    out << "|";
    for (std::size_t i = 0; i < inNames.size(); ++i) {
        out << formatCell(inNames[i], inWidths[i]) << "|";
    }
    out << "|";
    for (std::size_t i = 0; i < outNames.size(); ++i) {
        out << formatCell(outNames[i], outWidths[i]) << "|";
    }
    out << "\n";

    // Separator row
    out << "|";
    for (std::size_t i = 0; i < inNames.size(); ++i) {
        out << std::string(inWidths[i] + 2, '-') << "|";
    }
    out << "|";
    for (std::size_t i = 0; i < outNames.size(); ++i) {
        out << std::string(outWidths[i] + 2, '-') << "|";
    }
    out << "\n";

    // Data rows
    for (const auto& row : rows) {
        out << "|";
        for (std::size_t i = 0; i < inNames.size(); ++i) {
            const std::string val(1, toChar(row.inputs[i]));
            out << formatCell(val, inWidths[i]) << "|";
        }
        out << "|";
        for (std::size_t i = 0; i < outNames.size(); ++i) {
            const std::string val(1, toChar(row.outputs[i]));
            out << formatCell(val, outWidths[i]) << "|";
        }
        out << "\n";
    }
}

void TruthTableGenerator::exportCsv(const std::string& csvPath) {
    const auto& inNames = m_circuit.getPrimaryInputs();
    const auto& outNames = m_circuit.getPrimaryOutputs();

    if (inNames.empty() || outNames.empty()) {
        throw SimulatorException("Circuit must have both primary inputs and outputs to export CSV.");
    }

    const auto rows = computeAllRows();

    std::ofstream csv(csvPath);
    if (!csv.is_open()) {
        throw SimulatorException("Failed to open destination CSV file for writing: " + csvPath);
    }

    // CSV Header
    for (std::size_t i = 0; i < inNames.size(); ++i) {
        csv << inNames[i] << ",";
    }
    for (std::size_t i = 0; i < outNames.size(); ++i) {
        csv << outNames[i] << (i + 1 < outNames.size() ? "," : "\n");
    }

    // CSV Rows
    for (const auto& row : rows) {
        for (std::size_t i = 0; i < inNames.size(); ++i) {
            csv << toChar(row.inputs[i]) << ",";
        }
        for (std::size_t i = 0; i < outNames.size(); ++i) {
            csv << toChar(row.outputs[i]) << (i + 1 < outNames.size() ? "," : "\n");
        }
    }
}
