#ifndef CALCCORE_FA993
#define CALCCORE_FA993

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>

#include "calcentity.cpp"

class CalcFunctionData;

class CalcPrimaryFunctionData;

class CalcText : public CalcData {

protected:
    
    std::string value;
    
public:
    
    CalcText(std::string &name) {
        this->value = name;
    }
    
    std::string& get_value() {
        return this->value;
    }
    
    void to_string(std::ostringstream &buffer) {
        buffer << value;
    }
    
    CalcData* clone() {
        return new CalcText(this->value);
    }
    
    CalcNodeType get_node_type() {
        return TEXT;
    }
    
};

enum CalcSymbolType {
    
    PLUS,
    MINUS,
    ASTERISK,
    SLASH,
    MODULUS,
    CARET,
    PARENTHESISOPEN,
    PARENTHESISCLOSE,
    COMMA,
    EQUALS
    
};

class CalcSymbol: public CalcData
{
    std::string name;
    CalcSymbolType type;
    
public:
    
    CalcSymbol(std::string name, CalcSymbolType symType) {
        this->name = name;
        this->type = symType;
    }
    
    void to_string(std::ostringstream &buffer) {
        buffer << name;
    }
    
    std::string& get_name() {
        return this->name;
    }
    
    CalcSymbolType get_type() {
        return this->type;
    }
    
    CalcData* clone() {
        //no clone support for this
        return this;
    }
    
    CalcNodeType get_node_type() {
        return SYMBOL;
    }
    
    bool operator ==  (const CalcSymbol &c2) {
        return this->type == c2.type;
    }
    
    bool operator ==  (const CalcSymbolType &c2) {
        return this->type == c2;
    }
    
};

char from_symbol(CalcSymbolType type)
{
    switch (type)
    {
    case PLUS:
        return '+';
    case MINUS:
        return '-';
    case ASTERISK:
        return '*';
    case SLASH:
        return '/';
    case MODULUS:
        return '%';
    case CARET:
        return '^';
    case PARENTHESISOPEN:
        return '(';
    case PARENTHESISCLOSE:
        return ')';
    case COMMA:
        return ',';
    case EQUALS:
        return '=';
    default:
        throw "Unrecognized symbol";
    }
}

CalcSymbol* PLUS_SYMBOL = new CalcSymbol(std::string("plus"), PLUS);
CalcSymbol* MINUS_SYMBOL = new CalcSymbol(std::string("minus"), MINUS);
CalcSymbol* ASTERISK_SYMBOL = new CalcSymbol(std::string("asterisk"), ASTERISK);
CalcSymbol* SLASH_SYMBOL = new CalcSymbol(std::string("slash"), SLASH);
CalcSymbol* MODULUS_SYMBOL = new CalcSymbol(std::string("modulus"), MODULUS);
CalcSymbol* CARET_SYMBOL = new CalcSymbol(std::string("caret"), CARET);
CalcSymbol* PARENTHESISOPEN_SYMBOL = new CalcSymbol(std::string("parenthesis open"), PARENTHESISOPEN);
CalcSymbol* PARENTHESISCLOSE_SYMBOL = new CalcSymbol(std::string("parenthesis close"), PARENTHESISCLOSE);
CalcSymbol* COMMA_SYMBOL = new CalcSymbol(std::string("comma"), COMMA);
CalcSymbol* EQUALS_SYMBOL = new CalcSymbol(std::string("equals"), EQUALS);

class CalcSecondaryFunctionData;

class CalcNode
{

    CalcNode *prev = nullptr;
    CalcNode *next = nullptr;
    CalcData *data;
    bool parsed = false;

public:
    CalcNode()
    {
    }

    CalcNode(CalcSymbol* symType)
    {
        data = symType;
    }

    CalcNode(CalcValue* numData)
    {
        data = numData;
    }

    CalcNode(std::string &text)
    {
        data = new CalcText(text);
    }

    CalcNode(CalcSecondaryFunctionData *dat);

    CalcNode(CalcPrimaryFunctionData *dat);

    CalcNode *clone()
    {
        CalcNode *cl = new CalcNode();
        cl->parsed = this->parsed;
        cl->data = this->data->clone();
        return cl;
    }

    void to_string(std::ostringstream &buffer)
    {
        CalcNode *cn = this;
        cn->get_data()->to_string(buffer);
    }

    void to_debug_string(std::ostringstream &buffer) {
        CalcNode *cn = this;
        this->to_string(buffer);
        if(cn->next != nullptr){
            buffer << "---";
            cn->next->to_debug_string(buffer);
        }
    }

