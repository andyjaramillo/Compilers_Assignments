#include "interp.h"
#include "ast.h"
#include "environment.h"
#include "exceptions.h"
#include "function.h"
#include "array.h"
#include "string.h"
#include "node.h"
#include "value.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <queue>
#include <set>
#include <stack>
#include <string>

Value Interpreter::intrinsic_print(Value args[], unsigned num_args, const Location &loc, Interpreter *interp){
    if (num_args != 1) {
         EvaluationError::raise(loc, "Wrong number of arguments passed to print function");
    }
  printf("%s", args[0].as_str().c_str());
  return Value();
}

Value Interpreter::intrinsic_println(Value args[], unsigned num_args, const Location &loc, Interpreter *interp){
    if (num_args != 1) {
         EvaluationError::raise(loc, "Wrong number of arguments passed to println function");
    }
  printf("%s\n", args[0].as_str().c_str());
  return Value();
}

Value Interpreter::intrinsic_readint(Value args[], unsigned num_args, const Location &loc, Interpreter *interp){
    int x;
     if (scanf("%d", &x) == 0) {
        EvaluationError::raise(loc, "Wrong number of arguments passed to readint function");
    } else {
        return Value(x);
    }
}

//USING IT PROPERLY
//Put the array as the first element in args
Value Interpreter::intrinsic_mkarr(Value args[], unsigned num_args, const Location &loc, Interpreter *interp){
    //so Value ars[] is a set of all the arguments, one of those can be a value that has the array associated with it
    std::vector<Value> content;
    for (size_t i = 0; i < num_args; ++i) {
        content.push_back(args[i]);
    }
    Array * array = new Array(content);
    return Value(array);
}
//USING IT PROPERLY
//Put the array as the first element in args
Value Interpreter::intrinsic_len(Value args[], unsigned num_args, const Location &loc, Interpreter *interp){
    if(num_args > 1 || num_args == 0) { 
        EvaluationError::raise(loc, "Wrong number of arguments passed to length function"); 
    }
    //so Value ars[] is a set of all the arguments, one of those can be a value that has the array associated with it
    if(args[0].get_kind() != VALUE_ARRAY) {
         EvaluationError::raise(loc, "First parameter is not a array"); 
    }
    unsigned len = args[0].get_array()->get_content_len();
    return Value(len);
}

//USING IT PROPERLY
//Put the array as the first element in args
//Put the get index in the second element
Value Interpreter::intrinsic_get(Value args[], unsigned num_args, const Location &loc, Interpreter *interp){
    if(num_args != 2) { 
        EvaluationError::raise(loc, "Wrong number of arguments passed to get function"); 
    }
    //so Value ars[] is a set of all the arguments, one of those can be a value that has the array associated with it
    if(args[0].get_kind() != VALUE_ARRAY) {
         EvaluationError::raise(loc, "First parameter is not a array"); 
    }
    Array * array = args[0].get_array();
    if(args[1].get_kind() != VALUE_INT) {
         EvaluationError::raise(loc, "Second parameter is not a integer for index"); 
    }
    unsigned index = args[1].get_ival();
    if(index > array->get_content_len() - 1 || index < 0) {
         EvaluationError::raise(loc, "Invalid index"); 
    }
    Value retrieved_element = array->get_content_element(index);
    return retrieved_element;
}
//USING IT PROPERLY
//Put the array as the first element in args
//Put the get index in the second element
//Put the value to store in the third element
Value Interpreter::intrinsic_set(Value args[], unsigned num_args, const Location &loc, Interpreter *interp){
     if(num_args != 3) { 
        EvaluationError::raise(loc, "Wrong number of arguments passed to set function"); 
    }
    //so Value ars[] is a set of all the arguments, one of those can be a value that has the array associated with it
    if(args[0].get_kind() != VALUE_ARRAY) {
         EvaluationError::raise(loc, "First parameter is not a array"); 
    }
    Array * array = args[0].get_array();
     if(args[1].get_kind() != VALUE_INT) {
         EvaluationError::raise(loc, "Second parameter is not a integer for index"); 
    }
    unsigned index = args[1].get_ival();
    Value to_store = args[2];
    if(index > array->get_content_len() - 1 || index < 0) {
        EvaluationError::raise(loc, "Invalid index"); 
    }
    array->set_content(to_store, index);
    return to_store;
}
//USING IT PROPERLY
//Put the array as the first element in args
//Put the value to store in the second element
Value Interpreter::intrinsic_push(Value args[], unsigned num_args, const Location &loc, Interpreter *interp){
     if(num_args != 2) { 
        EvaluationError::raise(loc, "Wrong number of arguments passed to set function"); 
    }
    //so Value ars[] is a set of all the arguments, one of those can be a value that has the array associated with it
    if(args[0].get_kind() != VALUE_ARRAY) {
         EvaluationError::raise(loc, "First parameter is not a array"); 
    }
    Array * array = args[0].get_array();
        
    Value to_push = args[1];
    array->push_content(to_push);

    return to_push;

}
//USING IT PROPERLY
//Put the array as the first element in args
Value Interpreter::intrinsic_pop(Value args[], unsigned num_args, const Location &loc, Interpreter *interp){
   if(num_args != 1) { 
        EvaluationError::raise(loc, "Wrong number of arguments passed to set function"); 
    }
    //so Value ars[] is a set of all the arguments, one of those can be a value that has the array associated with it
    if(args[0].get_kind() != VALUE_ARRAY) {
         EvaluationError::raise(loc, "First parameter is not a array"); 
    }
    Array * array = args[0].get_array();
     if(array->get_content_len() == 0) { 
        EvaluationError::raise(loc, "Array is empty"); 
    }
    return array->pop_content();
}

