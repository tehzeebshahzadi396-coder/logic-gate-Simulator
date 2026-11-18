/**
 * @file main.cpp
 * @brief Command-line interface and interactive menu for the Logic Gate Simulator.
 */

#include "Circuit.h"
#include "Exceptions.h"
#include "NetlistParser.h"
#include "TruthTableGenerator.h"
#include <cctype>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {

void printBanner() {
    std::cout << "========================================================================\n"
              << "       DIGITAL CIRCUIT & LOGIC GATE SIMULATOR (C++17)\n"
              << "            Object Oriented Programming Project\n"
              << "========================================================================\n";
}

void printUsage(const char* progName) {
    std::cout << "Usage: " << progName << " <file.netlist> [options]\n\n"
              << "Options:\n"
              << "  --table                  Generate and display truth table (default)\n"
              << "  --set <A=1,B=0,...>      Simulate a single custom input vector\n"
              << "  --csv <filename.csv>     Export the generated truth table to CSV\n"
              << "  --verbose                Display multi-pass signal propagation details\n"
              << "  --help, -h               Show this help message\n\n"
              << "Without arguments, the simulator enters interactive console menu mode.\n";
}

LogicState parseSignalChar(char c) {
    switch (c) {
        case '0': return LogicState::LOW;
        case '1': return LogicState::HIGH;
        case 'x':
        case 'X': return LogicState::UNKNOWN;
        default:
            throw SimulatorException(std::string("Invalid logic value '") + c + "'. Expected 0, 1, or X.");
    }
}

void applyInputVector(Circuit& circuit, const std::string& inputStr) {
    std::istringstream iss(inputStr);
    std::string token;
    while (std::getline(iss, token, ',')) {
        // Strip whitespace
        token.erase(0, token.find_first_not_of(" \t\r\n"));
        token.erase(token.find_last_not_of(" \t\r\n") + 1);
        if (token.empty()) {
            continue;
        }

        const auto eqPos = token.find('=');
        if (eqPos == std::string::npos || eqPos == 0 || eqPos + 1 >= token.length()) {
            throw SimulatorException("Malformed assignment '" + token + "'. Expected NAME=VALUE format (e.g. A=1).");
        }

        const std::string name = token.substr(0, eqPos);
        const std::string valStr = token.substr(eqPos + 1);
        const LogicState val = parseSignalChar(valStr[0]);

        circuit.setInput(name, val);
    }
}

void printCircuitSummary(const Circuit& circuit) {
    std::cout << "\n--- Circuit Architecture Summary ---\n"
              << "Primary Inputs  (" << circuit.getPrimaryInputs().size() << "): ";
    for (const auto& inName : circuit.getPrimaryInputs()) {
        std::cout << inName << " ";
    }
    std::cout << "\nPrimary Outputs (" << circuit.getPrimaryOutputs().size() << "): ";
    for (const auto& outName : circuit.getPrimaryOutputs()) {
        std::cout << outName << " ";
    }
    std::cout << "\nTotal Wires     (" << circuit.getWires().size() << "): ";
    for (const auto& [name, wire] : circuit.getWires()) {
        (void)wire;
        std::cout << name << " ";
    }
    std::cout << "\nTotal Gates     (" << circuit.getGates().size() << "):\n";
    for (const auto& gate : circuit.getGates()) {
        std::cout << "  - [" << gate->getTypeName() << "] ID: " << gate->getId() << " (Inputs: ";
        for (std::size_t i = 0; i < gate->getInputs().size(); ++i) {
            std::cout << gate->getInputs()[i]->getName() << (i + 1 < gate->getInputs().size() ? ", " : "");
        }
        std::cout << " -> Output: " << gate->getOutput()->getName() << ")\n";
    }
    std::cout << "------------------------------------\n\n";
}

