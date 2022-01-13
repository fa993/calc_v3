#ifndef CALCULATOR_FA993
#define CALCULATOR_FA993

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>

#include "calcfunctions.cpp"

bool is_space(char x)
{
    return std::isspace(x);
}

void trim(std::string &input)
{
    std::string::iterator it1 = std::find_if_not(input.begin(), input.end(), &is_space);
    input.erase(input.begin(), it1);
    std::string::reverse_iterator it2 = std::find_if_not(input.rbegin(), input.rend(), &is_space);
    input.erase(it2.base(), input.end());
}

CalcNode *to_node(char x)
{
    switch (x)
    {
    case '+':
        return new CalcNode(PLUS_SYMBOL);
    case '-':
        return new CalcNode(MINUS_SYMBOL);
    case '*':
        return new CalcNode(ASTERISK_SYMBOL);
    case '/':
        return new CalcNode(SLASH_SYMBOL);
    case '%':
        return new CalcNode(MODULUS_SYMBOL);
    case '^':
        return new CalcNode(CARET_SYMBOL);
    case '(':
        return new CalcNode(PARENTHESISOPEN_SYMBOL);
    case ')':
        return new CalcNode(PARENTHESISCLOSE_SYMBOL);
    case ',':
        return new CalcNode(COMMA_SYMBOL);
    case '=':
        return new CalcNode(EQUALS_SYMBOL);
    default:
        throw std::string("Unrecognized symbol") + x;
    }
}


CalcNode *to_node(std::string &input)
{
    //first check if input is number then check if input is text and is function
    trim(input);
    //check if there is whitespace in string
    if(input.size() == 0) {
        return nullptr;
    }
    bool contains_space = std::find_if(input.begin(), input.end(), &is_space) != input.end();
    if (contains_space)
    {
        throw std::string("Stray Whitespace: ") + input;
    }
    char *endptr;
    double d = std::strtod(input.c_str(), &endptr);
    if (endptr == input.c_str() || *endptr != 0)
    {
        //failed
        //assume input is text and proceed
        return new CalcNode(input);
    }
    else
    {
        //passed
        return new CalcNode(new CalcNumber(d));
    }
}

CalcNode *parse_to_list(std::string &input)
{
    std::string buffer = "";
    CalcNode *head = new CalcNode();
    CalcNode *stayHead = head;
    for (std::string::iterator it = input.begin(); it != input.end(); it++)
    {
        char x = *it;
//        std::cout << x << std::endl;
        if (x == '+' || x == '-' || x == '*' || x == '/' || x == '^' || x == '%' || x == '(' || x == ')' || x == ',' || x == '=')
        {
            //push symbol type to head after parsing buffer
            if (!buffer.empty())
            {
                CalcNode *nd = to_node(buffer);
                if(nd != nullptr) {
                    head->push_node(nd);
                    head = nd;
                }
                buffer.clear();
            }
            //push symbol
            CalcNode *nd = to_node(x);
            head->push_node(nd);
            head = nd;
        }
        else
        {
            buffer.push_back(*it);
        }
        // std::cout << head << " " << stayHead << std::endl;
    }
    if (!buffer.empty())
    {
        if(buffer.size() == 1) {
            CalcNode *nd = to_node(buffer[0]);
            head->push_node(nd);
            head = nd;
            buffer.clear();
        } else {
            CalcNode *nd = to_node(buffer);
            if(nd != nullptr) {
                head->push_node(nd);
                head = nd;
            }
            buffer.clear();
        }
    }
    CalcNode* actHead = stayHead->get_next();
    actHead->set_prev(nullptr);
    stayHead->disconnect();
    delete stayHead;
    // std::cout << actHead << std::endl;
    return actHead;
}

void do_skip_right(CalcNode *main, CalcNode *side)
{
    main->set_next(side->get_next());
    if (side->get_next() != nullptr)
    {
        side->get_next()->set_prev(main);
    }
}

