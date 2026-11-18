#pragma once

/**
 * @file TruthTableGenerator.h
 * @brief Generates complete truth tables and exports to ASCII table or CSV format.
 */

#include "Circuit.h"
#include <iosfwd>
#include <string>
#include <vector>

class TruthTableGenerator {
public:
    /**
     * @brief Constructs a TruthTableGenerator for a target circuit.
     * @param circuit Reference to the circuit to evaluate.
     */
    explicit TruthTableGenerator(Circuit& circuit);

    /**
     * @brief Generates and writes an aligned ASCII truth table to the stream.
     * @param out Output stream to receive the formatted table.
     * @throws SimulatorException If number of primary inputs exceeds 20.
     */
    void printTable(std::ostream& out = std::cout);

    /**
     * @brief Exports the truth table to a Comma-Separated Values (CSV) file.
     * @param csvPath Destination filesystem path for the CSV output.
     * @throws SimulatorException On input limit or file I/O error.
     */
    void exportCsv(const std::string& csvPath);

private:
    Circuit& m_circuit;

    struct TableRow {
        std::vector<LogicState> inputs;
        std::vector<LogicState> outputs;
    };

    std::vector<TableRow> computeAllRows();
};
