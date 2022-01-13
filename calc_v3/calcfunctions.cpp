#ifndef CALCFUNCTIONS_FA993
#define CALCFUNCTIONS_FA993

#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <sstream>

#include "calccore.cpp"

CalcValue* ZERO = new CalcNumber(0);
CalcValue* ONE = new CalcNumber(1);

class CalcUserDefinedFuncton : public CalcPrimaryFunctionData
{
    
private:
    
    void visitor() {
        //do substitute
        expressionTree = pre_visitor(expressionTree);
    }
    
    CalcData* pre_visitor(CalcData* data){
        if(data->get_node_type() == TEXT) {
            std::string name = ((CalcText*)data)->get_value();
            for(size_t i = 0; i < this->params.size(); i++) {
                if (name == params[i]->get_value()) {
                    return this->args[i];
                }
            }
        } else {
            visitor_actual(data);
        }
        return data;
    }
    
    void visitor_actual(CalcData* toVisit) {
        if(toVisit->get_node_type() == SECONDARY_FUNCTION || toVisit->get_node_type() == PRIMARY_FUNCTION) {
            CalcFunctionData* funcData = ((CalcFunctionData*)toVisit);
            for(int j = 0; j < funcData->get_arg_size(); j++) {
                CalcData* dt = funcData->get_arg(j);
                if(dt->get_node_type() == SECONDARY_FUNCTION || dt->get_node_type() == TEXT) {
                    std::string name;
                    if(dt->get_node_type() == SECONDARY_FUNCTION) {
                        name = ((CalcSecondaryFunctionData*)dt)->get_name();
                    } else {
                        name = ((CalcText*)dt)->get_value();
                    }
                    for(size_t i = 0; i < this->params.size(); i++) {
                        if (name == params[i]->get_value()) {
                            funcData->set_arg(j, this->args[i]);
                            if(dt->get_node_type() == SECONDARY_FUNCTION) {
                                ((CalcSecondaryFunctionData*)funcData->get_arg(j))->set_args(((CalcSecondaryFunctionData*)dt)->get_args());
                            }
                        }
                    }
                }
                if(funcData->get_arg(j)->get_node_type() == SECONDARY_FUNCTION || funcData->get_arg(j)->get_node_type() == PRIMARY_FUNCTION) {
                    for(size_t k = 0; k < ((CalcFunctionData*)funcData->get_arg(j))->get_arg_size(); k++) {
                        ((CalcFunctionData*)funcData->get_arg(j))->set_arg(k, pre_visitor(((CalcFunctionData*)funcData->get_arg(j))->get_arg(k)));
                    }
                }
            }
        }
    }
    
protected:
    
    std::string name;
    CalcData* expressionTree;
    std::vector<CalcText*> params;
    
    std::vector<CalcData*> args;
    
public:
    
    CalcUserDefinedFuncton(std::string& name, CalcData* expressionTree, std::vector<CalcText*>& params) : CalcPrimaryFunctionData() {
        this->name = name;
        this->expressionTree = expressionTree;
        this->params = params;
    }
    
    CalcData *evaluate() {
        switch (expressionTree->get_node_type()) {
            case VALUE:
            case TEXT:
                return expressionTree;
            case PRIMARY_FUNCTION:
                return ((CalcPrimaryFunctionData*)expressionTree)->evaluate();
            default:
                throw "Unexpected";
        }
    }

    void push_arg(CalcData* arg) {
        if(this->args.size() == this->params.size()) {
            throw "Exceeded arg limit";
        }
        this->args.push_back(arg);
        if(this->args.size() == this->params.size()) {
            this->visitor();
        }
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
    
    void to_string(std::ostringstream& buffer) {
        buffer << name << '(';
        expressionTree->to_string(buffer);
        buffer << ')';
    }
    
    CalcFunctionData* clone_self() {
        CalcPrimaryFunctionData* dat = new CalcUserDefinedFuncton(name, expressionTree->clone(), params);
        dat->set_wrap_brackets(this->get_wrap_brackets());
        return dat;
    }

    
};

class CalcBinaryBusFunction : public CalcPrimaryFunctionData
{

protected:
    std::vector<CalcData *> args;

