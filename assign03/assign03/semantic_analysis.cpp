#include <cassert>
#include <algorithm>
#include <utility>
#include <map>
#include "grammar_symbols.h"
#include "parse.tab.h"
#include "node.h"
#include "ast.h"
#include "exceptions.h"
#include "semantic_analysis.h"
#include <iostream>
#include <set>
#include <stack>

SemanticAnalysis::SemanticAnalysis()
  : m_global_symtab(new SymbolTable(nullptr)) {
  m_cur_symtab = m_global_symtab;
}

SemanticAnalysis::~SemanticAnalysis() {
}

void SemanticAnalysis::visit_struct_type(Node *n) {
  if(n->get_tag() != AST_STRUCT_TYPE){
    SemanticError::raise(n->get_loc(), "struct type error: not a struct");
  }
  Symbol * def = m_cur_symtab->lookup_recursive("struct " + n->get_kid(0)->get_str());
  if(def == nullptr){
    SemanticError::raise(n->get_loc(), "struct type error: struct type does not exist");
  }
n->set_type(def->get_type());
}

void SemanticAnalysis::visit_union_type(Node *n) {
  RuntimeError::raise("union types aren't supported");
}

void SemanticAnalysis::visit_variable_declaration(Node *n) {
 
visit(n->get_kid(1));

SymbolKind kind = SymbolKind::VARIABLE;
TypeQualifier qual;
bool is_signed = false;
  std::shared_ptr<Type> type = n->get_kid(1)->get_type();
  std::string name;
  std::shared_ptr<Type> parent_function = m_cur_symtab->get_fn_type();
 for(unsigned i = 0; i < n->get_kid(2)->get_num_kids(); i++){
  //for each we need to update
   std::shared_ptr<Type> type = n->get_kid(1)->get_type();
    Node * nonIdent = n->get_kid(2)->get_kid(i);
    name = returnToken(nonIdent);
     Symbol* def = m_cur_symtab->lookup_local(name);
    if(def){
      SemanticError::raise(n->get_loc(), "variable declaration error: local variable already declared");
    }
    while(nonIdent->get_kid(0)->get_tag() != TOK_IDENT) {
          if(nonIdent->get_tag() == AST_ARRAY_DECLARATOR){
      if(parent_function){
           for(unsigned p =0 ; p < parent_function->get_num_members(); p++) {
        if(parent_function->get_member(p).get_name() == name){
          SemanticError::raise(n->get_loc(), "variable declaration error: parameter variable already exists");
        }
      }
        }
     
        std::string size = returnSize(n->get_kid(2)->get_kid(i));
        std::shared_ptr<ArrayType> arr(new ArrayType(type, std::stoi(size)));
        type = arr;
    } else if(nonIdent->get_tag() == AST_POINTER_DECLARATOR){
     if(parent_function){  
           for(unsigned p =0 ; p <parent_function->get_num_members(); p++) {
        if(parent_function->get_member(p).get_name() == name){
          SemanticError::raise(n->get_loc(), "variable declaration error: variable already exists");
        }
      }
        }
      std::shared_ptr<PointerType> point(new PointerType(type));
      type = point;
    } else if(nonIdent->get_tag() == AST_NAMED_DECLARATOR){
      
      if(parent_function){
        for(unsigned p =0 ; p < m_cur_symtab->get_fn_type()->get_num_members(); p++) {
         if(m_cur_symtab->get_fn_type()->get_member(p).get_name() == name){
          SemanticError::raise(n->get_loc(), "variable declaration error: variable already exists");
        }
      }
        }
     
      if(n->get_kid(1)->get_type()->is_void()){
        SemanticError::raise(n->get_loc(), "variable declaration error: non pointer void variables are not allowed");
      }
    } else{
        SemanticError::raise(n->get_loc(), "incorrect declarator list");
    }
      nonIdent = nonIdent->get_kid(0);
    }
     m_cur_symtab->declare(kind, name, type);
  
  }

   n->set_type(n->get_kid(1)->get_type());
}

