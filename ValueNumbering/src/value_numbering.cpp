#include<exception>
#include<fstream>
#include<iostream>
#include<map>
#include<regex>
#include<string>
#include<vector>

#define VALPREFIX "vAlUeNuM"

using namespace std;

struct StmtElements {
    string res="";
    string op="";
    string op1="";
    string op2="";
    string comment="";
};

ostream & operator << (ostream & outs, const StmtElements & stmt){
    return outs << "Result: "<< stmt.res<<" := "<<"Op1: "<<stmt.op1<<" Op: "<<stmt.op<<" Op2: "<<stmt.op2;
}

string get_statement(StmtElements s){
    string t("");
    t = t + s.res + ":=" + s.op1 + s.op + s.op2 + (s.comment == "" ? "" : " // " + s.comment);
    return t;
}

/// @brief Read the lines given in a program
/// @param filename relative path of the sorcecode of the file to read
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
        lines.push_back(t);
    }
    source_file.close();
}

/// @brief Extracts the result, operand 1, operation, operand 2 from a statement of type x := y + z
/// @param stmt the line containing the statement
/// @return a StmtElements object
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

/// @brief Get the expression key for a statement. If the operation is commutative the lexicographically smaller operand will appear first
/// @param stmt The statement containing the expression
/// @return The string key of the passed statement
string get_exp_key(StmtElements stmt){
    if(stmt.op == "+" || stmt.op == "*"){
        return min(stmt.op1,stmt.op2) + stmt.op + max(stmt.op1,stmt.op2);
    }
    return stmt.op1 + stmt.op + stmt.op2;
}

vector<string> get_dependent_expressions(unordered_map<string, vector<string > > dependent_expressions, string op){
    if(dependent_expressions.find(op) == dependent_expressions.end()){
        return vector<string> ();
    }
    return dependent_expressions[op];
}

int main(int argc, char **argv){
    cout<<"Reading file "<<argv[1]<<"\n";
    vector<string> lines;
    vector<StmtElements> statements;
    vector<StmtElements> newStatements;
    regex stmtpat("([a-zA-Z_0-9]+)\\s*(:=)\\s*([a-zA-Z_0-9]+)\\s*([-+/\\*])\\s*([a-zA-Z_0-9]+)\\s*");
    // value map will store the latest value name that is given to an expression
    unordered_map<string, string> value_map;
    // dependent_expression store the mapping of operands to the live expressions that depend on them
    unordered_map<string, vector<string > > dependent_expressions;
    // used_map store the mapping of value names that are used on the right side of some statement
    unordered_map<string, bool> used_map;
    read_file_into_vector(argv[1], lines);
    StmtElements current;
    // pass 1: read all lines, construct the live expressions, their value name, replace live expressions, add copy operations
    for(auto line: lines){
        current = get_elements(line, stmtpat);
        cout<<current<<"\n";
        string rexpr = get_exp_key(current);
        StmtElements newCopy;
        newCopy.res = "";
        // if the expression on the right side is present in value map, then replace it with value name
        if(value_map.find(rexpr) != value_map.end()){
            current.op1 = value_map[rexpr];
            current.op = "";
            current.op2 = "";
            used_map.emplace(value_map[rexpr], true);
            current.comment = "Replaced "+rexpr+" with "+value_map[rexpr];
        }else{
            // if it is not present then add it to value map with a new value name and make the expression dependent on the operands
            value_map.emplace(rexpr, new_value_name());
            vector<string> t = get_dependent_expressions(dependent_expressions, current.op1);
            t.push_back(rexpr);
            dependent_expressions.emplace(current.op1, t);
            t = get_dependent_expressions(dependent_expressions, current.op2);
            t.push_back(rexpr);
            dependent_expressions.emplace(current.op2, t);
            newCopy.res = value_map[rexpr];
            newCopy.op1 = current.res;
            newCopy.op = "";
            newCopy.op2 = "";
        }
        //kill all the expressions that include the result as operand and are live
        vector<string> dependent_exprs = get_dependent_expressions(dependent_expressions, current.res);
        for(auto expr: dependent_exprs){
            value_map.erase(expr);
        }
        dependent_expressions.erase(current.res);
        cout<<"Pushing back current: "<<current<<"\n";
        statements.push_back(current);
        if(newCopy.res != ""){
            statements.push_back(newCopy);
        }
    }

    for(auto stmt: statements){
        //skip valuenumber copies that are not used
        if (stmt.res.find(VALPREFIX) != -1 && !used_map[stmt.res]){
            cout<<"Skipping "<<get_statement(stmt)<<"\n";
            continue;
        }
        cout<<get_statement(stmt)<<"\n";
        newStatements.push_back(stmt);
    }
    for(auto stmt: newStatements){
        cout<<get_statement(stmt)<<"\n";
    }
}
