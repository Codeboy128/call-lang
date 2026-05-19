#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <functional>
#include "parser.h"

struct SerializedNode {
    size_t id;
    size_t type;  // Store enum as integer
    std::vector<size_t> children_ids;
};

class ASTSerializer {
private:
    std::vector<SerializedNode> nodes;
    std::unordered_map<size_t, size_t> node_map;  // original_id -> index in nodes array
    size_t next_id;

public:
    ASTSerializer() : next_id(0) {}

    std::vector<SerializedNode> serialize(const std::unique_ptr<ASTNode>& root) {
        nodes.clear();
        node_map.clear();
        next_id = 0;

        serializeNode(root.get());
        return nodes;
    }

private:
    size_t serializeNode(const ASTNode* node) {
        if (!node) return 0;

        // Check if node already serialized
        auto it = node_map.find(node->id);
        if (it != node_map.end()) {
            return it->second;
        }

        // Create serialized node
        SerializedNode sn;
        sn.id = next_id++;
        sn.type = static_cast<size_t>(node->type);

        // Store mapping
        node_map[node->id] = sn.id;

        // Serialize children first
        for (const auto& child : node->children) {
            size_t child_id = serializeNode(child.get());
            sn.children_ids.push_back(child_id);
        }

        nodes.push_back(std::move(sn));
        return sn.id;
    }
};

// Alternative: Write directly to a binary format (more compact)
#include <fstream>

void writeASTToBinaryFile(const std::unique_ptr<ASTNode>& root, const std::string& filename) {
    ASTSerializer serializer;
    auto serialized = serializer.serialize(root);

    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file for writing: " + filename);
    }

    // Write node count
    size_t node_count = serialized.size();
    file.write(reinterpret_cast<const char*>(&node_count), sizeof(node_count));

    // Write each node
    for (const auto& node : serialized) {
        // Write id
        file.write(reinterpret_cast<const char*>(&node.id), sizeof(node.id));

        // Write type
        file.write(reinterpret_cast<const char*>(&node.type), sizeof(node.type));

        // Write children count
        size_t child_count = node.children_ids.size();
        file.write(reinterpret_cast<const char*>(&child_count), sizeof(child_count));

        // Write children ids
        for (size_t child_id : node.children_ids) {
            file.write(reinterpret_cast<const char*>(&child_id), sizeof(child_id));
        }
    }

    file.close();
}

// Text-based format for debugging/human readability
void writeASTToTextFile(const std::unique_ptr<ASTNode>& root, const std::string& filename) {
    ASTSerializer serializer;
    auto serialized = serializer.serialize(root);

    std::ofstream file(filename);
    if (!file) {
        throw std::runtime_error("Cannot open file for writing: " + filename);
    }

    // Write node count
    file << serialized.size() << "\n";

    // Write each node
    for (const auto& node : serialized) {
        file << node.id << " " << node.type << " [";
        for (size_t i = 0; i < node.children_ids.size(); ++i) {
            if (i > 0) file << ",";
            file << node.children_ids[i];
        }
        file << "]\n";
    }

    file.close();
}

// Function to read AST from binary file
std::vector<SerializedNode> readASTFromBinaryFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file for reading: " + filename);
    }

    size_t node_count;
    file.read(reinterpret_cast<char*>(&node_count), sizeof(node_count));

    std::vector<SerializedNode> nodes;
    nodes.reserve(node_count);

    for (size_t i = 0; i < node_count; ++i) {
        SerializedNode node;

        // Read id
        file.read(reinterpret_cast<char*>(&node.id), sizeof(node.id));

        // Read type
        file.read(reinterpret_cast<char*>(&node.type), sizeof(node.type));

        // Read children count
        size_t child_count;
        file.read(reinterpret_cast<char*>(&child_count), sizeof(child_count));

        // Read children ids
        node.children_ids.resize(child_count);
        for (size_t j = 0; j < child_count; ++j) {
            file.read(reinterpret_cast<char*>(&node.children_ids[j]), sizeof(node.children_ids[j]));
        }

        nodes.push_back(std::move(node));
    }

    file.close();
    return nodes;
}

// Utility function to print serialized AST for debugging
// void printSerializedAST(const std::vector<SerializedNode>& nodes) {
//     static const std::string type_strings[] = {
//         "PROGRAM", "ASSIGNMENT", "EVENT_DECL", "FUNC_DECL",
//         "VARIABLE", "LITERAL", "FUNC_CALL", "BLOCK", "ARRAY"
//     };
//
//     std::cout << "Serialized AST (" << nodes.size() << " nodes):\n";
//     for (const auto& node : nodes) {
//         std::cout << "Node " << node.id
//         << " [Type: " << type_strings[node.type] << "]"
//         << " Children: [";
//         for (size_t i = 0; i < node.children_ids.size(); ++i) {
//             if (i > 0) std::cout << ", ";
//             std::cout << node.children_ids[i];
//         }
//         std::cout << "]\n";
//     }
// }






// Function to print serialized AST nodes
void printSerializedAST(const std::vector<SerializedNode>& nodes) {
    static const std::string type_strings[] = {
        "PROGRAM", "ASSIGNMENT", "EVENT_DECL", "FUNC_DECL",
        "VARIABLE", "LITERAL", "FUNC_CALL", "BLOCK", "ARRAY"
    };

    std::cout << "=== Serialized AST ===\n";
    std::cout << "Total nodes: " << nodes.size() << "\n\n";

    for (const auto& node : nodes) {
        // Print node ID and type
        std::cout << "Node " << node.id
        << " | Type: " << type_strings[node.type]
        << " | Children: ";

        // Print children IDs
        if (node.children_ids.empty()) {
            std::cout << "(none)";
        } else {
            std::cout << "[";
            for (size_t i = 0; i < node.children_ids.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << node.children_ids[i];
            }
            std::cout << "]";
        }
        std::cout << "\n";
    }
    std::cout << "=====================\n";
}