    void push_node(CalcNode *nextNode)
    {
        this->next = nextNode;
        nextNode->prev = this;
    }

    bool is_symbol(CalcSymbol *symType)
    {
        return this->get_type() == SYMBOL && static_cast<CalcSymbol *>(this->data)->get_type() == symType->get_type();
    }

    CalcNodeType get_type() {
        return this->data->get_node_type();
    }

    CalcData *get_data()
    {
        return this->data;
    }

    void set_data(CalcData *newData)
    {
        this->data = newData;
    }

    void set_data(CalcSymbol* newData)
    {
        this->data = newData;
    }

    CalcNode *get_next()
    {
        if (this->parsed)
        {
            throw "Node has been Disconnected";
        }
        return this->next;
    }

    void set_next(CalcNode *newNext)
    {
        if (this->parsed)
        {
            throw "Node has been Disconnected";
        }
        this->next = newNext;
    }

    CalcNode *get_prev()
    {
        if (this->parsed)
        {
            throw "Node has been Disconnected";
        }
        return this->prev;
    }

    void set_prev(CalcNode *newPrev)
    {
        if (this->parsed)
        {
            throw "Node has been Disconnected";
        }
        this->prev = newPrev;
    }

    void disconnect()
    {
        this->parsed = true;
        this->prev = nullptr;
        this->next = nullptr;
    }

    ~CalcNode()
    {
    }
};

class CalcFunctionData : public CalcData {
    
public:

    virtual CalcData* get_arg(size_t index) = 0;
    
    virtual void set_arg(size_t index, CalcData* newArg) = 0;
    
    virtual size_t get_arg_size() = 0;
    
};

class CalcPrimaryFunctionData : public CalcFunctionData
{

protected:
    
    bool wrapWithBrackets = false;
    
public:
    
    CalcPrimaryFunctionData() {
        this->wrapWithBrackets = false;
    }
    
    CalcNodeType get_node_type() {
        return PRIMARY_FUNCTION;
    }
    
    virtual bool is_inverse(const std::string &name)
    {
        return false;
    }
    
    virtual CalcData *evaluate() = 0;
    
    virtual void push_arg(CalcData* arg) = 0;

    virtual ~CalcPrimaryFunctionData() {
        
    }
    
    void set_wrap_brackets(bool wrap) {
        this->wrapWithBrackets = wrap;
    }
    
    bool get_wrap_brackets() {
        return this->wrapWithBrackets;
    }

};

class CalcSecondaryFunctionData : public CalcFunctionData
{

    std::string name;
    std::vector<CalcData *> args;
    bool wrapWithBrackets = false;

public:
    CalcSecondaryFunctionData(std::string &name)
    {
        this->name = name;
        this->wrapWithBrackets = false;
    }
    
    CalcNodeType get_node_type() {
        return SECONDARY_FUNCTION;
    }

    void push_arg(CalcData *nd)
    {
        this->args.push_back(nd);
    }

    std::vector<CalcData *> &get_args()
    {
        return this->args;
    }
    
    void set_args(std::vector<CalcData*>& args){
        this->args = args;
    }
    
    size_t get_arg_size() {
        return this->args.size();
    }
    
    void set_arg(size_t index, CalcData* newArg) {
        this->args[index] = newArg;
    }
    
    CalcData* get_arg(size_t index) {
        return this->args[index];
    }
    
    std::string& get_name() {
        return this->name;
    }
    
    void set_wrap_brackets(bool wrap) {
        this->wrapWithBrackets = wrap;
    }
    
    bool get_wrap_brackets() {
        return this->wrapWithBrackets;
    }

    CalcData *clone()
    {
        throw "Unsupported as of now";
    }

    void to_string(std::ostringstream &buffer)
    {
        buffer << this->get_name();
        buffer << '(';
        bool first = true;
        for (std::vector<CalcData *>::iterator it1 = args.begin(); it1 != args.end(); it1++)
        {
            if (!first)
            {
                buffer << ", ";
            }
            else
            {
                first = false;
            }
            CalcData *cn = *it1;
            cn->to_string(buffer);
        }
        buffer << ") ";
    }

    ~CalcSecondaryFunctionData()
    {
        for (std::vector<CalcData *>::iterator it = args.begin(); it != args.end(); ++it)
        {
            delete *it;
        }
        std::vector<CalcData *>().swap(args);
    }
};

CalcNode::CalcNode(CalcSecondaryFunctionData *dat)
{
    data = dat;
    parsed = false;
}

CalcNode::CalcNode(CalcPrimaryFunctionData *dat)
{
    data = dat;
    parsed = true;
}

#endif