    virtual CalcValue* get_initial_value() = 0;

    virtual CalcValue* binary_eval(CalcValue* acc, CalcValue* arg) = 0;

    virtual CalcPrimaryFunctionData *create(std::vector<CalcData *> &newArgs) = 0;

    virtual const char get_connecting_symbol() = 0;

    virtual const std::string &get_name() = 0;

public:
    
    CalcBinaryBusFunction() : CalcPrimaryFunctionData() {
        
    }

    void push_arg(CalcData* arg) {
        this->args.push_back(arg);
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

    void to_string(std::ostringstream &buffer)
    {
        bool first = true;
        if(wrapWithBrackets) {
            buffer << '(';
        }
        for (std::vector<CalcData *>::iterator it1 = this->args.begin(); it1 != this->args.end(); it1++)
        {
            CalcData *cn = (*it1);
            if (first)
            {
                first = false;
            }
            else
            {
                buffer << ' ';
                if (cn->get_node_type() == PRIMARY_FUNCTION && ((CalcPrimaryFunctionData *)cn)->is_inverse(get_name()))
                {
                    
                } else {
                    buffer << get_connecting_symbol() << ' ';
                }
            }
            cn->to_string(buffer);
        }
        if(wrapWithBrackets) {
            buffer << ')';
        }
    }

    CalcData *evaluate()
    {
        auto acc = get_initial_value();
        std::vector<CalcData *> evaled;
        for (std::vector<CalcData *>::iterator it1 = this->args.begin(); it1 != this->args.end(); it1++)
        {
            CalcData *cn = *it1;
            if (cn->get_node_type() == VALUE)
            {
                acc = binary_eval(acc, (CalcValue *)cn);
            }
            else if (cn->get_node_type() == PRIMARY_FUNCTION)
            {
                cn = ((CalcPrimaryFunctionData *)cn)->evaluate();
                if (cn->get_node_type() == VALUE)
                {
                    acc = binary_eval(acc, (CalcValue *)cn);
                } else {
                    evaled.push_back(cn);
                }
            }
            else
            {
                evaled.push_back(cn->clone());
            }
        }
        if (evaled.size() == 0)
        {
            return acc;
        }
        else
        {
            if (!acc->equals(get_initial_value()))
            {
                evaled.push_back(acc);
            }
            return create(evaled);
        }
    }
    
    CalcFunctionData* clone_self() {
        CalcPrimaryFunctionData* dat = create(args);
        dat->set_wrap_brackets(this->get_wrap_brackets());
        return dat;
    }

    ~CalcBinaryBusFunction() {
        for (std::vector<CalcData *>::iterator it = args.begin(); it != args.end(); ++it)
        {
            delete *it;
        }
        std::vector<CalcData *>().swap(args);
    }

};

class CalcBinaryFunction : public CalcPrimaryFunctionData {
    
private:
    
    CalcData* resolveArgs(CalcData* arg) {
        if (arg->get_node_type() == PRIMARY_FUNCTION)
        {
            return ((CalcPrimaryFunctionData *)arg)->evaluate();
        }
        else
        {
            return arg;
        }
    }
    
    CalcValue* resolveStageTwo(CalcData* arg) {
        if(arg->get_node_type() == VALUE) {
            return (CalcValue*) arg;
        } else {
            return nullptr;
        }
    }
    
protected:
    
    CalcData *arg1 = nullptr;
    CalcData *arg2 = nullptr;

    virtual CalcValue* apply(CalcValue* arg1, CalcValue* arg2) = 0;

    virtual CalcPrimaryFunctionData *create(CalcData *arg1, CalcData* arg2) = 0;

    virtual const char get_connecting_symbol() = 0;
    
public:
    
    CalcBinaryFunction() : CalcPrimaryFunctionData() {
    }

    void push_arg(CalcData* arg) {
        if(this->arg1 == nullptr) {
            this->arg1 = arg;
        } else if(this->arg2 == nullptr) {
            this->arg2 = arg;
        } else {
            throw "Exceeding arguement cap during second stage parsing";
        }
    }
    
