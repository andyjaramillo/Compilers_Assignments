#include <cassert>
#include <algorithm>
#include <memory>
#include "ast.h"
#include "environment.h"
#include "node.h"
#include "exceptions.h"
#include "function.h"
#include "interp.h"
#include "value.h"
#include <stack>
#include <queue>
#include <iostream>
#include <set>
#include <string>
Interpreter::Interpreter(Node *ast_to_adopt)
  : m_ast(ast_to_adopt) {
}

Interpreter::~Interpreter() {
  delete m_ast;
}

void Interpreter::analyze() {
  // TODO: implement

    analyzeBranch(m_ast);
    block.clear();
}

void Interpreter::analyzeBranch(Node * root) {

     if (root == nullptr) {
        return;
    }
   
    if(root->get_tag() == AST_VARREF) {
         if(block.get_value_by_string(root->get_str()) == -1){
            SemanticError::raise(root->get_loc(), "Reference to undefined name '%s' " , root->get_str().c_str());
         }
    } else if (root->get_tag() == AST_VARDEF) {
        block.vardef_var_ref(root->get_last_kid()->get_str());
    }

    // Recursively visit all children of the current node
    for (unsigned int j=0; j < root->get_num_kids(); j++) {
        analyzeBranch(root->get_kid(j));
    }

}

Value Interpreter::execute() {
    // TODO: implement
    std::queue<Node *> astQueue;
    astQueue.push(m_ast);
    while (!astQueue.empty()) {
        Node *currentNode = astQueue.front();
        astQueue.pop();
        int currentNodeTag = currentNode->get_tag();
        if (currentNodeTag == AST_INT_LITERAL) {
            block.assign_var_ref(currentNode->get_str(),
                                 std::stoi(currentNode->get_str()));
            Value result(std::stoi(currentNode->get_str()));
            if (astQueue.empty()) {
              return result;
            }
        } else if (currentNodeTag == AST_VARREF) {
            int block_result =
                block.get_value_by_string(currentNode->get_str());
            Value result(block_result);
            if (astQueue.empty()) {
              return result;
            }
        } else if (currentNodeTag == AST_ASSIGNMENT) {
            int childval = evaluate(currentNode->get_kid(1));
            block.assign_var_ref(currentNode->get_kid(0)->get_str(), childval);
            Value result(childval);
            if (astQueue.empty()) {
              return result;
            }
        } else if (currentNodeTag == AST_VARDEF) {
            block.vardef_var_ref(currentNode->get_kid(0)->get_str());
            // unary operator
        } else if (currentNodeTag == AST_ADD || currentNodeTag == AST_SUB ||
                   currentNodeTag == AST_MULTIPLY ||
                   currentNodeTag == AST_DIVIDE ||
                   currentNodeTag == AST_COMPARE ||
                   currentNodeTag == AST_NOTCOMPARE ||
                   currentNodeTag == AST_LESS ||
                   currentNodeTag == AST_LESSOREQUAL ||
                   currentNodeTag == AST_GREATER ||
                   currentNodeTag == AST_GREATEROREQUAL ||
                   currentNodeTag == AST_AND || currentNodeTag == AST_OR) {
            // binary operators
            return evaluate(currentNode);
        } else {
            for (unsigned int i = 0; i < currentNode->get_num_kids(); i++) {
              astQueue.push(currentNode->get_kid(i));
            }
        }
    }
    Value result;
    return result;
}

int Interpreter::evaluate(Node *root) {
    int rootTag = root->get_tag();
    if (rootTag == AST_ADD) {
        int left = evaluate(root->get_kid(0));
        int right = evaluate(root->get_kid(1));
        return left + right;
    } else if (rootTag == AST_SUB) {
        int left = evaluate(root->get_kid(0));
        int right = evaluate(root->get_kid(1));
        return left - right;
    } else if (rootTag == AST_MULTIPLY) {
        // handle if the computation is with variables or integer literals
        int left = evaluate(root->get_kid(0));
        int right = evaluate(root->get_kid(1));
        return left * right;
    } else if (rootTag == AST_DIVIDE) {
        // handle if the computation is with variables or integer literals
        int left = evaluate(root->get_kid(0));
        int right = evaluate(root->get_kid(1));
        if (right == 0) {
          EvaluationError::raise(root->get_loc(), "cant divide by 0");
        }
        return left / right;
    } else if (rootTag == AST_AND) {
        int left = evaluate(root->get_kid(0));
        int right = evaluate(root->get_kid(1));
        return (left != 0 && right != 0);
    } else if (rootTag == AST_OR) {
        int left = evaluate(root->get_kid(0));
        int right = evaluate(root->get_kid(1));
        return (left != 0 || right != 0);
    } else if (rootTag == AST_COMPARE) {
        int left = evaluate(root->get_kid(0));
        int right = evaluate(root->get_kid(1));
        return left == right;
    } else if (rootTag == AST_NOTCOMPARE) {
        int left = evaluate(root->get_kid(0));
        int right = evaluate(root->get_kid(1));
        return left != right;
    } else if (rootTag == AST_LESS) {
        int left = evaluate(root->get_kid(0));
        int right = evaluate(root->get_kid(1));
        return left < right;
    } else if (rootTag == AST_LESSOREQUAL) {
        int left = evaluate(root->get_kid(0));
        int right = evaluate(root->get_kid(1));
        return left <= right;
    } else if (rootTag == AST_GREATER) {
        int left = evaluate(root->get_kid(0));
        int right = evaluate(root->get_kid(1));
        return left > right;
    } else if (rootTag == AST_GREATEROREQUAL) {
        int left = evaluate(root->get_kid(0));
        int right = evaluate(root->get_kid(1));
        return left >= right;
    } else if (rootTag == AST_INT_LITERAL) {
        return std::stoi(root->get_str());
    } else if (rootTag == AST_VARREF) {
        return block.get_value_by_string(root->get_str());
    } else {
        SyntaxError::raise(root->get_loc(), "Unexpected token");
    }
}

std::array<int, 2> Interpreter::int_literal_or_var_ref(Node *left, Node *right) {
    int leftValue;
    int rightValue;
    if (left->get_tag() == AST_VARREF) {
        leftValue = block.get_value_by_string(left->get_str());
    } else {
        leftValue = std::stoi(left->get_str());
    }
    if (right->get_tag() == AST_VARREF) {
        rightValue = block.get_value_by_string(right->get_str());
    } else {
        rightValue = std::stoi(right->get_str());
    }
    std::array<int, 2> returnArray;
    returnArray[0] = leftValue;
    returnArray[1] = rightValue;
    return returnArray;
}
