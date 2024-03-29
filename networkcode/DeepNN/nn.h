#include <iostream>
#include <vector>
#include <cmath>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <iostream>
#include <cstdlib>

class NNLevel {
    public:
        NNLevel(int inputCount, int outpuCount);
        std::vector<double> FeedForward(std::vector<double> *neuralNetworkInputValues);
        double sigmoid(double x);
        double relu(double x);
        std::vector<std::vector<double>> weights;
        std::vector<double> biases;
        std::vector<double> levelNeuronOutputValue, networkNetValues;
        int levelInputCount, levelOutputCount;
    private:
};

// generate weights and biases for level according to input
NNLevel::NNLevel(int inputCount, int outpuCount) {
    srand(time(NULL));
    levelInputCount = inputCount;
    levelOutputCount = outpuCount;
    for (int i=0; i < outpuCount; i++) {
        std::vector<double> neuronWeights;
        for (int j=0; j < inputCount; j++) {
            double randval = rand() % 100;
            double randomdouble = randval/100;
            int negativeOrPositive = rand() % 2;
            if (negativeOrPositive == 0){
                neuronWeights.push_back(randomdouble);
            } else {
                randomdouble = -randomdouble;
                neuronWeights.push_back(randomdouble);
            }
        }
        weights.push_back(neuronWeights);
        double randval = rand() % 100;
        double randomdouble = randval/100;
        biases.push_back(0.7);
    }
}

// return an output and sets variable by using the input and calculating the output by using the weights an biases
std::vector<double> NNLevel::FeedForward(std::vector<double> *neuralNetworkInputValues) {
    levelNeuronOutputValue.clear();
    networkNetValues.clear();
    for (int i=0; i < levelOutputCount; i++) {
        double neuronValue, neuronValueSigmoid;
        neuronValue = 0;
        for (int j=0; j < levelInputCount; j++) {
            neuronValue += weights.at(i).at(j) * neuralNetworkInputValues->at(j);
        }
        networkNetValues.push_back(neuronValue - biases.at(i));
        neuronValueSigmoid = relu(neuronValue - biases.at(i)); // uses relu as activation function
        levelNeuronOutputValue.push_back(neuronValueSigmoid);
    }
    return levelNeuronOutputValue;
}

double NNLevel::sigmoid(double x) { // sigmoid function returns an value between 0 and 1
    double e = exp(-x);
    double value = 1 / (1 + e);
    return value;
}

double NNLevel::relu(double x) {
    if (x > 0) {
        return x;
    } else {
        // return 0.001*x;
        return 0;
    }
}

class NeuralNetwork {
    public:
        NeuralNetwork(std::vector<int> neuralNetworkLevels = {16, 6, 4});
        std::vector<double> feedForward(std::vector<double> networkInputValues);
        void GradientError(std::vector<double> target);
        std::vector<double> propagate(std::vector<double>* inputs, std::vector<double>* outputValues, std::vector<double>* targets, NNLevel* level, double* learningRate);
        std::vector<double> propagate2(std::vector<double>* inputs, std::vector<double>* outputValues, std::vector<double>* targets, NNLevel* level, double* learningRate);
        void backPropogation(std::vector<double>* newTargets, std::vector<double>* input);
        double derivativeRelu(double x);
        void saveNeuralNetwork();
        void loadNeuralNetwork(nlohmann::json networkData);
        std::vector<NNLevel> levels;
        std::vector<double> networkOutput;
        std::vector<double> networkError;
        std::vector<std::vector<std::vector<double>>> newWeigths, newWeightsUpdate;
        std::vector<std::vector<double>> newBiases, newBiasesUpdate;
        double errorSum, timesRun;
        std::vector<double> networkInput;
    private:
};

// generates neuralnetwork layered according to the input vector
NeuralNetwork::NeuralNetwork(std::vector<int> neuralNetworkLevels) {
    for (int i=0; i < neuralNetworkLevels.size()-1; i++) {
        NNLevel level(neuralNetworkLevels.at(i), neuralNetworkLevels.at(i+1));
        levels.push_back(level);
    }
}

// get input and calculates an output using the feedforward function of the levels
std::vector<double> NeuralNetwork::feedForward(std::vector<double> networkInputValues) {
    networkInput = networkInputValues;
    std::vector<double> neuralNetworkInputValues = networkInputValues;
    for (int i=0; i < levels.size(); i++) {
        neuralNetworkInputValues = levels.at(i).FeedForward(&neuralNetworkInputValues);
    }
    networkOutput = neuralNetworkInputValues;
    return networkOutput;
}

// calculates the diference between the output and the expected/target output and updates the errorSum variable
void NeuralNetwork::GradientError(std::vector<double> target) {
    errorSum = 0;
    for (int i=0; i < networkOutput.size(); i++) {
        double error = 0.5 * pow((target.at(i) - networkOutput.at(i)), 2);
        networkError.push_back(error);
        errorSum += error;
    }
}