Value Interpreter::intrinsic_substr(Value args[], unsigned num_args, const Location &loc, Interpreter *interp){
     if(num_args != 3) { 
        EvaluationError::raise(loc, "Wrong number of arguments passed to substr function"); 
    }
    if(args[0].get_kind() != VALUE_STRING){
        EvaluationError::raise(loc, "1st parameter is not a string"); 
    }
    std::vector<std::string> full_string = args[0].get_string()->get_sub_string();
    if(args[1].get_kind() != VALUE_INT){
        EvaluationError::raise(loc, "2nd parameter is not an integer"); 
    }
    unsigned start_index = args[1].get_ival();
     if(args[2].get_kind() != VALUE_INT){
        EvaluationError::raise(loc, "3rd parameter is not an integer"); 
    }
    unsigned end_index = args[2].get_ival();
    if( start_index < 0 || end_index < 0 || start_index > full_string.size() || end_index > full_string.size() || start_index + end_index > full_string.size()){
        std::string empty("");
         String * allocated_string = new String(empty);
        return Value(allocated_string);
    }
    std::string actual;
    for (const std::string& str : full_string) {
        actual += str;
    }
    std::string parsed_string = actual.substr(start_index, (end_index-start_index) + start_index);
    std::vector<std::string> parsed_vector_string;
    for(unsigned i =0; i < parsed_string.size(); i++) {
        std::string tmp;
        tmp = tmp + parsed_string[i];
        parsed_vector_string.push_back(tmp);
    }
    String * allocated_string = new String(parsed_vector_string);
    return Value(allocated_string);
}


Value Interpreter::intrinsic_strcat(Value args[], unsigned num_args, const Location &loc, Interpreter *interp){
     if(num_args != 2) { 
        EvaluationError::raise(loc, "Wrong number of arguments passed to strcat function"); 
    }
    if(args[0].get_kind() != VALUE_STRING){
        EvaluationError::raise(loc, "1st parameter is not a string"); 
    }
    std::vector<std::string> left_string = args[0].get_string()->get_sub_string();
    if(args[1].get_kind() != VALUE_STRING){
        EvaluationError::raise(loc, "2nd parameter is not a string"); 
    }
    std::vector<std::string> right_string = args[1].get_string()->get_sub_string();
    left_string.insert( left_string.end(), right_string.begin(), right_string.end());
    String * allocated_string = new String(left_string);
    return Value(allocated_string);
}