    size_t get_arg_size() {
        return 2;
    }
    
    void set_arg(size_t index, CalcData* newArg) {
        if(index == 0) {
            arg1 = newArg;
        } else if(index == 1) {
            arg2 = newArg;
        } else {
            throw "Index out of bounds";
        }
    }
    
    CalcData* get_arg(size_t index) {
        if(index == 0) {
            return arg1;
        } else if(index == 1) {
            return arg2;
        } else {
            throw "Index out of bounds";
        }
    }

    void to_string(std::ostringstream &buffer)
    {
        if(wrapWithBrackets) {
            buffer << '(';
        }
        arg1->to_string(buffer);
        buffer << ' ' << get_connecting_symbol() << ' ';
        arg2->to_string(buffer);
        if(wrapWithBrackets) {
            buffer << ')';
        }
    }

    CalcData *evaluate()
    {
        CalcData* newArg1 = resolveArgs(arg1);
        CalcData* newArg2 = resolveArgs(arg2);
        
        CalcValue* resArg1 = resolveStageTwo(newArg1);
        CalcValue* resArg2 = resolveStageTwo(newArg2);
        
        if(resArg1 == nullptr || resArg2 == nullptr ) {
            return create(newArg1, newArg2);
        } else {
            return apply(resArg1, resArg2);
        }
        
    }
    
    CalcFunctionData* clone_self() {
        CalcPrimaryFunctionData* dat = create(arg1, arg2);
        dat->set_wrap_brackets(this->get_wrap_brackets());
        return dat;
    }

    ~CalcBinaryFunction() {
        delete this->arg1;
        delete this->arg2;
    }
    
};

class CalcSymbolicUnaryFunction : public CalcPrimaryFunctionData
{

protected:
    CalcData *arg = nullptr;

    virtual CalcValue* apply(CalcValue* arg) = 0;

    virtual CalcPrimaryFunctionData *create(CalcData *arg) = 0;

    virtual const char get_connecting_symbol() = 0;

public:
    
    CalcSymbolicUnaryFunction() : CalcPrimaryFunctionData() {
    }

    void push_arg(CalcData* arg) {
        if(this->arg == nullptr) {
            this->arg = arg;
        } else {
            throw "Exceeding arguement cap during second stage parsing";
        }
    }
    
    size_t get_arg_size() {
        return 1;
    }
    
    void set_arg(size_t index, CalcData* newArg) {
        if(index == 0) {
            this->arg = newArg;
        } else {
            throw "Index out of bounds";
        }
    }
    
    CalcData* get_arg(size_t index) {
        if(index == 0) {
            return this->arg;
        } else {
            throw "Index out of bounds";
        }
    }

    void to_string(std::ostringstream &buffer)
    {
        if(wrapWithBrackets) {
            buffer << '(';
        }
        buffer << get_connecting_symbol() << ' ';
        arg->to_string(buffer);
        if(wrapWithBrackets) {
            buffer << ')';
        }
    }

    CalcData *evaluate()
    {
        if (this->arg->get_node_type() == VALUE)
        {
            return apply((CalcValue*) arg);
        }
        else if (this->arg->get_node_type() == PRIMARY_FUNCTION)
        {
            CalcData *cn = ((CalcPrimaryFunctionData *)this->arg)->evaluate();
            if (cn->get_node_type() == VALUE)
            {
                return apply((CalcValue*)cn);
            }
            else
            {
                return create(cn);
            }
        }
        else
        {
            return create(this->arg->clone());
        }
    }
    
    CalcFunctionData* clone_self() {
        CalcPrimaryFunctionData* dat = create(arg);
        dat->set_wrap_brackets(this->get_wrap_brackets());
        return dat;
    }

    ~CalcSymbolicUnaryFunction() {
        delete this->arg;
    }

};

class CalcUnaryFunction : public CalcPrimaryFunctionData
{

protected:
    CalcData *arg = nullptr;