int runInteractiveMenu() {
    printBanner();
    std::unique_ptr<Circuit> circuit;
    std::string loadedPath;

    while (true) {
        std::cout << "\n--- Interactive Menu ---\n"
                  << "1. Load Netlist File\n"
                  << "2. Show Circuit Summary\n"
                  << "3. Generate Truth Table\n"
                  << "4. Simulate Custom Input Vector\n"
                  << "5. Exit\n"
                  << "Enter selection (1-5): ";

        std::string choiceStr;
        if (!std::getline(std::cin, choiceStr)) {
            break;
        }

        if (choiceStr.empty()) {
            continue;
        }

        const char choice = choiceStr[0];
        if (choice == '5') {
            std::cout << "Exiting simulator. Goodbye!\n";
            return 0;
        }

        try {
            switch (choice) {
                case '1': {
                    std::cout << "Enter netlist path: ";
                    std::string path;
                    if (std::getline(std::cin, path) && !path.empty()) {
                        circuit = std::make_unique<Circuit>(NetlistParser::parseFile(path));
                        loadedPath = path;
                        std::cout << "Successfully loaded and validated circuit from: " << path << "\n";
                    }
                    break;
                }
                case '2': {
                    if (!circuit) {
                        std::cout << "No circuit loaded. Please choose option 1 to load a netlist.\n";
                    } else {
                        printCircuitSummary(*circuit);
                    }
                    break;
                }
                case '3': {
                    if (!circuit) {
                        std::cout << "No circuit loaded. Please choose option 1 to load a netlist.\n";
                    } else {
                        TruthTableGenerator tt(*circuit);
                        std::cout << "\nTruth Table for " << loadedPath << ":\n";
                        tt.printTable(std::cout);

                        std::cout << "Export to CSV? (y/N): ";
                        std::string ans;
                        if (std::getline(std::cin, ans) && (ans == "y" || ans == "Y")) {
                            std::cout << "Enter CSV filename: ";
                            std::string csvFile;
                            if (std::getline(std::cin, csvFile) && !csvFile.empty()) {
                                tt.exportCsv(csvFile);
                                std::cout << "Exported truth table to " << csvFile << "\n";
                            }
                        }
                    }
                    break;
                }
                case '4': {
                    if (!circuit) {
                        std::cout << "No circuit loaded. Please choose option 1 to load a netlist.\n";
                    } else {
                        std::cout << "Circuit inputs are: ";
                        for (const auto& inName : circuit->getPrimaryInputs()) {
                            std::cout << inName << " ";
                        }
                        std::cout << "\nEnter assignment string (e.g. A=1,B=0,Cin=1): ";
                        std::string assignStr;
                        if (std::getline(std::cin, assignStr) && !assignStr.empty()) {
                            circuit->resetAll();
                            applyInputVector(*circuit, assignStr);
                            const std::size_t passes = circuit->simulate(true, std::cout);
                            std::cout << "Outputs:\n";
                            for (const auto& [name, state] : circuit->getOutputs()) {
                                std::cout << "  " << name << " = " << toChar(state) << "\n";
                            }
                            std::cout << "Settled in " << passes << " pass(es).\n";
                        }
                    }
                    break;
                }
                default:
                    std::cout << "Invalid choice. Please enter a number between 1 and 5.\n";
                    break;
            }
        } catch (const NetlistParseException& e) {
            std::cerr << "[Parse Error] " << e.what() << "\n";
        } catch (const SimulatorException& e) {
            std::cerr << "[Simulation Error] " << e.what() << "\n";
        } catch (const std::exception& e) {
            std::cerr << "[Error] " << e.what() << "\n";
        }
    }

    return 0;
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc == 1) {
        return runInteractiveMenu();
    }

    std::string netlistPath;
    std::string setVector;
    std::string csvPath;
    bool modeTable = false;
    bool modeSet = false;
    bool verbose = false;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printBanner();
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--table") {
            modeTable = true;
        } else if (arg == "--verbose") {
            verbose = true;
        } else if (arg == "--set") {
            if (i + 1 < argc) {
                setVector = argv[++i];
                modeSet = true;
            } else {
                std::cerr << "Error: --set requires an argument (e.g. --set A=1,B=0)\n";
                return 1;
            }
        } else if (arg.rfind("--set=", 0) == 0) {
            setVector = arg.substr(6);
            modeSet = true;
        } else if (arg == "--csv") {
            if (i + 1 < argc) {
                csvPath = argv[++i];
            } else {
                std::cerr << "Error: --csv requires an output file path\n";
                return 1;
            }
        } else if (arg.rfind("--csv=", 0) == 0) {
            csvPath = arg.substr(6);
        } else if (!arg.empty() && arg[0] != '-') {
            netlistPath = arg;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    if (netlistPath.empty()) {
        std::cerr << "Error: No netlist file specified.\n";
        printUsage(argv[0]);
        return 1;
    }

    // Default to table mode if --set was not explicitly given
    if (!modeSet) {
        modeTable = true;
    }

    printBanner();

    try {
        Circuit circuit = NetlistParser::parseFile(netlistPath);

        if (modeSet) {
            circuit.resetAll();
            applyInputVector(circuit, setVector);
            const std::size_t passes = circuit.simulate(verbose, std::cout);

            std::cout << "Simulation result for [" << setVector << "]:\n";
            for (const auto& [name, state] : circuit.getOutputs()) {
                std::cout << "  " << name << " = " << toChar(state) << "\n";
            }
            std::cout << "Settled in " << passes << " pass(es).\n";
        }

        if (modeTable) {
            TruthTableGenerator tt(circuit);
            tt.printTable(std::cout);

            if (!csvPath.empty()) {
                tt.exportCsv(csvPath);
                std::cout << "Exported truth table to: " << csvPath << "\n";
            }
        }

        return 0;
    } catch (const NetlistParseException& e) {
        std::cerr << "\n[Netlist Parse Error] " << e.what() << "\n";
        return 2;
    } catch (const SimulatorException& e) {
        std::cerr << "\n[Circuit Validation / Simulation Error] " << e.what() << "\n";
        return 3;
    } catch (const std::exception& e) {
        std::cerr << "\n[System Error] " << e.what() << "\n";
        return 1;
    }
}
