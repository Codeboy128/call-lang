// linear_ir.h
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <sstream>

// === Linear IR Definitions ===

enum class IROpcode {
    // Variable operations
    LOAD_CONST,     // Load constant value
    LOAD_VAR,       // Load variable value
    STORE_VAR,      // Store to variable

    // Function/Event operations
    FUNC_DECL,      // Declare function
    EVENT_DECL,     // Declare event handler
    CALL,           // Call function
    PARAM,          // Function parameter
    RETURN,         // Return from function

    // Block operations
    ENTER_SCOPE,    // Enter new scope
    EXIT_SCOPE,     // Exit scope

    // Array operations
    ARRAY_NEW,      // Create array
    ARRAY_PUSH,     // Push element to array

    // Program structure
    PROGRAM_START,  // Program start
    PROGRAM_END     // Program end
};

struct IRInstruction {
    IROpcode opcode;
    std::string operand1;
    std::string operand2;
    std::string result;

    IRInstruction(IROpcode op, std::string op1 = "", std::string op2 = "", std::string res = "")
    : opcode(op), operand1(std::move(op1)), operand2(std::move(op2)), result(std::move(res)) {}

    std::string toString() const {
        static const std::string opcode_names[] = {
            "LOAD_CONST", "LOAD_VAR", "STORE_VAR",
            "FUNC_DECL", "EVENT_DECL", "CALL", "PARAM", "RETURN",
            "ENTER_SCOPE", "EXIT_SCOPE",
            "ARRAY_NEW", "ARRAY_PUSH",
            "PROGRAM_START", "PROGRAM_END"
        };

        std::ostringstream oss;
        oss << opcode_names[static_cast<int>(opcode)];
        if (!operand1.empty()) oss << " " << operand1;
        if (!operand2.empty()) oss << " " << operand2;
        if (!result.empty()) oss << " -> " << result;
        return oss.str();
    }
};

class LinearIR {
public:
    std::vector<IRInstruction> instructions;

    void addInstruction(IROpcode op, std::string op1 = "", std::string op2 = "", std::string res = "") {
        instructions.emplace_back(op, op1, op2, res);
    }

    void print() const {
        for (size_t i = 0; i < instructions.size(); ++i) {
            std::cout << i << ": " << instructions[i].toString() << "\n";
        }
    }
};

// === AST to Linear IR Converter ===

class ASTToLinearIR {
private:
    LinearIR ir;
    int temp_counter = 0;
    int label_counter = 0;

    std::string newTemp() {
        return "$t" + std::to_string(temp_counter++);
    }

    std::string newLabel() {
        return "L" + std::to_string(label_counter++);
    }

public:
    LinearIR convert(ASTNode* root) {
        ir = LinearIR();
        temp_counter = 0;
        label_counter = 0;

        ir.addInstruction(IROpcode::PROGRAM_START);

        // Process all top-level statements
        for (auto& child : root->children) {
            convertStatement(child.get());
        }

        ir.addInstruction(IROpcode::PROGRAM_END);
        return ir;
    }

private:
    void convertStatement(ASTNode* node) {
        switch (node->type) {
            case ASTNodeType::ASSIGNMENT:
                convertAssignment(node);
                break;
            case ASTNodeType::EVENT_DECL:
                convertEventDeclaration(node);
                break;
            case ASTNodeType::FUNC_DECL:
                convertFunctionDeclaration(node);
                break;
            case ASTNodeType::BLOCK:
                convertBlock(node);
                break;
            default:
                // Expression statements
                std::string result = convertExpression(node);
                // If expression result isn't used, we might want to pop it
                break;
        }
    }

    void convertAssignment(ASTNode* node) {
        // node->children[0] is the variable
        // node->children[1] is the expression
        std::string varName = node->children[0]->value;
        std::string exprResult = convertExpression(node->children[1].get());

        ir.addInstruction(IROpcode::STORE_VAR, exprResult, "", varName);
    }