void do_skip_left(CalcNode *main, CalcNode *side)
{
    main->set_prev(side->get_prev());
    if (side->get_prev() != nullptr)
    {
        side->get_prev()->set_next(main);
    }
}

void do_fuse_center(CalcNode *left, CalcNode *right, CalcNode *fuseTo)
{
    // fuseTo->set_next(right->get_next());
    // fuseTo->set_prev(left->get_prev());
    // right->get_next()->set_prev(fuseTo);
    // left->get_prev()->set_next(fuseTo);
    do_skip_right(fuseTo, right);
    do_skip_left(fuseTo, left);
}

CalcNode *fuse_nodes(CalcNode *centralNode, RuleBook *book)
{
    CalcNode *newCentralNode;
    std::string na = book->get_function(static_cast<CalcSymbol*>(centralNode->get_data()));
    std::cout << na << std::endl; 
    size_t lim = book->get_limit(na);
    if (lim == 2)
    {
        //no need to do additional checking since function is binary
        CalcSecondaryFunctionData *dat = new CalcSecondaryFunctionData(na);
        newCentralNode = new CalcNode(dat);
        CalcNode *fuse1 = centralNode->get_prev();
        CalcNode *fuse2 = centralNode->get_next();
        do_fuse_center(fuse1, fuse2, newCentralNode);
        dat->push_arg(fuse1->get_data());
        dat->push_arg(fuse2->get_data());
        fuse1->disconnect();
        fuse2->disconnect();
        delete fuse1;
        delete fuse2;
    }
    else if (lim == -1)
    {
        //do additional checking for chaining
        CalcNode *left = centralNode->get_prev();
        CalcNode *right = centralNode->get_next();
        //you only need to check left because parsing is left to right
        if(left->get_type() == SECONDARY_FUNCTION && static_cast<CalcSecondaryFunctionData *>(left->get_data())->get_name() == na){
            //found chain match to the left
            do_skip_right(left, right);
            static_cast<CalcSecondaryFunctionData *>(left->get_data())->push_arg(right->get_data());
            right->disconnect();
            newCentralNode = left;
            delete right;
        } else {
            //found no chain match
            CalcSecondaryFunctionData *dat = new CalcSecondaryFunctionData(na);
            newCentralNode = new CalcNode(dat);
            do_fuse_center(left, right, newCentralNode);
            dat->push_arg(left->get_data());
            dat->push_arg(right->get_data());
            left->disconnect();
            right->disconnect();
            delete left;
            delete right;
        }
    }
    else
    {
        throw "Unexpected arguement cap";
    }
    centralNode->disconnect();
    delete centralNode;
    return newCentralNode;
}

void wrap_node(CalcNode *central, CalcSecondaryFunctionData *wrapperData)
{
    CalcNode *wrapper = new CalcNode(wrapperData);
    CalcNode *toWrap = central->get_next();
    wrapperData->push_arg(toWrap->get_data());
    central->set_next(wrapper);
    wrapper->set_prev(central);
    do_skip_right(wrapper, toWrap);
    toWrap->disconnect();
    delete toWrap;
}

CalcNode* push_args_to_function(CalcSecondaryFunctionData* dat, CalcNode* bracketOpen) {
    bool prime = false;
    while(!bracketOpen->is_symbol(PARENTHESISCLOSE_SYMBOL)) {
        if(prime) {
            dat->push_arg(bracketOpen->get_data());
            prime = false;
        }
        if(bracketOpen->is_symbol(COMMA_SYMBOL) || bracketOpen->is_symbol(PARENTHESISOPEN_SYMBOL)){
            prime = true;
        } else if(prime == true) {
            throw "Error parsing args for this function: " + dat->get_name();
        }
        bracketOpen = bracketOpen->get_next();
    }
    return bracketOpen;
}

