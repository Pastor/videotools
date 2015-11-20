#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "seriesanalyzer.h"

#define BUFFER_LENGTH 1280
#define OUTPUT_BUFFER_LENGTH 16

real str2real(const std::string &str);
int str2int(const std::string &str);

int main(int argc, char *argv[])
{   
    std::ifstream fstream;
    SeriesAnalyzer analyzer;
    uint row = 0;
    uint column = 0;
    std::string str;
    char delim = '\t';
    bool f_output = false;
    std::ofstream ofstream;

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
            case 'w':
                str = (++(*argv));
                analyzer.setWindowsize( str2int(str) );
                break;
            case 'o':
                str = (++(*argv));
                analyzer.setOverlaysize( str2int(str) );
                break;
            case 'k':
                str = (++(*argv));
                analyzer.setIntervalFactor( str2real(str) );
                break;
            case 'd':
                delim = *(++(*argv));
                break;
            case 'f':
                f_output = true;
                ofstream.open(++(*argv));
                break;
            case 'h':
                std::cout << APP_NAME << " v." << APP_VERS << "\n"
                          << "Options:\n"
                          << " -i[filename] - input filename\n"
                          << " -r[row] - set start row number\n"
                          << " -c[col] - set column\n"
                          << " -w[x] - set window size to x, default: " << SERIESANALYZER_WINDOWSIZE << "\n"
                          << " -o[x] - set overlay to x, default: " << SERIESANALYZER_OVERLAYSIZE << "\n"
                          << " -k[x] - set confidence interval koeffitient to x, default: " << SERIESANALYZER_INTERVALFACTOR << "\n"
                          << " -d[x] - deliminator symbol, default tabulation\n"
                          << " -f[filename] - output filename, output containes all counts with marks\n" << APP_DESIGNER;
                return 0;
            }
    }

    if(!fstream.is_open())   {
        std::cout << "Failed to open input file" << std::endl;
        return -1;
    }

    if(f_output)
        if(!ofstream.is_open()) {
            std::cout << "Failed to open output file" << std::endl;
            return -2;
        }

    char buffer[BUFFER_LENGTH];
    std::string tempstr;
    real tempvalue = 0.0;
    for(uint i = 0; i < row; i++)
        if(fstream)
            fstream.getline(buffer,BUFFER_LENGTH,'\n');
    while(fstream)  {
        for(uint j = 0; j < column; j++)
            fstream.getline(buffer,BUFFER_LENGTH, delim);
        fstream.getline(buffer,BUFFER_LENGTH,'\n');
        tempstr = buffer;
        tempvalue = str2real(tempstr);
        analyzer.enrollNextValue(tempvalue);
        //std::cout << buffer << std::endl;
    }
    analyzer.endAnalysis();

    int n = analyzer.getRecordsCount();
    std::cout << "File processed, " << n << " series found:" << std::endl;
    DataSeria seria;
    for(int i = 0; i < n; i++)  {
        seria = analyzer.getRecord(i);
        std::cout << "Seria" << i+1 << ":\t" << seria.startframe
                  << " - " << seria.endframe << ", type " << seria.type
                  << std::endl;
    }

    if(f_output) {
        char oline[OUTPUT_BUFFER_LENGTH];
        std::sprintf(oline, "Frame%cProctype", delim);
        ofstream.write(oline, 14);
        for(int i = 0; i < n; i++) {
            seria = analyzer.getRecord(i);
            for(int j = seria.startframe; j <= seria.endframe; j++) {
                std::sprintf(oline,"\n%06d%c%d", j, delim, seria.type);
                ofstream.write(oline, 9);
            }
        }
        std::cout << "Output file recorded";
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