Value Interpreter::intrinsic_strlen(Value args[], unsigned num_args, const Location &loc, Interpreter *interp){
     if(num_args != 1) { 
        EvaluationError::raise(loc, "Wrong number of arguments passed to strlen function"); 
    }
     if(args[0].get_kind() != VALUE_STRING){
        EvaluationError::raise(loc, "1st parameter is not a string"); 
    }
    std::vector<std::string> current_string = args[0].get_string()->get_sub_string();
    return Value(current_string.size());
}


Interpreter::Interpreter(Node *ast_to_adopt)
  : m_ast(ast_to_adopt), block(new Environment()) {
    block->bind("print", Value(&intrinsic_print));
    block->bind("println", Value(&intrinsic_println));
    block->bind("readint", Value(&intrinsic_readint));
    block->bind("mkarr", Value(&intrinsic_mkarr));
    block->bind("len", Value(&intrinsic_len));
    block->bind("get", Value(&intrinsic_get));
    block->bind("set", Value(&intrinsic_set));
    block->bind("push", Value(&intrinsic_push));
    block->bind("pop", Value(&intrinsic_pop));
    block->bind("substr", Value(&intrinsic_substr));
    block->bind("strcat", Value(&intrinsic_strcat));
    block->bind("strlen", Value(&intrinsic_strlen));
}

Interpreter::~Interpreter() {
  delete m_ast;
  delete block;
}

void Interpreter::analyze() {
  // TODO: implement

    analyzeBranch(m_ast, block);
    block->clear();
    block->bind("print", Value(&intrinsic_print));
    block->bind("println", Value(&intrinsic_println));
    block->bind("readint", Value(&intrinsic_readint));
    block->bind("mkarr", Value(&intrinsic_mkarr));
    block->bind("len", Value(&intrinsic_len));
    block->bind("get", Value(&intrinsic_get));
    block->bind("set", Value(&intrinsic_set));
    block->bind("push", Value(&intrinsic_push));
    block->bind("pop", Value(&intrinsic_pop));
    block->bind("substr", Value(&intrinsic_substr));
    block->bind("strcat", Value(&intrinsic_strcat));
    block->bind("strlen", Value(&intrinsic_strlen));
}

bool Interpreter::checkIfInstrinsic(std::string varRef) {

    if(varRef != "print" && varRef != "println" && varRef != "readint" && varRef != "mkarr" && varRef != "len" && varRef != "get" && varRef != "set" && varRef != "push" && varRef != "pop" && varRef != "substr" && varRef != "strcat" && varRef != "strlen"){
        return false;
    } else {
        return true;
    }
}


