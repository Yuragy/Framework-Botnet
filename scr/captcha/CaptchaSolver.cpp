#include "CaptchaSolver.h"
#include "Logger.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <opencv2/imgproc.hpp>

CaptchaSolver::CaptchaSolver(const std::string &modelPath, const std::string &inputOp, const std::string &outputOp) 
    : model(TF_NewGraph(), TF_DeleteGraph), 
      session(nullptr, TF_DeleteSession), 
      status(TF_NewStatus(), TF_DeleteStatus), 
      inputOperation(inputOp), 
      outputOperation(outputOp),
      resizeWidth(128), 
      resizeHeight(128), 
      blurKernelSize(3), 
      threshold(1.0) {
    loadModel(modelPath);
}

CaptchaSolver::~CaptchaSolver() {}

void CaptchaSolver::loadModel(const std::string &modelPath) {
    TF_SessionOptions* sess_opts = TF_NewSessionOptions();
    session.reset(TF_NewSession(model.get(), sess_opts, status.get()));
    TF_DeleteSessionOptions(sess_opts);

    if (TF_GetCode(status.get()) != TF_OK) {
        logTensorFlowError(status.get());
        return;
    }

    std::ifstream file(modelPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        Logger::log(Logger::ERROR, "Failed to open model file: " + modelPath);
        return;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);
    if (file.read(buffer.data(), size)) {
        TF_Buffer* graph_def = TF_NewBufferFromString(buffer.data(), size);
        TF_ImportGraphDefOptions* import_opts = TF_NewImportGraphDefOptions();
        TF_GraphImportGraphDef(model.get(), graph_def, import_opts, status.get());
        if (TF_GetCode(status.get()) != TF_OK) {
            logTensorFlowError(status.get());
        }
        TF_DeleteBuffer(graph_def);
        TF_DeleteImportGraphDefOptions(import_opts);
    } else {
        Logger::log(Logger::ERROR, "Failed to read model file: " + modelPath);
    }
}

cv::Mat CaptchaSolver::preprocessImage(const cv::Mat &img) {
    cv::Mat processedImg;
    cv::resize(img, processedImg, cv::Size(resizeWidth, resizeHeight));
    processedImg.convertTo(processedImg, CV_32FC1, 1.0 / 255.0);

    cv::GaussianBlur(processedImg, processedImg, cv::Size(blurKernelSize, blurKernelSize), 0);
    cv::threshold(processedImg, processedImg, 0, 1, cv::THRESH_BINARY + cv::THRESH_OTSU);

    return processedImg;
}

void CaptchaSolver::logTensorFlowError(TF_Status* status) {
    Logger::log(Logger::ERROR, "TensorFlow error: " + std::string(TF_Message(status)));
}

std::vector<std::string> CaptchaSolver::decodeOutput(TF_Tensor* output_tensor) {
    auto output_data = static_cast<float*>(TF_TensorData(output_tensor));
    std::vector<std::string> decoded_captcha;
    for (int i = 0; i < TF_TensorElementCount(output_tensor); ++i) {
        decoded_captcha.push_back(std::to_string(static_cast<int>(output_data[i])));
    }
    return decoded_captcha;
}

std::string CaptchaSolver::solveCaptcha(const std::string &captchaImage) {
    Logger::log(Logger::INFO, "Solving captcha");

    cv::Mat img = cv::imread(captchaImage, cv::IMREAD_GRAYSCALE);
    if (img.empty()) {
        throw std::runtime_error("Failed to load captcha image: " + captchaImage);
    }

    cv::Mat processedImg = preprocessImage(img);

    const int64_t dims[4] = {1, processedImg.rows, processedImg.cols, 1};
    TF_Tensor* input_tensor = TF_NewTensor(TF_FLOAT, dims, 4, processedImg.data, processedImg.total() * processedImg.elemSize(), nullptr, nullptr);
    if (!input_tensor) {
        throw std::runtime_error("Failed to create input tensor");
    }

    TF_Output input_op = {TF_GraphOperationByName(model.get(), inputOperation.c_str()), 0};
    TF_Output output_op = {TF_GraphOperationByName(model.get(), outputOperation.c_str()), 0};
    TF_Tensor* output_tensors[1] = {nullptr};

    TF_SessionRun(session.get(), nullptr,
                  &input_op, &input_tensor, 1,
                  &output_op, output_tensors, 1,
                  nullptr, 0, nullptr, status.get());

    if (TF_GetCode(status.get()) != TF_OK) {
        logTensorFlowError(status.get());
        TF_DeleteTensor(input_tensor);
        return "";
    }

    std::vector<std::string> decoded_captcha = decodeOutput(output_tensors[0]);

    TF_DeleteTensor(input_tensor);
    TF_DeleteTensor(output_tensors[0]);

    std::string result;
    for (const auto& ch : decoded_captcha) {
        result += ch;
    }

    return result;
}

void CaptchaSolver::setPreprocessingParams(int resizeWidth, int resizeHeight, int blurKernelSize, double threshold) {
    this->resizeWidth = resizeWidth;
    this->resizeHeight = resizeHeight;
    this->blurKernelSize = blurKernelSize;
    this->threshold = threshold;
}