double derivativeSigmoid(double Zj) {
    double sigmoidDerivative, sigmoid1, sigmoid2;
    sigmoid1 = 1 /(1+exp(-Zj));
    sigmoid2 = 1 - (1/(1+exp(-Zj)));
    sigmoidDerivative = sigmoid1 * sigmoid2;
    return sigmoidDerivative;
}

double NeuralNetwork::derivativeRelu(double x) {
    if (x > 0) {
        return 1;
    } else {
        return 0;
    }
}

std::vector<double> NeuralNetwork::propagate(std::vector<double>* inputs, std::vector<double>* outputValues, std::vector<double>* targets, NNLevel* level, double* learningRate) {
    std::vector<std::vector<double>> newWeightValues;
    std::vector<double> Zj;
    for (int i=0; i < outputValues->size(); i++) {
        double ZjValue;
        for (int j=0; j < inputs->size(); j++) {
            ZjValue += inputs->at(j) * level->weights.at(i).at(j);
        }
        ZjValue += level->biases.at(i);
        Zj.push_back(ZjValue);
    }

    std::vector<std::vector<double>> weightGradients;
    for (int i=0; i < outputValues->size(); i++) { // calculates the gradient of the weights by using the chainrule and derivatives
        double dATOdZ = derivativeRelu(Zj.at(i)); // derivative A / derivative Z

        std::vector<double> weightGradient;
        for (int j=0; j < level->weights.at(i).size(); j++) {
            double a = inputs->at(j);
            double Aj = (level->levelNeuronOutputValue.at(i) - targets->at(i));

            double costOverWeightRatio = a * Aj * dATOdZ;
            weightGradient.push_back(costOverWeightRatio);
        }
        weightGradients.push_back(weightGradient);
    }
    newWeightsUpdate.insert(newWeightsUpdate.begin(), weightGradients);

    std::vector<double> biasGradients;
    for (int i=0; i < outputValues->size(); i++) {
        double dATOdZ = derivativeRelu(Zj.at(i)); // derivative A / derivative Z
        double Aj = (level->levelNeuronOutputValue.at(i) - targets->at(i));

        double costOverWeightRatio = 1 * Aj * dATOdZ;
        biasGradients.push_back(costOverWeightRatio);
    }
    newBiasesUpdate.insert(newBiasesUpdate.begin(), biasGradients);

    std::vector<double> nextLayerTarget;
    for (int i=0; i < inputs->size(); i++) { // calculates the target for the next network
        double derivative;
        for (int j=0; j < outputValues->size(); j++) {
            double w = level->weights.at(j).at(i);
            double a  = derivativeRelu(Zj.at(j));
            double Co = (outputValues->at(j) - targets->at(j));
            double v = w * a * Co;
            derivative += v;
        }
        nextLayerTarget.push_back(derivative);
    }

    for (int i=0; i < outputValues->size(); i++) {
        for (int j=0; j < level->weights.at(i).size(); j++) {
            double newWeight = level->weights.at(i).at(j) + (*learningRate * weightGradients.at(i).at(j));
            level->weights.at(i).at(j) = newWeight;
        }
        double newBias = level->biases.at(i) + (*learningRate * biasGradients.at(i));
        level->biases.at(i) = newBias;
    }
    
    std::vector<double> costs;
    for (int i=0; i < outputValues->size(); i++) {
        double value = pow((outputValues->at(i)-targets->at(i)),2);
        costs.push_back(value);
    }

    return nextLayerTarget;
}

std::vector<double> NeuralNetwork::propagate2(std::vector<double>* inputs, std::vector<double>* outputValues, std::vector<double>* targets, NNLevel* level, double* learningRate) {
    std::vector<std::vector<double>> newWeightValues;
    std::vector<double> Zj, costs;

    for (int i=0; i < outputValues->size(); i++) {
        double ZjValue;
        for (int j=0; j < inputs->size(); j++) {
            ZjValue += inputs->at(j) * level->weights.at(i).at(j);
        }
        ZjValue += level->biases.at(i);
        Zj.push_back(ZjValue);
    }
    
    std::vector<std::vector<double>> weightGradients;
    for (int i=0; i < outputValues->size(); i++) {
        double Aj = targets->at(i);
        double dATOdZ = derivativeRelu(Zj.at(i)); // derivative A / derivative Z

        std::vector<double> weightGradient;
        for (int j=0; j < level->weights.at(i).size(); j++) {
            double a = inputs->at(j);

            double costOverWeightRatio = a * Aj * dATOdZ;
            weightGradient.push_back(costOverWeightRatio);
        }
        weightGradients.push_back(weightGradient);
    }
    newWeightsUpdate.insert(newWeightsUpdate.begin(), weightGradients);
    std::vector<double> biasGradients;
    for (int i=0; i < outputValues->size(); i++) {
        double dATOdZ = derivativeRelu(Zj.at(i)); // derivative A / derivative Z
        double Aj = targets->at(i);

        double costOverWeightRatio = 1 * Aj * dATOdZ;
        biasGradients.push_back(costOverWeightRatio);
    }
    newBiasesUpdate.insert(newBiasesUpdate.begin(), biasGradients);

    std::vector<double> nextLayerTarget;
    for (int i=0; i < inputs->size(); i++) {
        double derivative;
        for (int j=0; j < outputValues->size(); j++) {
            double w = level->weights.at(j).at(i);
            double a  = derivativeRelu(Zj.at(j));
            double Co = targets->at(j);
            double v = w * a * Co;
            derivative += v;
        }
        nextLayerTarget.push_back(derivative);
    }

    std::vector<double> weights;
    for (int i=0; i < outputValues->size(); i++) {
        for (int j=0; j < level->weights.at(i).size(); j++) {
            double newWeight = level->weights.at(i).at(j) + (*learningRate * weightGradients.at(i).at(j));
            level->weights.at(i).at(j) = newWeight;
        }

        double newBias = level->biases.at(i) + (*learningRate * biasGradients.at(i));
        level->biases.at(i) = newBias;

    }
    
    return nextLayerTarget;
}