    virtual CalcValue* apply(CalcValue* arg) = 0;

    virtual CalcPrimaryFunctionData *create(CalcData *arg) = 0;

    virtual const std::string &get_name() = 0;

public:
    
    CalcUnaryFunction() : CalcPrimaryFunctionData() {
    }

    void push_arg(CalcData* arg) {
        if(this->arg == nullptr) {
            this->arg = arg;
        } else {
            throw "Exceeding arguement cap during second stage parsing";
        }
    }
    
    size_t get_arg_size() {
        return 1;
    }
    
    void set_arg(size_t index, CalcData* newArg) {
        if(index == 0) {
            this->arg = newArg;
        } else {
            throw "Index out of bounds";
        }
    }
    
    CalcData* get_arg(size_t index) {
        if(index == 0) {
            return this->arg;
        } else {
            throw "Index out of bounds";
        }
    }

    void to_string(std::ostringstream &buffer)
    {
        if(wrapWithBrackets) {
            buffer << '(';
        }
        buffer << get_name() << '(';
        arg->to_string(buffer);
        buffer << ')';
        if(wrapWithBrackets) {
            buffer << ')';
        }
    }

    CalcData *evaluate()
    {
        if (this->arg->get_node_type() == VALUE)
        {
            return apply((CalcValue*) arg);
        }
        else if (this->arg->get_node_type() == PRIMARY_FUNCTION)
        {
            CalcData *cn = ((CalcPrimaryFunctionData *)this->arg)->evaluate();
            if (cn->get_node_type() == VALUE)
            {
                return apply((CalcValue*)cn);
            }
            else
            {
                return create(cn);
            }
        }
        else
        {
            return create(this->arg->clone());
        }
    }
    
    CalcFunctionData* clone_self() {
        CalcPrimaryFunctionData* dat = create(arg);
        dat->set_wrap_brackets(this->get_wrap_brackets());
        return dat;
    }

    ~CalcUnaryFunction() {
        delete this->arg;
    }

};

class CalcAddFunction : public CalcBinaryBusFunction
{

    static const std::string NAME;

public:
    CalcAddFunction() : CalcBinaryBusFunction()
    {
    }

    CalcAddFunction(std::vector<CalcData *> &args) : CalcBinaryBusFunction()
    {
        this->args = args;
    }

    CalcValue* get_initial_value()
    {
        return ZERO;
    }

    const char get_connecting_symbol()
    {
        return '+';
    }

    const std::string &get_name()
    {
        return NAME;
    }

    CalcPrimaryFunctionData *create(std::vector<CalcData *> &newArgs)
    {
        return new CalcAddFunction(newArgs);
    }

    CalcValue* binary_eval(CalcValue* acc, CalcValue* arg)
    {
        return acc->add(arg);
    }
};

class CalcMultiplyFunction : public CalcBinaryBusFunction
{

    static const std::string NAME;

public:
    CalcMultiplyFunction() : CalcBinaryBusFunction()
    {
    }

    CalcMultiplyFunction(std::vector<CalcData *> &args) : CalcBinaryBusFunction()
    {
        this->args = args;
    }

    CalcValue* get_initial_value()
    {
        return ONE;
    }

    const char get_connecting_symbol()
    {
        return '*';
    }

    const std::string &get_name()
    {
        return NAME;
    }

    CalcPrimaryFunctionData *create(std::vector<CalcData *> &newArgs)
    {
        return new CalcMultiplyFunction(newArgs);
    }

    CalcValue* binary_eval(CalcValue* acc, CalcValue* arg)
    {
        return acc->multiply(arg);
    }
};

class CalcModulusFunction: public CalcBinaryFunction {

public:
    CalcModulusFunction() : CalcBinaryFunction()
    {
    }

    CalcModulusFunction(CalcData* arg1, CalcData* arg2) : CalcBinaryFunction()
    {
        this->arg1 = arg1;
        this->arg2 = arg2;
    }

    const char get_connecting_symbol()
    {
        return '%';
    }

    CalcPrimaryFunctionData *create(CalcData* arg1, CalcData* arg2)
    {
        return new CalcModulusFunction(arg1, arg2);
    }

