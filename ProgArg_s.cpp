/* 
 * File:   ProgArg_s.cpp
 * Author: Thor
 * 
 * Created on 26. marts 2015, 21:51
 */
#include <cstdlib>
#include <stdint.h>
#include <sstream>
#include <iostream>
#include <vector>
#include "ProgArg_s.h"

using namespace std;

ProgArg_s::ProgArg_s() {
}

/**
 * Copy Constructor (auto generated) 
 * @param orig
 */
ProgArg_s::ProgArg_s(const ProgArg_s& orig) {
}

/**
 * Class to hold argument to this program.
 * Holds the argument option an parameter. 
 * @param uint8_t no
 * @param const string literal
 */
//ProgArg_s::ProgArg_s(uint8_t number, const string literal, uint8_t parameter, uint8_t length/* = 0*/)){
//    
//}

/**
 * Setup the argument. Should be called for this object to make sense
 * 
 * @param uint8_t number of this arg in arg list 
 * @param string literal of this arg (what user should type)
 * @param uint8_t parameter type can be NOTHING, STRING, NUMBER 
 * @param uint8_t length of parameter value string (default 0)
 */
void ProgArg_s::setArgumet(const string literal, uint8_t parameter, uint8_t length/* = 0*/) {
    this->isNumberArg = false;
    this->expectedParam = parameter;
    this->literal = literal;
}

/**
 * Check if the parameter is valid based on the setArgumet 
 * called when this arg was created
 * @param string parameterValue
 * @return true if value passed all test
 */
bool ProgArg_s::isValid(string parameterValue) {
    if (expectedParam == NOTHING) {
        hasValue = true;
        return true;
    } else if (expectedParam == STRING) {
        if (possibleParamValus.empty()) {
            hasValue = true;
            this->paramVal = parameterValue;
            return true;
        } else {
            for (uint8_t i = 0; i < possibleParamValus.size(); i++) {
                if (possibleParamValus.at(i) == parameterValue) {
                    this->paramVal = parameterValue;
                    hasValue = true;
                    return true;
                }
            }
            hasValue = false;
            return false;
        }
    } else if (expectedParam == NUMBER) {
        std::istringstream convert(parameterValue);
        if (!(convert >> paramValNo)) {
            notANumber = true;
            hasValue = false;
            return false;
        } else {
            if (upperLimit == lowerLimit) {
                this->paramVal = parameterValue;
                hasValue = true;
                return true;
            } else if ((paramValNo <= upperLimit) && (paramValNo >= lowerLimit)) {
                this->paramVal = parameterValue;
                hasValue = true;
                return true;
            } else {
                outOfBound = true;
                this->paramVal = parameterValue;
                hasValue = false;
                return false;
            }
        }
    }
    hasValue = false;
    return false;
}

/**
 * Is this arg literal equal to the compStr
 * @param string compStr
 * @return true if so 
 */
bool ProgArg_s::equals(string compStr) {
    if (this->isNumberArg) {
        std::istringstream convert(compStr);
        if (!(convert >> argNumberValue)) {
            notANumber = true;
            return false;
        } else {
            if (argNumberValue >= lowerLimit && argNumberValue <= upperLimit) {
                return true;
            } else {
                outOfBound = true;
                return false;
            }
        }
    } else return (literal == compStr);
}

string ProgArg_s::getParamError() {
    stringstream paramErrorMsg;

    if (expectedParam == STRING) {
        paramErrorMsg << "Wrong parameter. Expected: ";
        if (!possibleParamValus.empty()) {
            paramErrorMsg << " with content: ";
            for (uint8_t i = 0; i < possibleParamValus.size(); i++) {
                paramErrorMsg << "\"" << possibleParamValus.at(i) << "\" ";
            }
        }
    }

    if (expectedParam == NUMBER) {
        if (notANumber) {
            paramErrorMsg << "Expected a number ";
        }
        if (outOfBound) {
            paramErrorMsg << "Number entered is out of bounds. Bounds is ";
            paramErrorMsg << "(" << lowerLimit << " to " << upperLimit << ").";
        }
    }

    if (this->isNumberArg)
        paramErrorMsg << " This error concerns the parameter that followed a number argument ";
    else
        paramErrorMsg << " This error concerns the parameter that followed the argument " << "'" << literal << "'";

    paramErrorMsg << endl;
    return paramErrorMsg.str();
}

/**
 * Print an error telling what was expected for this arg to be satisfied
 */
string ProgArg_s::getArgError() {
    stringstream argErrorMsg;
    if (this->isNumberArg) {
        if (notANumber) {
            argErrorMsg << "This is not a number";
        }
        if (outOfBound) {
            argErrorMsg << "Number entered is out of bounds";
            argErrorMsg << "(" << lowerLimit << " to " << upperLimit << ").";
        }
    } else {
        argErrorMsg << "Wrong argument expected somthing like" << "'" << literal << "'.";
    }
    argErrorMsg << endl;
    return argErrorMsg.str();
}

std::string ProgArg_s::getLiteral()  {
    return literal;
}

void ProgArg_s::setParamVal(string paramVal) {
    this->paramVal = paramVal;
}

string ProgArg_s::getParamVal() {
    return paramVal;
}

void ProgArg_s::setParamValNo(uint32_t paramValNo) {
    this->paramValNo = paramValNo;
}

int32_t ProgArg_s::getParamValNo() {
    return paramValNo;
}

ProgArg_s::~ProgArg_s() {
}

