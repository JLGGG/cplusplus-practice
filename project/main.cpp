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