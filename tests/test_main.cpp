/**
 * @file test_main.cpp
 * @brief Standalone test suite verifying unit and integration requirements.
 */

#include "Circuit.h"
#include "Exceptions.h"
#include "Gates.h"
#include "LogicState.h"
#include "NetlistParser.h"
#include "Wire.h"
#include <cassert>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace {

int g_passedTests = 0;
int g_failedTests = 0;

std::string resolvePath(const std::string& relPath) {
    std::ifstream f1(relPath);
    if (f1.good()) {
        return relPath;
    }
    std::ifstream f2("../" + relPath);
    if (f2.good()) {
        return "../" + relPath;
    }
    return relPath;
}

void runTest(const std::string& testName, const std::function<void()>& fn) {
    std::cout << "[ RUN      ] " << testName << std::endl;
    try {
        fn();
        g_passedTests++;
        std::cout << "[       OK ] " << testName << std::endl;
    } catch (const std::exception& e) {
        g_failedTests++;
        std::cerr << "[  FAILED  ] " << testName << " with exception: " << e.what() << std::endl;
    } catch (...) {
        g_failedTests++;
        std::cerr << "[  FAILED  ] " << testName << " with unknown exception!" << std::endl;
    }
}

#define ASSERT_TRUE(cond) \
    do { \
        if (!(cond)) { \
            throw std::runtime_error("Assertion failed: " #cond " at " __FILE__ ":" + std::to_string(__LINE__)); \
        } \
    } while (0)

#define ASSERT_FALSE(cond) ASSERT_TRUE(!(cond))

#define ASSERT_EQ(val1, val2) \
    do { \
        if (!((val1) == (val2))) { \
            throw std::runtime_error("Assertion failed: " #val1 " == " #val2 " at " __FILE__ ":" + std::to_string(__LINE__)); \
        } \
    } while (0)

// 1. Test LogicState and Kleene 3-Valued Logic
void testLogicStateAndKleene() {
    // Basic operations
    ASSERT_EQ(toChar(LogicState::LOW), '0');
    ASSERT_EQ(toChar(LogicState::HIGH), '1');
    ASSERT_EQ(toChar(LogicState::UNKNOWN), 'X');
    ASSERT_EQ(fromBool(true), LogicState::HIGH);
    ASSERT_EQ(fromBool(false), LogicState::LOW);

    // NOT
    ASSERT_EQ(logicNot(LogicState::LOW), LogicState::HIGH);
    ASSERT_EQ(logicNot(LogicState::HIGH), LogicState::LOW);
    ASSERT_EQ(logicNot(LogicState::UNKNOWN), LogicState::UNKNOWN);

    // Kleene AND: LOW dominates
    ASSERT_EQ(logicAnd(LogicState::LOW, LogicState::LOW), LogicState::LOW);
    ASSERT_EQ(logicAnd(LogicState::LOW, LogicState::HIGH), LogicState::LOW);
    ASSERT_EQ(logicAnd(LogicState::HIGH, LogicState::LOW), LogicState::LOW);
    ASSERT_EQ(logicAnd(LogicState::HIGH, LogicState::HIGH), LogicState::HIGH);
    ASSERT_EQ(logicAnd(LogicState::LOW, LogicState::UNKNOWN), LogicState::LOW);
    ASSERT_EQ(logicAnd(LogicState::UNKNOWN, LogicState::LOW), LogicState::LOW);
    ASSERT_EQ(logicAnd(LogicState::HIGH, LogicState::UNKNOWN), LogicState::UNKNOWN);
    ASSERT_EQ(logicAnd(LogicState::UNKNOWN, LogicState::HIGH), LogicState::UNKNOWN);
    ASSERT_EQ(logicAnd(LogicState::UNKNOWN, LogicState::UNKNOWN), LogicState::UNKNOWN);

    // Kleene OR: HIGH dominates
    ASSERT_EQ(logicOr(LogicState::LOW, LogicState::LOW), LogicState::LOW);
    ASSERT_EQ(logicOr(LogicState::LOW, LogicState::HIGH), LogicState::HIGH);
    ASSERT_EQ(logicOr(LogicState::HIGH, LogicState::LOW), LogicState::HIGH);
    ASSERT_EQ(logicOr(LogicState::HIGH, LogicState::HIGH), LogicState::HIGH);
    ASSERT_EQ(logicOr(LogicState::HIGH, LogicState::UNKNOWN), LogicState::HIGH);
    ASSERT_EQ(logicOr(LogicState::UNKNOWN, LogicState::HIGH), LogicState::HIGH);
    ASSERT_EQ(logicOr(LogicState::LOW, LogicState::UNKNOWN), LogicState::UNKNOWN);
    ASSERT_EQ(logicOr(LogicState::UNKNOWN, LogicState::LOW), LogicState::UNKNOWN);
    ASSERT_EQ(logicOr(LogicState::UNKNOWN, LogicState::UNKNOWN), LogicState::UNKNOWN);

    // Kleene XOR: UNKNOWN propagates
    ASSERT_EQ(logicXor(LogicState::LOW, LogicState::LOW), LogicState::LOW);
    ASSERT_EQ(logicXor(LogicState::LOW, LogicState::HIGH), LogicState::HIGH);
    ASSERT_EQ(logicXor(LogicState::HIGH, LogicState::LOW), LogicState::HIGH);
    ASSERT_EQ(logicXor(LogicState::HIGH, LogicState::HIGH), LogicState::LOW);
    ASSERT_EQ(logicXor(LogicState::LOW, LogicState::UNKNOWN), LogicState::UNKNOWN);
    ASSERT_EQ(logicXor(LogicState::HIGH, LogicState::UNKNOWN), LogicState::UNKNOWN);
    ASSERT_EQ(logicXor(LogicState::UNKNOWN, LogicState::UNKNOWN), LogicState::UNKNOWN);
}

// 2. Test Wire Encapsulation
void testWireEncapsulation() {
    Wire w("sigA");
    ASSERT_EQ(w.getName(), "sigA");
    ASSERT_EQ(w.getState(), LogicState::UNKNOWN);
    ASSERT_FALSE(w.isPrimaryInput());
    ASSERT_FALSE(w.isPrimaryOutput());
    ASSERT_FALSE(w.hasDriver());

    // State changes return dirty flag
    ASSERT_TRUE(w.setState(LogicState::HIGH));
    ASSERT_EQ(w.getState(), LogicState::HIGH);
    ASSERT_FALSE(w.setState(LogicState::HIGH)); // No change

    ASSERT_TRUE(w.setState(LogicState::LOW));
    ASSERT_EQ(w.getState(), LogicState::LOW);

    w.reset();
    ASSERT_EQ(w.getState(), LogicState::UNKNOWN);

    w.setPrimaryInput(true);
    ASSERT_TRUE(w.isPrimaryInput());
    ASSERT_TRUE(w.hasDriver()); // Primary input implicitly drives wire
}

// 3. Test Individual Gate Truth Tables (2 and 3 inputs, ternary)
void testGateTruthTables() {
    auto wA = std::make_shared<Wire>("A");
    auto wB = std::make_shared<Wire>("B");
    auto wC = std::make_shared<Wire>("C");
    auto wOut = std::make_shared<Wire>("Out");

    // NOT Gate
    NOTGate notG("not1", {wA}, wOut);
    ASSERT_EQ(notG.getTypeName(), "NOT");
    wA->setState(LogicState::LOW);
    notG.evaluate();
    ASSERT_EQ(wOut->getState(), LogicState::HIGH);
    wA->setState(LogicState::HIGH);
    notG.evaluate();
    ASSERT_EQ(wOut->getState(), LogicState::LOW);
    wA->setState(LogicState::UNKNOWN);
    notG.evaluate();
    ASSERT_EQ(wOut->getState(), LogicState::UNKNOWN);

    // 2-Input AND Gate
    ANDGate andG("and1", {wA, wB}, wOut);
    wA->setState(LogicState::LOW);
    wB->setState(LogicState::UNKNOWN);
    andG.evaluate();
    ASSERT_EQ(wOut->getState(), LogicState::LOW); // LOW dominates

    wA->setState(LogicState::HIGH);
    wB->setState(LogicState::HIGH);
    andG.evaluate();
    ASSERT_EQ(wOut->getState(), LogicState::HIGH);

    // 3-Input OR Gate
    ORGate or3("or3", {wA, wB, wC}, wOut);
    wA->setState(LogicState::LOW);
    wB->setState(LogicState::UNKNOWN);
    wC->setState(LogicState::HIGH);
    or3.evaluate();
    ASSERT_EQ(wOut->getState(), LogicState::HIGH); // HIGH dominates

    // 3-Input XOR Gate (Parity)
    XORGate xor3("xor3", {wA, wB, wC}, wOut);
    wA->setState(LogicState::HIGH);
    wB->setState(LogicState::HIGH);
    wC->setState(LogicState::HIGH);
    xor3.evaluate();
    ASSERT_EQ(wOut->getState(), LogicState::HIGH); // Odd parity -> HIGH

    wC->setState(LogicState::LOW);
    xor3.evaluate();
    ASSERT_EQ(wOut->getState(), LogicState::LOW); // Even parity -> LOW

    // Arity validation exceptions
    bool caught = false;
    try {
        NOTGate invalidNot("bad", {wA, wB}, wOut);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    ASSERT_TRUE(caught);
}

// 4. Test Polymorphism
void testPolymorphism() {
    auto in1 = std::make_shared<Wire>("in1");
    auto in2 = std::make_shared<Wire>("in2");
    auto out1 = std::make_shared<Wire>("out1");
    auto out2 = std::make_shared<Wire>("out2");
    auto out3 = std::make_shared<Wire>("out3");
    auto out4 = std::make_shared<Wire>("out4");
    auto out5 = std::make_shared<Wire>("out5");
    auto out6 = std::make_shared<Wire>("out6");
    auto out7 = std::make_shared<Wire>("out7");

    std::vector<std::shared_ptr<Gate>> gates;
    gates.push_back(std::make_shared<ANDGate>("g1", std::vector{in1, in2}, out1));
    gates.push_back(std::make_shared<ORGate>("g2", std::vector{in1, in2}, out2));
    gates.push_back(std::make_shared<NOTGate>("g3", std::vector{in1}, out3));
    gates.push_back(std::make_shared<NANDGate>("g4", std::vector{in1, in2}, out4));
    gates.push_back(std::make_shared<NORGate>("g5", std::vector{in1, in2}, out5));
    gates.push_back(std::make_shared<XORGate>("g6", std::vector{in1, in2}, out6));
    gates.push_back(std::make_shared<XNORGate>("g7", std::vector{in1, in2}, out7));

    in1->setState(LogicState::HIGH);
    in2->setState(LogicState::LOW);

    for (const auto& g : gates) {
        g->evaluate();
    }

    ASSERT_EQ(out1->getState(), LogicState::LOW);  // 1 AND 0 = 0
    ASSERT_EQ(out2->getState(), LogicState::HIGH); // 1 OR 0 = 1
    ASSERT_EQ(out3->getState(), LogicState::LOW);  // NOT 1 = 0
    ASSERT_EQ(out4->getState(), LogicState::HIGH); // 1 NAND 0 = 1
    ASSERT_EQ(out5->getState(), LogicState::LOW);  // 1 NOR 0 = 0
    ASSERT_EQ(out6->getState(), LogicState::HIGH); // 1 XOR 0 = 1
    ASSERT_EQ(out7->getState(), LogicState::LOW);  // 1 XNOR 0 = 0
}

// 5. Test Half Adder
void testHalfAdder() {
    Circuit circuit = NetlistParser::parseFile(resolvePath("netlists/half_adder.netlist"));

    struct TestCase {
        LogicState a;
        LogicState b;
        LogicState expectedSum;
        LogicState expectedCout;
    };

    const std::vector<TestCase> cases = {
        {LogicState::LOW,  LogicState::LOW,  LogicState::LOW,  LogicState::LOW},
        {LogicState::LOW,  LogicState::HIGH, LogicState::HIGH, LogicState::LOW},
        {LogicState::HIGH, LogicState::LOW,  LogicState::HIGH, LogicState::LOW},
        {LogicState::HIGH, LogicState::HIGH, LogicState::LOW,  LogicState::HIGH}
    };

    for (const auto& tc : cases) {
        circuit.resetAll();
        circuit.setInput("A", tc.a);
        circuit.setInput("B", tc.b);
        circuit.simulate();
        ASSERT_EQ(circuit.getWire("Sum")->getState(), tc.expectedSum);
        ASSERT_EQ(circuit.getWire("Cout")->getState(), tc.expectedCout);
    }
}

// 6. Test Full Adder (all 8 combinations verified against arithmetic)
void testFullAdder() {
    Circuit circuit = NetlistParser::parseFile(resolvePath("netlists/full_adder.netlist"));

    for (int a = 0; a <= 1; ++a) {
        for (int b = 0; b <= 1; ++b) {
            for (int cin = 0; cin <= 1; ++cin) {
                circuit.resetAll();
                circuit.setInput("A", fromBool(a));
                circuit.setInput("B", fromBool(b));
                circuit.setInput("Cin", fromBool(cin));
                circuit.simulate();

                const int sumInt = a ^ b ^ cin;
                const int coutInt = (a & b) | (b & cin) | (a & cin);

                ASSERT_EQ(circuit.getWire("Sum")->getState(), fromBool(sumInt));
                ASSERT_EQ(circuit.getWire("Cout")->getState(), fromBool(coutInt));
            }
        }
    }
}

// 7. Test 2:1 Multiplexer (all 8 combinations)
void testMux2to1() {
    Circuit circuit = NetlistParser::parseFile(resolvePath("netlists/mux_2to1.netlist"));

    for (int d0 = 0; d0 <= 1; ++d0) {
        for (int d1 = 0; d1 <= 1; ++d1) {
            for (int s = 0; s <= 1; ++s) {
                circuit.resetAll();
                circuit.setInput("D0", fromBool(d0));
                circuit.setInput("D1", fromBool(d1));
                circuit.setInput("S", fromBool(s));
                circuit.simulate();

                const int expectedY = (s == 0) ? d0 : d1;
                ASSERT_EQ(circuit.getWire("Y")->getState(), fromBool(expectedY));
            }
        }
    }
}

// 8. Test 4-bit Ripple Carry Adder (Exhaustive 512 input vectors: 16 * 16 * 2)
void test4BitAdderExhaustive512() {
    Circuit circuit = NetlistParser::parseFile(resolvePath("netlists/carry_adder_4bit.netlist"));

    for (int a = 0; a < 16; ++a) {
        for (int b = 0; b < 16; ++b) {
            for (int cin = 0; cin <= 1; ++cin) {
                circuit.resetAll();

                // Set A bits
                circuit.setInput("A0", fromBool((a >> 0) & 1));
                circuit.setInput("A1", fromBool((a >> 1) & 1));
                circuit.setInput("A2", fromBool((a >> 2) & 1));
                circuit.setInput("A3", fromBool((a >> 3) & 1));

                // Set B bits
                circuit.setInput("B0", fromBool((b >> 0) & 1));
                circuit.setInput("B1", fromBool((b >> 1) & 1));
                circuit.setInput("B2", fromBool((b >> 2) & 1));
                circuit.setInput("B3", fromBool((b >> 3) & 1));

                // Set Cin
                circuit.setInput("Cin", fromBool(cin));

                circuit.simulate();

                const int expectedResult = a + b + cin;
                const int expectedSum = expectedResult & 0xF;
                const int expectedCout = (expectedResult >> 4) & 1;

                const int actualS0 = (circuit.getWire("S0")->getState() == LogicState::HIGH) ? 1 : 0;
                const int actualS1 = (circuit.getWire("S1")->getState() == LogicState::HIGH) ? 1 : 0;
                const int actualS2 = (circuit.getWire("S2")->getState() == LogicState::HIGH) ? 1 : 0;
                const int actualS3 = (circuit.getWire("S3")->getState() == LogicState::HIGH) ? 1 : 0;
                const int actualSum = (actualS3 << 3) | (actualS2 << 2) | (actualS1 << 1) | actualS0;
                const int actualCout = (circuit.getWire("Cout")->getState() == LogicState::HIGH) ? 1 : 0;

                ASSERT_EQ(actualSum, expectedSum);
                ASSERT_EQ(actualCout, expectedCout);
            }
        }
    }
}

// 9. Test Multi-pass Settling with Reverse Topological Order
void testMultiPassReverseOrder() {
    Circuit circuit;
    circuit.declareInput("A");
    circuit.declareInput("B");
    circuit.declareOutput("Y");
    circuit.addWire("mid");

    // Add gates in REVERSE topological order:
    // Gate g2 depends on 'mid', but g1 (which produces 'mid') is declared second.
    auto g2 = GateFactory::create("AND", "g2", {circuit.getWire("mid"), circuit.getWire("B")}, circuit.getWire("Y"));
    auto g1 = GateFactory::create("OR", "g1", {circuit.getWire("A"), circuit.getWire("B")}, circuit.getWire("mid"));

    circuit.addGate(g2);
    circuit.addGate(g1);
    circuit.validate();

    circuit.resetAll();
    circuit.setInput("A", LogicState::HIGH);
    circuit.setInput("B", LogicState::HIGH);

    const std::size_t passes = circuit.simulate();

    // Verify propagation took more than 1 pass due to reverse declaration order
    ASSERT_TRUE(passes > 1);
    ASSERT_EQ(circuit.getWire("Y")->getState(), LogicState::HIGH);
}

// 10. Test Invalid Netlist Exceptions & Line Numbers
void testInvalidNetlists() {
    // 1. Syntax Error
    {
        bool caught = false;
        try {
            NetlistParser::parseFile(resolvePath("netlists/invalid/err_syntax.netlist"));
        } catch (const NetlistParseException& e) {
            caught = true;
            ASSERT_EQ(e.getLineNumber(), 4);
        }
        ASSERT_TRUE(caught);
    }

    // 2. Unknown Gate Type
    {
        bool caught = false;
        try {
            NetlistParser::parseFile(resolvePath("netlists/invalid/err_unknown_gate.netlist"));
        } catch (const UnknownGateTypeException& e) {
            caught = true;
            ASSERT_EQ(e.getLineNumber(), 4);
            ASSERT_EQ(e.getGateType(), "MAGIC");
        }
        ASSERT_TRUE(caught);
    }

    // 3. Undeclared Wire
    {
        bool caught = false;
        try {
            NetlistParser::parseFile(resolvePath("netlists/invalid/err_undeclared_wire.netlist"));
        } catch (const NetlistParseException& e) {
            caught = true;
            ASSERT_EQ(e.getLineNumber(), 4);
        }
        ASSERT_TRUE(caught);
    }

    // 4. Duplicate Wire
    {
        bool caught = false;
        try {
            NetlistParser::parseFile(resolvePath("netlists/invalid/err_duplicate_wire.netlist"));
        } catch (const NetlistParseException& e) {
            caught = true;
            ASSERT_EQ(e.getLineNumber(), 4);
        }
        ASSERT_TRUE(caught);
    }

    // 5. Unconnected Pin
    {
        bool caught = false;
        try {
            NetlistParser::parseFile(resolvePath("netlists/invalid/err_unconnected_pin.netlist"));
        } catch (const UnconnectedPinException&) {
            caught = true;
        }
        ASSERT_TRUE(caught);
    }

    // 6. Multiple Drivers
    {
        bool caught = false;
        try {
            NetlistParser::parseFile(resolvePath("netlists/invalid/err_multiple_drivers.netlist"));
        } catch (const MultipleDriverException&) {
            caught = true;
        }
        ASSERT_TRUE(caught);
    }

    // 7. Feedback Loop (Cycle Detection)
    {
        bool caught = false;
        try {
            NetlistParser::parseFile(resolvePath("netlists/invalid/err_feedback_loop.netlist"));
        } catch (const CircuitInstabilityException& e) {
            caught = true;
            const std::string msg = e.what();
            ASSERT_TRUE(msg.find("g1 -> g2 -> g1") != std::string::npos ||
                        msg.find("g2 -> g1 -> g2") != std::string::npos);
        }
        ASSERT_TRUE(caught);
    }

    // 8. Missing Output
    {
        bool caught = false;
        try {
            NetlistParser::parseFile(resolvePath("netlists/invalid/err_no_output.netlist"));
        } catch (const NetlistParseException&) {
            caught = true;
        }
        ASSERT_TRUE(caught);
    }
}

} // namespace

int main() {
    std::cout << "========================================================================\n"
              << "            DIGITAL CIRCUIT SIMULATOR TEST SUITE\n"
              << "            Object Oriented Programming Project\n"
              << "========================================================================\n\n";

    runTest("1. LogicState & Kleene 3-Valued Algebra", testLogicStateAndKleene);
    runTest("2. Wire Encapsulation & Dirty Flag Tracking", testWireEncapsulation);
    runTest("3. Gate Truth Tables & Arity Bounds", testGateTruthTables);
    runTest("4. Polymorphism & Base Pointer Gate Dispatch", testPolymorphism);
    runTest("5. Half Adder Circuit Simulation", testHalfAdder);
    runTest("6. Full Adder 8-State Arithmetic Verification", testFullAdder);
    runTest("7. 2:1 Multiplexer All Combinations", testMux2to1);
    runTest("8. 4-Bit Adder 512 Exhaustive Combinations", test4BitAdderExhaustive512);
    runTest("9. Multi-pass Reverse Topological Settling", testMultiPassReverseOrder);
    runTest("10. Invalid Netlists & Custom Exception Triggers", testInvalidNetlists);

    std::cout << "\n========================================================================\n"
              << "Test Summary: " << g_passedTests << " Passed, "
              << g_failedTests << " Failed.\n"
              << "========================================================================\n";

    return (g_failedTests == 0) ? 0 : 1;
}