void SemanticAnalysis::visit_basic_type(Node *n) {
  bool isVoid = false;
  bool isSigned = false;
  bool isUnsigned = false;
  bool isShort = false;
  bool isInt = false;
  bool isLong = false;
  bool isConst = false;
  bool isVoltatile = false;
  bool isChar = false;

  if(n->has_type()){
    return;
  }

//first initialize the list 
  std::vector<Node *> kids;
  for(unsigned i = 0; i < n->get_num_kids(); i++){
    kids.push_back(n->get_kid(i));
  }

  for(unsigned i = 0; i < n->get_num_kids(); i++){
    if(kids[i]->get_tag() == TOK_CONST && isConst == false){
        isConst = true;
        kids.erase(kids.begin() + i);
    } else if(kids[i]->get_tag() == TOK_VOLATILE && isVoltatile == false){
        isVoltatile = true;
        kids.erase(kids.begin() + i);
    } else if((kids[i]->get_tag() == TOK_CONST && isConst == true) || (kids[i]->get_tag() == TOK_VOLATILE && isVoltatile == false)){
      SemanticError::raise(n->get_loc(), "too many qualified types");
    }
  }

//now we determine the layout
for(unsigned i =0; i < kids.size(); i++){
    if(kids[i]->get_tag() == TOK_VOID && !isVoid){
    isVoid = true;
  }
    else if(kids[i]->get_tag() == TOK_SIGNED && !isSigned){
        isSigned = true;
    } else if(kids[i]->get_tag() == TOK_UNSIGNED && !isUnsigned){
        isUnsigned = true;
    } else if(kids[i]->get_tag() == TOK_CHAR && !isChar){
        isChar = true;
    } else if(kids[i]->get_tag() == TOK_SHORT && !isShort){
        isShort=true;
    } else if(kids[i]->get_tag() == TOK_LONG && !isLong) {
        isLong = true;
    } else if(kids[i]->get_tag() == TOK_INT && !isInt){
      isInt = true;
    } else {
       SemanticError::raise(n->get_loc(), "invalid typing");
    }
  
}
  
 
std::shared_ptr<BasicType> type = create_basic_type(isSigned, isUnsigned, isVoid, isInt, isChar, isShort, isLong);

if(type == nullptr) {
  SemanticError::raise(n->get_loc(), "basic type error: invalid typing");
}
TypeQualifier qual;
  if(isConst || isVoltatile){
    if(isVoid){
      SemanticError::raise(n->get_loc(), "basic type error: cannot add const or volatile to void");
    }
    if(isConst){
      qual = TypeQualifier::CONST;
      std::shared_ptr<QualifiedType> qualified(new QualifiedType(type, qual));
      n->set_type(qualified);
    } else {
      qual = TypeQualifier::VOLATILE;
      std::shared_ptr<QualifiedType> qualified(new QualifiedType(type, qual));
      n->set_type(qualified);
    }
  } else {
    n->set_type(type);
  }
}

void SemanticAnalysis::visit_function_definition(Node *n) {

  //first check if it has been defined already

  Symbol * def = m_cur_symtab->lookup_recursive(n->get_kid(1)->get_str());
  if(def != nullptr && def->is_defined()){
    SemanticError::raise(n->get_loc(), "function definiton error: function already defined");
  }
  if(def == nullptr) {
    visit_basic_type(n->get_kid(0));
    std::shared_ptr<Type> func(new FunctionType(n->get_kid(0)->get_type()));
    SymbolKind kind = SymbolKind::FUNCTION;
    m_cur_symtab->define(kind, n->get_kid(1)->get_str(), func);
    enter_scope();
         m_cur_symtab->set_fn_type(func);
      visit_children(n);
  } else {
      enter_scope();
      m_cur_symtab->set_fn_type(def->get_type());
      visit_children(n);
  }
  leave_scope();

}

void SemanticAnalysis::visit_function_declaration(Node *n) {

   if(m_cur_symtab->has_symbol_local(n->get_kid(1)->get_str())){
    SemanticError::raise(n->get_loc(), "function already has been declared");
  }
    visit_basic_type(n->get_kid(0));
    std::shared_ptr<Type> func(new FunctionType(n->get_kid(0)->get_type()));
    SymbolKind kind = SymbolKind::FUNCTION;
     m_cur_symtab->declare(kind, n->get_kid(1)->get_str(), func);
    enter_scope();

    m_cur_symtab->set_fn_type(func);
    for(unsigned i = 0; i < n->get_kid(2)->get_num_kids(); i++){
      visit(n->get_kid(2)->get_kid(i));
    }
    leave_scope();
   
}

void SemanticAnalysis::visit_function_parameter(Node *n) {
  std::shared_ptr<Type> parent_function = m_cur_symtab->get_fn_type();
  Symbol * defined = m_cur_symtab->lookup_recursive(n->get_kid(1)->get_kid(0)->get_str());
  if(defined) {
    SemanticError::raise(n->get_loc(), "function parameter error: cannot add duplicate parameter");
  }
  
  visit(n->get_kid(0));

SymbolKind kind = SymbolKind::VARIABLE;
TypeQualifier qual;
bool is_signed = false;
  std::shared_ptr<Type> type = n->get_kid(0)->get_type();
    std::string name = returnToken(n->get_kid(1)->get_kid(0));
    Symbol* def = m_cur_symtab->lookup_local(name);
    if(def){
      SemanticError::raise(n->get_loc(), "function parameter error: local variable already declared");
    }
    if(n->get_kid(1)->get_tag() == AST_ARRAY_DECLARATOR){
      if(parent_function){
           for(unsigned p =0 ; p < parent_function->get_num_members(); p++) {
        if(parent_function->get_member(p).get_name() == n->get_kid(1)->get_kid(0)->get_kid(0)->get_kid(0)->get_str()){
          SemanticError::raise(n->get_loc(), "function parameter error: parameter variable already exists");
        }
      }
        }
     
        if(type->is_signed()){
          is_signed = true;
        }
        std::string size = n->get_kid(1)->get_kid(1)->get_str();
        std::shared_ptr<ArrayType> arr(new ArrayType(type, std::stoi(size)));
        std::string name = returnToken(n->get_kid(1));
        m_cur_symtab->declare(kind, name, arr);
        n->set_type(arr);
         Member * mem = new Member(name,arr);
        parent_function->add_member(*mem);
    } else if(n->get_kid(1)->get_tag() == AST_POINTER_DECLARATOR){
     if(parent_function){  
           for(unsigned p =0 ; p <parent_function->get_num_members(); p++) {
        if(parent_function->get_member(p).get_name() == name){
          SemanticError::raise(n->get_loc(), "function parameter error: variable already exists");
        }
      }
        }
      std::shared_ptr<PointerType> point(new PointerType(type));
      m_cur_symtab->declare(kind, name , point);
      n->set_type(point);
     Member * mem = new Member(name,point);
      parent_function->add_member(*mem);
    } else if(n->get_kid(1)->get_tag() == AST_NAMED_DECLARATOR){
      
      if(parent_function){
        for(unsigned p =0; p < m_cur_symtab->get_fn_type()->get_num_members(); p++) {
         if(m_cur_symtab->get_fn_type()->get_member(p).get_name() == name){
          SemanticError::raise(n->get_loc(), "function parameter error: variable already exists");
        }
      }
        }
     
      if(n->get_kid(0)->get_type()->is_void()){
        SemanticError::raise(n->get_loc(), "function parameter error: non pointer void variables are not allowed");
      }
      
        m_cur_symtab->define(kind, name, type);
        n->set_type(type);
        Member * mem = new Member(name,type);
        parent_function->add_member(*mem);
    } else{
        SemanticError::raise(n->get_loc(), "incorrect declarator list");
    }
 
}

