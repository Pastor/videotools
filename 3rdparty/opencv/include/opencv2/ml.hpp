/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Copyright (C) 2013, OpenCV Foundation, all rights reserved.
// Copyright (C) 2014, Itseez Inc, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#ifndef __OPENCV_ML_HPP__
#define __OPENCV_ML_HPP__

#ifdef __cplusplus
#  include "opencv2/core.hpp"
#endif

#ifdef __cplusplus

#include <float.h>
#include <map>
#include <iostream>

/**
  @defgroup ml Machine Learning

  The Machine Learning Library (MLL) is a set of classes and functions for statistical
  classification, regression, and clustering of data.

  Most of the classification and regression algorithms are implemented as C++ classes. As the
  algorithms have different sets of features (like an ability to handle missing measurements or
  categorical input variables), there is a little common ground between the classes. This common
  ground is defined by the class cv::ml::StatModel that all the other ML classes are derived from.

  See detailed overview here: @ref ml_intro.
 */

namespace cv
{

namespace ml
{

//! @addtogroup ml
//! @{

/** @brief Variable types */
enum VariableTypes
{
    VAR_NUMERICAL    =0, //!< same as VAR_ORDERED
    VAR_ORDERED      =0, //!< ordered variables
    VAR_CATEGORICAL  =1  //!< categorical variables
};

/** @brief %Error types */
enum ErrorTypes
{
    TEST_ERROR = 0,
    TRAIN_ERROR = 1
};

/** @brief Sample types */
enum SampleTypes
{
    ROW_SAMPLE = 0, //!< each training sample is a row of samples
    COL_SAMPLE = 1  //!< each training sample occupies a column of samples
};

/** @brief The structure represents the logarithmic grid range of statmodel parameters.

It is used for optimizing statmodel accuracy by varying model parameters, the accuracy estimate
being computed by cross-validation.
 */
class CV_EXPORTS_W_MAP ParamGrid
{
public:
    /** @brief Default constructor */
    ParamGrid();
    /** @brief Constructor with parameters */
    ParamGrid(double _minVal, double _maxVal, double _logStep);

    CV_PROP_RW double minVal; //!< Minimum value of the statmodel parameter. Default value is 0.
    CV_PROP_RW double maxVal; //!< Maximum value of the statmodel parameter. Default value is 0.
    /** @brief Logarithmic step for iterating the statmodel parameter.

    The grid determines the following iteration sequence of the statmodel parameter values:
    \f[(minVal, minVal*step, minVal*{step}^2, \dots,  minVal*{logStep}^n),\f]
    where \f$n\f$ is the maximal index satisfying
    \f[\texttt{minVal} * \texttt{logStep} ^n <  \texttt{maxVal}\f]
    The grid is logarithmic, so logStep must always be greater then 1. Default value is 1.
    */
    CV_PROP_RW double logStep;
};

/** @brief Class encapsulating training data.

Please note that the class only specifies the interface of training data, but not implementation.
All the statistical model classes in _ml_ module accepts Ptr\<TrainData\> as parameter. In other
words, you can create your own class derived from TrainData and pass smart pointer to the instance
of this class into StatModel::train.

@sa @ref ml_intro_data
 */
class CV_EXPORTS TrainData
{
public:
    static inline float missingValue() { return FLT_MAX; }
    virtual ~TrainData();

    virtual int getLayout() const = 0;
    virtual int getNTrainSamples() const = 0;
    virtual int getNTestSamples() const = 0;
    virtual int getNSamples() const = 0;
    virtual int getNVars() const = 0;
    virtual int getNAllVars() const = 0;

    virtual void getSample(InputArray varIdx, int sidx, float* buf) const = 0;
    virtual Mat getSamples() const = 0;
    virtual Mat getMissing() const = 0;

    /** @brief Returns matrix of train samples

    @param layout The requested layout. If it's different from the initial one, the matrix is
        transposed. See ml::SampleTypes.
    @param compressSamples if true, the function returns only the training samples (specified by
        sampleIdx)
    @param compressVars if true, the function returns the shorter training samples, containing only
        the active variables.

    In current implementation the function tries to avoid physical data copying and returns the
    matrix stored inside TrainData (unless the transposition or compression is needed).
     */
    virtual Mat getTrainSamples(int layout=ROW_SAMPLE,
                                bool compressSamples=true,
                                bool compressVars=true) const = 0;

    /** @brief Returns the vector of responses

    The function returns ordered or the original categorical responses. Usually it's used in
    regression algorithms.
     */
    virtual Mat getTrainResponses() const = 0;

    /** @brief Returns the vector of normalized categorical responses

    The function returns vector of responses. Each response is integer from `0` to `<number of
    classes>-1`. The actual label value can be retrieved then from the class label vector, see
    TrainData::getClassLabels.
     */
    virtual Mat getTrainNormCatResponses() const = 0;
    virtual Mat getTestResponses() const = 0;
    virtual Mat getTestNormCatResponses() const = 0;
    virtual Mat getResponses() const = 0;
    virtual Mat getNormCatResponses() const = 0;
    virtual Mat getSampleWeights() const = 0;
    virtual Mat getTrainSampleWeights() const = 0;
    virtual Mat getTestSampleWeights() const = 0;
    virtual Mat getVarIdx() const = 0;
    virtual Mat getVarType() const = 0;
    virtual int getResponseType() const = 0;
    virtual Mat getTrainSampleIdx() const = 0;
    virtual Mat getTestSampleIdx() const = 0;
    virtual void getValues(int vi, InputArray sidx, float* values) const = 0;
    virtual void getNormCatValues(int vi, InputArray sidx, int* values) const = 0;
    virtual Mat getDefaultSubstValues() const = 0;

    virtual int getCatCount(int vi) const = 0;

    /** @brief Returns the vector of class labels

    The function returns vector of unique labels occurred in the responses.
     */
    virtual Mat getClassLabels() const = 0;

    virtual Mat getCatOfs() const = 0;
    virtual Mat getCatMap() const = 0;

    /** @brief Splits the training data into the training and test parts
    @sa TrainData::setTrainTestSplitRatio
     */
    virtual void setTrainTestSplit(int count, bool shuffle=true) = 0;

    /** @brief Splits the training data into the training and test parts

    The function selects a subset of specified relative size and then returns it as the training
    set. If the function is not called, all the data is used for training. Please, note that for
    each of TrainData::getTrain\* there is corresponding TrainData::getTest\*, so that the test
    subset can be retrieved and processed as well.
    @sa TrainData::setTrainTestSplit
     */
    virtual void setTrainTestSplitRatio(double ratio, bool shuffle=true) = 0;
    virtual void shuffleTrainTest() = 0;

    static Mat getSubVector(const Mat& vec, const Mat& idx);

    /** @brief Reads the dataset from a .csv file and returns the ready-to-use training data.

    @param filename The input file name
    @param headerLineCount The number of lines in the beginning to skip; besides the header, the
        function also skips empty lines and lines staring with `#`
    @param responseStartIdx Index of the first output variable. If -1, the function considers the
        last variable as the response
    @param responseEndIdx Index of the last output variable + 1. If -1, then there is single
        response variable at responseStartIdx.
    @param varTypeSpec The optional text string that specifies the variables' types. It has the
        format `ord[n1-n2,n3,n4-n5,...]cat[n6,n7-n8,...]`. That is, variables from `n1 to n2`
        (inclusive range), `n3`, `n4 to n5` ... are considered ordered and `n6`, `n7 to n8` ... are
        considered as categorical. The range `[n1..n2] + [n3] + [n4..n5] + ... + [n6] + [n7..n8]`
        should cover all the variables. If varTypeSpec is not specified, then algorithm uses the
        following rules:
        - all input variables are considered ordered by default. If some column contains has non-
          numerical values, e.g. 'apple', 'pear', 'apple', 'apple', 'mango', the corresponding
          variable is considered categorical.
        - if there are several output variables, they are all considered as ordered. Error is
          reported when non-numerical values are used.
        - if there is a single output variable, then if its values are non-numerical or are all
          integers, then it's considered categorical. Otherwise, it's considered ordered.
    @param delimiter The character used to separate values in each line.
    @param missch The character used to specify missing measurements. It should not be a digit.
        Although it's a non-numerical value, it surely does not affect the decision of whether the
        variable ordered or categorical.
     */
    static Ptr<TrainData> loadFromCSV(const String& filename,
                                      int headerLineCount,
                                      int responseStartIdx=-1,
                                      int responseEndIdx=-1,
                                      const String& varTypeSpec=String(),
                                      char delimiter=',',
                                      char missch='?');