void NeuralNetwork::backPropogation(std::vector<double>* newTargets, std::vector<double>* input) {
    std::vector<double> targets;
    for (int i=0; i < newTargets->size(); i++) {
        targets.push_back(newTargets->at(i));
    }

    feedForward(*input);
    double learningRate = 0.1;

    // the loop starts at levels.size()-1
    // the propagate function is different from the propagate2 function because the calculations 
    //for the nextlayertarget and gardients of the weight and biases are diferent 
    for (int i=levels.size()-1; -1 < i; i--) { // loop backwards trough te levels for backpropagation
        std::vector<double>* targetsPtr = new std::vector<double>(targets);
        if (i == 0) {
            targets = propagate2(input, &levels.at(i).levelNeuronOutputValue, targetsPtr, &levels.at(i), &learningRate);
        } else if (i == levels.size()-1) {
            targets = propagate(&levels.at(i-1).levelNeuronOutputValue, &levels.at(i).levelNeuronOutputValue, targetsPtr, &levels.at(i), &learningRate);
        } else {
            targets = propagate2(&levels.at(i-1).levelNeuronOutputValue, &levels.at(i).levelNeuronOutputValue, targetsPtr, &levels.at(i), &learningRate);
        }
        delete targetsPtr;
        targetsPtr = NULL;
    }
}

void NeuralNetwork::saveNeuralNetwork() {
    nlohmann::json jsonfile;

    jsonfile["weights"]["lenght"] = levels.size();
    jsonfile["biases"]["lenght"] = levels.size();

    for (int i=0; i < levels.size(); i++) {
        jsonfile["weights"][std::to_string(i)]["lenght"] = levels.at(i).weights.size();
        for (int j=0; j < levels.at(i).weights.size(); j++) {
            jsonfile["weights"][std::to_string(i)][std::to_string(j)]["lenght"] = levels.at(i).weights.at(j).size();
            for (int k=0; k < levels.at(i).weights.at(j).size(); k++) {
                jsonfile["weights"][std::to_string(i)][std::to_string(j)][std::to_string(k)] = levels.at(i).weights.at(j).at(k);
            }
        }
        jsonfile["biases"][std::to_string(i)]["lenght"] = levels.at(i).biases.size();
        for (int j=0; j < levels.at(i).biases.size(); j++) {
            jsonfile["biases"][std::to_string(i)][std::to_string(j)] = levels.at(i).biases.at(j);
        }
    }

    std::ofstream testfile;
    testfile.open ("NeuralNetworks/NN.json");
    testfile << jsonfile;
    testfile.close();
}

void NeuralNetwork::loadNeuralNetwork(nlohmann::json networkData) {    
    int lenght1 = networkData["weights"]["lenght"].get<int>();
    int lenght2 = networkData["biases"]["lenght"].get<int>();

    for (int i=0; i < lenght1; i++) {
        int lenghti = networkData["weights"][std::to_string(i)]["lenght"].get<int>();
        for (int j=0; j < lenghti; j++) {
            int lenghtj = networkData["weights"][std::to_string(i)][std::to_string(j)]["lenght"].get<int>();
            for (int k=0; k < lenghtj; k++) {
                levels.at(i).weights.at(j).at(k) = networkData["weights"][std::to_string(i)][std::to_string(j)][std::to_string(k)].get<double>();
            }
        }
        int lenghti2 = networkData["biases"][std::to_string(i)]["lenght"].get<int>();

        for (int j=0; j < lenghti2; j++) {
            levels.at(i).biases.at(j) = networkData["biases"][std::to_string(i)][std::to_string(j)].get<double>();
        }
    }
}
