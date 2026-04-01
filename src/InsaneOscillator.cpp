#include "sierrachart.h"

// Define the DLL name
#define SCDLLName InsaneOscillator

// Implement the study function
SCSFExport void scsf_InsaneOscillator(SCStudyInterfaceRef sc)
{
    // Auto loop and update always
    sc.AutoLoop = 1;
    sc.UpdateAlways = 0;

    // Inputs for RSI length, source, and overbought/oversold levels
    SCInputRef rsiLength = sc.Input[0];
    SCInputRef rsiSource = sc.Input[1];
    SCInputRef rsiOB = sc.Input[2];
    SCInputRef rsiOS = sc.Input[3];

    // Pine-exact RSI calculations
    if (sc.SetDefaults)
    {
        sc.GraphName = "Insane Oscillator";
        rsiLength.Name = "RSI Length";
        rsiSource.Name = "RSI Source";
        rsiOB.Name = "RSI Overbought";
        rsiOS.Name = "RSI Oversold";
        rsiLength.SetInt(14);
        rsiOB.SetFloat(70);
        rsiOS.SetFloat(30);
        return;
    }

    // Calculate RSI using RMA/Wilder smoothing
    float gain = 0, loss = 0;
    for (int i = 1; i <= rsiLength; ++i)
    {
        float change = sc.Close[i] - sc.Close[i - 1];
        gain += (change > 0) ? change : 0;
        loss += (change < 0) ? -change : 0;
    }
    gain /= rsiLength;
    loss /= rsiLength;

    // Wilder recursion
    for (int i = rsiLength; i < sc.ArraySize; ++i)
    {
        gain = (gain * (rsiLength - 1) + (sc.Close[i] - sc.Close[i - 1]) * (sc.Close[i] - sc.Close[i - 1] > 0 ? 1 : 0)) / rsiLength;
        loss = (loss * (rsiLength - 1) + (sc.Close[i] - sc.Close[i - 1]) * (sc.Close[i] - sc.Close[i - 1] < 0 ? 1 : 0)) / rsiLength;
    }

    // Calculate RSI
    float rsi = 100 - (100 / (1 + (gain / loss)));
    sc.Subgraph[0].Data[sc.Index] = rsi;
    sc.Subgraph[1].Data[sc.Index] = 50; // Midline

    // Placeholder for markers
    // TODO: Implement Trampoline markers and Squeeze markers

    // Enforce bar-close-only signals
    if (sc.Index < sc.ArraySize - 1)
    {
        // Plot markers only if the current index is not the last bar
        // TODO: Add conditions for markers here
    }

    // Keep RSI computed for all bars
    // Comments mapping to Pine lines 62-64 and barstate.isconfirmed
}