    /** @brief Creates training data from in-memory arrays.

    @param samples matrix of samples. It should have CV_32F type.
    @param layout see ml::SampleTypes.
    @param responses matrix of responses. If the responses are scalar, they should be stored as a
        single row or as a single column. The matrix should have type CV_32F or CV_32S (in the
        former case the responses are considered as ordered by default; in the latter case - as
        categorical)
    @param varIdx vector specifying which variables to use for training. It can be an integer vector
        (CV_32S) containing 0-based variable indices or byte vector (CV_8U) containing a mask of
        active variables.
    @param sampleIdx vector specifying which samples to use for training. It can be an integer
        vector (CV_32S) containing 0-based sample indices or byte vector (CV_8U) containing a mask
        of training samples.
    @param sampleWeights optional vector with weights for each sample. It should have CV_32F type.
    @param varType optional vector of type CV_8U and size `<number_of_variables_in_samples> +
        <number_of_variables_in_responses>`, containing types of each input and output variable. See
        ml::VariableTypes.
     */
    static Ptr<TrainData> create(InputArray samples, int layout, InputArray responses,
                                 InputArray varIdx=noArray(), InputArray sampleIdx=noArray(),
                                 InputArray sampleWeights=noArray(), InputArray varType=noArray());
};

/** @brief Base class for statistical models in OpenCV ML.
 */
class CV_EXPORTS_W StatModel : public Algorithm
{
public:
    /** Predict options */
    enum Flags {
        UPDATE_MODEL = 1,
        RAW_OUTPUT=1, //!< makes the method return the raw results (the sum), not the class label
        COMPRESSED_INPUT=2,
        PREPROCESSED_INPUT=4
    };
    virtual void clear();

    /** @brief Returns the number of variables in training samples */
    virtual int getVarCount() const = 0;

    /** @brief Returns true if the model is trained */
    virtual bool isTrained() const = 0;
    /** @brief Returns true if the model is classifier */
    virtual bool isClassifier() const = 0;

    /** @brief Trains the statistical model

    @param trainData training data that can be loaded from file using TrainData::loadFromCSV or
        created with TrainData::create.
    @param flags optional flags, depending on the model. Some of the models can be updated with the
        new training samples, not completely overwritten (such as NormalBayesClassifier or ANN_MLP).
     */
    virtual bool train( const Ptr<TrainData>& trainData, int flags=0 );

    /** @brief Trains the statistical model

    @param samples training samples
    @param layout See ml::SampleTypes.
    @param responses vector of responses associated with the training samples.
    */
    virtual bool train( InputArray samples, int layout, InputArray responses );

    /** @brief Computes error on the training or test dataset

    @param data the training data
    @param test if true, the error is computed over the test subset of the data, otherwise it's
        computed over the training subset of the data. Please note that if you loaded a completely
        different dataset to evaluate already trained classifier, you will probably want not to set
        the test subset at all with TrainData::setTrainTestSplitRatio and specify test=false, so
        that the error is computed for the whole new set. Yes, this sounds a bit confusing.
    @param resp the optional output responses.

    The method uses StatModel::predict to compute the error. For regression models the error is
    computed as RMS, for classifiers - as a percent of missclassified samples (0%-100%).
     */
    virtual float calcError( const Ptr<TrainData>& data, bool test, OutputArray resp ) const;

    /** @brief Predicts response(s) for the provided sample(s)

    @param samples The input samples, floating-point matrix
    @param results The optional output matrix of results.
    @param flags The optional flags, model-dependent. See cv::ml::StatModel::Flags.
     */
    virtual float predict( InputArray samples, OutputArray results=noArray(), int flags=0 ) const = 0;

    /** @brief Loads model from the file

    This is static template method of StatModel. It's usage is following (in the case of SVM):
    @code
    Ptr<SVM> svm = StatModel::load<SVM>("my_svm_model.xml");
    @endcode
    In order to make this method work, the derived class must overwrite Algorithm::read(const
    FileNode& fn).
     */
    template<typename _Tp> static Ptr<_Tp> load(const String& filename)
    {
        FileStorage fs(filename, FileStorage::READ);
        Ptr<_Tp> model = _Tp::create();
        model->read(fs.getFirstTopLevelNode());
        return model->isTrained() ? model : Ptr<_Tp>();
    }

    /** @brief Loads model from a String

    @param strModel The string variable containing the model you want to load.

    This is static template method of StatModel. It's usage is following (in the case of SVM):
    @code
    Ptr<SVM> svm = StatModel::loadFromString<SVM>(myStringModel);
    @endcode
     */
    template<typename _Tp> static Ptr<_Tp> loadFromString(const String& strModel)
    {
        FileStorage fs(strModel, FileStorage::READ + FileStorage::MEMORY);
        Ptr<_Tp> model = _Tp::create();
        model->read(fs.getFirstTopLevelNode());
        return model->isTrained() ? model : Ptr<_Tp>();
    }


    /** @brief Creates new statistical model and trains it

    @param data training data that can be loaded from file using TrainData::loadFromCSV or
        created with TrainData::create.
    @param p model parameters
    @param flags optional flags, depending on the model. Some of the models can be updated with the
        new training samples, not completely overwritten (such as NormalBayesClassifier or ANN_MLP).
     */
    template<typename _Tp> static Ptr<_Tp> train(const Ptr<TrainData>& data, const typename _Tp::Params& p, int flags=0)
    {
        Ptr<_Tp> model = _Tp::create(p);
        return !model.empty() && model->train(data, flags) ? model : Ptr<_Tp>();
    }

    /** @brief Creates new statistical model and trains it

    @param samples training samples
    @param layout See ml::SampleTypes.
    @param responses vector of responses associated with the training samples.
    @param p model parameters
    @param flags optional flags, depending on the model. Some of the models can be updated with the
        new training samples, not completely overwritten (such as NormalBayesClassifier or ANN_MLP).
    */
    template<typename _Tp> static Ptr<_Tp> train(InputArray samples, int layout, InputArray responses,
                                                 const typename _Tp::Params& p, int flags=0)
    {
        Ptr<_Tp> model = _Tp::create(p);
        return !model.empty() && model->train(TrainData::create(samples, layout, responses), flags) ? model : Ptr<_Tp>();
    }

    /** @brief Saves the model to a file.

    In order to make this method work, the derived class must overwrite
    Algorithm::write(FileStorage& fs).
     */
    virtual void save(const String& filename) const;
    virtual String getDefaultModelName() const = 0;
};

/****************************************************************************************\
*                                 Normal Bayes Classifier                                *
\****************************************************************************************/

/** @brief Bayes classifier for normally distributed data.

@sa @ref ml_intro_bayes
 */
class CV_EXPORTS_W NormalBayesClassifier : public StatModel
{
public:
    class CV_EXPORTS_W Params
    {
    public:
        Params();
    };
    /** @brief Predicts the response for sample(s).

    The method estimates the most probable classes for input vectors. Input vectors (one or more)
    are stored as rows of the matrix inputs. In case of multiple input vectors, there should be one
    output vector outputs. The predicted class for a single input vector is returned by the method.
    The vector outputProbs contains the output probabilities corresponding to each element of
    result.
     */
    virtual float predictProb( InputArray inputs, OutputArray outputs,
                               OutputArray outputProbs, int flags=0 ) const = 0;
    virtual void setParams(const Params& params) = 0;
    virtual Params getParams() const = 0;

