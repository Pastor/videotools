#include "seriesanalyzer.h"

SeriesAnalyzer::SeriesAnalyzer(uint window, uint overlay, real interval)
{
    clear();
    v_window = NULL;
    v_overlay = NULL;
    setWindowsize(window);
    setOverlaysize(overlay);
    setIntervalFactor(interval);
}

SeriesAnalyzer::~SeriesAnalyzer()
{
    delete[] v_window;
    delete[] v_overlay;
}

void SeriesAnalyzer::setOverlaysize(uint value)
{
    if(value <= m_windowsize) {
        m_overlaysize = value;
        m_overlaypos = 0;
        if(m_overlaysize > 0)   {
            delete[] v_overlay;
            v_overlay = new real[m_overlaysize];
        }
    }
}

void SeriesAnalyzer::setWindowsize(uint value)
{
    m_windowsize = value;
    m_windowpos = 0;
    if(m_windowsize > 0)   {
        delete[] v_window;
        v_window = new real[m_windowsize];
    }
}

void SeriesAnalyzer::enrollNextValue(real value)
{
    if((m_windowpos == 0) && (m_overlaysize > 0) && (m_counter > 0))    {
       for(uint i = 0; i < m_overlaysize; i++)
           v_window[i] = v_overlay[i];
       m_windowpos += m_overlaysize;
    } else if ( m_windowpos >= (m_windowsize - m_overlaysize) ) {
        v_overlay[ m_overlaypos++ ] = value;
        if(m_overlaypos == m_overlaysize)
            m_overlaypos = 0;
    }

    v_window[ m_windowpos++ ] = value;

    if( m_windowpos == m_windowsize)    {
        m_windowpos = 0;
        computeMoments();
    }
}

void SeriesAnalyzer::enrollArray(real *pt, uint length)
{
    for(uint i = 0; i < length; i++)
        enrollNextValue(pt[i]);
}

void SeriesAnalyzer::clear()
{
    m_seria.type = 0;
    m_seria.startframe = 1;
    m_counter = 0;
    m_windowpos = 0;
    m_overlaypos = 0;
    for(uint i = 0; i < SERIESANALYZER_VECTORSIZE; i++)
        v_type[i] = 0;
    v_series.clear();
}

void SeriesAnalyzer::setIntervalFactor(real value)
{
    if(value > 0.0)
        m_intervalfactor = value;
}

void SeriesAnalyzer::computeMoments()
{
    real mean = 0.0;
    for(uint i = 0; i < m_windowsize; i++)
        mean += v_window[i];
    mean /= m_windowsize;
    real stdev = 0.0;
    for(uint i = 0; i < m_windowsize; i++)
        stdev += (v_window[i] - mean)*(v_window[i] - mean);
    stdev = std::sqrt(stdev / (m_windowsize - 1));

    v_means[loop(m_counter)] = mean;
    v_stdevs[loop(m_counter)] = stdev / std::sqrt((real)m_windowsize);

    real d = std::abs( v_means[loop(m_counter)] - v_means[loop(m_counter-1)] );
    real s = ( v_stdevs[loop(m_counter)] + v_stdevs[loop(m_counter-1)] ) / 2.0;

    if( d < (s * m_intervalfactor) )
        v_type[loop(m_counter - 1)] = 0;  // process is likely to be stationar
    else
        v_type[loop(m_counter - 1)] = 1;  // process is likely to be transient

    v_type[loop(m_counter - 2)] = median(v_type[loop(m_counter - 3)],v_type[loop(m_counter - 2)],v_type[loop(m_counter - 1)]);

    if(m_counter > 2)   {
        if( m_seria.type != v_type[loop(m_counter - 2)]) {
            m_seria.endframe = m_windowsize + (m_windowsize - m_overlaysize)*(m_counter - 3); // this equation takes into account time shift
            updateOutput();
            m_seria.type = v_type[loop(m_counter - 2)];
            m_seria.startframe = m_seria.endframe + 1;
        }
    }
    m_counter++;
}

void SeriesAnalyzer::endAnalysis()
{
    m_seria.endframe = m_windowsize + (m_windowsize - m_overlaysize)*(m_counter - 1); // end of data
    updateOutput();
}

void SeriesAnalyzer::updateOutput()
{
    v_series.push_back(m_seria);
}

int SeriesAnalyzer::getRecordsCount() const
{
    return v_series.size();
}

DataSeria SeriesAnalyzer::getRecord(int index) const
{
    return v_series[index];
}
