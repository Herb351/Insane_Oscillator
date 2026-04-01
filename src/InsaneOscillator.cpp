#include "sierrachart.h"

SCDLLName("InsaneOscillator")

SCSFExport void scsf_InsaneOscillator(SCStudyInterfaceRef sc)
{
    SCInputRef In_RsiLength = sc.Input[0];
    SCInputRef In_RsiOB     = sc.Input[1];
    SCInputRef In_RsiOS     = sc.Input[2];

    SCSubgraphRef SG_RSI     = sc.Subgraph[0];
    SCSubgraphRef SG_Midline = sc.Subgraph[1];
    SCSubgraphRef SG_UpRMA   = sc.Subgraph[2]; // internal
    SCSubgraphRef SG_DownRMA = sc.Subgraph[3]; // internal

    if (sc.SetDefaults)
    {
        sc.GraphName = "Insane Oscillator";
        sc.StudyDescription =
            "Pine-exact RSI (ta.rma-based) on Close. RSI plotted in Region 1 with 0-100 scale. "
            "Marker logic to be added later (bar-close-only).";

        sc.AutoLoop = 1;
        sc.UpdateAlways = 0;

        // Put RSI in its own region (like TradingView)
        SG_RSI.GraphRegion = 1;
        SG_Midline.GraphRegion = 1;
        SG_UpRMA.GraphRegion = 1;
        SG_DownRMA.GraphRegion = 1;

        // Inputs
        In_RsiLength.Name = "RSI Length";
        In_RsiLength.SetInt(14);
        In_RsiLength.SetIntLimits(1, 2000);

        In_RsiOB.Name = "RSI Overbought";
        In_RsiOB.SetFloat(70.0f);

        In_RsiOS.Name = "RSI Oversold";
        In_RsiOS.SetFloat(30.0f);

        // Subgraphs
        SG_RSI.Name = "RSI";
        SG_RSI.DrawStyle = DRAWSTYLE_LINE;
        SG_RSI.PrimaryColor = RGB(220, 220, 220);
        SG_RSI.LineWidth = 2;
        SG_RSI.DrawZeros = false;

        SG_Midline.Name = "Midline (50)";
        SG_Midline.DrawStyle = DRAWSTYLE_LINE;
        SG_Midline.PrimaryColor = RGB(120, 120, 120);
        SG_Midline.LineWidth = 1;
        SG_Midline.DrawZeros = false;

        // Internal RMA series (hidden)
        SG_UpRMA.Name = "UpRMA (internal)";
        SG_UpRMA.DrawStyle = DRAWSTYLE_HIDDEN;
        SG_UpRMA.DrawZeros = false;

        SG_DownRMA.Name = "DownRMA (internal)";
        SG_DownRMA.DrawStyle = DRAWSTYLE_HIDDEN;
        SG_DownRMA.DrawZeros = false;

        return;
    }

    const int Len = In_RsiLength.GetInt();
    const int i   = sc.Index;

    // Keep RSI region anchored to 0..100 by always outputting these
    SG_Midline[i] = 50.0f;

    // Need prior bar for change()
    if (i == 0)
    {
        SG_RSI[i] = 50.0f;
        SG_UpRMA[i] = 0.0f;
        SG_DownRMA[i] = 0.0f;
        return;
    }

    // Pine:
    // up   = ta.rma(max(change(close), 0), Len)
    // down = ta.rma(-min(change(close), 0), Len)
    // rsi  = down==0 ? 100 : up==0 ? 0 : 100 - 100/(1 + up/down)
    const float change = (float)(sc.Close[i] - sc.Close[i - 1]);
    const float up     = (change > 0.0f) ? change : 0.0f;
    const float down   = (change < 0.0f) ? -change : 0.0f;

    // Not enough data to seed
    if (i < Len)
    {
        SG_UpRMA[i] = 0.0f;
        SG_DownRMA[i] = 0.0f;
        SG_RSI[i] = 50.0f;
        return;
    }

    // Seed at i == Len with SMA of first Len change bars (1..Len)
    if (i == Len)
    {
        float sumUp = 0.0f;
        float sumDown = 0.0f;

        for (int k = 1; k <= Len; ++k)
        {
            const float ch = (float)(sc.Close[k] - sc.Close[k - 1]);
            sumUp   += (ch > 0.0f) ? ch : 0.0f;
            sumDown += (ch < 0.0f) ? -ch : 0.0f;
        }

        SG_UpRMA[i]   = sumUp / (float)Len;
        SG_DownRMA[i] = sumDown / (float)Len;
    }
    else
    {
        // Wilder recursion (RMA)
        SG_UpRMA[i]   = (SG_UpRMA[i - 1] * (Len - 1) + up) / (float)Len;
        SG_DownRMA[i] = (SG_DownRMA[i - 1] * (Len - 1) + down) / (float)Len;
    }

    const float upRma   = SG_UpRMA[i];
    const float downRma = SG_DownRMA[i];

    float rsi;
    if (downRma == 0.0f)
        rsi = 100.0f;
    else if (upRma == 0.0f)
        rsi = 0.0f;
    else
        rsi = 100.0f - (100.0f / (1.0f + (upRma / downRma)));

    // Clamp just to keep the scale clean (should already be in range)
    if (rsi < 0.0f) rsi = 0.0f;
    if (rsi > 100.0f) rsi = 100.0f;

    SG_RSI[i] = rsi;

    // Bar-close-only marker gating placeholder
    const bool barClosed = (i < sc.ArraySize - 1);
    if (barClosed)
    {
        // TODO: Trampoline + Squeeze markers later
    }
}
