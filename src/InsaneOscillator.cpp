#include "sierrachart.h"

SCDLLName("InsaneOscillator");

// Subgraphs
SCSubgraph RSI;
SCSubgraph Midline;
SCSubgraph UpRMA;
SCSubgraph DownRMA;

// Input
int Length = 14;

void scsf_InsaneOscillator(SCStudyInterfaceRef sc)
{
    // Set up RSI and Midline subgraphs
    RSI.Name = "RSI";
    Midline.Name = "Midline";

    // Initialization
    if (sc.SetDefaults)
    {
        sc.GraphName = "Insane Oscillator";
        sc.StudyDescription = "Insane Oscillator using Wilder RMA";
        sc.AutoLoop = 0; // Disable default AutoLoop
        return;
    }

    // Compute the gains and losses
    float Gain = 0;
    float Loss = 0;

    // Calculate SMA for the first 'Length' gains/losses
    if (sc.Index < Length) return; // Ensure we have enough data

    for (int i = sc.Index - Length + 1; i <= sc.Index; i++)
    {
        float change = sc.Close[i] - sc.Close[i - 1];
        if (change > 0) Gain += change;
        else Loss -= change;
    }

    Gain /= Length;
    Loss /= Length;

    // Recursive RMA calculation
    UpRMA[sc.Index] = (Gain * (1.0 / Length)) + (UpRMA[sc.Index - 1] * (Length - 1) / Length);
    DownRMA[sc.Index] = (Loss * (1.0 / Length)) + (DownRMA[sc.Index - 1] * (Length - 1) / Length);

    // Calculate RSI
    if (DownRMA[sc.Index] != 0)
        RSI[sc.Index] = 100 - (100 / (1 + (UpRMA[sc.Index] / DownRMA[sc.Index])));
    else
        RSI[sc.Index] = 100; // Maximum RSI when DownRMA is zero

    // Set midline
    Midline[sc.Index] = 50;

    // Placeholder for bar-close-only marker gating
    // Markers not yet implemented
}