    /** @brief Creates empty model

    @param params The model parameters. There is none so far, the structure is used as a placeholder
        for possible extensions.

    Use StatModel::train to train the model:
    @code
    StatModel::train<NormalBayesClassifier>(traindata, params); // to create and train the model
    StatModel::load<NormalBayesClassifier>(filename); // load the pre-trained model
    @endcode
     */
    static Ptr<NormalBayesClassifier> create(const Params& params=Params());
};

/****************************************************************************************\
*                          K-Nearest Neighbour Classifier                                *
\****************************************************************************************/

/** @brief The class implements K-Nearest Neighbors model

@sa @ref ml_intro_knn
 */
class CV_EXPORTS_W KNearest : public StatModel
{
public:
    class CV_EXPORTS_W_MAP Params
    {
    public:
        /** @brief Constructor with parameters */
        Params(int defaultK=10, bool isclassifier_=true, int Emax_=INT_MAX, int algorithmType_=BRUTE_FORCE);

        CV_PROP_RW int defaultK; //!< default number of neighbors to use in predict method
        CV_PROP_RW bool isclassifier; //!< whether classification or regression model should be trained
        CV_PROP_RW int Emax; //!< for implementation with KDTree
        CV_PROP_RW int algorithmType; //!< See KNearest::Types
    };
    virtual void setParams(const Params& p) = 0;
    virtual Params getParams() const = 0;

    /** @brief Finds the neighbors and predicts responses for input vectors.

    @param samples Input samples stored by rows. It is a single-precision floating-point matrix of
        `<number_of_samples> * k` size.
    @param k Number of used nearest neighbors. Should be greater than 1.
    @param results Vector with results of prediction (regression or classification) for each input
        sample. It is a single-precision floating-point vector with `<number_of_samples>` elements.
    @param neighborResponses Optional output values for corresponding neighbors. It is a single-
        precision floating-point matrix of `<number_of_samples> * k` size.
    @param dist Optional output distances from the input vectors to the corresponding neighbors. It
        is a single-precision floating-point matrix of `<number_of_samples> * k` size.

    For each input vector (a row of the matrix samples), the method finds the k nearest neighbors.
    In case of regression, the predicted result is a mean value of the particular vector's neighbor
    responses. In case of classification, the class is determined by voting.

    For each input vector, the neighbors are sorted by their distances to the vector.

    In case of C++ interface you can use output pointers to empty matrices and the function will
    allocate memory itself.

    If only a single input vector is passed, all output matrices are optional and the predicted
    value is returned by the method.

    The function is parallelized with the TBB library.
     */
    virtual float findNearest( InputArray samples, int k,
                               OutputArray results,
                               OutputArray neighborResponses=noArray(),
                               OutputArray dist=noArray() ) const = 0;

    enum Types { BRUTE_FORCE=1, KDTREE=2 };

    /** @brief Creates the empty model

    @param params The model parameters

    The static method creates empty %KNearest classifier. It should be then trained using train
    method (see StatModel::train). Alternatively, you can load boost model from file using:
    `StatModel::load<KNearest>(filename)`
     */
    static Ptr<KNearest> create(const Params& params=Params());
};

/****************************************************************************************\
*                                   Support Vector Machines                              *
\****************************************************************************************/

/** @brief Support Vector Machines.

@sa @ref ml_intro_svm
 */
class CV_EXPORTS_W SVM : public StatModel
{
public:
    /** @brief %SVM training parameters.

    The structure must be initialized and passed to the training method of %SVM.
     */
    class CV_EXPORTS_W_MAP Params
    {
    public:
        /** @brief Default constructor */
        Params();
        /** @brief Constructor with parameters */
        Params( int svm_type, int kernel_type,
                double degree, double gamma, double coef0,
                double Cvalue, double nu, double p,
                const Mat& classWeights, TermCriteria termCrit );

        /** Type of a %SVM formulation. See SVM::Types. Default value is SVM::C_SVC. */
        CV_PROP_RW int         svmType;
        /** Type of a %SVM kernel. See SVM::KernelTypes. Default value is SVM::RBF. */
        CV_PROP_RW int         kernelType;
        /** Parameter \f$\gamma\f$ of a kernel function (SVM::POLY / SVM::RBF / SVM::SIGMOID /
        SVM::CHI2). Default value is 1. */
        CV_PROP_RW double      gamma;
        /** Parameter coef0 of a kernel function (SVM::POLY / SVM::SIGMOID). Default value is 0. */
        CV_PROP_RW double      coef0;
        /** Parameter degree of a kernel function (SVM::POLY). Default value is 0. */
        CV_PROP_RW double      degree;

        /** Parameter C of a %SVM optimization problem (SVM::C_SVC / SVM::EPS_SVR / SVM::NU_SVR).
        Default value is 0. */
        CV_PROP_RW double      C;
        /** Parameter \f$\nu\f$ of a %SVM optimization problem (SVM::NU_SVC / SVM::ONE_CLASS /
        SVM::NU_SVR). Default value is 0. */
        CV_PROP_RW double      nu;
        /** Parameter \f$\epsilon\f$ of a %SVM optimization problem (SVM::EPS_SVR). Default value is 0. */
        CV_PROP_RW double      p;

        /** Optional weights in the SVM::C_SVC problem , assigned to particular classes. They are
        multiplied by C so the parameter C of class \#i becomes classWeights(i) \* C. Thus these
        weights affect the misclassification penalty for different classes. The larger weight, the
        larger penalty on misclassification of data from the corresponding class. Default value is
        empty Mat.*/
        CV_PROP_RW Mat         classWeights;
        /** Termination criteria of the iterative %SVM training procedure which solves a partial
        case of constrained quadratic optimization problem. You can specify tolerance and/or the
        maximum number of iterations. Default value is TermCriteria(
        TermCriteria::MAX_ITER + TermCriteria::EPS, 1000, FLT_EPSILON );*/
        CV_PROP_RW TermCriteria termCrit;
    };

    class CV_EXPORTS Kernel : public Algorithm
    {
    public:
        virtual int getType() const = 0;
        virtual void calc( int vcount, int n, const float* vecs, const float* another, float* results ) = 0;
    };

    //! %SVM type
    enum Types {
        /** C-Support Vector Classification. n-class classification (n \f$\geq\f$ 2), allows
        imperfect separation of classes with penalty multiplier C for outliers. */
        C_SVC=100,
        /** \f$\nu\f$-Support Vector Classification. n-class classification with possible
        imperfect separation. Parameter \f$\nu\f$ (in the range 0..1, the larger the value, the smoother
        the decision boundary) is used instead of C. */
        NU_SVC=101,
        /** Distribution Estimation (One-class %SVM). All the training data are from
        the same class, %SVM builds a boundary that separates the class from the rest of the feature
        space. */
        ONE_CLASS=102,
        /** \f$\epsilon\f$-Support Vector Regression. The distance between feature vectors
        from the training set and the fitting hyper-plane must be less than p. For outliers the
        penalty multiplier C is used. */
        EPS_SVR=103,
        /** \f$\nu\f$-Support Vector Regression. \f$\nu\f$ is used instead of p.
        See @cite LibSVM for details. */
        NU_SVR=104
    };

