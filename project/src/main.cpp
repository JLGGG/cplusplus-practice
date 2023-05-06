#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

//-----------------------
// Lexer
//-----------------------

// The lexer returns tokens [0-255] if it is an unknown character, otherwise one of these for known things.
enum class Token {
    tok_eof = -1,

    // commands
    tok_def = -2,
    tok_extern = -3,

    // primary
    tok_identifier = -4,
    tok_number = -5
}

static std::string identifier_str; // Filled in if tok_identifier
static double num_val; // Filled in if tok_number

// getTok: Return the next token from standard input.
static int getTok() {
    static int last_char = ' ';

    // Skip any whitespace.
    while (isspace(last_char)) {
        last_char = getchar();
    }

    if (isalpha(last_char)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
        identifier_str = last_char;
        while (isalnum((last_char = getchar()))) {
            identifier_str += last_char;
        }

        if (identifier_str == "def") {
            return Token::tok_def;
        } else if (identifier_str == "extern") {
            return Token::tok_extern;
        }
        return tok_identifier;
    }

    if (isdigit(last_char) || last_char == '.') { // Number: [0-9.]+
        std::string num_str;
        do {
            num_str += last_char;
            last_char = getchar();
        } while (isdigit(last_char) || last_char == '.');

        num_val = strtod(num_str.c_str(), nullptr);
        return tok_number;
    }

    if (last_char == '#') {
        // Comment until end of line.
        do
            last_char = getchar();
        while (last_char != EOF && last_char != '\n' && last_char != '\r');

        if (last_char != EOF) {
            return getTok();
        }
    }

    // Check for end of file. Don't eat the EOF.
    if (last_char == EOF) {
        return tok_eof;
    }

    // Otherwise, just return the character as its ascii value.
    int this_char = last_char;
    last_char = getchar();
    return this_char;
}

//-----------------------
// Abstract Syntax Tree (aka Parse Tree)
//-----------------------

namespace {
    // ExprAST - Base class for all expression nodes.
    class ExprAST {
    public:
        virtual ~ExprAST() = default;
    };

    // NumberExprAST - Expression class for numeric literals like "1.0".
    class NumberExprAST : public ExprAST {
    private:
        double val_;
    public:
        NumberExprAST(double val) : val_{val} {}
    };

    // VariableExprAST - Expression class for referencing a variable, like "a".
    class VariableExprAST : public ExprAST {
    private:
        std::string name_;
    public:
        VariableExprAST(std::string const& name) : name_{name} {}
    };

    // BinaryExprAST - Expression class for a binary operator.
    class BinaryExprAST : public ExprAST {
    private:
        char op_;
        std::unique_ptr<ExprAST> lhs_, rhs_;
    public:
        BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
            : op_{op}, lhs_{std::move(lhs)}, rhs_{std::move(rhs)} {}
    };

    // CallExprAST - Expression class for function calls.
    class CallExprAST : public ExprAST {
    private:
        std::string callee_;
        std::vector<std::unique_ptr<ExprAST>> args_;

    public:
        CallExprAST(std::string callee, std::vector<std::unique_ptr<ExprAST>> args)
            : callee_{callee}, args_{std::move(args)} {}
    };

    // PrototypeAST - This class represents the "prototype" for a function,
    // which captures its name, and its argument names (thus implicitly the number of arguments the function takes).
    class PrototypeAST {
    private:
        std::string name_;
        std::vector<std::string> args_;

    public:
        PrototypeAST(std::string name, std::vector<std::string> args)
            : name_{name}, args_{std::move(args)} {}

        std::string const& getName() const { return name_; }
    };
    
    // FunctionAST - This class represents a function definition itself.
    class FunctionAST {
    private:
        std::unique_ptr<PrototypeAST> proto_;
        std::unique_ptr<ExprAST> body_;

    public:
        FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body)
            : proto_{std::move(proto)}, body_{std::move(body)} {}
    };
} // end anonymous namespace

//-----------------------
// Parser
//-----------------------

// CurTok/getNextToken - Provide a simple token buffer. CurTok is the current token the parser is looking at.
// getNextToken reads another token from the lexer and updates CurTok with its results.
static int cur_tok;
static int getNextToken() { return cur_tok = getTok();}

// BinopPrecedence - This holds the precedence for each binary operator that is defined.
static std::map<char, int> binop_precedence;

// GetTokPrecedence - Get the precedence of the pending binary operator token.
static int getTokPrecedence() {
    if (!isascii(cur_tok)) {
        return -1;
    }

    // Make sure it's a declared binop.
    int tok_prec = binop_precedence[cur_tok];
    if (tok_prec <= 0) {
        return -1;
    }
    return tok_prec;
}

// LogError* - These are little helper functions for error handling.
std::unique_ptr<ExprAST> logError(const char* str) {
    fmt::print(stderr, "Error: {}\n", str);
    return nullptr;
}
std::unique_ptr<PrototypeAST> logErrorP(const char* str) {
    logError(str);
    return nullptr;
}

static std::unique_ptr<ExprAST> parseExpression();

// numberexpr ::= number
static std::unique_ptr<ExprAST> parseNumberExpr() {
    auto result = std::make_unique<NumberExprAST>(num_val);
    getNextToken(); // consume the number
    return std::move(result);
}

// parenexpr ::= '(' expression ')'
static std::unique_ptr<ExprAST> parseParenExpr() {
    getNextToken(); // consume '('
    auto v = parseExpression();
    if (!v) {
        return nullptr;
    }

    if (cur_tok != ')') {
        return logError("expected ')'");
    }
    getNextToken(); // consume ')'
    return v;
}

