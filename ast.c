#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define AST Node Types
typedef enum {
    NODE_PROGRAM,
    NODE_PRINT_STATEMENT,
    NODE_ASSIGNMENT,
    NODE_IF_STATEMENT,
    NODE_WHILE_LOOP,
    NODE_FOR_LOOP,
    NODE_REPEAT_UNTIL,
    NODE_SELECT_CASE,
    NODE_CASE,
    NODE_GOSUB,
    NODE_RETURN,
    NODE_ON_ERROR_GOTO,
    NODE_DATA,
    NODE_READ,
    NODE_RESTORE,
    NODE_DIM,
    NODE_DEF_FN,
    NODE_DEF_PROC,
    NODE_FUNCTION_CALL,
    NODE_POKE,
    NODE_PEEK,
    NODE_ALLOCATE,
    NODE_FREE,
    NODE_END,
    NODE_STOP
} NodeType;

// AST Node structure
typedef struct ASTNode {
    NodeType node_type;
    char* value;
    struct ASTNode** children;
    int children_count;
} ASTNode;

// Function to create an AST Node
ASTNode* create_ast_node(NodeType type, char* value, int children_count) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->node_type = type;
    node->value = value ? strdup(value) : NULL;
    node->children_count = children_count;
    node->children = children_count > 0 ? (ASTNode**)malloc(sizeof(ASTNode*) * children_count) : NULL;
    return node;
}

// Function to add a child to an AST Node
void add_child(ASTNode* parent, ASTNode* child) {
    if (parent->children_count > 0) {
        parent->children[parent->children_count - 1] = child;
        parent->children_count--;
    }
}

// Helper functions to create specific AST nodes

ASTNode* create_program_node(ASTNode** children, int children_count) {
    ASTNode* node = create_ast_node(NODE_PROGRAM, NULL, children_count);
    for (int i = 0; i < children_count; i++) {
        node->children[i] = children[i];
    }
    return node;
}

ASTNode* create_print_node(ASTNode* expression) {
    ASTNode* node = create_ast_node(NODE_PRINT_STATEMENT, NULL, 1);
    add_child(node, expression);
    return node;
}

ASTNode* create_assignment_node(ASTNode* variable, ASTNode* expression) {
    ASTNode* node = create_ast_node(NODE_ASSIGNMENT, NULL, 2);
    add_child(node, variable);
    add_child(node, expression);
    return node;
}

ASTNode* create_if_node(ASTNode* condition, ASTNode* then_block, ASTNode* else_block) {
    int children_count = else_block ? 3 : 2;
    ASTNode* node = create_ast_node(NODE_IF_STATEMENT, NULL, children_count);
    add_child(node, condition);
    add_child(node, then_block);
    if (else_block) add_child(node, else_block);
    return node;
}

ASTNode* create_while_node(ASTNode* condition, ASTNode* block) {
    ASTNode* node = create_ast_node(NODE_WHILE_LOOP, NULL, 2);
    add_child(node, condition);
    add_child(node, block);
    return node;
}

ASTNode* create_for_node(ASTNode* init, ASTNode* condition, ASTNode* step, ASTNode* block) {
    ASTNode* node = create_ast_node(NODE_FOR_LOOP, NULL, 4);
    add_child(node, init);
    add_child(node, condition);
    add_child(node, step);
    add_child(node, block);
    return node;
}

ASTNode* create_repeat_until_node(ASTNode* block, ASTNode* condition) {
    ASTNode* node = create_ast_node(NODE_REPEAT_UNTIL, NULL, 2);
    add_child(node, block);
    add_child(node, condition);
    return node;
}

ASTNode* create_select_case_node(ASTNode* expression, ASTNode** cases, int case_count) {
    ASTNode* node = create_ast_node(NODE_SELECT_CASE, NULL, case_count + 1);
    add_child(node, expression);
    for (int i = 0; i < case_count; i++) {
        add_child(node, cases[i]);
    }
    return node;
}