    /** @brief %SVM kernel type

    A comparison of different kernels on the following 2D test case with four classes. Four
    SVM::C_SVC SVMs have been trained (one against rest) with auto_train. Evaluation on three
    different kernels (SVM::CHI2, SVM::INTER, SVM::RBF). The color depicts the class with max score.
    Bright means max-score \> 0, dark means max-score \< 0.
    ![image](pics/SVM_Comparison.png)
    */
    enum KernelTypes {
        CUSTOM=-1,
        /** Linear kernel. No mapping is done, linear discrimination (or regression) is
        done in the original feature space. It is the fastest option. \f$K(x_i, x_j) = x_i^T x_j\f$. */
        LINEAR=0,
        /** Polynomial kernel:
        \f$K(x_i, x_j) = (\gamma x_i^T x_j + coef0)^{degree}, \gamma > 0\f$. */
        POLY=1,
        /** Radial basis function (RBF), a good choice in most cases.
        \f$K(x_i, x_j) = e^{-\gamma ||x_i - x_j||^2}, \gamma > 0\f$. */
        RBF=2,
        /** Sigmoid kernel: \f$K(x_i, x_j) = \tanh(\gamma x_i^T x_j + coef0)\f$. */
        SIGMOID=3,
        /** Exponential Chi2 kernel, similar to the RBF kernel:
        \f$K(x_i, x_j) = e^{-\gamma \chi^2(x_i,x_j)}, \chi^2(x_i,x_j) = (x_i-x_j)^2/(x_i+x_j), \gamma > 0\f$. */
        CHI2=4,
        /** Histogram intersection kernel. A fast kernel. \f$K(x_i, x_j) = min(x_i,x_j)\f$. */
        INTER=5
    };

    //! %SVM params type
    enum ParamTypes {
        C=0,
        GAMMA=1,
        P=2,
        NU=3,
        COEF=4,
        DEGREE=5
    };

    /** @brief Trains an %SVM with optimal parameters.

    @param data the training data that can be constructed using TrainData::create or
        TrainData::loadFromCSV.
    @param kFold Cross-validation parameter. The training set is divided into kFold subsets. One
        subset is used to test the model, the others form the train set. So, the %SVM algorithm is
        executed kFold times.
    @param Cgrid grid for C
    @param gammaGrid grid for gamma
    @param pGrid grid for p
    @param nuGrid grid for nu
    @param coeffGrid grid for coeff
    @param degreeGrid grid for degree
    @param balanced If true and the problem is 2-class classification then the method creates more
        balanced cross-validation subsets that is proportions between classes in subsets are close
        to such proportion in the whole train dataset.

    The method trains the %SVM model automatically by choosing the optimal parameters C, gamma, p,
    nu, coef0, degree from SVM::Params. Parameters are considered optimal when the cross-validation
    estimate of the test set error is minimal.

    If there is no need to optimize a parameter, the corresponding grid step should be set to any
    value less than or equal to 1. For example, to avoid optimization in gamma, set `gammaGrid.step
    = 0`, `gammaGrid.minVal`, `gamma_grid.maxVal` as arbitrary numbers. In this case, the value
    `params.gamma` is taken for gamma.

    And, finally, if the optimization in a parameter is required but the corresponding grid is
    unknown, you may call the function SVM::getDefaultGrid. To generate a grid, for example, for
    gamma, call `SVM::getDefaultGrid(SVM::GAMMA)`.

    This function works for the classification (SVM::C_SVC or SVM::NU_SVC) as well as for the
    regression (SVM::EPS_SVR or SVM::NU_SVR). If it is SVM::ONE_CLASS, no optimization is made and
    the usual %SVM with parameters specified in params is executed.
     */
    virtual bool trainAuto( const Ptr<TrainData>& data, int kFold = 10,
                    ParamGrid Cgrid = SVM::getDefaultGrid(SVM::C),
                    ParamGrid gammaGrid  = SVM::getDefaultGrid(SVM::GAMMA),
                    ParamGrid pGrid      = SVM::getDefaultGrid(SVM::P),
                    ParamGrid nuGrid     = SVM::getDefaultGrid(SVM::NU),
                    ParamGrid coeffGrid  = SVM::getDefaultGrid(SVM::COEF),
                    ParamGrid degreeGrid = SVM::getDefaultGrid(SVM::DEGREE),
                    bool balanced=false) = 0;

    /** @brief Retrieves all the support vectors

    The method returns all the support vector as floating-point matrix, where support vectors are
    stored as matrix rows.
     */
    CV_WRAP virtual Mat getSupportVectors() const = 0;

    virtual void setParams(const Params& p, const Ptr<Kernel>& customKernel=Ptr<Kernel>()) = 0;

    /** @brief Returns the current %SVM parameters.

    This function may be used to get the optimal parameters obtained while automatically training
    SVM::trainAuto.
     */
    virtual Params getParams() const = 0;
    virtual Ptr<Kernel> getKernel() const = 0;

    /** @brief Retrieves the decision function

    @param i the index of the decision function. If the problem solved is regression, 1-class or
        2-class classification, then there will be just one decision function and the index should
        always be 0. Otherwise, in the case of N-class classification, there will be \f$N(N-1)/2\f$
        decision functions.
    @param alpha the optional output vector for weights, corresponding to different support vectors.
        In the case of linear %SVM all the alpha's will be 1's.
    @param svidx the optional output vector of indices of support vectors within the matrix of
        support vectors (which can be retrieved by SVM::getSupportVectors). In the case of linear
        %SVM each decision function consists of a single "compressed" support vector.

    The method returns rho parameter of the decision function, a scalar subtracted from the weighted
    sum of kernel responses.
     */
    virtual double getDecisionFunction(int i, OutputArray alpha, OutputArray svidx) const = 0;

    /** @brief Generates a grid for %SVM parameters.

    @param param_id %SVM parameters IDs that must be one of the SVM::ParamTypes. The grid is
        generated for the parameter with this ID.

    The function generates a grid for the specified parameter of the %SVM algorithm. The grid may be
    passed to the function SVM::trainAuto.
     */
    static ParamGrid getDefaultGrid( int param_id );

    /** @brief Creates empty model

    @param p %SVM parameters
    @param customKernel the optional custom kernel to use. It must implement SVM::Kernel interface.

    Use StatModel::train to train the model:
    @code
        StatModel::train<SVM>(traindata, params); // to create and train the model
        // or
        StatModel::load<SVM>(filename); // to load the pre-trained model.
    @endcode
    Since %SVM has several parameters, you may want to find the best parameters for your problem. It
    can be done with SVM::trainAuto.
     */
    static Ptr<SVM> create(const Params& p=Params(), const Ptr<Kernel>& customKernel=Ptr<Kernel>());
};

/****************************************************************************************\
*                              Expectation - Maximization                                *
\****************************************************************************************/

/** @brief The class implements the Expectation Maximization algorithm.

@sa @ref ml_intro_em
 */
class CV_EXPORTS_W EM : public StatModel
{
public:
    //! Type of covariation matrices
    enum Types {
        /** A scaled identity matrix \f$\mu_k * I\f$. There is the only
        parameter \f$\mu_k\f$ to be estimated for each matrix. The option may be used in special cases,
        when the constraint is relevant, or as a first step in the optimization (for example in case
        when the data is preprocessed with PCA). The results of such preliminary estimation may be
        passed again to the optimization procedure, this time with
        covMatType=EM::COV_MAT_DIAGONAL. */
        COV_MAT_SPHERICAL=0,
        /** A diagonal matrix with positive diagonal elements. The number of
        free parameters is d for each matrix. This is most commonly used option yielding good
        estimation results. */
        COV_MAT_DIAGONAL=1,
        /** A symmetric positively defined matrix. The number of free
        parameters in each matrix is about \f$d^2/2\f$. It is not recommended to use this option, unless
        there is pretty accurate initial estimation of the parameters and/or a huge number of
        training samples. */
        COV_MAT_GENERIC=2,
        COV_MAT_DEFAULT=COV_MAT_DIAGONAL
    };

    //! Default parameters
    enum {DEFAULT_NCLUSTERS=5, DEFAULT_MAX_ITERS=100};

    //! The initial step
    enum {START_E_STEP=1, START_M_STEP=2, START_AUTO_STEP=0};