    CalcValue* apply(CalcValue* acc, CalcValue* arg)
    {
        return acc->mod(arg);
    }
    
};

class CalcPowerFunction: public CalcBinaryFunction {

public:
    CalcPowerFunction() : CalcBinaryFunction()
    {
    }

    CalcPowerFunction(CalcData* arg1, CalcData* arg2) : CalcBinaryFunction()
    {
        this->arg1 = arg1;
        this->arg2 = arg2;
    }

    const char get_connecting_symbol()
    {
        return '^';
    }

    CalcPrimaryFunctionData *create(CalcData* arg1, CalcData* arg2)
    {
        return new CalcPowerFunction(arg1, arg2);
    }

    CalcValue* apply(CalcValue* acc, CalcValue* arg)
    {
        return acc->pow(arg);
    }
    
};

class CalcNegateFunction : public CalcSymbolicUnaryFunction
{

public:
    
    bool is_inverse(const std::string &name)
    {
        return name == "add";
    }
    
    CalcNegateFunction() : CalcSymbolicUnaryFunction() {
        
    }

    CalcNegateFunction(CalcData *arg) : CalcSymbolicUnaryFunction()
    {
        this->arg = arg;
    }

    const char get_connecting_symbol()
    {
        return '-';
    }

    CalcValue* apply(CalcValue* arg)
    {
        return ZERO->subtract(arg);
    }

    CalcPrimaryFunctionData *create(CalcData *arg)
    {
        return new CalcNegateFunction(arg);
    }
    
};

class CalcInverseFunction : public CalcSymbolicUnaryFunction
{

public:
    
    CalcInverseFunction() : CalcSymbolicUnaryFunction()
    {
    }

    CalcInverseFunction(CalcData *arg) : CalcSymbolicUnaryFunction()
    {
        this->arg = arg;
    }
    
    bool is_inverse(const std::string &name)
    {
        return name == "multiply";
    }

    const char get_connecting_symbol()
    {
        return '/';
    }

    CalcValue* apply(CalcValue* arg)
    {
        return ONE->divide(arg);
    }

    CalcPrimaryFunctionData *create(CalcData *arg)
    {
        return new CalcInverseFunction(arg);
    }
    
};

class CalcSineFunction : public CalcUnaryFunction
{
    
    static const std::string NAME;

public:
    
    CalcSineFunction() : CalcUnaryFunction()
    {
    }

    CalcSineFunction(CalcData *arg) : CalcUnaryFunction()
    {
        this->arg = arg;
    }

    const std::string &get_name()
    {
        return NAME;
    }
    
    CalcValue* apply(CalcValue* arg)
    {
        return arg->sine();
    }

    CalcPrimaryFunctionData *create(CalcData *arg)
    {
        return new CalcSineFunction(arg);
    }
    
};

const std::string CalcAddFunction::NAME = std::string("add");
const std::string CalcMultiplyFunction::NAME = std::string("multiply");
const std::string CalcSineFunction::NAME = std::string("sin");

class RuleBook
{

    std::map<CalcSymbolType, std::string> symbolToFunction;
    std::map<std::string, size_t> argCap;
    std::map<std::string, CalcPrimaryFunctionData*> defaultFunctions;
    std::map<std::string, CalcPrimaryFunctionData*> registeredFunctions;

public:
    RuleBook()
    {
        this->symbolToFunction[PLUS] = "add";
        this->symbolToFunction[MINUS] = "negate";
        this->symbolToFunction[ASTERISK] = "multiply";
        this->symbolToFunction[SLASH] = "invert";
        this->symbolToFunction[MODULUS] = "mod";
        this->symbolToFunction[CARET] = "pow";

        this->argCap["add"] = -1;
        this->argCap["negate"] = 1;
        this->argCap["multiply"] = -1;
        this->argCap["invert"] = 1;
        this->argCap["mod"] = 2;
        this->argCap["pow"] = 2;
        this->argCap["sin"] = 1;
        
        this->defaultFunctions["add"] = new CalcAddFunction();
        this->defaultFunctions["negate"] = new CalcNegateFunction();
        this->defaultFunctions["multiply"] = new CalcMultiplyFunction();
        this->defaultFunctions["invert"] = new CalcInverseFunction();
        this->defaultFunctions["mod"] = new CalcModulusFunction();
        this->defaultFunctions["pow"] = new CalcPowerFunction();
        this->defaultFunctions["sin"] = new CalcSineFunction();
    }
    
