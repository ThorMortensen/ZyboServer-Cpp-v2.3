/* 
 * File:   ProgArg_s.h
 * Author: Thor
 *
 * Created on 26. marts 2015, 21:51
 */

#ifndef PROGARG_S_H
#define	PROGARG_S_H

#include <vector>
#include <cstdlib>
#include <stdint.h>
#include <stdexcept>

/**
 * Class to hold argument to this program.
 * Holds the argument option an parameter. 
 * @param uint8_t number of this arg in arg list 
 * @param string literal of this arg (what user should type)
 * @param uint8_t parameter type can be NOTHING, STRING, NUMBER 
 * @param uint8_t length of parameter value string (default 0)
 */
class ProgArg_s {
public:

#define NOTHING 0
#define STRING  1
#define NUMBER  2



    ProgArg_s(); //Default constructor for arrays

    ProgArg_s(int64_t lowerLimit, int64_t upperLimit, uint8_t parameter = NOTHING) :
    lowerLimit(lowerLimit), upperLimit(upperLimit) {
        if (parameter == NUMBER) {
            throw std::invalid_argument( "Dont use NUMBER with number ProgArg-argument " );
        } else
            expectedParam = parameter;
        notANumber = true;
        outOfBound = false;
        this->isNumberArg = true;
        hasValue = false;
        paramValNo = 0;        

    };

    ProgArg_s(const std::string literal, uint8_t parameter, int64_t lowerLimit = 0, int64_t upperLimit = 0) : //ASK FOR POINTER INSTEAD OF COPY STRING????
    number(number), literal(literal), expectedParam(parameter), lowerLimit(lowerLimit), upperLimit(upperLimit) {
        this->isNumberArg = false;
        hasValue = false;
        outOfBound = false;
        notANumber = true;
        paramValNo = 0;
    };

    ProgArg_s(const ProgArg_s& orig);

    bool hasValue;

    //Posible parameter string values for this argument
    std::vector< std::string > possibleParamValus;

    void setArgumet(const std::string literal, uint8_t parameter, uint8_t length = 0);
    bool isValid(std::string parameterValue);
    bool equals(std::string compStr);
    std::string getArgError();
    std::string getParamError();
    void setParamVal(std::string paramVal);
    std::string getParamVal();
    void setParamValNo(uint32_t paramValNo);
    int32_t getParamValNo();
    virtual ~ProgArg_s();

    std::string getLiteral();
    
    int64_t argNumberValue;

private:
    bool isNumberArg;
    int64_t lowerLimit;
    int64_t upperLimit;

    bool notANumber;
    bool outOfBound;
    uint8_t expectedParam;
    uint8_t number;
    std::string literal;
    std::string paramVal;

    int32_t paramValNo;


};

#endif	/* PROGARG_S_H */