void SemanticAnalysis::visit_statement_list(Node *n) {
  enter_scope();
  visit_children(n);
  leave_scope();
}

void SemanticAnalysis::visit_return_expression_statement(Node *n) {

  visit_children(n);
  std::shared_ptr<Type> parent_func = m_cur_symtab->get_fn_type();

  if(!n->get_kid(0)->get_type()->is_integral() || !parent_func->get_base_type()->is_integral()){
    if(!(n->get_kid(0)->get_type()->is_same(parent_func->get_base_type()->get_unqualified_type()))){
      SemanticError::raise(n->get_loc(), "return expression error: cannot return non integral types that do not match");
    }
  }

}

void SemanticAnalysis::visit_struct_type_definition(Node *n) {
  std::string name = n->get_kid(0)->get_str();
  Symbol * def = m_cur_symtab->lookup_local("struct " + name);
  if(def) {
    SemanticError::raise(n->get_loc(), "struct type definition error: cannot redfine");
  }
  std::shared_ptr<Type> struct_type(new StructType(name));
  SymbolKind type = SymbolKind::TYPE;
  m_cur_symtab->define(type, "struct " + name, struct_type);
  enter_scope();
  visit_children(n);

 //enter_scope();
   typedef std::vector<Symbol *>::const_iterator const_iterator;
   for(auto it = m_cur_symtab->cbegin(); it !=  m_cur_symtab->cend(); it++){
  std::string memName = (*it)->get_name();
      std::shared_ptr<Type> ty((*it)->get_type());
      Member * mem = new Member(memName, ty);
      struct_type->add_member(*mem);
   }

  leave_scope();

}

