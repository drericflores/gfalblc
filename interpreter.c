#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Define a structure for AST nodes
typedef struct ASTNode {
    char *node_type;
    char *value;
    struct ASTNode **children;
    int children_count;
} ASTNode;

// Define a structure for the Interpreter
typedef struct Interpreter {
    void (*output_callback)(const char*);
    int *variables;
    bool running;
    ASTNode **return_stack;
    int return_stack_size;
    int return_stack_capacity;
    int data_pointer;
} Interpreter;

// Function prototypes
Interpreter* interpreter_new(void (*output_callback)(const char*));
void interpreter_init(Interpreter* interpreter);
void interpreter_free(Interpreter* interpreter);
void run_program(Interpreter* interpreter, ASTNode* ast);
void execute_statement(Interpreter* interpreter, ASTNode* node);
void execute_print(Interpreter* interpreter, ASTNode* node);
void execute_assignment(Interpreter* interpreter, ASTNode* node);
void execute_if(Interpreter* interpreter, ASTNode* node);
void execute_while(Interpreter* interpreter, ASTNode* node);
void execute_for(Interpreter* interpreter, ASTNode* node);
void execute_repeat_until(Interpreter* interpreter, ASTNode* node);
void execute_select_case(Interpreter* interpreter, ASTNode* node);
void execute_goto(Interpreter* interpreter, ASTNode* node);
void execute_gosub(Interpreter* interpreter, ASTNode* node);
void execute_return(Interpreter* interpreter, ASTNode* node);
void execute_on_error_goto(Interpreter* interpreter, ASTNode* node);
void execute_data(Interpreter* interpreter, ASTNode* node);
void execute_read(Interpreter* interpreter, ASTNode* node);
int evaluate_expression(Interpreter* interpreter, ASTNode* node);
void execute_block(Interpreter* interpreter, ASTNode* node);

// Helper function prototypes
void push_return_stack(Interpreter* interpreter, ASTNode* node);
ASTNode* pop_return_stack(Interpreter* interpreter);

// Initialize a new interpreter
Interpreter* interpreter_new(void (*output_callback)(const char*)) {
    Interpreter* interpreter = (Interpreter*)malloc(sizeof(Interpreter));
    interpreter->output_callback = output_callback;
    interpreter->variables = (int*)calloc(1000, sizeof(int)); // Arbitrary size
    interpreter->running = true;
    interpreter->return_stack = NULL;
    interpreter->return_stack_size = 0;
    interpreter->return_stack_capacity = 0;
    interpreter->data_pointer = 0;
    return interpreter;
}

// Initialize interpreter state
void interpreter_init(Interpreter* interpreter) {
    memset(interpreter->variables, 0, 1000 * sizeof(int));
    interpreter->running = true;
    free(interpreter->return_stack);
    interpreter->return_stack = NULL;
    interpreter->return_stack_size = 0;
    interpreter->return_stack_capacity = 0;
    interpreter->data_pointer = 0;
    printf("[DEBUG] Interpreter initialized.\n");
}

// Free interpreter resources
void interpreter_free(Interpreter* interpreter) {
    free(interpreter->variables);
    free(interpreter->return_stack);
    interpreter->running = false;
    printf("[DEBUG] Interpreter resources have been freed.\n");
    free(interpreter);
}

// Run the program by traversing the AST
void run_program(Interpreter* interpreter, ASTNode* ast) {
    if (strcmp(ast->node_type, "program") != 0) {
        fprintf(stderr, "Expected program node\n");
        exit(1);
    }
    printf("[DEBUG] Starting program execution.\n");
    for (int i = 0; i < ast->children_count; i++) {
        execute_statement(interpreter, ast->children[i]);
        if (!interpreter->running) break;
    }
}