CalcNode* parse_node_for_bracket(CalcNode* head, RuleBook* book) {
    while (head != nullptr)
    {
        if(head->is_symbol(PARENTHESISOPEN_SYMBOL)){
            return head;
        }
        head = head->get_next();
    }
    return nullptr;
}

CalcNode *parse_node_by_precedence(CalcNode *head, RuleBook *book, CalcSymbol* sym, CalcSymbol* alt = nullptr)
{
    CalcNode *prevHead = head;
    while (head != nullptr)
    {
        if(head->is_symbol(PARENTHESISCLOSE_SYMBOL)){
            break;
        }
        if (alt != nullptr && head->is_symbol(alt))
        {
            CalcSecondaryFunctionData *dat = new CalcSecondaryFunctionData(book->get_function(alt));
            wrap_node(head, dat);
            head->set_data(sym);
        }
        if (head->is_symbol(sym))
        {
            head = fuse_nodes(head, book);
        }
        prevHead = head;
        head = head->get_next();
    }

    while (prevHead->get_prev() != nullptr)
    {
        prevHead = prevHead->get_prev();
    }

    return prevHead;
}

CalcNode* parse_for_equals(CalcNode* head, RuleBook* book) {
    CalcNode* prevHead = head;
    while(head != nullptr) {
        if(head->is_symbol(PARENTHESISCLOSE_SYMBOL)){
            break;
        }
        if(head->is_symbol(EQUALS_SYMBOL)) {
            //do stuff
            CalcData* lvalue = head->get_prev()->get_data();
            CalcData* rvalue = head->get_next()->get_data();
            if(lvalue->get_node_type() == TEXT || lvalue->get_node_type() == SECONDARY_FUNCTION) {
                std::vector<CalcText*> params;
                if(lvalue->get_node_type() == SECONDARY_FUNCTION) {
                    CalcSecondaryFunctionData* dt =((CalcSecondaryFunctionData*)lvalue);
                    for(int i = 0; i < dt->get_args().size(); i++) {
                        if(dt->get_args()[i]->get_node_type() != TEXT) {
                            throw "Invalid Function definition";
                        }
                        params.push_back((CalcText*)(dt->get_args()[i]));
                    }
                }
                rvalue = book->pre_substitute(rvalue);
                std::string name;
                if(lvalue->get_node_type() == TEXT) {
                    name = ((CalcText*)lvalue)->get_value();
                } else {
                    name =((CalcSecondaryFunctionData*)lvalue)->get_name();
                }
                CalcUserDefinedFuncton* myFunc = new CalcUserDefinedFuncton(name, rvalue, params);
                book->put_function(name, myFunc, params.size());
                CalcNode* newT = new CalcNode(name);
                newT->set_next(head->get_next()->get_next());
                if(head->get_next()->get_next() != nullptr){
                    head->get_next()->get_next()->set_prev(newT);
                }
                newT->set_prev(head->get_prev()->get_prev());
                if(head->get_prev()->get_prev() != nullptr) {
                    head->get_prev()->get_prev()->set_next(newT);
                }
                head = newT;
            } else {
                throw "Invalid lvalue to assign to";
            }
        }
        prevHead = head;
        head = head->get_next();
    }
    while(prevHead->get_prev() != nullptr) {
        prevHead = prevHead->get_prev();
    }
    return prevHead;
}

