/*--------------------------------------------------------------------------------------
    Класс SeriesAnalyzer предназначен для обработки одномерных последовательностей
случайных данных с целью распознавания в них квазистационарных и переходных серий.
Класс предоставляет два интерфейса для обработки данных:
    - по одному отсчёту при помощи метода enrollNextValue;
    - или вектора данных при помощи метода enrollArray.
Во втором случае доступ осуществляется по указателю, поэтому вызывающая сторона
должна контролировать целостность данных до окончания вызова. По завершении обработки
последовательности данных следует вызвать метод endAnalysis, который сделает ещё одну запись
в массив выходных данных.
Для получения результатов распознавания следует использовать методы getRecord и getRecordsCount
Управление параметрами распознавания осуществляется методами:
    setOverlaysize - управляет перекрытием между последовательными выборками
        setWindowsize - управляет размером выборки для оценки моментов
            setIntervalFactor - управляет шириной доверительного интервала
            А.А.Таранов, 2015
 * -------------------------------------------------------------------------------------*/

#ifndef SERIESANALYZER_H
#define SERIESANALYZER_H

#include <cmath>
#include <vector>

#define SERIESANALYZER_OVERLAYSIZE 0
#define SERIESANALYZER_WINDOWSIZE 300
#define SERIESANALYZER_VECTORSIZE 4
#define SERIESANALYZER_INTERVALFACTOR 3.0

typedef unsigned int uint;
typedef double real;

struct DataSeria
{
    uint type;          // '0' means stationar and '1' means transient
    uint startframe;    // frame number of series start
    uint endframe;      // frame number of series end
};

class SeriesAnalyzer
{
public:
    SeriesAnalyzer(uint window = SERIESANALYZER_WINDOWSIZE, uint overlay = SERIESANALYZER_OVERLAYSIZE, real interval = SERIESANALYZER_INTERVALFACTOR);
    ~SeriesAnalyzer();

    void setOverlaysize(uint value);        // an interface to m_overlay
    void setWindowsize(uint value);         // an interface to m_windowsize
    void enrollNextValue(real value);       // enrolls value
    void enrollArray(real *pt, uint length);// enrolls array
    void clear();                           // clears v_series internal data
    void setIntervalFactor(real value);     // an interface to m_intervalfactor
    int getRecordsCount() const;            // returns number of counts stored in v_series
    void endAnalysis();                     // should be called to make last record in v_series
    DataSeria getRecord(int index) const;   // returns output record by index

private:
    real *v_overlay;                    // overlay vector
    uint m_overlaypos;                  // specifies position in overlay vector
    uint m_overlaysize;                 // specifies overlay of successive windows

    real *v_window;                     // accumulates data for analysis
    uint m_windowpos;                   // stores position in window
    uint m_windowsize;                  // specifies width of analyzer winow

    real m_intervalfactor;                      // koeff for probability interval
    real v_means[SERIESANALYZER_VECTORSIZE];    // stores means of input data
    real v_stdevs[SERIESANALYZER_VECTORSIZE];   // stores stdevs of input data
    uint v_type[SERIESANALYZER_VECTORSIZE];     // stores types of input data series
    DataSeria m_seria;                          // stores last data series information
    int m_counter;                              // stores number of enrolled windows
    std::vector<DataSeria> v_series;            // accumulates series for output

    uint median(char a, char b, char c) const;  // median of three elements
    int loop(int diff) const;                   // loop index operation
    void computeMoments();                       // evaluate moment based ob window data and increment m_counter
    void updateOutput();                        // pushes m_seria into v_series
};

inline uint SeriesAnalyzer::median(char a, char b, char c) const
{
    uint mid;

    if ((a <= b) && (a <= c))
    {
        mid = (b <= c) ? b : c;
    }
    else if ((b <= a) && (b <= c))
    {
        mid = (a <= c) ? a : c;
    }
    else
    {
        mid = (a <= b) ? a : b;
    }
    return mid;
}

inline int SeriesAnalyzer::loop(int diff) const
{
    return (SERIESANALYZER_VECTORSIZE + (diff % SERIESANALYZER_VECTORSIZE)) % SERIESANALYZER_VECTORSIZE;
}

#endif // SERIESANALYZER_H