ASTNode* create_case_node(ASTNode* value, ASTNode* block) {
    ASTNode* node = create_ast_node(NODE_CASE, NULL, 2);
    add_child(node, value);
    add_child(node, block);
    return node;
}

ASTNode* create_gosub_node(char* label) {
    return create_ast_node(NODE_GOSUB, label, 0);
}

ASTNode* create_return_node() {
    return create_ast_node(NODE_RETURN, NULL, 0);
}

ASTNode* create_on_error_goto_node(char* label) {
    return create_ast_node(NODE_ON_ERROR_GOTO, label, 0);
}

ASTNode* create_data_node(ASTNode** values, int value_count) {
    ASTNode* node = create_ast_node(NODE_DATA, NULL, value_count);
    for (int i = 0; i < value_count; i++) {
        add_child(node, values[i]);
    }
    return node;
}

ASTNode* create_read_node(ASTNode* variable) {
    ASTNode* node = create_ast_node(NODE_READ, NULL, 1);
    add_child(node, variable);
    return node;
}

ASTNode* create_restore_node(int line_number) {
    char buffer[20];
    sprintf(buffer, "%d", line_number);
    return create_ast_node(NODE_RESTORE, line_number ? strdup(buffer) : NULL, 0);
}

ASTNode* create_dim_node(ASTNode* variable, ASTNode** dimensions, int dimension_count) {
    ASTNode* node = create_ast_node(NODE_DIM, NULL, dimension_count + 1);
    add_child(node, variable);
    for (int i = 0; i < dimension_count; i++) {
        add_child(node, dimensions[i]);
    }
    return node;
}

ASTNode* create_def_fn_node(char* name, ASTNode* parameters, ASTNode* body) {
    ASTNode* node = create_ast_node(NODE_DEF_FN, name, 2);
    add_child(node, parameters);
    add_child(node, body);
    return node;
}

ASTNode* create_def_proc_node(char* name, ASTNode* parameters, ASTNode* body) {
    ASTNode* node = create_ast_node(NODE_DEF_PROC, name, 2);
    add_child(node, parameters);
    add_child(node, body);
    return node;
}

ASTNode* create_function_call_node(char* name, ASTNode** arguments, int argument_count) {
    ASTNode* node = create_ast_node(NODE_FUNCTION_CALL, name, argument_count);
    for (int i = 0; i < argument_count; i++) {
        add_child(node, arguments[i]);
    }
    return node;
}

ASTNode* create_poke_node(ASTNode* address, ASTNode* value) {
    ASTNode* node = create_ast_node(NODE_POKE, NULL, 2);
    add_child(node, address);
    add_child(node, value);
    return node;
}

ASTNode* create_peek_node(ASTNode* address) {
    ASTNode* node = create_ast_node(NODE_PEEK, NULL, 1);
    add_child(node, address);
    return node;
}

ASTNode* create_allocate_node(ASTNode* variable, ASTNode* size) {
    ASTNode* node = create_ast_node(NODE_ALLOCATE, NULL, 2);
    add_child(node, variable);
    add_child(node, size);
    return node;
}

ASTNode* create_free_node(ASTNode* variable) {
    ASTNode* node = create_ast_node(NODE_FREE, NULL, 1);
    add_child(node, variable);
    return node;
}

ASTNode* create_end_node() {
    return create_ast_node(NODE_END, NULL, 0);
}

ASTNode* create_stop_node() {
    return create_ast_node(NODE_STOP, NULL, 0);
}

// Function to free an AST Node and its children
void free_ast_node(ASTNode* node) {
    if (!node) return;
    if (node->value) free(node->value);
    if (node->children) {
        for (int i = 0; i < node->children_count; i++) {
            free_ast_node(node->children[i]);
        }
        free(node->children);
    }
    free(node);
}

// Example usage
int main() {
    // Example to create and print an AST Node
    ASTNode* print_node = create_print_node(create_ast_node(NODE_IDENTIFIER, "Hello", 0));
    printf("Created a print node with value: %s\n", print_node->children[0]->value);
    
    // Free the created AST Node
    free_ast_node(print_node);
    
    return 0;
}