// identifierexpr
// ::= identifier
// ::= identifier '(' expression* ')'
static std::unique_ptr<ExprAST> parseIdentifierExpr() {
    std::string id_name = identifier_str;

    getNextToken(); // consume identifier

    if (cur_tok != '(') { // Simple variable ref.
        return std::make_unique<VariableExprAST>(id_name);
    }

    // Call.
    getNextToken(); // consume '('
    std::vector<std::unique_ptr<ExprAST>> args;
    if (cur_tok != ')') {
        while (true) {
            if (auto arg = parseExpression()) {
                args.push_back(std::move(arg));
            } else {
                return nullptr;
            }

            if (cur_tok == ')') {
                break;
            }

            if (cur_tok != ',') {
                return logError("Expected ')' or ',' in argument list");
            }
            getNextToken();
        }
    }

    // Eat the ')'.
    getNextToken();

    return std::make_unique<CallExprAST>(id_name, std::move(args));
}

// primary
// ::= identifierexpr
// ::= numberexpr
// ::= parenexpr
static std::unique_ptr<ExprAST> parsePrimary() {
    switch (cur_tok) {
    default:
        return logError("unknown token when expecting an expression");
    case tok_identifier:
        return parseIdentifierExpr();
    case tok_number:
        return parseNumberExpr();
    case '(':
        return parseParenExpr();
    }
}

// binoprhs
// ::= ('+' primary)*
static std::unique_ptr<ExprAST> parseBinOpRHS(int expr_prec, std::unique_ptr<ExprAST> lhs) {
    // If this is a binop, find its precedence.
    while (true) {
        int tok_prec = getTokPrecedence();

        // If this is a binop that binds at least as tightly as the current binop,
        // consume it, otherwise we are done.
        if (tok_prec < expr_prec) {
            return lhs;
        }

        // Okay, we know this is a binop.
        int binop = cur_tok;
        getNextToken(); // consume binop

        // Parse the primary expression after the binary operator.
        auto rhs = parsePrimary();
        if (!rhs) {
            return nullptr;
        }

        // If BinOp binds less tightly with RHS than the operator after RHS,
        // let the pending operator take RHS as its LHS.
        int next_prec = getTokPrecedence();
        if (tok_prec < next_prec) {
            rhs = parseBinOpRHS(tok_prec + 1, std::move(rhs));
            if (!rhs) {
                return nullptr;
            }
        }

        // Merge LHS/RHS.
        lhs = std::make_unique<BinaryExprAST>(binop, std::move(lhs), std::move(rhs));
    }
}

// expression
// ::= primary binoprhs
//
static std::unique_ptr<ExprAST> parseExpression() {
    auto lhs = parsePrimary();
    if (!lhs) {
        return nullptr;
    }

    return parseBinOpRHS(0, std::move(lhs));
}

// prototype
// ::= id '(' id* ')'
static std::unique_ptr<PrototypeAST> parsePrototype() {
    if (cur_tok != tok_identifier) {
        return logErrorP("Expected function name in prototype");
    }

    std::string fn_name = identifier_str;
    getNextToken();

    if (cur_tok != '(') {
        return logErrorP("Expected '(' in prototype");
    }

    std::vector<std::string> arg_names;
    while (getNextToken() == tok_identifier) {
        arg_names.push_back(identifier_str);
    }
    if (cur_tok != ')') {
        return logErrorP("Expected ')' in prototype");
    }

    // success.
    getNextToken(); // eat ')'.

    return std::make_unique<PrototypeAST>(fn_name, std::move(arg_names));
}

// definition ::= 'def' prototype expression
static std::unique_ptr<FunctionAST> parseDefinition() {
    getNextToken(); // eat def.
    auto proto = parsePrototype();
    if (!proto) {
        return nullptr;
    }

    if (auto e = parseExpression()) {
        return std::make_unique<FunctionAST>(std::move(proto), std::move(e));
    }
    return nullptr;
}

// toplevelexpr ::= expression
static std::unique_ptr<FunctionAST> parseTopLevelExpr() {
    if (auto e = parseExpression()) {
        // Make an anonymous proto.
        auto proto = std::make_unique<PrototypeAST>("__anon_expr", std::vector<std::string>());
        return std::make_unique<FunctionAST>(std::move(proto), std::move(e));
    }
    return nullptr;
}

// external ::= 'extern' prototype
static std::unique_ptr<PrototypeAST> parseExtern() {
    getNextToken(); // eat extern.
    return parsePrototype();
}

//-----------------------
// Top-Level parsing
// ----------------------

static void handleDefinition() {
    if (parseDefinition()) {
        fmt::print(stderr, "Parsed a function definition.\n");
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void handleExtern() {
    if (parseExtern()) {
        fmt::print(stderr, "Parsed an extern.\n");
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void handleTopLevelExpression() {
    // Evaluate a top-level expression into an anonymous function.
    if (parseTopLevelExpr()) {
        fmt::print(stderr, "Parsed a top-level expr.\n");
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

// top ::= definition | external | expression | ';'
static void mainLoop() {
    while (true) {
        fmt::print("ready> ");
        switch (cur_tok) {
        case tok_eof:
            return;
        case ';': // ignore top-level semicolons.
            getNextToken();
            break;
        case tok_def:
            handleDefinition();
            break;
        case tok_extern:
            handleExtern();
            break;
        default:
            handleTopLevelExpression();
            break;
        }
    }
}

//-----------------------
// Main driver code.
//-----------------------

int main() {
    // Install standard binary operators.
    // 1 is lowest precedence.
    binop_precedence['<'] = 10;
    binop_precedence['+'] = 20;
    binop_precedence['-'] = 20;
    binop_precedence['*'] = 40; // highest.

    // Prime the first token.
    fmt::print("ready> ");
    getNextToken();

    // Run the main "interpreter loop" now.
    mainLoop();

    return 0;
}