    /** @brief The class describes %EM training parameters.
    */
    class CV_EXPORTS_W_MAP Params
    {
    public:
        /** @brief The constructor

        @param nclusters The number of mixture components in the Gaussian mixture model. Default
            value of the parameter is EM::DEFAULT_NCLUSTERS=5. Some of %EM implementation could
            determine the optimal number of mixtures within a specified value range, but that is not
            the case in ML yet.
        @param covMatType Constraint on covariance matrices which defines type of matrices. See
            EM::Types.
        @param termCrit The termination criteria of the %EM algorithm. The %EM algorithm can be
            terminated by the number of iterations termCrit.maxCount (number of M-steps) or when
            relative change of likelihood logarithm is less than termCrit.epsilon. Default maximum
            number of iterations is EM::DEFAULT_MAX_ITERS=100.
         */
        explicit Params(int nclusters=DEFAULT_NCLUSTERS, int covMatType=EM::COV_MAT_DIAGONAL,
                        const TermCriteria& termCrit=TermCriteria(TermCriteria::COUNT+TermCriteria::EPS,
                                                                  EM::DEFAULT_MAX_ITERS, 1e-6));
        CV_PROP_RW int nclusters;
        CV_PROP_RW int covMatType;
        CV_PROP_RW TermCriteria termCrit;
    };

    virtual void setParams(const Params& p) = 0;
    virtual Params getParams() const = 0;
    /** @brief Returns weights of the mixtures

    Returns vector with the number of elements equal to the number of mixtures.
     */
    virtual Mat getWeights() const = 0;
    /** @brief Returns the cluster centers (means of the Gaussian mixture)

    Returns matrix with the number of rows equal to the number of mixtures and number of columns
    equal to the space dimensionality.
     */
    virtual Mat getMeans() const = 0;
    /** @brief Returns covariation matrices

    Returns vector of covariation matrices. Number of matrices is the number of gaussian mixtures,
    each matrix is a square floating-point matrix NxN, where N is the space dimensionality.
     */
    virtual void getCovs(std::vector<Mat>& covs) const = 0;

    /** @brief Returns a likelihood logarithm value and an index of the most probable mixture component
    for the given sample.

    @param sample A sample for classification. It should be a one-channel matrix of
        \f$1 \times dims\f$ or \f$dims \times 1\f$ size.
    @param probs Optional output matrix that contains posterior probabilities of each component
        given the sample. It has \f$1 \times nclusters\f$ size and CV_64FC1 type.

    The method returns a two-element double vector. Zero element is a likelihood logarithm value for
    the sample. First element is an index of the most probable mixture component for the given
    sample.
     */
    CV_WRAP virtual Vec2d predict2(InputArray sample, OutputArray probs) const = 0;

    virtual bool train( const Ptr<TrainData>& trainData, int flags=0 ) = 0;

    /** @brief Static method that estimate the Gaussian mixture parameters from a samples set

    This variation starts with Expectation step. Initial values of the model parameters will be
    estimated by the k-means algorithm.

    Unlike many of the ML models, %EM is an unsupervised learning algorithm and it does not take
    responses (class labels or function values) as input. Instead, it computes the *Maximum
    Likelihood Estimate* of the Gaussian mixture parameters from an input sample set, stores all the
    parameters inside the structure: \f$p_{i,k}\f$ in probs, \f$a_k\f$ in means , \f$S_k\f$ in
    covs[k], \f$\pi_k\f$ in weights , and optionally computes the output "class label" for each
    sample: \f$\texttt{labels}_i=\texttt{arg max}_k(p_{i,k}), i=1..N\f$ (indices of the most
    probable mixture component for each sample).

    The trained model can be used further for prediction, just like any other classifier. The
    trained model is similar to the NormalBayesClassifier.

    @param samples Samples from which the Gaussian mixture model will be estimated. It should be a
        one-channel matrix, each row of which is a sample. If the matrix does not have CV_64F type
        it will be converted to the inner matrix of such type for the further computing.
    @param logLikelihoods The optional output matrix that contains a likelihood logarithm value for
        each sample. It has \f$nsamples \times 1\f$ size and CV_64FC1 type.
    @param labels The optional output "class label" for each sample:
        \f$\texttt{labels}_i=\texttt{arg max}_k(p_{i,k}), i=1..N\f$ (indices of the most probable
        mixture component for each sample). It has \f$nsamples \times 1\f$ size and CV_32SC1 type.
    @param probs The optional output matrix that contains posterior probabilities of each Gaussian
        mixture component given the each sample. It has \f$nsamples \times nclusters\f$ size and
        CV_64FC1 type.
    @param params The Gaussian mixture params, see EM::Params description
     */
    static Ptr<EM> train(InputArray samples,
                          OutputArray logLikelihoods=noArray(),
                          OutputArray labels=noArray(),
                          OutputArray probs=noArray(),
                          const Params& params=Params());

    /** @brief Static method that estimate the Gaussian mixture parameters from a samples set

    This variation starts with Expectation step. You need to provide initial means \f$a_k\f$ of
    mixture components. Optionally you can pass initial weights \f$\pi_k\f$ and covariance matrices
    \f$S_k\f$ of mixture components.

    @param samples Samples from which the Gaussian mixture model will be estimated. It should be a
        one-channel matrix, each row of which is a sample. If the matrix does not have CV_64F type
        it will be converted to the inner matrix of such type for the further computing.
    @param means0 Initial means \f$a_k\f$ of mixture components. It is a one-channel matrix of
        \f$nclusters \times dims\f$ size. If the matrix does not have CV_64F type it will be
        converted to the inner matrix of such type for the further computing.
    @param covs0 The vector of initial covariance matrices \f$S_k\f$ of mixture components. Each of
        covariance matrices is a one-channel matrix of \f$dims \times dims\f$ size. If the matrices
        do not have CV_64F type they will be converted to the inner matrices of such type for the
        further computing.
    @param weights0 Initial weights \f$\pi_k\f$ of mixture components. It should be a one-channel
        floating-point matrix with \f$1 \times nclusters\f$ or \f$nclusters \times 1\f$ size.
    @param logLikelihoods The optional output matrix that contains a likelihood logarithm value for
        each sample. It has \f$nsamples \times 1\f$ size and CV_64FC1 type.
    @param labels The optional output "class label" for each sample:
        \f$\texttt{labels}_i=\texttt{arg max}_k(p_{i,k}), i=1..N\f$ (indices of the most probable
        mixture component for each sample). It has \f$nsamples \times 1\f$ size and CV_32SC1 type.
    @param probs The optional output matrix that contains posterior probabilities of each Gaussian
        mixture component given the each sample. It has \f$nsamples \times nclusters\f$ size and
        CV_64FC1 type.
    @param params The Gaussian mixture params, see EM::Params description
    */
    static Ptr<EM> train_startWithE(InputArray samples, InputArray means0,
                                     InputArray covs0=noArray(),
                                     InputArray weights0=noArray(),
                                     OutputArray logLikelihoods=noArray(),
                                     OutputArray labels=noArray(),
                                     OutputArray probs=noArray(),
                                     const Params& params=Params());

    /** @brief Static method that estimate the Gaussian mixture parameters from a samples set

    This variation starts with Maximization step. You need to provide initial probabilities
    \f$p_{i,k}\f$ to use this option.

    @param samples Samples from which the Gaussian mixture model will be estimated. It should be a
        one-channel matrix, each row of which is a sample. If the matrix does not have CV_64F type
        it will be converted to the inner matrix of such type for the further computing.
    @param probs0
    @param logLikelihoods The optional output matrix that contains a likelihood logarithm value for
        each sample. It has \f$nsamples \times 1\f$ size and CV_64FC1 type.
    @param labels The optional output "class label" for each sample:
        \f$\texttt{labels}_i=\texttt{arg max}_k(p_{i,k}), i=1..N\f$ (indices of the most probable
        mixture component for each sample). It has \f$nsamples \times 1\f$ size and CV_32SC1 type.
    @param probs The optional output matrix that contains posterior probabilities of each Gaussian
        mixture component given the each sample. It has \f$nsamples \times nclusters\f$ size and
        CV_64FC1 type.
    @param params The Gaussian mixture params, see EM::Params description
    */
    static Ptr<EM> train_startWithM(InputArray samples, InputArray probs0,
                                     OutputArray logLikelihoods=noArray(),
                                     OutputArray labels=noArray(),
                                     OutputArray probs=noArray(),
                                     const Params& params=Params());