void SemanticAnalysis::visit_binary_expression(Node *n) {
   
  int expression = n->get_kid(0)->get_tag();
  //we need to get the left and right expression.
 visit(n->get_kid(1));
  visit(n->get_kid(2));
   if(n->get_kid(1)->get_type()->is_function() && n->get_kid(1)->get_tag() == AST_VARIABLE_REF){
              SemanticError::raise(n->get_loc(), "binary expression error: wrong function token");
          }
           if(n->get_kid(2)->get_type()->is_function() && n->get_kid(2)->get_tag() == AST_VARIABLE_REF){
              SemanticError::raise(n->get_loc(), "binary expression error: wrong function token");
          }
   if(expression == TOK_ASSIGN){     
      if(n->get_kid(1)->get_tag() == AST_LITERAL_VALUE || n->get_kid(1)->get_tag() == AST_BINARY_EXPRESSION){
        SemanticError::raise(n->get_loc(), "binary expression error, cannot assign to literal value");
      }

     if(n->get_kid(1)->get_type()->is_integral() && n->get_kid(2)->get_type()->is_integral()){
        if(n->get_kid(2)->get_type()->is_void()){
          SemanticError::raise(n->get_loc(), "binary expression error: cannot assign with void type");
        }
        
         if(!n->get_kid(1)->get_type()->is_const() && n->get_kid(2)->get_type()->get_unqualified_type()->is_const()){
        SemanticError::raise(n->get_loc(), "binary expression error: cannot remove a const qualifier");
     }
        if(!n->get_kid(1)->get_type()->is_volatile() && n->get_kid(2)->get_type()->get_unqualified_type()->is_volatile()){
          SemanticError::raise(n->get_loc(), "binary expression error: cannot remove a voltatile qualifier");
        }
     } else if(n->get_kid(1)->get_type()->is_integral() && !n->get_kid(2)->get_type()->is_integral()){
          if(n->get_kid(2)->get_type()->is_pointer() || n->get_kid(2)->get_type()->is_struct() || n->get_kid(2)->get_type()->is_array()){
             SemanticError::raise(n->get_loc(), "binary expression error: cannot assign non integral to integral");
           }
        if(!n->get_kid(2)->get_type()->is_function() && !n->get_kid(2)->get_type()->is_integral()){
          SemanticError::raise(n->get_loc(), "binary expression error: cannot assign non integral to integral");
        }
         if(n->get_kid(2)->get_type()->is_array() || n->get_kid(2)->get_type()->is_struct()){
               if(!n->get_kid(1)->get_type()->is_const() && n->get_kid(2)->get_type()->is_const()){
             SemanticError::raise(n->get_loc(), "binary expression error: cannot remove a const qualifier");
           }
        if(!n->get_kid(1)->get_type()->is_volatile() && n->get_kid(2)->get_type()->is_volatile()){
            SemanticError::raise(n->get_loc(), "binary expression error: cannot remove a volatile qualifier");
        }
          if(n->get_kid(2)->get_type()->is_void()){
          SemanticError::raise(n->get_loc(), "binary expression error: cannot assign with void type");
        }
        } else {
           if(!n->get_kid(1)->get_type()->is_const() && n->get_kid(2)->get_type()->get_base_type()->is_const()){
             SemanticError::raise(n->get_loc(), "binary expression error: cannot remove a const qualifier");
           }
        if(!n->get_kid(1)->get_type()->is_volatile() && n->get_kid(2)->get_type()->get_base_type()->is_volatile()){
            SemanticError::raise(n->get_loc(), "binary expression error: cannot remove a volatile qualifier");
        }
          if(n->get_kid(2)->get_type()->get_base_type()->is_void()){
          SemanticError::raise(n->get_loc(), "binary expression error: cannot assign with void type");
        }
        }
     } else if(!n->get_kid(1)->get_type()->is_integral() && !n->get_kid(2)->get_type()->is_integral()){
        if(n->get_kid(1)->get_type()->is_function()){
          SemanticError::raise(n->get_loc(), "binary expression error: l value cannot be a function");
        }
        if(n->get_kid(1)->get_type()->is_array()){
            SemanticError::raise(n->get_loc(), "binary expression error: cannot set array to pointer");
        }
        if(n->get_kid(1)->get_type()->is_struct()){
           SemanticError::raise(n->get_loc(), "binary expression error: cannot set array to pointer");
        }        
   
        if(n->get_kid(2)->get_type()->is_array() || n->get_kid(2)->get_type()->is_struct() ){
               if(!n->get_kid(1)->get_type()->get_base_type()->is_const() && n->get_kid(2)->get_type()->is_const()){
             SemanticError::raise(n->get_loc(), "binary expression error: cannot remove a const qualifier");
           }
        if(!n->get_kid(1)->get_type()->get_base_type()->is_volatile() && n->get_kid(2)->get_type()->is_volatile()){
            SemanticError::raise(n->get_loc(), "binary expression error: cannot remove a volatile qualifier");
        }
         if(n->get_kid(2)->get_type()->is_void()){
          SemanticError::raise(n->get_loc(), "binary expression error: cannot assign with void type");
        }
        
        } else {
           if(!n->get_kid(1)->get_type()->get_base_type()->is_const() && n->get_kid(2)->get_type()->get_base_type()->is_const()){
             SemanticError::raise(n->get_loc(), "binary expression error: cannot remove a const qualifier");
           }
        if(!n->get_kid(1)->get_type()->get_base_type()->is_volatile() && n->get_kid(2)->get_type()->get_base_type()->is_volatile()){
            SemanticError::raise(n->get_loc(), "binary expression error: cannot remove a volatile qualifier");
        }
         if(n->get_kid(2)->get_type()->get_base_type()->is_void()){
          SemanticError::raise(n->get_loc(), "binary expression error: cannot assign with void type");
        }
        } 
     } 
     else{
        SemanticError::raise(n->get_loc(), "binary expression error, invalid combination of tokens");
     }
    } else if(expression == TOK_LOGICAL_AND || expression == TOK_LOGICAL_OR){
          if(n->get_kid(1)->get_type()->is_void() ||  n->get_kid(2)->get_type()->is_void()) {
            SemanticError::raise(n->get_loc(), "binary expression error: neither operand can be void");
          }
          if(n->get_kid(1)->get_type()->is_array() || n->get_kid(1)->get_type()->is_struct() || n->get_kid(2)->get_type()->is_array() || n->get_kid(2)->get_type()->is_struct()){
            SemanticError::raise(n->get_loc(), "binary expression error: neither operand can be struct or array");
          }
         n->set_type(n->get_kid(1)->get_type());
    } else if(expression == TOK_EQUALITY){
          if((n->get_kid(1)->get_type()->is_integral() && !n->get_kid(2)->get_type()->is_integral()) || (!n->get_kid(1)->get_type()->is_integral() && n->get_kid(2)->get_type()->is_integral())){
            SemanticError::raise(n->get_loc(), "binary expression error: both must be integral types");
          }

          if((!n->get_kid(1)->get_type()->is_integral() && !n->get_kid(2)->get_type()->is_integral()) && !(n->get_kid(1)->get_type()->is_same(n->get_kid(2)->get_type()->get_unqualified_type())) ){
            SemanticError::raise(n->get_loc(), "binary expression error: since both types are non integral, they must be equal");
          }
           n->set_type(n->get_kid(1)->get_type());
    } else if(expression == TOK_MOD || expression == TOK_ASTERISK || expression == TOK_DIVIDE){
          isLessPrecise(n->get_kid(1), n->get_kid(2), n);
           if(n->get_kid(1)->get_type()->is_pointer() || n->get_kid(2)->get_type()->is_pointer()){
            SemanticError::raise(n->get_loc(), "binary expression error: cannot perform mod, multiply, or divide on pointer ");
          }
          
    } else if(expression == TOK_PLUS || expression == TOK_MINUS || expression == TOK_LT || expression == TOK_LTE || expression == TOK_GT || expression == TOK_GTE) {
          if((!n->get_kid(1)->get_type()->is_integral() && !n->get_kid(1)->get_type()->is_function())  || (!n->get_kid(2)->get_type()->is_integral() && !n->get_kid(2)->get_type()->is_function()) ){
            if(n->get_kid(1)->get_type()->is_pointer() && n->get_kid(2)->get_type()->is_pointer()){
              SemanticError::raise(n->get_loc(), "binary expression error: cannot do arithmetic on two pointers");
            }
          } 
         isLessPrecise(n->get_kid(1), n->get_kid(2), n);
    } else {
        SemanticError::raise(n->get_loc(), "binary expression error: invalid binary operator");
  }
}

