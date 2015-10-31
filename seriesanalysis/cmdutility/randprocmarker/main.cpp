#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "seriesanalyzer.h"

#define BUFFER_LENGTH 256

real str2real(const std::string &str);
int str2int(const std::string &str);

int main(int argc, char *argv[])
{   
    std::ifstream fstream;
    uint row = 0;
    uint column = 0;
    std::string str;

    while( (--argc > 0) && ((*++argv)[0] == '-') ) {
            char option = *++argv[0];
            switch (option) {
            case 'i':
                fstream.open(++(*argv));
                break;
            case 'r':
                str = (++(*argv));
                row = str2int(str);
                break;
            case 'c':
                str = (++(*argv));
                column = str2int(str);
                break;
            }
    }

    if(fstream.is_open())   {
        std::cout << "File opened" << std::endl;
    } else {
        std::cout << "Failed to open" << std::endl;
        return -1;
    }

    SeriesAnalyzer analyzer(300, 0, 2.0);

    char buffer[BUFFER_LENGTH];
    std::string tempstr;
    real tempvalue = 0.0;
    for(uint i = 0; i < row; i++)
        fstream.getline(buffer,BUFFER_LENGTH,'\n');
    while(fstream)  {
        for(uint j = 0; j < column; j++) {
            fstream.getline(buffer,BUFFER_LENGTH,'\t');
        }
        fstream.getline(buffer,BUFFER_LENGTH,'\n');
        tempstr = buffer;
        tempvalue = str2real(tempstr);
        analyzer.enrollNextValue(tempvalue);
        //std::cout << tempvalue << '\n';
    }
    analyzer.endAnalysis();

    int n = analyzer.getRecordsCount();
    std::cout << n << " series found" << std::endl;
    DataSeria seria;
    for(int i = 0; i < n; i++)  {
        seria = analyzer.getRecord(i);
        std::cout << "Seria_" << i << ", from " << seria.startframe
                  << " to " << seria.endframe << ", type " << seria.type
                  << std::endl;
    }

    return 0;
}

real str2real(const std::string &str)
{
     std::istringstream ss(str);
     real result;
     return (ss >> result ? result : 0);
}

int str2int(const std::string &str)
{
     std::istringstream ss(str);
     int result;
     return (ss >> result ? result : 0);
}
