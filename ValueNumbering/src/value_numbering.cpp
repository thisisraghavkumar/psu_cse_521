#include<exception>
#include<fstream>
#include<iostream>
#include<map>
#include<regex>
#include<string>
#include<vector>

#define VALPREFIX "vAlUeNuM"

using namespace std;

// A struct to access elements of a statement of type `res := op1 op op2 // comment`
struct StmtElements {
    string res="";
    string op="";
    string op1="";
    string op2="";
    string comment="";
};

// Print a `StmtElements` struct
ostream & operator << (ostream & outs, const StmtElements & stmt){
    return outs << "Result: "<< stmt.res<<" := "<<"Op1: "<<stmt.op1<<" Op: "<<stmt.op<<" Op2: "<<stmt.op2;
}

// Convert a `StmtElements` struct into a well formatted string
string get_statement(StmtElements s){
    string t("");
    t = t + s.res + " := " + s.op1 + " " + s.op + (s.op == ""?"":" ") + s.op2 + (s.op2 == ""?"":" ") + (s.comment == "" ? "" : " // " + s.comment);
    return t;
}

/// @brief Read the lines given in a program
/// @param filename relative path of the file to read
/// @param lines vector to which the lines read from the file will be appended
void read_file_into_vector(string filename, vector<string> &lines){
    fstream source_file(filename, source_file.in);
    if(!source_file.is_open()){
        cout<<"Error in opening file "<<filename<<"\n";
        throw new exception();
    }
    string t;
    while (getline(source_file, t))
    {
        if(t == ""){
            continue;
        }
        lines.push_back(t);
    }
    source_file.close();
}

/// @brief Write the elements of a vector of strings into a file
/// @param filename the filename to write to
/// @param lines the vector of statements to add
void write_vector_into_file(string filename, vector<string> &lines){
    fstream dest_file(filename, dest_file.out);
    if (!dest_file.is_open()){
        cout<<"Error in opening file "<<filename<<"\n";
        throw new exception();
    }
    string t;
    for(auto line: lines){
        dest_file<<line<<"\n";
    }
    dest_file.close();
}

/// @brief Extracts the `StmtElements` struct from a statement
/// @param stmt the statement
/// @return a `StmtElements` object
StmtElements get_elements(string stmt, regex &stmtpat){
    smatch m;
    bool found = regex_search(stmt, m, stmtpat);
    if(!found){
        cout<<"Pattern not found in statement "<<stmt<<"\n";
        return StmtElements();
    }
    StmtElements s;
    s.res = m[1];
    s.op1 = m[3];
    s.op = m[4];
    s.op2 = m[5];
    return s;
}

/// @brief Get a new value number to assign as a temporary
/// @return A new value name
string new_value_name(){
    static int lastValueNumber = 0;
    lastValueNumber++;
    return string(VALPREFIX)+to_string(lastValueNumber);
}

/// @brief Get the temporary value name for any index
/// @param i index for which to get value name
/// @return a string containing the value name for that index
string get_value_name(int i){
    return string(VALPREFIX)+to_string(i);
}

/// @brief Get the expression key for a statement. If the operation is commutative
/// then the lexicographically smaller operand will appear first
/// @param stmt The statement containing the expression
/// @return The string key of the passed statement
string get_exp_key(StmtElements stmt){
    if(stmt.op == "+" || stmt.op == "*"){
        return min(stmt.op1,stmt.op2) + stmt.op + max(stmt.op1,stmt.op2);
    }
    return stmt.op1 + stmt.op + stmt.op2;
}

/// @brief Returns the list of dependent expression keys that are dependent on operand.
/// @param dependent_expressions a mapping from an operand name to a vector of expression keys
/// @param op the operand for which dependent expressions are needed
/// @return a vector of keys corresponding to expressions that depend on `op`. Returns an empty vector if `op` is not found in `dependent_expressions`.
vector<string> get_dependent_expressions(unordered_map<string, vector<string > > dependent_expressions, string op){
    if(dependent_expressions.find(op) == dependent_expressions.end()){
        return vector<string> ();
    }
    return dependent_expressions[op];
}

/// @brief Utility function to print all pairs of a mapping
/// @param vmap the map to print
void print_map(unordered_map<string, string> &vmap){
    cout<<"----------------------------------------\n";
    for(auto entry: vmap){
        cout<<entry.first<<" -> "<<entry.second<<"\n";
    }
}