void SemanticAnalysis::visit_unary_expression(Node *n) {
    visit(n->get_kid(1));
   if(n->get_kid(0)->get_tag() == TOK_NOT || n->get_kid(0)->get_tag() == TOK_MINUS || n->get_kid(0)->get_tag() == TOK_BITWISE_COMPL){
        if(n->get_kid(1)->get_tag() == AST_LITERAL_VALUE) {
          std::shared_ptr<Type> literal(new BasicType(BasicTypeKind::INT, true));
          n->set_type(literal);
        } else if(n->get_kid(1)->get_tag() == AST_VARIABLE_REF){
            if(!n->get_kid(1)->get_type()->is_integral()){
              SemanticError::raise(n->get_loc(), "unary expression error: not an integral");
            }
            n->set_type(n->get_kid(1)->get_type());
        } else {
          SemanticError::raise(n->get_loc(), "unary expression error: invalid unary term");
        }
    } else if(n->get_kid(0)->get_tag() == TOK_ASTERISK){
        if(!n->get_kid(1)->get_type()->is_pointer()){
          SemanticError::raise(n->get_loc(), "unary expression error: cannot dereference or reference a non pointer");
        }
         n->set_type(n->get_kid(1)->get_type()->get_base_type());
    } else if(n->get_kid(0)->get_tag() == TOK_AMPERSAND){
         if(n->get_kid(1)->get_tag() == AST_LITERAL_VALUE){
          SemanticError::raise(n->get_loc(), "unary expression error: cannot reference a literal value");
        }
         std::shared_ptr<Type> point(new PointerType(n->get_kid(1)->get_type()));
         n->set_type(point);
    }
    
    else {
      SemanticError::raise(n->get_loc(), "unary expression error: invalid unary operator");
    }

}

void SemanticAnalysis::visit_postfix_expression(Node *n) {
  // TODO: implement

  visit_children(n);
}

void SemanticAnalysis::visit_conditional_expression(Node *n) {
  // TODO: implement

  visit_children(n);
}

void SemanticAnalysis::visit_cast_expression(Node *n) {
  // TODO: implement
    Node *left = n->get_kid(1);

    if (left->get_type()->get_basic_type_kind() < BasicTypeKind::INT)
      n->set_kid(1, left = promote_to_int(left));

}

void SemanticAnalysis::visit_function_call_expression(Node *n) {
  // TODO: implement

  visit(n->get_kid(0));

  //first we want to make sure this function exists
  Symbol * func = m_cur_symtab->lookup_recursive(n->get_kid(0)->get_kid(0)->get_str());
  if(func == nullptr){
    SemanticError::raise(n->get_loc(), "Function call expression error: function does not exist");
  }
  //get the list of parameters for the function
  unsigned num_member = n->get_kid(0)->get_type()->get_num_members();
  if(num_member !=  n->get_kid(1)->get_num_kids()){
    SemanticError::raise(n->get_loc(), "function call error: number of parameters do not match");
  }
  //now we want to make sure the types line up
  for(unsigned i =0; i < n->get_kid(1)->get_num_kids(); i++){
    visit(n->get_kid(1)->get_kid(i));
    if(n->get_kid(1)->get_kid(i)->get_type()->is_integral() && func->get_type()->get_member(i).get_type()->is_integral()){
         if(!(n->get_kid(1)->get_kid(i)->get_type()->is_same(func->get_type()->get_member(i).get_type()->get_unqualified_type()))){
      SemanticError::raise(n->get_loc(), "Function call expression error: parameter types do not line up");
    }
    } else if(!n->get_kid(1)->get_kid(i)->get_type()->is_integral() && !func->get_type()->get_member(i).get_type()->is_integral()){
       if(!(n->get_kid(1)->get_kid(i)->get_type()->get_base_type()->get_unqualified_type()->is_same(func->get_type()->get_member(i).get_type()->get_base_type()->get_unqualified_type()))){
      SemanticError::raise(n->get_loc(), "Function call expression error: parameter types do not line up");
    }
    } else {
       SemanticError::raise(n->get_loc(), "Function call expression error: parameter types do not line up");
    }
   
  }
  //assuming all is good with parameters, we set the type of the node to the return type 
  n->set_type(n->get_kid(0)->get_type());

}

