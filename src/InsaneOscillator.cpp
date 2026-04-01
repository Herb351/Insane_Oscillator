#include "sierrachart.h"

SCDLLName("InsaneOscillator")

SCSFExport void scsf_InsaneOscillator(SCStudyInterfaceRef sc) {
    sc.AutoLoop = 1;
    sc.UpdateAlways = 0;

    // Inputs for RSI Length/OB/OS
    int rsiLength = 14;
    float overbought = 70.0f;
    float oversold = 30.0f;

    // Subgraphs
    SCSubgraphRef rsi = sc.Subgraph[0];
    SCSubgraphRef midline = sc.Subgraph[1];
    SCSubgraphRef upRMA = sc.Subgraph[2];
    SCSubgraphRef downRMA = sc.Subgraph[3];

    // Initialize subgraph properties
    rsi.Name = "RSI";
    midline.Name = "Midline";
    upRMA.Name = "UpRMA";
    downRMA.Name = "DownRMA";

    // Set the graph region to 1
    rsi.GraphRegion = 1;
    midline.GraphRegion = 1;
    upRMA.GraphRegion = 1;
    downRMA.GraphRegion = 1;

    // Marker gating placeholder
    bool barClosed = (sc.Index < sc.ArraySize - 1);

    // Implement Pine-exact RSI
    if (sc.Index >= rsiLength) {
        float sum = 0;
        for (int i = 1; i <= rsiLength; ++i) {
            sum += sc.Close[sc.Index - i];
        }
        float seedSMA = sum / rsiLength;
        float prevRMA = seedSMA;
        for (int i = rsiLength; i <= sc.Index; ++i) {
            float x = sc.Close[i];
            if (i > rsiLength) {
                prevRMA = (prevRMA * (rsiLength - 1) + x) / rsiLength;
            }
            rsi[i] = (prevRMA);
        }
    }

    // Pine guards for RMA
    upRMA[sc.Index] = (downRMA[sc.Index] == 0) ? 0 : upRMA[sc.Index];
    downRMA[sc.Index] = (upRMA[sc.Index] == 0) ? 100 : downRMA[sc.Index];
}