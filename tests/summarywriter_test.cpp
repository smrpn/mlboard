/**
 * @file tests/summarywriter_test.cpp
 * @author Jeffin Sam
 */
#include "catch.hpp"
#include <mlboard/mlboard.hpp>
#include <mlpack/methods/ann/ffn.hpp>
#include <mlpack/methods/ann/loss_functions/mean_squared_error.hpp>
#include <mlpack/methods/logistic_regression/logistic_regression.hpp>
#include <sstream>
#include <cstdio>
#include <sys/stat.h>

// For windows mkdir.
#ifdef _WIN32
    #include <direct.h>
#endif

using namespace mlpack::ann;
using namespace mlpack;
using namespace mlpack::regression;

class SummaryWriterTestsFixture 
{
 public:
  static mlboard::FileWriter* f1;
  static size_t currentSize;
  static bool deleteLogs;
  SummaryWriterTestsFixture()
  {
    if(deleteLogs)
    {
      #if defined(_WIN32)
        _mkdir("_templogs");
      #else
        mkdir("_templogs", 0777);
      #endif
      deleteLogs = false;
    }
  };  
};

mlboard::FileWriter* SummaryWriterTestsFixture::f1;
size_t SummaryWriterTestsFixture::currentSize = 0;
bool SummaryWriterTestsFixture::deleteLogs = true;

/**
 * Test the Image summary.
 */
TEST_CASE_METHOD(SummaryWriterTestsFixture, "Writing a Image summary to file", "[SummaryWriter]")
{	
  f1 = new mlboard::FileWriter("_templogs");
  std::ifstream fin("./data/multiple_image.jpg", std::ios::binary);
  std::ostringstream ss;

  ss << fin.rdbuf();
  std::string image(ss.str());
  fin.close();
  mlboard::SummaryWriter<mlboard::FileWriter>::Image(
      "Test Image", 1, image, 512, 512, 3, *f1, "Sample Image",
      "This is a Sample image logged using mlboard.");
}

/**
 * Test the scaler summary.
 */
TEST_CASE_METHOD(SummaryWriterTestsFixture, "Writing a scaler summary to file",
                 "[SummaryWriter]")
{	
  for (int i = 1; i < 25; i++)
  {
    mlboard::SummaryWriter<mlboard::FileWriter>::Scalar("scalerSample_1",
      i, 1.1, *f1);
  }
}

/**
 * Test the PRCurve summary.
 */
TEST_CASE_METHOD(SummaryWriterTestsFixture, "Writing a PrCurve summary to file",
                 "[SummaryWriter]")
{
  std::vector<double> labels = {1, 1, 1, 1, 1, 1, 1, 1, 0, 1};
  std::vector<double> predictions = {0.6458941, 0.3843817, 0.4375872,
      0.2975346, 0.891773, 0.05671298, 0.96366274, 0.2726563,
      0.3834415, 0.47766513};
  mlboard::SummaryWriter<mlboard::FileWriter>::PRCurve("test_pr_curve",
      labels, predictions, *f1);
}

/**
 * Test the PRCurve summary using arma vec.
 */
TEST_CASE_METHOD(SummaryWriterTestsFixture,
    "Writing a PrCurve summary using arma vec file", "[SummaryWriter]")
{
  arma::rowvec labels = {1, 1, 1, 1, 1, 1, 1, 1, 0, 1};
  arma::rowvec predictions = {0.6458941, 0.3843817, 0.4375872,
      0.2975346, 0.891773, 0.05671298, 0.96366274, 0.2726563,
      0.3834415, 0.47766513};
  mlboard::SummaryWriter<mlboard::FileWriter>::PRCurve("test_pr_curve_arma_vec",
      labels, predictions, *f1);
}

/**
 * Test text summary.
 */
TEST_CASE_METHOD(SummaryWriterTestsFixture, "Writing text summary to file",
                 "[SummaryWriter]")
{	
  mlboard::SummaryWriter<mlboard::FileWriter>::Text("add Text support ", 1,
      "Test case for checking text support in mlboard.", *f1);
  mlboard::SummaryWriter<mlboard::FileWriter>::Text("add Text support", 2,
      " Project developed during GSoc 2020 ", *f1);
}

/**
 * Test text summary.
 */
TEST_CASE_METHOD(SummaryWriterTestsFixture, "Writing summary using callback to file",
                 "[SummaryWriter]")
{	
  arma::mat data;
  arma::mat labels;

  data::Load("./data/lab1.csv", data, true);
  data::Load("./data/lab3.csv", labels, true);

  FFN<MeanSquaredError<>, RandomInitialization> model;

  model.Add<Linear<>>(1, 2);
  model.Add<SigmoidLayer<>>();
  model.Add<Linear<>>(2, 1);
  model.Add<SigmoidLayer<>>();

  std::stringstream stream;
  ens::StandardSGD opt(0.1, 1, 1000);
  model.Train(data, labels, opt, ens::MlboardLogger(*f1));
}

/**
 * Test text summary.
 */
TEST_CASE_METHOD(SummaryWriterTestsFixture, "Writing summary using second constructor callback to file",
                 "[SummaryWriter]")
{	
  arma::mat data("1 2 3;"
                 "1 2 3");
  arma::Row<size_t> responses("1 1 0");

  ens::StandardSGD sgd(0.1, 1, 50);
  LogisticRegression<> logisticRegression(data, responses, sgd, 0.001);
  std::stringstream stream;

  ens::MlboardLogger cb(*f1, 
        [&]()
      {
        return logisticRegression.ComputeAccuracy(data, responses)/100;
      },
      "lraccuracy","lrloss"
  );
  // Now train a logistic regression object on it.
  logisticRegression.Train<ens::StandardSGD>(data, responses, sgd,
                                             cb);
}

/**
 * Test multiple Image summary.
 */
TEST_CASE_METHOD(SummaryWriterTestsFixture, "Writing multiple Images summary to file",
                 "[SummaryWriter]")
{	
  deleteLogs = true;
  arma::Mat<unsigned char> matrix;
  mlpack::data::ImageInfo info;
  std::vector<std::string> files = {"./data/single_image.jpg",
      "./data/single_image.jpg"};

  // Creating the matrix which has image.
  mlpack::data::Load(files, matrix, info, false);

  // Now we can log the matrix.
  mlboard::SummaryWriter<mlboard::FileWriter>::Image(
      "Multiple Image", 1, matrix, info, *f1, "Sample Multiple Image",
      "This is a Sample multiple image logged using mlboard.");
  f1->Close();

  #ifndef KEEP_TEST_LOGS
    remove(f1->FileName().c_str());
  #endif
}