int main(int argc, char **argv){
    cout<<"Reading file "<<argv[1]<<"\n";
    vector<string> lines;
    vector<StmtElements> statements;
    vector<StmtElements> newStatements;
    vector<string> outputLines;
    string outputFileName = string(argv[1]);
    outputFileName += "_vno";
    
    // each statement will look like res := a + b
    regex stmtpat("([a-zA-Z_]+)\\s*(:=)\\s*([a-zA-Z_0-9]+)\\s*([-+/\\*])\\s*([a-zA-Z_0-9]+)\\s*");
    
    // value map will store the latest value name that is given to an expression
    unordered_map<string, string> value_map;

    // dependent_expression store the mapping of operands to the key of expressions that depend on them and are not killed
    unordered_map<string, vector<string > > dependent_expressions;

    // used_map store the mapping of value names that are used on the right side of some statement
    unordered_map<string, bool> used_map;

    read_file_into_vector(argv[1], lines);
    StmtElements current;

    // 1. read statement,
    // 2. replace operands by their value name
    // 3. compute the key of expression on the right (thus rexpr)
    // 4. if a the key exists in value map, replace the expression on right with the value number
    // 5. else give a new value number to rexpr, add it to value map, then swap the value name of the operands with the operands
    // 6. kill the value numbers that depend on the result of current expression
    for(auto line: lines){
        current = get_elements(line, stmtpat);
        StmtElements currentCopy, newCopy;

        // store a copy of current statement
        currentCopy.res = current.res;
        currentCopy.op1 = current.op1;
        currentCopy.op = current.op;
        currentCopy.op2 = current.op2;
        newCopy.res = "";

        // replace operands with their value number if they exist, else add them
        if (value_map.find(current.op1)!=value_map.end()){
            current.op1 = value_map[current.op1];
        }else{
            value_map.insert_or_assign(current.op1, current.op1);
        }
        if (value_map.find(current.op2)!=value_map.end()){
            current.op2 = value_map[current.op2];
        }else{
            value_map.insert_or_assign(current.op2, current.op2);
        }

        // calculate the right expression after replacing operands with their mapped value
        string rexpr = get_exp_key(current);

        // if the expression on the right side is present in value map, then replace it with value name and mark the value name as used
        if(value_map.find(rexpr) != value_map.end()){
            current.op1 = value_map[rexpr];
            current.op = "";
            current.op2 = "";
            used_map.insert_or_assign(value_map[rexpr], true);
            current.comment = "Replaced "+rexpr+" with "+value_map[rexpr];
        }else{
            // if the expression on the right is not present then add it to value map with a new value name and make the expression dependent on the operands
            value_map.insert_or_assign(rexpr, new_value_name());
            vector<string> t = get_dependent_expressions(dependent_expressions, current.op1);
            t.push_back(rexpr);
            dependent_expressions.insert_or_assign(current.op1, t);
            t = get_dependent_expressions(dependent_expressions, current.op2);
            t.push_back(rexpr);
            dependent_expressions.insert_or_assign(current.op2, t);
            
            // construct a statement that copies the result of current statement to the new temporary known by value number
            newCopy.res = value_map[rexpr];
            newCopy.op1 = current.res;
            newCopy.op = "";
            newCopy.op2 = "";

            // restore the current statement's operands
            current = currentCopy;
        }

        //kill all the expressions that include the result as operand and are live
        vector<string> dependent_exprs = get_dependent_expressions(dependent_expressions, current.res);
        
        // set the value number of the result as the value number associated with the key of right expression
        value_map.insert_or_assign(current.res, value_map[rexpr]);
        
        // remove all the keys in value map that depend on the result and then erase the record of dependent expressions
        for(auto expr: dependent_exprs){
           value_map.erase(expr);
        }
        dependent_expressions.erase(current.res);
        
        // add current statement to the program
        statements.push_back(current);

        // add the copy statement to the program
        if(newCopy.res != ""){
            statements.push_back(newCopy);
        }
    }
    for(auto stmt: statements){
        //skip operations that copy results into value numbers that are not used
        if (stmt.res.find(VALPREFIX) != -1 && !used_map[stmt.res]){
            continue;
        }
        cout<<get_statement(stmt)<<"\n";
        newStatements.push_back(stmt);
    }
    for(auto stmt: newStatements){
        outputLines.push_back(get_statement(stmt));
    }
    write_vector_into_file(outputFileName, outputLines);
}