void SemanticAnalysis::visit_field_ref_expression(Node *n) {
  // TODO: implement
  //first check if this is a struct
  visit(n->get_kid(0));
  if(!(n->get_kid(0)->get_type()->is_struct())){
    SemanticError::raise(n->get_loc(), "field ref error: left side must be a struct with parameters");
  }
  if(n->get_kid(0)->get_type()->is_pointer()){
    SemanticError::raise(n->get_loc(), "field ref error: requires indirect pointer");
  }
  
 bool found = false;
 for(unsigned i =0; i < n->get_kid(0)->get_type()->get_num_members(); i++) {
      if(n->get_kid(0)->get_type()->get_member(i).get_name() == n->get_kid(1)->get_str()){
        found = true;
        n->set_type(n->get_kid(0)->get_type()->get_member(i).get_type());
      }
   }
   if(found == false){
    SemanticError::raise(n->get_loc(), "field ref error: member does not exist");
   }
}

void SemanticAnalysis::visit_indirect_field_ref_expression(Node *n) {
  // TODO: implement
  visit(n->get_kid(0));
  
  if(!(n->get_kid(0)->get_type()->is_pointer())){
    SemanticError::raise(n->get_loc(), "indirect field ref error: left side must be a struct with parameters");
  }  

  std::shared_ptr<Type> refType = n->get_kid(0)->get_type()->get_base_type()->find_member(n->get_kid(1)->get_str())->get_type();
  n->set_type(refType);

}

void SemanticAnalysis::visit_array_element_ref_expression(Node *n) {
  // TODO: implement

  visit(n->get_kid(0));
  visit(n->get_kid(1));
  std::shared_ptr<Type> right_base = n->get_kid(1)->get_type();
  BasicTypeKind right_type = right_base->get_basic_type_kind();
  if(right_type != BasicTypeKind::INT) {
    SemanticError::raise(n->get_loc(), "array elemtent ref: subscript must be ");
  }
  n->set_type(n->get_kid(0)->get_type()->get_base_type());
}

void SemanticAnalysis::visit_variable_ref(Node *n) {
  // TODO: implement
  Symbol * ref = m_cur_symtab->lookup_recursive(n->get_kid(0)->get_str());
  if(ref == nullptr){
    SemanticError::raise(n->get_loc(), "variable ref error: reference has not be declared or does not exist");
  }
  n->set_type(ref->get_type());
}

void SemanticAnalysis::visit_literal_value(Node *n) {
  // TODO: ADD SIGNED
  if(n->get_tag() != AST_LITERAL_VALUE) {
    SemanticError::raise(n->get_loc(), "literal value error: not a literal value");
  }
  if(n->get_kid(0)->get_tag() == TOK_INT_LIT){
      std::shared_ptr<Type> literal(new BasicType(BasicTypeKind::INT, true));
      n->set_type(literal);
  } else if(n->get_kid(0)->get_tag() == TOK_CHAR_LIT){
      std::shared_ptr<Type> literal(new BasicType(BasicTypeKind::CHAR, true));
      n->set_type(literal);
  } else if(n->get_kid(0)->get_tag() == TOK_STR_LIT) {
      std::shared_ptr<Type> literal(new BasicType(BasicTypeKind::CHAR, true));
      std::shared_ptr<Type> arr(new ArrayType(literal, n->get_kid(0)->get_str().size()));
      n->set_type(arr);
  } else {
     SemanticError::raise(n->get_loc(), "literal value error: not a literal value");
  }
}