// Compact single-line printing (useful for file output)
void printSerializedASTCompact(const std::vector<SerializedNode>& nodes) {
    static const std::string type_strings[] = {
        "PROGRAM", "ASSIGNMENT", "EVENT_DECL", "FUNC_DECL",
        "VARIABLE", "LITERAL", "FUNC_CALL", "BLOCK", "ARRAY"
    };

    // Header
    std::cout << nodes.size() << "\n";

    // Body: each line format: id type [child1,child2,...]
    for (const auto& node : nodes) {
        std::cout << node.id << " " << node.type << " [";
        for (size_t i = 0; i < node.children_ids.size(); ++i) {
            if (i > 0) std::cout << ",";
            std::cout << node.children_ids[i];
        }
        std::cout << "]\n";
    }
}

// Tree-like hierarchical printing (shows structure)
void printSerializedASTHierarchical(const std::vector<SerializedNode>& nodes, size_t node_id = 0, int depth = 0) {
    static const std::string type_strings[] = {
        "PROGRAM", "ASSIGNMENT", "EVENT_DECL", "FUNC_DECL",
        "VARIABLE", "LITERAL", "FUNC_CALL", "BLOCK", "ARRAY"
    };

    if (node_id >= nodes.size()) return;

    const auto& node = nodes[node_id];

    // Print indentation
    for (int i = 0; i < depth; ++i) std::cout << "  ";

    // Print node info
    std::cout << "[" << node.id << "] " << type_strings[node.type];

    // Print children recursively
    if (!node.children_ids.empty()) {
        std::cout << " {\n";
        for (size_t child_id : node.children_ids) {
            // Find child index in array
            auto it = std::find_if(nodes.begin(), nodes.end(),
                                   [child_id](const SerializedNode& n) { return n.id == child_id; });

            if (it != nodes.end()) {
                size_t child_index = std::distance(nodes.begin(), it);
                printSerializedASTHierarchical(nodes, child_index, depth + 1);
            }
        }
        for (int i = 0; i < depth; ++i) std::cout << "  ";
        std::cout << "}\n";
    } else {
        std::cout << "\n";
    }
}

// Verbose printing with additional info
void printSerializedASTVerbose(const std::vector<SerializedNode>& nodes) {
    static const std::string type_strings[] = {
        "PROGRAM", "ASSIGNMENT", "EVENT_DECL", "FUNC_DECL",
        "VARIABLE", "LITERAL", "FUNC_CALL", "BLOCK", "ARRAY"
    };

    std::cout << "╔══════════════════════════════════════════╗\n";
    std::cout << "║       SERIALIZED AST DUMP               ║\n";
    std::cout << "╚══════════════════════════════════════════╝\n\n";

    std::cout << "Total nodes: " << nodes.size() << "\n";
    std::cout << "Memory: " << (nodes.size() * sizeof(SerializedNode)) << " bytes\n\n";

    // Statistics
    std::vector<size_t> type_count(9, 0);
    for (const auto& node : nodes) {
        type_count[node.type]++;
    }

    std::cout << "Node Type Distribution:\n";
    for (size_t i = 0; i < type_count.size(); ++i) {
        if (type_count[i] > 0) {
            std::cout << "  " << type_strings[i] << ": " << type_count[i] << "\n";
        }
    }
    std::cout << "\n";

    // Detailed node listing
    std::cout << "Detailed Node Listing:\n";
    std::cout << "─────────────────────────────────────────\n";

    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];

        std::cout << "Node #" << i << " (ID: " << node.id << ")\n";
        std::cout << "  Type: " << type_strings[node.type] << " (" << node.type << ")\n";
        std::cout << "  Children: ";

        if (node.children_ids.empty()) {
            std::cout << "Leaf node\n";
        } else {
            std::cout << node.children_ids.size() << " child(ren)\n";
            for (size_t child_id : node.children_ids) {
                auto it = std::find_if(nodes.begin(), nodes.end(),
                                       [child_id](const SerializedNode& n) { return n.id == child_id; });

                if (it != nodes.end()) {
                    std::cout << "    → Child ID " << child_id
                    << " (" << type_strings[it->type] << ")\n";
                }
            }
        }
        std::cout << "\n";
    }

    // Build and print adjacency list
    std::cout << "Adjacency List:\n";
    std::cout << "─────────────────────────────────────────\n";
    for (const auto& node : nodes) {
        std::cout << node.id << " (" << type_strings[node.type] << ") -> ";
        if (node.children_ids.empty()) {
            std::cout << "∅";
        } else {
            for (size_t i = 0; i < node.children_ids.size(); ++i) {
                if (i > 0) std::cout << " → ";
                std::cout << node.children_ids[i];
            }
        }
        std::cout << "\n";
    }

    // Root node identification
    if (!nodes.empty()) {
        std::cout << "\nRoot node: ID " << nodes[0].id
        << " (" << type_strings[nodes[0].type] << ")\n";
    }
}

// Main function demonstrating all print variants
void demoASTPrinting(const std::unique_ptr<ASTNode>& root) {
    ASTSerializer serializer;
    auto serialized = serializer.serialize(root);

    std::cout << "\n=== Standard Print ===\n";
    printSerializedAST(serialized);

    std::cout << "\n=== Compact Print ===\n";
    printSerializedASTCompact(serialized);

    std::cout << "\n=== Hierarchical Print ===\n";
    if (!serialized.empty()) {
        printSerializedASTHierarchical(serialized, 0, 0);
    }

    std::cout << "\n=== Verbose Print ===\n";
    printSerializedASTVerbose(serialized);
}