    /** @brief Creates empty %EM model

    @param params %EM parameters

    The model should be trained then using StatModel::train(traindata, flags) method. Alternatively, you
    can use one of the EM::train\* methods or load it from file using StatModel::load\<EM\>(filename).
     */
    static Ptr<EM> create(const Params& params=Params());
};

/****************************************************************************************\
*                                      Decision Tree                                     *
\****************************************************************************************/

/** @brief The class represents a single decision tree or a collection of decision trees.

The current public interface of the class allows user to train only a single decision tree, however
the class is capable of storing multiple decision trees and using them for prediction (by summing
responses or using a voting schemes), and the derived from DTrees classes (such as RTrees and Boost)
use this capability to implement decision tree ensembles.

@sa @ref ml_intro_trees
*/
class CV_EXPORTS_W DTrees : public StatModel
{
public:
    /** Predict options */
    enum Flags { PREDICT_AUTO=0, PREDICT_SUM=(1<<8), PREDICT_MAX_VOTE=(2<<8), PREDICT_MASK=(3<<8) };

    /** @brief The structure contains all the decision tree training parameters.

    You can initialize it by default constructor and then override any parameters directly before
    training, or the structure may be fully initialized using the advanced variant of the
    constructor.
     */
    class CV_EXPORTS_W_MAP Params
    {
    public:
        /** @brief Default constructor. */
        Params();
        /** @brief Constructor with parameters */
        Params( int maxDepth, int minSampleCount,
               double regressionAccuracy, bool useSurrogates,
               int maxCategories, int CVFolds,
               bool use1SERule, bool truncatePrunedTree,
               const Mat& priors );

        /** @brief Cluster possible values of a categorical variable into K\<=maxCategories clusters
            to find a suboptimal split.

        If a discrete variable, on which the training procedure tries to make a split, takes more
        than maxCategories values, the precise best subset estimation may take a very long time
        because the algorithm is exponential. Instead, many decision trees engines (including our
        implementation) try to find sub-optimal split in this case by clustering all the samples
        into maxCategories clusters that is some categories are merged together. The clustering is
        applied only in n \> 2-class classification problems for categorical variables with N \>
        max_categories possible values. In case of regression and 2-class classification the optimal
        split can be found efficiently without employing clustering, thus the parameter is not used
        in these cases. Default value is 10.*/
        CV_PROP_RW int   maxCategories;
        /** @brief The maximum possible depth of the tree.

        That is the training algorithms attempts to split a node while its depth is less than
        maxDepth. The root node has zero depth. The actual depth may be smaller if the other
        termination criteria are met (see the outline of the training procedure @ref ml_intro_trees
        "here"), and/or if the tree is pruned. Default value is INT_MAX.*/
        CV_PROP_RW int   maxDepth;
        /** If the number of samples in a node is less than this parameter then the node will not be
        split. Default value is 10.*/
        CV_PROP_RW int   minSampleCount;
        /** If CVFolds \> 1 then algorithms prunes the built decision tree using K-fold
        cross-validation procedure where K is equal to CVFolds. Default value is 10.*/
        CV_PROP_RW int   CVFolds;
        /** @brief If true then surrogate splits will be built.

        These splits allow to work with missing data and compute variable importance correctly.
        @note currently it's not implemented. Default value is false.*/
        CV_PROP_RW bool  useSurrogates;
        /** If true then a pruning will be harsher. This will make a tree more compact and more
        resistant to the training data noise but a bit less accurate. Default value is true.*/
        CV_PROP_RW bool  use1SERule;
        /** If true then pruned branches are physically removed from the tree. Otherwise they are
        retained and it is possible to get results from the original unpruned (or pruned less
        aggressively) tree. Default value is true.*/
        CV_PROP_RW bool  truncatePrunedTree;
        /** @brief Termination criteria for regression trees.

        If all absolute differences between an estimated value in a node and values of train samples
        in this node are less than this parameter then the node will not be split further. Default
        value is 0.01f*/
        CV_PROP_RW float regressionAccuracy;
        /** @brief The array of a priori class probabilities, sorted by the class label value.

        The parameter can be used to tune the decision tree preferences toward a certain class. For
        example, if you want to detect some rare anomaly occurrence, the training base will likely
        contain much more normal cases than anomalies, so a very good classification performance
        will be achieved just by considering every case as normal. To avoid this, the priors can be
        specified, where the anomaly probability is artificially increased (up to 0.5 or even
        greater), so the weight of the misclassified anomalies becomes much bigger, and the tree is
        adjusted properly.

        You can also think about this parameter as weights of prediction categories which determine
        relative weights that you give to misclassification. That is, if the weight of the first
        category is 1 and the weight of the second category is 10, then each mistake in predicting
        the second category is equivalent to making 10 mistakes in predicting the first category.
        Default value is empty Mat.*/
        CV_PROP_RW Mat priors;
    };

    /** @brief The class represents a decision tree node.
     */
    class CV_EXPORTS Node
    {
    public:
        Node();
        double value; //!< Value at the node: a class label in case of classification or estimated
                      //!< function value in case of regression.
        int classIdx; //!< Class index normalized to 0..class_count-1 range and assigned to the
                      //!< node. It is used internally in classification trees and tree ensembles.
        int parent; //!< Index of the parent node
        int left; //!< Index of the left child node
        int right; //!< Index of right child node
        int defaultDir; //!< Default direction where to go (-1: left or +1: right). It helps in the
                        //!< case of missing values.
        int split; //!< Index of the first split
    };

    /** @brief The class represents split in a decision tree.
     */
    class CV_EXPORTS Split
    {
    public:
        Split();
        int varIdx; //!< Index of variable on which the split is created.
        bool inversed; //!< If true, then the inverse split rule is used (i.e. left and right
                       //!< branches are exchanged in the rule expressions below).
        float quality; //!< The split quality, a positive number. It is used to choose the best split.
        int next; //!< Index of the next split in the list of splits for the node
        float c; /**< The threshold value in case of split on an ordered variable.
                      The rule is:
                      @code{.none}
                      if var_value < c
                        then next_node <- left
                        else next_node <- right
                      @endcode */
        int subsetOfs; /**< Offset of the bitset used by the split on a categorical variable.
                            The rule is:
                            @code{.none}
                            if bitset[var_value] == 1
                                then next_node <- left
                                else next_node <- right
                            @endcode */
    };

    /** @brief Sets the training parameters
     */
    virtual void setDParams(const Params& p);
    /** @brief Returns the training parameters
     */
    virtual Params getDParams() const;

    /** @brief Returns indices of root nodes
    */
    virtual const std::vector<int>& getRoots() const = 0;
    /** @brief Returns all the nodes

    all the node indices are indices in the returned vector
     */
    virtual const std::vector<Node>& getNodes() const = 0;
    /** @brief Returns all the splits

    all the split indices are indices in the returned vector
     */
    virtual const std::vector<Split>& getSplits() const = 0;
    /** @brief Returns all the bitsets for categorical splits

    Split::subsetOfs is an offset in the returned vector
     */
    virtual const std::vector<int>& getSubsets() const = 0;