// Execute a specific statement node
void execute_statement(Interpreter* interpreter, ASTNode* node) {
    if (!interpreter->running) return;

    if (strcmp(node->node_type, "print_statement") == 0) {
        execute_print(interpreter, node);
    } else if (strcmp(node->node_type, "assignment") == 0) {
        execute_assignment(interpreter, node);
    } else if (strcmp(node->node_type, "if_statement") == 0) {
        execute_if(interpreter, node);
    } else if (strcmp(node->node_type, "while_loop") == 0) {
        execute_while(interpreter, node);
    } else if (strcmp(node->node_type, "for_loop") == 0) {
        execute_for(interpreter, node);
    } else if (strcmp(node->node_type, "repeat_until") == 0) {
        execute_repeat_until(interpreter, node);
    } else if (strcmp(node->node_type, "select_case") == 0) {
        execute_select_case(interpreter, node);
    } else if (strcmp(node->node_type, "goto_statement") == 0) {
        execute_goto(interpreter, node);
    } else if (strcmp(node->node_type, "gosub_statement") == 0) {
        execute_gosub(interpreter, node);
    } else if (strcmp(node->node_type, "return_statement") == 0) {
        execute_return(interpreter, node);
    } else if (strcmp(node->node_type, "on_error_goto") == 0) {
        execute_on_error_goto(interpreter, node);
    } else if (strcmp(node->node_type, "data_statement") == 0) {
        execute_data(interpreter, node);
    } else if (strcmp(node->node_type, "read_statement") == 0) {
        execute_read(interpreter, node);
    } else {
        fprintf(stderr, "Unknown statement type: %s\n", node->node_type);
        exit(1);
    }
}

// Execute a print statement
void execute_print(Interpreter* interpreter, ASTNode* node) {
    int expr_value = evaluate_expression(interpreter, node->children[0]);
    char output[50];
    snprintf(output, sizeof(output), "%d", expr_value);
    if (interpreter->output_callback) {
        interpreter->output_callback(output);
    } else {
        printf("%s\n", output);
    }
    printf("[DEBUG] Printed: %s\n", output);
}

// Execute an assignment statement
void execute_assignment(Interpreter* interpreter, ASTNode* node) {
    int expr_value = evaluate_expression(interpreter, node->children[1]);
    int var_index = atoi(node->children[0]->value); // Assuming variable names are numbers for simplicity
    interpreter->variables[var_index] = expr_value;
    printf("[DEBUG] Assigned %s = %d\n", node->children[0]->value, expr_value);
}

// Execute an if statement
void execute_if(Interpreter* interpreter, ASTNode* node) {
    int condition_value = evaluate_expression(interpreter, node->children[0]);
    if (condition_value) {
        printf("[DEBUG] IF condition met, executing THEN branch\n");
        execute_block(interpreter, node->children[1]);
    } else if (node->children_count > 2) {
        printf("[DEBUG] IF condition not met, executing ELSE branch\n");
        execute_block(interpreter, node->children[2]);
    }
}

// Execute a while loop
void execute_while(Interpreter* interpreter, ASTNode* node) {
    while (evaluate_expression(interpreter, node->children[0])) {
        printf("[DEBUG] WHILE loop condition met, executing loop body\n");
        execute_block(interpreter, node->children[1]);
        if (!interpreter->running) break;
    }
}

// Execute a for loop
void execute_for(Interpreter* interpreter, ASTNode* node) {
    int init = evaluate_expression(interpreter, node->children[0]);
    int end = evaluate_expression(interpreter, node->children[1]);
    int step = (node->children_count > 2) ? evaluate_expression(interpreter, node->children[2]) : 1;
    int var_index = atoi(node->children[0]->children[0]->value);
    for (int i = init; i <= end; i += step) {
        interpreter->variables[var_index] = i;
        printf("[DEBUG] FOR loop iteration %s = %d\n", node->children[0]->children[0]->value, i);
        execute_block(interpreter, node->children[3]);
        if (!interpreter->running) break;
    }
}

// Execute a repeat-until loop
void execute_repeat_until(Interpreter* interpreter, ASTNode* node) {
    do {
        printf("[DEBUG] Executing REPEAT loop body\n");
        execute_block(interpreter, node->children[0]);
    } while (!evaluate_expression(interpreter, node->children[1]) && interpreter->running);
}

// Execute a select-case statement
void execute_select_case(Interpreter* interpreter, ASTNode* node) {
    int expr_value = evaluate_expression(interpreter, node->children[0]);
    for (int i = 1; i < node->children_count; i++) {
        int case_value = evaluate_expression(interpreter, node->children[i]->children[0]);
        if (expr_value == case_value) {
            execute_block(interpreter, node->children[i]->children[1]);
            break;
        }
    }
}