    void convertEventDeclaration(ASTNode* node) {
        std::string eventName = node->value;

        // Skip parameters for now (they're in children before the body)
        // The body is the last child
        ir.addInstruction(IROpcode::EVENT_DECL, eventName, std::to_string(node->children.size() - 1));

        // Process parameters
        for (size_t i = 0; i < node->children.size() - 1; ++i) {
            ir.addInstruction(IROpcode::PARAM, node->children[i]->value, "", "param_" + std::to_string(i));
        }

        // Process body
        ir.addInstruction(IROpcode::ENTER_SCOPE);
        convertStatement(node->children.back().get());
        ir.addInstruction(IROpcode::EXIT_SCOPE);
    }

    void convertFunctionDeclaration(ASTNode* node) {
        std::string funcName = node->value;

        // The body is the last child, others are parameters
        int paramCount = static_cast<int>(node->children.size()) - 1;
        ir.addInstruction(IROpcode::FUNC_DECL, funcName, std::to_string(paramCount));

        // Process parameters
        for (int i = 0; i < paramCount; ++i) {
            ir.addInstruction(IROpcode::PARAM, node->children[i]->value, "", "param_" + std::to_string(i));
        }

        // Process body
        ir.addInstruction(IROpcode::ENTER_SCOPE);
        std::string returnValue = convertExpression(node->children.back().get());
        ir.addInstruction(IROpcode::RETURN, returnValue);
        ir.addInstruction(IROpcode::EXIT_SCOPE);
    }

    void convertBlock(ASTNode* node) {
        ir.addInstruction(IROpcode::ENTER_SCOPE);
        for (auto& child : node->children) {
            convertStatement(child.get());
        }
        ir.addInstruction(IROpcode::EXIT_SCOPE);
    }

    std::string convertExpression(ASTNode* node) {
        switch (node->type) {
            case ASTNodeType::LITERAL: {
                std::string temp = newTemp();
                ir.addInstruction(IROpcode::LOAD_CONST, node->value, "", temp);
                return temp;
            }

            case ASTNodeType::VARIABLE: {
                std::string temp = newTemp();
                ir.addInstruction(IROpcode::LOAD_VAR, node->value, "", temp);
                return temp;
            }

            case ASTNodeType::FUNC_CALL: {
                std::string funcName = node->value;
                std::vector<std::string> argTemps;

                // Evaluate all arguments first
                for (auto& child : node->children) {
                    argTemps.push_back(convertExpression(child.get()));
                }

                std::string result = newTemp();

                // Generate CALL instruction with all arguments
                std::string argsStr;
                for (size_t i = 0; i < argTemps.size(); ++i) {
                    if (i > 0) argsStr += " ";
                    argsStr += argTemps[i];
                }

                ir.addInstruction(IROpcode::CALL, funcName, argsStr, result);
                return result;
            }

            case ASTNodeType::ARRAY: {
                std::string arrayTemp = newTemp();
                ir.addInstruction(IROpcode::ARRAY_NEW, "", "", arrayTemp);

                for (auto& child : node->children) {
                    std::string elemTemp = convertExpression(child.get());
                    ir.addInstruction(IROpcode::ARRAY_PUSH, arrayTemp, elemTemp);
                }

                return arrayTemp;
            }

            case ASTNodeType::BLOCK: {
                // Block expressions return the last value
                ir.addInstruction(IROpcode::ENTER_SCOPE);
                std::string lastResult;
                for (size_t i = 0; i < node->children.size(); ++i) {
                    if (i == node->children.size() - 1) {
                        lastResult = convertExpression(node->children[i].get());
                    } else {
                        convertStatement(node->children[i].get());
                    }
                }
                std::string temp = newTemp();
                ir.addInstruction(IROpcode::LOAD_VAR, lastResult, "", temp);
                ir.addInstruction(IROpcode::EXIT_SCOPE);
                return temp;
            }

            default:
                throw std::runtime_error("Unknown expression type");
        }
    }
};

// Helper function to convert and print linear IR
void convertAndPrintIR(ASTNode* root) {
    ASTToLinearIR converter;
    LinearIR ir = converter.convert(root);
    std::cout << "\n=== Linear IR ===\n";
    ir.print();
}