std::shared_ptr<BasicType> SemanticAnalysis::create_basic_type(bool isSigned, bool isUnsigned, bool isVoid, bool isInt, bool isChar, bool isShort, bool isLong){
  std::shared_ptr<BasicType> type;
BasicTypeKind kind;
  if(isVoid && !isInt && !isChar && !isShort && !isLong && !isSigned && !isUnsigned) {
      kind = BasicTypeKind::VOID;
              std::shared_ptr<BasicType> example(new BasicType(kind, true));
              type = example;
              return type;
  }
  else if((isInt || ((isShort && !isLong) || (!isShort && isLong))) && !isChar && !isVoid){
      if(isLong && !isShort && (isSigned || !isUnsigned)){
             kind = BasicTypeKind::LONG;
            std::shared_ptr<BasicType> example(new BasicType(kind, true));
         type = example;
      } 
      else if(isLong  && isUnsigned && !isShort && !isSigned) {
                kind = BasicTypeKind::LONG;
            std::shared_ptr<BasicType> example(new BasicType(kind, false));
         type = example;
      } else if(isShort && !isLong && (isSigned || !isUnsigned)){
             kind = BasicTypeKind::SHORT;
std::shared_ptr<BasicType> example(new BasicType(kind, true));
         type = example;
      } else if(!isLong  && isUnsigned && isShort && !isSigned){
             kind = BasicTypeKind::SHORT;
std::shared_ptr<BasicType> example(new BasicType(kind, false));
         type = example;
        
      }
      else if(!isInt && isShort && !isLong && (isSigned || !isUnsigned)){
         kind = BasicTypeKind::SHORT;
std::shared_ptr<BasicType> example(new BasicType(kind, true));
         type = example;
      }
      else if(!isInt && isShort && !isLong && isUnsigned && !isSigned){
         kind = BasicTypeKind::SHORT;
std::shared_ptr<BasicType> example(new BasicType(kind, false));
         type = example;
      } 
      else if(isInt && !isShort && !isLong && (isSigned || !isUnsigned)){
                 kind = BasicTypeKind::INT;
            std::shared_ptr<BasicType> example(new BasicType(kind, true));
            type = example;
      } 
      else if(isInt && !isLong && !isShort && isUnsigned && !isSigned) {
              kind = BasicTypeKind::INT;
            std::shared_ptr<BasicType> example(new BasicType(kind, false));
            type = example;
      } 
      else {
          return type;
      }

  }
  else if(isChar && !isLong && !isShort && !isInt && !isVoid){
      if(isSigned || !isUnsigned) {
         kind = BasicTypeKind::CHAR;
              std::shared_ptr<BasicType> example(new BasicType(kind, true));
 type = example;
      } else{
         kind = BasicTypeKind::CHAR;
              std::shared_ptr<BasicType> example(new BasicType(kind, false));
 type = example;
      }
  } else if(isSigned && !isUnsigned && !isInt && !isChar && !isVoid && !isShort && !isLong){
        kind = BasicTypeKind::INT;
            std::shared_ptr<BasicType> example(new BasicType(kind, true));
            type = example;
  } else if (!isSigned && isUnsigned && !isInt && !isChar && !isVoid && !isShort && !isLong){
      kind = BasicTypeKind::INT;
            std::shared_ptr<BasicType> example(new BasicType(kind, false));
            type = example;
  }
  
  else{
      return type;
    }

}

std::shared_ptr<Type> SemanticAnalysis::evaluateExpression(Node * n) {
      if(n->get_tag() == TOK_INT_LIT || n->get_tag() == TOK_STR_LIT || n->get_tag() == TOK_CHAR_LIT || n->get_tag() == TOK_FP_LIT){
          if(n->get_tag() == TOK_INT_LIT || n->get_tag() == TOK_CHAR_LIT) {
            std::shared_ptr<Type> int_lit(new BasicType(BasicTypeKind::INT, false));
            return int_lit;
          } else if(n->get_tag() == TOK_STR_LIT){
            std::shared_ptr<Type> int_lit(new BasicType(BasicTypeKind::INT, false));
            std::shared_ptr<Type> arr_type(new ArrayType(int_lit, n->get_str().size()));
            return arr_type;
          } else if(n->get_tag() == TOK_FP_LIT) {
              std::shared_ptr<Type> fn_type(new FunctionType(m_cur_symtab->lookup_local(n->get_str())->get_type()));
              return fn_type;

          }
      } else if(n->get_tag() == TOK_IDENT){
          return m_cur_symtab->lookup_local(n->get_str())->get_type();
      } else if(n->get_tag() == AST_FUNCTION_CALL_EXPRESSION){
        return m_cur_symtab->lookup_recursive(n->get_kid(0)->get_kid(0)->get_str())->get_type();
      } else if(n->get_tag() == AST_BINARY_EXPRESSION) {
        if(n->get_kid(0)->get_tag() == TOK_ASSIGN) {
          SemanticError::raise(n->get_loc(), "can not have an assign in a nested binary expression");
        }
        std::shared_ptr<Type> l_part = evaluateExpression(n->get_kid(1));
        std::shared_ptr<Type> r_part = evaluateExpression(n->get_kid(2));

        return n->get_type();
      } else if(n->get_tag() == AST_UNARY_EXPRESSION) {

      }
      
      else {
          for(unsigned i =0; i < n->get_num_kids(); i++ ){
              return evaluateExpression(n->get_kid(i));
          }
      }
  }

void SemanticAnalysis::enter_scope() {
  SymbolTable *scope = new SymbolTable(m_cur_symtab);
  m_cur_symtab = scope;
}

std::shared_ptr<Type> SemanticAnalysis::getLvalue(Node *n) {
    if(n->get_tag() == TOK_IDENT){
      return m_cur_symtab->lookup_local(n->get_str())->get_type();
    } else if(n->get_tag() == AST_LITERAL_VALUE) {
      SemanticError::raise(n->get_loc(), "l value must be variable to assign to");
    }
    else{
      visit_children(n);
    }
}