CalcNode* parse_nodes(CalcNode *head, RuleBook *book)
{

    //TODO: do function and bracket parsing and function definition
    
    //TODO: Resolve memory leaks regarding dangling CalcNumber instances
    
    //TODO: Also add functionality for = sign
    
    CalcNode* possibleBracket = parse_node_for_bracket(head, book);
    while(possibleBracket != nullptr) {
        //recurse and do stuff
        if(possibleBracket->get_prev() != nullptr && possibleBracket->get_prev()->get_type() == TEXT){
            //parse prev as function
            CalcSecondaryFunctionData* sen = new CalcSecondaryFunctionData(((CalcText *) possibleBracket->get_prev()->get_data())->get_value());
            CalcNode* prevOfBracket = possibleBracket->get_prev();
            possibleBracket->set_prev(nullptr);
            possibleBracket = parse_nodes(possibleBracket->get_next(), book);
            //now find nearest close bracket from here and look for comma seperated singular nodes
            CalcNode* bracketEnd = push_args_to_function(sen, possibleBracket);
            CalcNode* funcNode = new CalcNode(sen);
            do_skip_left(funcNode, prevOfBracket);
            if(bracketEnd->get_next() != nullptr) {
                bracketEnd->get_next()->set_prev(funcNode);
                funcNode->set_next(bracketEnd->get_next());
            }
            while (possibleBracket != bracketEnd) {
                CalcNode* tmp = possibleBracket->get_next();
                delete possibleBracket;
                possibleBracket = tmp;
            }
            delete possibleBracket;
            
            if(funcNode->get_prev() == nullptr) {
                head = funcNode;
            }
        } else {
            CalcNode* prevOfBracket = possibleBracket->get_prev();
            possibleBracket->set_prev(nullptr);
            possibleBracket = parse_nodes(possibleBracket->get_next(), book);
            if(!possibleBracket->get_next()->get_next()->is_symbol(PARENTHESISCLOSE_SYMBOL)){
                throw "Error parsing bracket";
            }
            CalcNode* simplified = possibleBracket->get_next();
            if(simplified->get_data()->get_node_type() == SECONDARY_FUNCTION) {
                ((CalcSecondaryFunctionData* )simplified->get_data())->set_wrap_brackets(true);
            }
            if(prevOfBracket != nullptr){
                prevOfBracket->set_next(simplified);
            }
            simplified->set_prev(prevOfBracket);
            CalcNode* endBracketAfter = possibleBracket->get_next()->get_next()->get_next();
            if(endBracketAfter != nullptr) {
                delete endBracketAfter->get_prev();
                endBracketAfter->set_prev(simplified);
            }
            simplified->set_next(endBracketAfter);
            possibleBracket->disconnect();
            delete possibleBracket;
            if(simplified->get_prev() == nullptr) {
                head = simplified;
            }
        }
        possibleBracket = parse_node_for_bracket(head, book);
    }
    
    head = parse_node_by_precedence(head, book, CARET_SYMBOL);

    head = parse_node_by_precedence(head, book, MODULUS_SYMBOL);

    head = parse_node_by_precedence(head, book, ASTERISK_SYMBOL, SLASH_SYMBOL);

    head = parse_node_by_precedence(head, book, PLUS_SYMBOL, MINUS_SYMBOL);
    
    head = parse_for_equals(head, book);
    
    return head;
}

int main(int argc, char const *argv[])
{
    RuleBook *book = new RuleBook();
    std::string x;
    std::ostringstream st;
    while (true)
    {
        try
        {
            std::cout << ">>";
            getline(std::cin, x);
            if (x == "bye")
            {
                std::cout << "bye!" << std::endl;
                break;
            }
            char last = x.back();
            bool ls = last == ';';
            if (ls)
            {
                x.pop_back();
            }
            CalcNode *head = parse_to_list(x);
            // std::cout << "Hey" << std::endl;
            head->to_debug_string(st);
            std::cout << st.str() << std::endl;
            st.str("");
            head = parse_nodes(head, book);
            CalcData* stageTwoParsed = book->substitute(head->get_data());
            stageTwoParsed->to_string(st);
            std::cout << st.str() << std::endl;
            st.str("");
            if(stageTwoParsed->get_node_type() == PRIMARY_FUNCTION) {
                static_cast<CalcPrimaryFunctionData*>(stageTwoParsed)->evaluate()->to_string(st);
                std::cout << st.str() << std::endl;
                st.str("");
            }
        }
        catch (const char *msg)
        {
            std::cout << msg << std::endl;
        }
    }
    return 0;
}

#endif
