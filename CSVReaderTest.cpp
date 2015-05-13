///* 
// * File:   CSVReaderTest.cpp
// * Author: Thor
// *
// * Created on April 24, 2015, 10:56 AM
// */
//
//#include <cstdlib>
//#include <stdint.h>
//#include <iostream>
//#include <sstream>
//#include <iostream>
//#include <fstream>
//#include <bitset>
//#include <vector>
//
//#include <iostream>
//
//using namespace std;
//
//const char DELIMITER_COMMA = ',';
//
////______________ From the Internet!!!! ________________
////http://coliru.stacked-crooked.com/a/652f29c0500cf195) 
////http://stackoverflow.com/questions/53849/how-do-i-tokenize-a-string-in-c
//
//void tokenizeForCSV(std::string str, std::vector<string> &token_v) {
//    size_t start = str.find_first_not_of(DELIMITER_COMMA);
//    size_t end = start;
//
//    while (start != std::string::npos) {
//        // Find next occurence of delimiter
//        end = str.find(DELIMITER_COMMA, start);
//        // Push back the token found into vector
//        token_v.push_back(str.substr(start, end - start));
//        // Skip all occurences of the delimiter to find new start
//        start = str.find_first_not_of(DELIMITER_COMMA, end);
//    }
//}
//
///*
// * 
// */
//int main(int argc, char** argv) {
//
//    vector< vector<int> > fileValues;
//
//    vector<string> names;
//    vector<string> numberValStrings;
//
//
//
//    string lineInFile;
//    ifstream fileToRead;
//    fileToRead.open("log.csv");
//    getline(fileToRead, lineInFile);
//
//    tokenizeForCSV(lineInFile, names);
//
//    vector<int> numbers;
//
//    while (fileToRead.tellg() != -1) {
//        numbers.clear();
//        numberValStrings.clear();
//        getline(fileToRead, lineInFile);
//        tokenizeForCSV(lineInFile, numberValStrings);
//        for (int i = 0; i < numberValStrings.size(); i++) {
//            // cout << numberValStrings.at(i) << endl;
//            stringstream converter(numberValStrings.at(i));
//            int number = 0;
//            converter >> number;
//            numbers.push_back(number);
//        }
//        fileValues.push_back(numbers);
//    }
//    int sizeOfInputs = fileValues.size();
//    int biggestVal = 0;
//    for (int i = 0; i < sizeOfInputs-1; i++) {
//        int currentVal = fileValues.at(i).at(4);
//        if (biggestVal < currentVal)
//            biggestVal = currentVal;
//    }
//
//
//    cout << biggestVal + 0 << endl;
//
//
//
//
//
//
//    return 0;
//}
//
//////