void Interpreter::analyzeBranch(Node * root, Environment * current_block) {

     if (root == nullptr) {
        return;
    }

    if(root->get_tag() == AST_VARREF) {
        if(checkIfInstrinsic(root->get_str()) == false){
            Value result = current_block->get_value_by_string(root);
            if(result.get_kind() == VALUE_INT && result.get_ival() == -1){
                 SemanticError::raise(root->get_loc(), "Reference to undefined name '%s' " , root->get_str().c_str());
            }
        }
    } else if (root->get_tag() == AST_VARDEF) {
           Value k = Value();
        //    if(checkIfInstrinsic(root->get_last_kid()->get_str())){
        //         SemanticError::raise(root->get_loc(), "Cannot have function variable reference name");
        //    }
        if(current_block->has_value_by_node(root->get_last_kid())){
            SemanticError::raise(root->get_loc(), "variable name already exists");
        }
        current_block->vardef_var_ref(root->get_last_kid()->get_str(), k);
    }    else if(root->get_tag() == AST_STATEMENT && root->get_kid(0)->get_tag() == AST_ELSE) {
         SemanticError::raise(root->get_kid(0)->get_loc(), "Incorrect use of else body. If body missing");

    }
    else if(root->get_tag() == AST_FUNC){
        //add to block
       //Function(const std::string &name, const std::vector<std::string> &params, Environment *parent_env, Node *body);
                std::string fn_name;
                std::vector<std::string> param_names;
                Node *body;
                fn_name = root->get_kid(0)->get_str();
                if(root->get_kid(0)->get_tag() == AST_PARAMETER_LIST) {
                     Node* parameterList = root->get_kid(1);
                    for(unsigned i=0; i <parameterList->get_num_kids(); i++) {
                        param_names.push_back(parameterList->get_kid(i)->get_str());
                    }
                    body = root->get_kid(2);
                    Value fn_val(new Function(fn_name, param_names, current_block->get_parent(), body));
                    current_block->function_var_ref(fn_name, &fn_val);
                } else {
                    body = root->get_kid(1);
                       Value fn_val(new Function(fn_name, param_names, current_block->get_parent(), body));
                    current_block->function_var_ref(fn_name, &fn_val);
                }
               
                return;

    }
    else if(root->get_tag() == AST_FNCALL) {
        // Value fnCall = current_block->get_value_by_string(root->get_kid(0));
        // if(fnCall.get_kind() == VALUE_FUNCTION){
        //     if(current_block->get_value_by_string(root->get_kid(0)).get_function() == nullptr) {
        //     //function name not found
        //     EvaluationError::raise(root->get_loc(), "Function not defined");
        //  } else if(current_block->get_value_by_string(root->get_kid(0)).get_function()->get_num_params() != root->get_kid(1)->get_num_kids()){
        //      EvaluationError::raise(root->get_loc(), "Incorrect number of parameters");
        // }
        // }
    }

    // Recursively visit all children of the current node
 
    for (unsigned int j=0; j < root->get_num_kids(); j++) {
        if(root->get_tag() == AST_IF || root->get_tag() == AST_ELSE || root->get_tag() == AST_WHILE){
            
            Environment* inner_block = new Environment(current_block);
            analyzeBranch(root->get_kid(j), inner_block);
             delete inner_block;
        } else {
              analyzeBranch(root->get_kid(j), current_block);
        }
    }

}

 



Value Interpreter::execute() {
    // TODO: implement

    return recursive_execute(m_ast, block);
}

