#ifndef CALCENTITY_FA993
#define CALCENTITY_FA993

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <cmath>

enum CalcNodeType
{

    VALUE,
    TEXT,
    SYMBOL,
    SECONDARY_FUNCTION,
    PRIMARY_FUNCTION,

};

class CalcData {
    
public:
    
    virtual void to_string(std::ostringstream &buffer) = 0;
    
    virtual CalcData* clone() = 0;
    
    virtual CalcNodeType get_node_type() = 0;
    
    virtual ~CalcData() {
        
    }
};

enum CalcNumberType {
    
    NUMBER = 0,
//    COMPLEX = 1,
//    MATRIX = 2
    
};

enum CalcOperationType {
    
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    MOD,
    POWER,
    
};

class CalcNumber;

class CalcValue : public CalcData {
  
public:
    
    CalcNodeType get_node_type() {
        return VALUE;
    }
    
    virtual CalcNumberType get_number_type() = 0;
    
    virtual CalcValue* add(CalcValue* c1) = 0;
    
    virtual CalcValue* subtract(CalcValue* c1) = 0;
    
    virtual CalcValue* multiply(CalcValue* c1) = 0;
    
    virtual CalcValue* divide(CalcValue* c1) = 0;
    
    virtual CalcValue* mod(CalcValue* c1) = 0;
    
    virtual CalcValue* pow(CalcValue* c1) = 0;
    
    virtual CalcValue* sine() = 0;
    
    virtual bool equals(CalcValue* c1) = 0;
    
    virtual ~CalcValue() {
        
    }
    
};

class CalcNumber : public CalcValue {
  
    double value;
    
public:
    
    CalcNumber(double val) {
        this->value = val;
    }
    
    double get_value() {
        return this->value;
    }
    
    void set_value(double newVal) {
        this->value = newVal;
    }
    
    CalcNumberType get_number_type() {
        return NUMBER;
    }
    
    CalcValue* add(CalcValue* c1);
    
    CalcValue* subtract(CalcValue* c1);
    
    CalcValue* multiply(CalcValue* c1);
    
    CalcValue* divide(CalcValue* c1);
    
    CalcValue* mod(CalcValue* c1);
    
    CalcValue* pow(CalcValue* c1);
    
    CalcValue* sine() {
        return new CalcNumber(sin(this->value));
    }
    
    bool equals(CalcValue* c1){
        return c1->get_number_type() == NUMBER && ((CalcNumber*)c1)->get_value() == this->value;
    }
    
    void to_string(std::ostringstream &buffer) {
        buffer << this->value;
    }
    
    CalcValue* clone() {
        return new CalcNumber(this->value);
    }
    
};

CalcValue* num_num_add(CalcNumber* c1, CalcNumber* c2) {
    return new CalcNumber(c1->get_value() + c2->get_value());
}

CalcValue* num_num_subtract(CalcNumber* c1, CalcNumber* c2) {
    return new CalcNumber(c1->get_value() - c2->get_value());
}

CalcValue* num_num_multiply(CalcNumber* c1, CalcNumber* c2) {
    return new CalcNumber(c1->get_value() * c2->get_value());
}

CalcValue* num_num_divide(CalcNumber* c1, CalcNumber* c2) {
    return new CalcNumber(c1->get_value() / c2->get_value());
}

CalcValue* num_num_modulus(CalcNumber* c1, CalcNumber* c2) {
    return new CalcNumber((long)c1->get_value() % (long)c2->get_value());
}

CalcValue* num_num_power(CalcNumber* c1, CalcNumber* c2) {
    
    return new CalcNumber(pow(c1->get_value(), c2->get_value()));
}

CalcValue* resolve(CalcValue* c1, CalcValue* c2, CalcOperationType type) {
    switch (c1->get_number_type()) {
        case NUMBER:
            switch (c2->get_number_type()) {
                case NUMBER:
                    switch (type) {
                        case ADD:
                            return num_num_add((CalcNumber*)c1, (CalcNumber*)c2);
                        case SUBTRACT:
                            return num_num_subtract((CalcNumber*)c1, (CalcNumber*)c2);
                        case MULTIPLY:
                            return num_num_multiply((CalcNumber*)c1, (CalcNumber*)c2);
                        case DIVIDE:
                            return num_num_divide((CalcNumber*)c1, (CalcNumber*)c2);
                        case MOD:
                            return num_num_modulus((CalcNumber*)c1, (CalcNumber*)c2);
                        case POWER:
                            return num_num_power((CalcNumber*)c1, (CalcNumber*)c2);
                    }
            }
            break;
    }
}


CalcValue* CalcNumber::add(CalcValue* c1){
    return resolve(this, c1, ADD);
}

CalcValue* CalcNumber::subtract(CalcValue* c1) {
    return resolve(this, c1, SUBTRACT);
}

CalcValue* CalcNumber::multiply(CalcValue* c1) {
    return resolve(this, c1, MULTIPLY);
}

CalcValue* CalcNumber::divide(CalcValue* c1) {
    return resolve(this, c1, DIVIDE);
}

CalcValue* CalcNumber::mod(CalcValue* c1) {
    return resolve(this, c1, MOD);
}

CalcValue* CalcNumber::pow(CalcValue* c1) {
    return resolve(this, c1, POWER);
}

#endif
