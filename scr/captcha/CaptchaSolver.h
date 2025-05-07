#ifndef CAPTCHASOLVER_H
#define CAPTCHASOLVER_H

#include <string>
#include <tensorflow/c/c_api.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>

class CaptchaSolver {
public:
    CaptchaSolver(const std::string &modelPath, const std::string &inputOp = "input_layer", const std::string &outputOp = "output_layer");
    ~CaptchaSolver();

    std::string solveCaptcha(const std::string &captchaImage);

    void setPreprocessingParams(int resizeWidth, int resizeHeight, int blurKernelSize, double threshold);

private:
    void loadModel(const std::string &modelPath);
    cv::Mat preprocessImage(const cv::Mat &img);
    void logTensorFlowError(TF_Status* status);
    std::vector<std::string> decodeOutput(TF_Tensor* output_tensor);

    std::unique_ptr<TF_Graph, decltype(&TF_DeleteGraph)> model;
    std::unique_ptr<TF_Session, decltype(&TF_DeleteSession)> session;
    std::unique_ptr<TF_Status, decltype(&TF_DeleteStatus)> status;
    std::string inputOperation;
    std::string outputOperation;

    int resizeWidth;
    int resizeHeight;
    int blurKernelSize;
    double threshold;
};

#endif // CAPTCHASOLVER_H