Value Interpreter::recursive_execute(Node* ast, Environment * current_block) {
    int currentNodeTag = ast->get_tag();
    if (currentNodeTag == AST_INT_LITERAL) {
            Value result = Value(std::stoi(ast->get_str()));
          //  current_block->assign_var_ref(ast->get_str(), result);
              return result;
        } else if (currentNodeTag == AST_VARREF) {
            //assuming that every variable reference has a variable definition already. This is determined
            //from the analyze function.
            //We recursively check for this variable reference and return it
            Value block_result = current_block->get_value_by_string(ast);
            return block_result;
        } else if (currentNodeTag == AST_ASSIGNMENT) {
            //
            Value result = evaluate(ast->get_kid(1), current_block);
            current_block->assign_var_ref(ast->get_kid(0)->get_str(), result);
            return result;
        } else if (currentNodeTag == AST_VARDEF) {
            Value k = Value();
            current_block->vardef_var_ref(ast->get_kid(0)->get_str(), k);
            return k;
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
            Value result = Value(evaluate(ast, current_block));
            return result;
        } else if (currentNodeTag == AST_STATEMENT && ast->get_kid(0)->get_tag() == AST_IF) {
            Value if_condition = evaluate(ast->get_kid(0)->get_kid(0), current_block);
              if(if_condition.get_kind() != VALUE_INT) {
                EvaluationError::raise(ast->get_loc(), "While condition is not a numeric value");
            }
            if(if_condition.get_ival() != 0){
                Environment *  inner_block = new Environment(current_block);
                Value result = Value(0);
                if(ast->get_kid(0)->get_num_kids() > 1) {
                     result = recursive_execute(ast->get_kid(0)->get_kid(1), inner_block);
                }
                delete inner_block;
                return Value(0);
            } else {
                //execute else statement if it exists
                if(ast->get_num_kids() > 1 &&  ast->get_kid(1) != nullptr &&ast->get_kid(1)->get_tag() == AST_ELSE) {
                     Environment *  inner_block = new Environment(current_block);
                     Value result = Value(0);
                    if(ast->get_kid(0)->get_num_kids() >1) {
                        result = recursive_execute(ast->get_kid(1)->get_kid(0), inner_block);
                    }
                delete inner_block;
                 return Value(0);
                }
                }
                return Value(0);
        }  else if(currentNodeTag == AST_STATEMENT && ast->get_kid(0)->get_tag() == AST_WHILE) {
            Node * whileNode = ast->get_kid(0);
            //this works because there must be a parent operator for the loop condition
            Value loop_condition = evaluate(whileNode->get_kid(0)->get_kid(0), current_block);
            Environment* inner_block = new Environment(current_block);
            if(loop_condition.get_kind() != VALUE_INT) {
                EvaluationError::raise(ast->get_loc(), "While condition is not a numeric value");
            }
            while(loop_condition.get_ival() != 0) {
                if(whileNode->get_num_kids() <= 1) {
                    continue;
                }
            recursive_execute(whileNode->get_kid(1), inner_block);
            loop_condition = evaluate(whileNode->get_kid(0)->get_kid(0),current_block);
            }
            delete inner_block;
            return Value(0);
        } else if(currentNodeTag == AST_FNCALL){
            Value name = current_block->get_value_by_string(ast->get_kid(0));
            if(name.get_kind() == VALUE_INTRINSIC_FN){
                  //its an instrinsic function
                  unsigned argListNumber = 0;
                Node * argList = nullptr;
                  if(ast->get_num_kids() > 1) {
                    argListNumber = ast->get_kid(1)->get_num_kids();
                    argList= ast->get_kid(1);
                  }
                Value args[argListNumber];
                 IntrinsicFn fn = current_block->get_value_by_string(ast->get_kid(0)).get_intrinsic_fn();
                 Value result = Value(0);
                    for(unsigned i =0; i< argListNumber; i++) {
                    args[i] = evaluate(argList->get_kid(i), current_block);            
                } 
               return fn(args, argListNumber, ast->get_loc(), this);
            } else {
                // If the called function value represents an ordinary “user-defined” 
                // function (as opposed to an intrinsic function), 
                // the number of arguments should be compared to the called function’s 
                // number of parameters. If they are different, an EvaluationError should be raised.
                 unsigned num_of_parameters = 0;
                 for(unsigned i =0; i< ast->get_num_kids() ; i++) {
                    if(ast->get_kid(i)->get_tag() == AST_ARGLIST) {
                        num_of_parameters = ast->get_kid(i)->get_num_kids();
                    }
                 }
                Value function_value = current_block->get_value_by_string(ast->get_kid(0));
                unsigned function_params = function_value.get_function()->get_num_params();
                if(num_of_parameters != function_params){
                    EvaluationError::raise(ast->get_loc(), "mismatching in number of parameters");
                }
                Function * fn = function_value.get_function();
                Environment* inner_block = new Environment(fn->get_parent_env());
                 Value args[fn->get_num_params()];
                 for(unsigned i=0; i < num_of_parameters; i++) {
                    Value evaluated_parameter = evaluate(ast->get_kid(1)->get_kid(i), current_block);
                    inner_block->vardef_var_ref(fn->get_params()[i], evaluated_parameter);
                 }
                 Value result = Value(0);
                 if(fn->get_body() != nullptr){
                    for(unsigned i = 0; i < fn->get_body()->get_num_kids(); i++) {
                    result = recursive_execute(fn->get_body()->get_kid(i), inner_block);
                     }
                 }
                 delete inner_block;
                 return result;
                }
            } else if(currentNodeTag == AST_FUNC) {
         //       1. A function value should be created
        //        2. The name of the function should be bound (assigned) to the function name in the global environment

            // +--FUNCTION
            // |  +--VARREF[f]
            // |  +--PARAMETER_LIST
            // |  |  +--VARREF[x]
            // |  +--AST_STATEMENT_LIST
            // |     +--STATEMENT
            // |        +--ADD
            // |           +--VARREF[x]
            // |           +--VARREF[g]
                std::string fn_name;
                std::vector<std::string> param_names;
                Node *body = nullptr;
                fn_name = ast->get_kid(0)->get_str();
                Node * parameterList = nullptr;
                for(unsigned i =0; i < ast->get_num_kids(); i++) {
                    if(ast->get_kid(i)->get_tag() == AST_PARAMETER_LIST){
                        parameterList = ast->get_kid(i);
                    }
                }
                if(parameterList != nullptr){
                      for(unsigned i=0; i <parameterList->get_num_kids(); i++) {
                        param_names.push_back(parameterList->get_kid(i)->get_str());
                    }
                    if(ast->get_num_kids() > 2) {
                           body = ast->get_kid(2);
                    }
                    Value fn_val(new Function(fn_name, param_names, current_block->get_parent(), body));
                    current_block->function_var_ref(fn_name, &fn_val);
                } else {
                     if(ast->get_num_kids() > 1) {
                       body = ast->get_kid(1);
                    }
                       Value fn_val(new Function(fn_name, param_names, current_block->get_parent(), body));
                    current_block->function_var_ref(fn_name, &fn_val);
                }
               


                return Value(0);
            
        } else if(currentNodeTag == AST_STRING){
            Value res = evaluate(ast, current_block);
            return res;
        } else if (currentNodeTag == AST_STATEMENT_LIST) {
            for (unsigned int i = 0; i < ast->get_num_kids(); i++) {
            recursive_execute(ast->get_kid(i), current_block);
        }
            return Value(0);
        } 
        else {
            Value result;
            for (unsigned int i = 0; i < ast->get_num_kids(); i++) {
                if(i == ast->get_num_kids() - 1) {
                     result = recursive_execute(ast->get_kid(i), current_block);
                } else {
                    recursive_execute(ast->get_kid(i), current_block);
                }
           
        }
            return result;
        }
    }

