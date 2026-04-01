// InsaneOscillator.cpp

#include <iostream>

// Inputs
input int RSI_Length = 14;
input double RSI_Overbought = 70.0;
input double RSI_Oversold = 30.0;
input int Squeeze_Threshold = 20;

// Subgraphs
double RSI_Line;
double RSI_Midline;
double TrampolineUpT;
double TrampolineDownT;
double SqueezeBuyDiamond;
double SqueezeSellDiamond;

// Persistent variables
persistent double cRed = 0;
persistent double cGreen = 0;
persistent bool pos = false;
persistent bool neg = false;

// Calculate the Exact Pine RSI (RMA)
double ExactPineRSI(double input[], int length) {
    double sumGain = 0.0;
    double sumLoss = 0.0;
    for(int i = 1; i < length; i++) {
        double change = input[i] - input[i - 1];
        if(change > 0) sumGain += change;
        else sumLoss -= change;
    }
    double avgGain = sumGain / length;
    double avgLoss = sumLoss / length;
    return (avgLoss == 0) ? 100 : 100 - (100 / (1 + (avgGain / avgLoss)));
}

void OnCalculate() {
    // Your logic to calculate RSI, markers, and other functionalities goes here.
    // Trigger markers on closed bars only (similar to barstate.isconfirmed)
    if(/* condition for closed bar */) {
        // Trigger logic for markers
    }
}