    /** @brief Creates the empty model

    The static method creates empty decision tree with the specified parameters. It should be then
    trained using train method (see StatModel::train). Alternatively, you can load the model from
    file using StatModel::load\<DTrees\>(filename).
     */
    static Ptr<DTrees> create(const Params& params=Params());
};

/****************************************************************************************\
*                                   Random Trees Classifier                              *
\****************************************************************************************/

/** @brief The class implements the random forest predictor.

@sa @ref ml_intro_rtrees
 */
class CV_EXPORTS_W RTrees : public DTrees
{
public:
    /** @brief The set of training parameters for the forest is a superset of the training
    parameters for a single tree.

    However, random trees do not need all the functionality/features of decision trees. Most
    noticeably, the trees are not pruned, so the cross-validation parameters are not used.
     */
    class CV_EXPORTS_W_MAP Params : public DTrees::Params
    {
    public:
        /** @brief Default constructor. */
        Params();
        /** @brief Constructor with parameters. */
        Params( int maxDepth, int minSampleCount,
                double regressionAccuracy, bool useSurrogates,
                int maxCategories, const Mat& priors,
                bool calcVarImportance, int nactiveVars,
                TermCriteria termCrit );

        /** If true then variable importance will be calculated and then it can be retrieved by
        RTrees::getVarImportance. Default value is false.*/
        CV_PROP_RW bool calcVarImportance;
        /** The size of the randomly selected subset of features at each tree node and that are used
        to find the best split(s). If you set it to 0 then the size will be set to the square root
        of the total number of features. Default value is 0.*/
        CV_PROP_RW int nactiveVars;
        /** The termination criteria that specifies when the training algorithm stops - either when
        the specified number of trees is trained and added to the ensemble or when sufficient
        accuracy (measured as OOB error) is achieved. Typically the more trees you have the better
        the accuracy. However, the improvement in accuracy generally diminishes and asymptotes pass
        a certain number of trees. Also to keep in mind, the number of tree increases the prediction
        time linearly. Default value is TermCriteria(TermCriteria::MAX_ITERS + TermCriteria::EPS,
        50, 0.1)*/
        CV_PROP_RW TermCriteria termCrit;
    };

    virtual void setRParams(const Params& p) = 0;
    virtual Params getRParams() const = 0;

    /** @brief Returns the variable importance array.

    The method returns the variable importance vector, computed at the training stage when
    Params::calcVarImportance is set to true. If this flag was set to false, the empty matrix is
    returned.
     */
    virtual Mat getVarImportance() const = 0;

    /** @brief Creates the empty model

    Use StatModel::train to train the model, StatModel::train to create and train the model,
    StatModel::load to load the pre-trained model.
     */
    static Ptr<RTrees> create(const Params& params=Params());
};

/****************************************************************************************\
*                                   Boosted tree classifier                              *
\****************************************************************************************/

/** @brief Boosted tree classifier derived from DTrees

@sa @ref ml_intro_boost
 */
class CV_EXPORTS_W Boost : public DTrees
{
public:
    /** @brief Parameters of Boost trees.

    The structure is derived from DTrees::Params but not all of the decision tree parameters are
    supported. In particular, cross-validation is not supported.

    All parameters are public. You can initialize them by a constructor and then override some of
    them directly if you want.
     */
    class CV_EXPORTS_W_MAP Params : public DTrees::Params
    {
    public:
        CV_PROP_RW int boostType; //!< Type of the boosting algorithm. See Boost::Types.
                                  //!< Default value is Boost::REAL.
        CV_PROP_RW int weakCount; //!< The number of weak classifiers. Default value is 100.
        /** A threshold between 0 and 1 used to save computational time. Samples with summary weight
        \f$\leq 1 - weight_trim_rate\f$ do not participate in the *next* iteration of training. Set
        this parameter to 0 to turn off this functionality. Default value is 0.95.*/
        CV_PROP_RW double weightTrimRate;

        /** @brief Default constructor */
        Params();
        /** @brief Constructor with parameters */
        Params( int boostType, int weakCount, double weightTrimRate,
                int maxDepth, bool useSurrogates, const Mat& priors );
    };

    /** @brief Boosting type

    Gentle AdaBoost and Real AdaBoost are often the preferable choices.
    */
    enum Types {
        DISCRETE=0, //!< Discrete AdaBoost.
        REAL=1, //!< Real AdaBoost. It is a technique that utilizes confidence-rated predictions
                //!< and works well with categorical data.
        LOGIT=2, //!< LogitBoost. It can produce good regression fits.
        GENTLE=3 //!< Gentle AdaBoost. It puts less weight on outlier data points and for that
                 //!<reason is often good with regression data.
    };

    /** @brief Returns the boosting parameters */
    virtual Params getBParams() const = 0;
    /** @brief Sets the boosting parameters */
    virtual void setBParams(const Params& p) = 0;

    /** @brief Creates the empty model

    Use StatModel::train to train the model, StatModel::train\<Boost\>(traindata, params) to create
    and train the model, StatModel::load\<Boost\>(filename) to load the pre-trained model.
     */
    static Ptr<Boost> create(const Params& params=Params());
};

/****************************************************************************************\
*                                   Gradient Boosted Trees                               *
\****************************************************************************************/

/*class CV_EXPORTS_W GBTrees : public DTrees
{
public:
    struct CV_EXPORTS_W_MAP Params : public DTrees::Params
    {
        CV_PROP_RW int weakCount;
        CV_PROP_RW int lossFunctionType;
        CV_PROP_RW float subsamplePortion;
        CV_PROP_RW float shrinkage;

        Params();
        Params( int lossFunctionType, int weakCount, float shrinkage,
                float subsamplePortion, int maxDepth, bool useSurrogates );
    };

    enum {SQUARED_LOSS=0, ABSOLUTE_LOSS, HUBER_LOSS=3, DEVIANCE_LOSS};

    virtual void setK(int k) = 0;

    virtual float predictSerial( InputArray samples,
                                 OutputArray weakResponses, int flags) const = 0;

    static Ptr<GBTrees> create(const Params& p);
};*/

/****************************************************************************************\
*                              Artificial Neural Networks (ANN)                          *
\****************************************************************************************/

/////////////////////////////////// Multi-Layer Perceptrons //////////////////////////////

/** @brief Artificial Neural Networks - Multi-Layer Perceptrons.

Unlike many other models in ML that are constructed and trained at once, in the MLP model these
steps are separated. First, a network with the specified topology is created using the non-default
constructor or the method ANN_MLP::create. All the weights are set to zeros. Then, the network is
trained using a set of input and output vectors. The training procedure can be repeated more than
once, that is, the weights can be adjusted based on the new training data.

Additional flags for StatModel::train are available: ANN_MLP::TrainFlags.

@sa @ref ml_intro_ann
 */
class CV_EXPORTS_W ANN_MLP : public StatModel
{
public:
    /** @brief Parameters of the MLP and of the training algorithm.
    */
    struct CV_EXPORTS_W_MAP Params
    {
        /** @brief Default constructor */
        Params();
        /** @brief Constructor with parameters
        @note param1 sets Params::rp_dw0 for RPROP and Paramss::bp_dw_scale for BACKPROP.
        @note param2 sets Params::rp_dw_min for RPROP and Params::bp_moment_scale for BACKPROP.
         */
        Params( const Mat& layerSizes, int activateFunc, double fparam1, double fparam2,
                TermCriteria termCrit, int trainMethod, double param1, double param2=0 );

        /** Available training methods */
        enum TrainingMethods {
            BACKPROP=0, //!< The back-propagation algorithm.
            RPROP=1 //!< The RPROP algorithm. See @cite RPROP93 for details.
        };

        /**  Integer vector specifying the number of neurons in each layer including the input and
        output layers. The very first element specifies the number of elements in the input layer.
        The last element - number of elements in the output layer. Default value is empty Mat.*/
        CV_PROP_RW Mat layerSizes;
        /** The activation function for each neuron. Currently the default and the only fully
        supported activation function is ANN_MLP::SIGMOID_SYM. See ANN_MLP::ActivationFunctions.*/
        CV_PROP_RW int activateFunc;
        /** The first parameter of the activation function, \f$\alpha\f$. Default value is 0. */
        CV_PROP_RW double fparam1;
        /** The second parameter of the activation function, \f$\beta\f$. Default value is 0. */
        CV_PROP_RW double fparam2;

