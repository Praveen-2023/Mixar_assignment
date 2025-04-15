#pragma once

#include "Node.h"
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QCheckBox>
#include <random>

enum class NoiseType {
    Perlin,
    Simplex,
    Worley,
    Value,
    White
};

enum class NoiseOutputMode {
    Grayscale,
    Displacement,
    Normal
};

class NoiseGenerationNode : public Node {
public:
    NoiseGenerationNode();
    ~NoiseGenerationNode();

    // Node interface implementation
    void process() override;
    QWidget* createPropertiesWidget() override;
    bool isReady() const override;

    // Getters and setters
    NoiseType getNoiseType() const;
    void setNoiseType(NoiseType type);

    NoiseOutputMode getOutputMode() const;
    void setOutputMode(NoiseOutputMode mode);

    int getWidth() const;
    void setWidth(int width);

    int getHeight() const;
    void setHeight(int height);

    double getScale() const;
    void setScale(double scale);

    int getOctaves() const;
    void setOctaves(int octaves);

    double getPersistence() const;
    void setPersistence(double persistence);

    int getSeed() const;
    void setSeed(int seed);

private:
    // Processing parameters
    NoiseType noiseType_;
    NoiseOutputMode outputMode_;
    int width_;
    int height_;
    double scale_;
    int octaves_;
    double persistence_;
    int seed_;

    // Random number generator
    std::mt19937 rng_;

    // UI components
    QWidget* propertiesWidget_;
    QComboBox* noiseTypeComboBox_;
    QComboBox* outputModeComboBox_;
    QSpinBox* widthSpinBox_;
    QSpinBox* heightSpinBox_;
    QSlider* scaleSlider_;
    QLabel* scaleLabel_;
    QSlider* octavesSlider_;
    QLabel* octavesLabel_;
    QSlider* persistenceSlider_;
    QLabel* persistenceLabel_;
    QSpinBox* seedSpinBox_;
    QPushButton* randomSeedButton_;

    // Helper methods
    cv::Mat generateNoise();
    cv::Mat generatePerlinNoise();
    cv::Mat generateSimplexNoise();
    cv::Mat generateWorleyNoise();
    cv::Mat generateValueNoise();
    cv::Mat generateWhiteNoise();
    cv::Mat applyOutputMode(const cv::Mat& noise);

    // Noise generation helpers
    double perlinNoise(double x, double y);
    double simplexNoise(double x, double y);
    double worleyNoise(double x, double y);
    double valueNoise(double x, double y);
    double interpolate(double a, double b, double t);
    double fade(double t);
    double grad(int hash, double x, double y, double z);
};