// Execute a GOTO statement (not yet implemented)
void execute_goto(Interpreter* interpreter, ASTNode* node) {
    printf("[DEBUG] GOTO label %s - not yet implemented.\n", node->value);
}

// Execute a GOSUB statement
void execute_gosub(Interpreter* interpreter, ASTNode* node) {
    push_return_stack(interpreter, node);
    printf("[DEBUG] GOSUB to label %s - not yet implemented.\n", node->value);
}

// Execute a RETURN statement
void execute_return(Interpreter* interpreter, ASTNode* node) {
    if (interpreter->return_stack_size == 0) {
        fprintf(stderr, "RETURN called without a corresponding GOSUB\n");
        exit(1);
    }
    ASTNode* return_node = pop_return_stack(interpreter);
    printf("[DEBUG] RETURNING to label %s - not yet implemented.\n", return_node->value);
}

// Execute an ON ERROR GOTO statement (not yet implemented)
void execute_on_error_goto(Interpreter* interpreter, ASTNode* node) {
    printf("[DEBUG] ON ERROR GOTO label %s - not yet implemented.\n", node->value);
}

// Execute a DATA statement (not yet implemented)
void execute_data(Interpreter* interpreter, ASTNode* node) {
    printf("[DEBUG] DATA statement encountered - not yet implemented.\n");
}

// Execute a READ statement (not yet implemented)
void execute_read(Interpreter* interpreter, ASTNode* node) {
    printf("[DEBUG] READ into variable %s - not yet implemented.\n", node->children[0]->value);
}

// Evaluate an expression node
int evaluate_expression(Interpreter* interpreter, ASTNode* node) {
    if (strcmp(node->node_type, "int_literal") == 0) {
        return atoi(node->value);
    } else if (strcmp(node->node_type, "identifier") == 0) {
        int var_index = atoi(node->value);
        int value = interpreter->variables[var_index];
        printf("[DEBUG] Retrieved value for %s = %d\n", node->value, value);
        return value;
    } else if (strcmp(node->node_type, "operator") == 0) {
        int left = evaluate_expression(interpreter, node->children[0]);
        int right = evaluate_expression(interpreter, node->children[1]);
        if (strcmp(node->value, "+") == 0) return left + right;
        if (strcmp(node->value, "-") == 0) return left - right;
        if (strcmp(node->value, "*") == 0) return left * right;
        if (strcmp(node->value, "/") == 0) return left / right;
        fprintf(stderr, "Unknown operator: %s\n", node->value);
        exit(1);
    } else {
        fprintf(stderr, "Unknown expression type: %s\n", node->node_type);
        exit(1);
    }
}

// Execute a block of statements
void execute_block(Interpreter* interpreter, ASTNode* node) {
    if (strcmp(node->node_type, "block") != 0) {
        fprintf(stderr, "Expected block node\n");
        exit(1);
    }
    for (int i = 0; i < node->children_count; i++) {
        execute_statement(interpreter, node->children[i]);
        if (!interpreter->running) break;
    }
}

// Push an AST node onto the return stack
void push_return_stack(Interpreter* interpreter, ASTNode* node) {
    if (interpreter->return_stack_size >= interpreter->return_stack_capacity) {
        interpreter->return_stack_capacity = (interpreter->return_stack_capacity == 0) ? 10 : interpreter->return_stack_capacity * 2;
        interpreter->return_stack = (ASTNode**)realloc(interpreter->return_stack, interpreter->return_stack_capacity * sizeof(ASTNode*));
    }
    interpreter->return_stack[interpreter->return_stack_size++] = node;
}

// Pop an AST node from the return stack
ASTNode* pop_return_stack(Interpreter* interpreter) {
    if (interpreter->return_stack_size == 0) {
        return NULL;
    }
    return interpreter->return_stack[--interpreter->return_stack_size];
}

int main() {
    // Example usage
    Interpreter* interpreter = interpreter_new(NULL);
    interpreter_init(interpreter);

    // Example AST node setup and run
    // This should be replaced with the actual AST creation logic
    ASTNode* program_node = (ASTNode*)malloc(sizeof(ASTNode));
    program_node->node_type = "program";
    program_node->children_count = 0;
    program_node->children = NULL;

    run_program(interpreter, program_node);

    interpreter_free(interpreter);
    free(program_node);
    return 0;
}