Value Interpreter::evaluate(Node *root, Environment * current_block) {
    int rootTag = root->get_tag();
    if (rootTag == AST_ADD) {
        Value left = evaluate(root->get_kid(0),current_block);
        Value right = evaluate(root->get_kid(1),current_block);
        if(left.get_kind() != VALUE_INT || right.get_kind() != VALUE_INT){
             EvaluationError::raise(root->get_loc(), "Value is not an int");
        }
        return Value(left.get_ival() + right.get_ival());
    } else if (rootTag == AST_SUB) {
        Value left = evaluate(root->get_kid(0),current_block);
        Value right = evaluate(root->get_kid(1),current_block);
        if(left.get_kind() != VALUE_INT || right.get_kind() != VALUE_INT){
             EvaluationError::raise(root->get_loc(), "Value is not an int");
        }
        return Value(left.get_ival() - right.get_ival());
    } else if (rootTag == AST_MULTIPLY) {
        // handle if the computation is with variables or integer literals
        Value left = evaluate(root->get_kid(0),current_block);
        Value right = evaluate(root->get_kid(1),current_block);
        if(left.get_kind() != VALUE_INT || right.get_kind() != VALUE_INT){
             EvaluationError::raise(root->get_loc(), "Value is not an int");
        }
        return Value(left.get_ival() * right.get_ival());
    } else if (rootTag == AST_DIVIDE) {
        // handle if the computation is with variables or integer literals
        Value left = evaluate(root->get_kid(0),current_block);
        Value right = evaluate(root->get_kid(1),current_block);
        if(left.get_kind() != VALUE_INT || right.get_kind() != VALUE_INT){
             EvaluationError::raise(root->get_loc(), "Value is not an int");
        }
        if (right.get_ival() == 0) {
          EvaluationError::raise(root->get_loc(), "cant divide by 0");
        }
        
        return Value(left.get_ival() / right.get_ival());
    } else if (rootTag == AST_AND) {
        Value left = evaluate(root->get_kid(0),current_block);
        if(left.get_kind() != VALUE_INT){
             EvaluationError::raise(root->get_loc(), "Value is not an int");
        }
        if(left.get_ival() == 0) {
            return Value(0);
        }
        Value right = evaluate(root->get_kid(1),current_block);
        if(right.get_kind() != VALUE_INT){
             EvaluationError::raise(root->get_loc(), "Value is not an int");
        }
        return Value((left.get_ival() != 0 && right.get_ival() != 0));
    } else if (rootTag == AST_OR) {
        Value left = evaluate(root->get_kid(0),current_block);
        if(left.get_kind() != VALUE_INT){
             EvaluationError::raise(root->get_loc(), "Value is not an int");
        }
        if(left.get_ival() != 0) {
            return Value(1);
        }
        Value right = evaluate(root->get_kid(1),current_block);
        if(right.get_kind() != VALUE_INT){
             EvaluationError::raise(root->get_loc(), "Value is not an int");
        }
        
        return Value((left.get_ival() != 0 || right.get_ival() != 0));
    } else if (rootTag == AST_COMPARE) {
        Value left = evaluate(root->get_kid(0),current_block);
        Value right = evaluate(root->get_kid(1),current_block);
        if(left.get_kind() != VALUE_INT || right.get_kind() != VALUE_INT){
             EvaluationError::raise(root->get_loc(), "Value is not an int");
        }
        return Value(left.get_ival() == right.get_ival());
    } else if (rootTag == AST_NOTCOMPARE) {
        Value left = evaluate(root->get_kid(0),current_block);
        Value right = evaluate(root->get_kid(1),current_block);
        if(left.get_kind() != VALUE_INT || right.get_kind() != VALUE_INT){
             EvaluationError::raise(root->get_loc(), "Value is not an int");
        }
        return Value(left.get_ival() != right.get_ival());
    } else if (rootTag == AST_LESS) {
        Value left = evaluate(root->get_kid(0),current_block);
        Value right = evaluate(root->get_kid(1),current_block);
        if(left.get_kind() != VALUE_INT || right.get_kind() != VALUE_INT){
             EvaluationError::raise(root->get_loc(), "Value is not an int");
        }
        return Value(left.get_ival() < right.get_ival());
    } else if (rootTag == AST_LESSOREQUAL) {
        Value left = evaluate(root->get_kid(0),current_block);
        Value right = evaluate(root->get_kid(1),current_block);
        if(left.get_kind() != VALUE_INT || right.get_kind() != VALUE_INT){
             EvaluationError::raise(root->get_loc(), "Value is not an int");
        }
        return Value(left.get_ival() <= right.get_ival());
    } else if (rootTag == AST_GREATER) {
        Value left = evaluate(root->get_kid(0),current_block);
        Value right = evaluate(root->get_kid(1),current_block);
        if(left.get_kind() != VALUE_INT || right.get_kind() != VALUE_INT){
             EvaluationError::raise(root->get_loc(), "Value is not an int");
        }
        return Value(left.get_ival() > right.get_ival());
    } else if (rootTag == AST_GREATEROREQUAL) {
        Value left = evaluate(root->get_kid(0),current_block);
        Value right = evaluate(root->get_kid(1),current_block);
        if(left.get_kind() != VALUE_INT || right.get_kind() != VALUE_INT){
             EvaluationError::raise(root->get_loc(), "Value is not an int");
        }
        return Value(left.get_ival() >= right.get_ival());
    } else if (rootTag == AST_INT_LITERAL) {
        return Value(std::stoi(root->get_str()));
    } else if (rootTag == AST_VARREF) {
        return current_block->get_value_by_string(root);
    } else if(rootTag == AST_FNCALL){
            Value fn = recursive_execute(root, current_block);
   //         Value fn(current_block->get_value_by_string(root->get_kid(0)));
            return fn;
    } else if(rootTag == AST_STRING){
        std::string str = root->get_str();
        String * allocated_string = new String(str);
        return Value(allocated_string);

    } else {
        EvaluationError::raise(root->get_loc(), "Unexpected token");
    }
}