    void put_function(std::string& key, CalcPrimaryFunctionData* val, size_t limit) {
        this->argCap[key] = limit;
        this->registeredFunctions[key] = val;
    }

    std::string &get_function(CalcSymbol *type)
    {
        return this->symbolToFunction.at(type->get_type());
    }

    size_t get_limit(std::string &name)
    {
        return this->argCap.at(name);
    }

    CalcPrimaryFunctionData *create_from(std::string &name)
    {
        std::map<std::string, CalcPrimaryFunctionData*>::iterator it1 = this->defaultFunctions.find(name);
        if(it1 != this->defaultFunctions.end()) {
            //found
            return (CalcPrimaryFunctionData* )it1->second->clone();
        } else {
            std::map<std::string, CalcPrimaryFunctionData*>::iterator it2 = this->registeredFunctions.find(name);
            if(it2 != this->registeredFunctions.end()) {
                return (CalcPrimaryFunctionData* )it2->second->clone();
            }
            throw "Function Not Found: " + name;
        }
    }

    CalcData *substitute(CalcData *nd)
    {
        switch (nd->get_node_type())
        {
        case SECONDARY_FUNCTION:
            {
                CalcSecondaryFunctionData *dat = (CalcSecondaryFunctionData *)nd;
                size_t limit = this->get_limit(dat->get_name());
                if (limit != -1 && dat->get_args().size() != limit)
                {
                    std::stringstream st;
                    st << "Incorrect number of arguements, Expected " << limit << "Found " << dat->get_args().size();
                    throw st.str();
                }
                //TODO
                CalcPrimaryFunctionData* dt = create_from(dat->get_name());
                dt->set_wrap_brackets(dat->get_wrap_brackets());
                for(std::vector<CalcData*>::iterator it1 = dat->get_args().begin(); it1 != dat->get_args().end(); it1++) {
                    CalcData* r = this->substitute(*it1);
                    dt->push_arg(r);
                }
                return dt;
            }
        case VALUE:
        case TEXT:
            //no changes
            return nd->clone();
        case PRIMARY_FUNCTION:
        case SYMBOL:
            throw "Incorrect node type at this stage";
        }
    }
    
    CalcData* pre_substitute(CalcData *nd) {
        switch (nd->get_node_type())
        {
        case SECONDARY_FUNCTION:
            {
                CalcSecondaryFunctionData *dat = (CalcSecondaryFunctionData *)nd;
                size_t limit = this->get_limit(dat->get_name());
                if (limit != -1 && dat->get_args().size() != limit)
                {
                    std::stringstream st;
                    st << "Incorrect number of arguements, Expected " << limit << "Found " << dat->get_args().size();
                    throw st.str();
                }
                //TODO
                try {
                    CalcPrimaryFunctionData* dt = create_from(dat->get_name());
                    dt->set_wrap_brackets(dat->get_wrap_brackets());
                    for(std::vector<CalcData*>::iterator it1 = dat->get_args().begin(); it1 != dat->get_args().end(); it1++) {
                        CalcData* r = this->pre_substitute(*it1);
                        dt->push_arg(r);
                    }
                    return dt;
                }  catch(const char* msg) {
                    for(int i = 0; i < dat->get_args().size(); i++) {
                        dat->get_args()[i] = this->pre_substitute(dat->get_args()[i]);
                    }
                    return nd;
                }
            }
        case VALUE:
        case TEXT:
            //no changes
            return nd->clone();
        case PRIMARY_FUNCTION:
        case SYMBOL:
            throw "Incorrect node type at this stage";
        }
    }
};


#endif