void SemanticAnalysis::leave_scope() {
  m_cur_symtab = m_cur_symtab->get_parent();
  assert(m_cur_symtab != nullptr);
}

std::string SemanticAnalysis::returnToken(Node * n){
  std::stack<Node*> nodeStack;
    nodeStack.push(n);

    while (!nodeStack.empty()) {
        Node *current = nodeStack.top();
        nodeStack.pop();

        if (current->get_tag() == TOK_IDENT) {
            return current->get_str();
        }

        for (int i = current->get_num_kids() - 1; i >= 0; i--) {
            nodeStack.push(current->get_kid(i));
        }
    }
    return ""; 
}

std::string SemanticAnalysis::returnSize(Node *n){
  std::stack<Node*> nodeStack;
    nodeStack.push(n);

    while (!nodeStack.empty()) {
        Node *current = nodeStack.top();
        nodeStack.pop();

        if (current->get_tag() == TOK_INT_LIT) {
            return current->get_str();
        }

        for (int i = current->get_num_kids() - 1; i >= 0; i--) {
            nodeStack.push(current->get_kid(i));
        }
    }

    return ""; 
 
}

void SemanticAnalysis::isLessPrecise(Node * n1, Node * n2 , Node * setNode){ 

  bool isn1Signed = false;
  bool isn2Signed = false;
  if(n1->get_type()->is_integral()){
      if(n1->get_type()->is_signed()){
        isn1Signed = true;  
      }
  } else{
    if(n1->get_type()->get_base_type()->is_signed()){
        isn1Signed = true;  
      }
  }
  if(n2->get_type()->is_integral()){
      if(n2->get_type()->is_signed()){
        isn2Signed = true;  
      }
  } else{
    if(n2->get_type()->get_base_type()->is_signed()){
        isn2Signed = true;  
      }
  }
  std::shared_ptr<Type> type(n1->get_type());

  if(n2->get_type()->is_integral() && n2->get_type()->get_basic_type_kind() == BasicTypeKind::INT){
    if(n1->get_type()->is_integral() && (n1->get_type()->get_basic_type_kind() == BasicTypeKind::SHORT || n1->get_type()->get_basic_type_kind() == BasicTypeKind::CHAR)){
        type = n2->get_type();
    }
  } 
  //next check if one is generall less precise than the other
  if(determinePrecision(n1) > determinePrecision(n2) || determinePrecision(n2) > determinePrecision(n1)){
      if(determinePrecision(n1) > determinePrecision(n2)){
          if(isn1Signed && !isn2Signed){
             std::shared_ptr<Type> temp(new BasicType(n1->get_type()->get_basic_type_kind(), false));
              type = temp;
          } else {
               type = n1->get_type();
          }
      }
      if(determinePrecision(n2) > determinePrecision(n1)){
         if(!isn1Signed && isn2Signed){
             std::shared_ptr<Type> temp(new BasicType(n2->get_type()->get_basic_type_kind(), false));
              type = temp;
          } else {
               type = n2->get_type();
          }
      }
  }
   setNode->set_type(type);
}

int SemanticAnalysis::determinePrecision(Node * n){
  if(n->get_type()->is_integral()){
     if(n->get_type()->get_basic_type_kind() == BasicTypeKind::CHAR) {
      return 1;
    } else if(n->get_type()->get_basic_type_kind() == BasicTypeKind::SHORT){
      return 2;
    } else if(n->get_type()->get_basic_type_kind() == BasicTypeKind::INT){
      return 3;
    } else if(n->get_type()->get_basic_type_kind() == BasicTypeKind::LONG){
      return 4;
    } else {
      SemanticError::raise(n->get_loc(), "determine precision error: cannot compare or compute void type");
    }
  } else{
    if(n->get_type()->get_base_type()->get_basic_type_kind() == BasicTypeKind::CHAR) {
      return 1;
    } else if(n->get_type()->get_base_type()->get_basic_type_kind() == BasicTypeKind::SHORT){
      return 2;
    } else if(n->get_type()->get_base_type()->get_basic_type_kind() == BasicTypeKind::INT){
      return 3;
    } else if(n->get_type()->get_base_type()->get_basic_type_kind() == BasicTypeKind::LONG){
      return 4;
    } else {
      SemanticError::raise(n->get_loc(), "determine precision error: cannot compare or compute void type");
    }
    
  }
}


Node *SemanticAnalysis::promote_to_int(Node *n) {
  assert(n->get_type()->is_integral());
  assert(n->get_type()->get_basic_type_kind() < BasicTypeKind::INT);
  std::shared_ptr<Type> type(new BasicType(BasicTypeKind::INT, n->get_type()->is_signed()));
  return implicit_conversion(n, type);
}

Node *SemanticAnalysis::implicit_conversion(Node *n, const std::shared_ptr<Type> &type) {
  std::unique_ptr<Node> conversion(new Node(AST_IMPLICIT_CONVERSION, {n}));
  conversion->set_type(type);
  return conversion.release();
}
