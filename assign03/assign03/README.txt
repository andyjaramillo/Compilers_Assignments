JHED: ajarami6

Andreas Jaramillo

Interesting notes on implementation:
    
    Changing Array_Type::is_same():
        In order to check the first element types, I changed the is_same() functionality to check the base type rather
        than the size of the array since an array can be initialized to 0 as a parameter of a function and later an array with 
        a non-zero size can be put as that argument
    SemanticAnalysis::visit_struct_type_definition(Node *n):
        When adding members to the struct type, I first call visit_children(n) in order to add each member of the struct to the symbol
        table. Then rather than iterating over the tree again, I just iterate over the current symbol table since I entered the scope 
        in visit_struct_type_definition
    SemanticAnalysis::create_basic_type(bool isSigned, bool isUnsigned, bool isVoid, bool isInt, bool isChar, bool isShort, bool isLong):
        In order to process the non terminal list of basic types, I used a list of boolean values where I iterate over the types and 
        change the boolean value for each type that shows up. Then in the create_basic_type function, I create the type based on whuch boolean
        values are true and false.
    returnToken and returnSize:
        In order to find the TOK_INT_LIT or TOK_IDENT in case there is multiple children, I chose to implement a iterative approach rather
        than a recursive approach using a stack of Nodes. This was in order to find the name of the element at certain tags like AST_VARIABLE_DECLARATION
    Determine promotion for implicit casting:
        The determine promotion function is called anytime there is a operation that can cause implicit casting in binary expressions. In order to determine
        whether to promote, determinePrecision(Node * n) is supposed to give a value, ranking the type based on precision.



    