        /** Termination criteria of the training algorithm. You can specify the maximum number of
        iterations (maxCount) and/or how much the error could change between the iterations to make
        the algorithm continue (epsilon). Default value is TermCriteria(TermCriteria::MAX_ITER +
        TermCriteria::EPS, 1000, 0.01).*/
        CV_PROP_RW TermCriteria termCrit;
        /** Training method. Default value is Params::RPROP. See ANN_MLP::Params::TrainingMethods.*/
        CV_PROP_RW int trainMethod;

        // backpropagation parameters
        /** BPROP: Strength of the weight gradient term. The recommended value is about 0.1. Default
        value is 0.1.*/
        CV_PROP_RW double bpDWScale;
        /** BPROP: Strength of the momentum term (the difference between weights on the 2 previous
        iterations). This parameter provides some inertia to smooth the random fluctuations of the
        weights. It can vary from 0 (the feature is disabled) to 1 and beyond. The value 0.1 or so
        is good enough. Default value is 0.1.*/
        CV_PROP_RW double bpMomentScale;

        // rprop parameters
        /** RPROP: Initial value \f$\Delta_0\f$ of update-values \f$\Delta_{ij}\f$. Default value is 0.1.*/
        CV_PROP_RW double rpDW0;
        /** RPROP: Increase factor \f$\eta^+\f$. It must be \>1. Default value is 1.2.*/
        CV_PROP_RW double rpDWPlus;
        /** RPROP: Decrease factor \f$\eta^-\f$. It must be \<1. Default value is 0.5.*/
        CV_PROP_RW double rpDWMinus;
        /** RPROP: Update-values lower limit \f$\Delta_{min}\f$. It must be positive. Default value is FLT_EPSILON.*/
        CV_PROP_RW double rpDWMin;
        /** RPROP: Update-values upper limit \f$\Delta_{max}\f$. It must be \>1. Default value is 50.*/
        CV_PROP_RW double rpDWMax;
    };

    /** possible activation functions */
    enum ActivationFunctions {
        /** Identity function: \f$f(x)=x\f$ */
        IDENTITY = 0,
        /** Symmetrical sigmoid: \f$f(x)=\beta*(1-e^{-\alpha x})/(1+e^{-\alpha x}\f$
        @note
        If you are using the default sigmoid activation function with the default parameter values
        fparam1=0 and fparam2=0 then the function used is y = 1.7159\*tanh(2/3 \* x), so the output
        will range from [-1.7159, 1.7159], instead of [0,1].*/
        SIGMOID_SYM = 1,
        /** Gaussian function: \f$f(x)=\beta e^{-\alpha x*x}\f$ */
        GAUSSIAN = 2
    };

    /** Train options */
    enum TrainFlags {
        /** Update the network weights, rather than compute them from scratch. In the latter case
        the weights are initialized using the Nguyen-Widrow algorithm. */
        UPDATE_WEIGHTS = 1,
        /** Do not normalize the input vectors. If this flag is not set, the training algorithm
        normalizes each input feature independently, shifting its mean value to 0 and making the
        standard deviation equal to 1. If the network is assumed to be updated frequently, the new
        training data could be much different from original one. In this case, you should take care
        of proper normalization. */
        NO_INPUT_SCALE = 2,
        /** Do not normalize the output vectors. If the flag is not set, the training algorithm
        normalizes each output feature independently, by transforming it to the certain range
        depending on the used activation function. */
        NO_OUTPUT_SCALE = 4
    };

    virtual Mat getWeights(int layerIdx) const = 0;

    /** @brief Sets the new network parameters */
    virtual void setParams(const Params& p) = 0;

    /** @brief Retrieves the current network parameters */
    virtual Params getParams() const = 0;

    /** @brief Creates empty model

    Use StatModel::train to train the model, StatModel::train\<ANN_MLP\>(traindata, params) to
    create and train the model, StatModel::load\<ANN_MLP\>(filename) to load the pre-trained model.
    Note that the train method has optional flags: ANN_MLP::TrainFlags.
     */
    static Ptr<ANN_MLP> create(const Params& params=Params());
};

/****************************************************************************************\
*                           Logistic Regression                                          *
\****************************************************************************************/

/** @brief Implements Logistic Regression classifier.

@sa @ref ml_intro_lr
 */
class CV_EXPORTS LogisticRegression : public StatModel
{
public:
    class CV_EXPORTS Params
    {
    public:
        /** @brief Constructor */
        Params(double learning_rate = 0.001,
               int iters = 1000,
               int method = LogisticRegression::BATCH,
               int normalization = LogisticRegression::REG_L2,
               int reg = 1,
               int batch_size = 1);
        double alpha; //!< learning rate.
        int num_iters; //!< number of iterations.
        /** Kind of regularization to be applied. See LogisticRegression::RegKinds. */
        int norm;
        /** Enable or disable regularization. Set to positive integer (greater than zero) to enable
        and to 0 to disable. */
        int regularized;
        /** Kind of training method used. See LogisticRegression::Methods. */
        int train_method;
        /** Specifies the number of training samples taken in each step of Mini-Batch Gradient
        Descent. Will only be used if using LogisticRegression::MINI_BATCH training algorithm. It
        has to take values less than the total number of training samples. */
        int mini_batch_size;
        /** Termination criteria of the algorithm */
        TermCriteria term_crit;
    };

    //! Regularization kinds
    enum RegKinds {
        REG_L1 = 0, //!< %L1 norm
        REG_L2 = 1 //!< %L2 norm. Set Params::regularized \> 0 when using this kind
    };

    //! Training methods
    enum Methods {
        BATCH = 0,
        MINI_BATCH = 1 //!< Set Params::mini_batch_size to a positive integer when using this method.
    };

    /** @brief Predicts responses for input samples and returns a float type.

    @param samples The input data for the prediction algorithm. Matrix [m x n], where each row
        contains variables (features) of one object being classified. Should have data type CV_32F.
    @param results Predicted labels as a column matrix of type CV_32S.
    @param flags Not used.
     */
    virtual float predict( InputArray samples, OutputArray results=noArray(), int flags=0 ) const = 0;

    /** @brief This function returns the trained paramters arranged across rows.

    For a two class classifcation problem, it returns a row matrix. It returns learnt paramters of
    the Logistic Regression as a matrix of type CV_32F.
     */
    virtual Mat get_learnt_thetas() const = 0;

    /** @brief Creates empty model.

    @param params The training parameters for the classifier of type LogisticRegression::Params.

    Creates Logistic Regression model with parameters given.
     */
    static Ptr<LogisticRegression> create( const Params& params = Params() );
};

/****************************************************************************************\
*                           Auxilary functions declarations                              *
\****************************************************************************************/

/** @brief Generates _sample_ from multivariate normal distribution

@param mean an average row vector
@param cov symmetric covariation matrix
@param nsamples returned samples count
@param samples returned samples array
*/
CV_EXPORTS void randMVNormal( InputArray mean, InputArray cov, int nsamples, OutputArray samples);

/** @brief Generates sample from gaussian mixture distribution */
CV_EXPORTS void randGaussMixture( InputArray means, InputArray covs, InputArray weights,
                                  int nsamples, OutputArray samples, OutputArray sampClasses );

/** @brief Creates test set */
CV_EXPORTS void createConcentricSpheresTestSet( int nsamples, int nfeatures, int nclasses,
                                                OutputArray samples, OutputArray responses);

//! @} ml

}
}

#endif // __cplusplus
#endif // __OPENCV_ML_HPP__

/* End of file. */
