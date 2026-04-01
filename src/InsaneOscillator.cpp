#include "sierrachart.h"

SCDLLName("InsaneOscillator")

SCSFExport void scsf_InsaneOscillator(SCStudyInterfaceRef sc)
{
    // ===== Inputs =====
    SCInputRef In_RsiLength = sc.Input[0];
    SCInputRef In_RsiOB     = sc.Input[1];
    SCInputRef In_RsiOS     = sc.Input[2];

    SCInputRef In_ShowTramp        = sc.Input[3];
    SCInputRef In_BBWidthThreshold = sc.Input[4];
    SCInputRef In_TrampRsiLow      = sc.Input[5];
    SCInputRef In_TrampRsiHigh     = sc.Input[6];
    SCInputRef In_BBLength         = sc.Input[7];
    SCInputRef In_BBStdDev         = sc.Input[8];

    SCInputRef In_ShowTRText = sc.Input[9];
    SCInputRef In_TRFontSize = sc.Input[10];
    SCInputRef In_TROffset   = sc.Input[11];

    // Squeeze Relaxer v2.1 inputs
    SCInputRef In_ShowSqueeze = sc.Input[12];
    SCInputRef In_SqTolerance = sc.Input[13];
    SCInputRef In_AdxSqueeze  = sc.Input[14];
    SCInputRef In_AdxLen      = sc.Input[15];
    SCInputRef In_DiLen       = sc.Input[16];

    // ===== Subgraphs =====
    // Plotted
    SCSubgraphRef SG_RSI_Pine = sc.Subgraph[0];  // RSI line
    SCSubgraphRef SG_Midline  = sc.Subgraph[1];  // 50 line

    // Pine RSI internal RMA state
    SCSubgraphRef SG_UpRMA   = sc.Subgraph[2];
    SCSubgraphRef SG_DownRMA = sc.Subgraph[3];

    // Trampoline (IGNORE — no dots; TR text drawn via sc.UseTool)
    SCSubgraphRef SG_TrampUp = sc.Subgraph[4];
    SCSubgraphRef SG_TrampDn = sc.Subgraph[5];

    // Trampoline internals
    SCSubgraphRef SG_BB      = sc.Subgraph[6];
    SCSubgraphRef SG_RSI_Eng = sc.Subgraph[7];

    // Squeeze Relaxer markers
    SCSubgraphRef SG_SQZ_Buy  = sc.Subgraph[8];
    SCSubgraphRef SG_SQZ_Sell = sc.Subgraph[9];

    // ADX internals
    SCSubgraphRef SG_ADX_trRMA  = sc.Subgraph[10];
    SCSubgraphRef SG_ADX_pRMA   = sc.Subgraph[11];
    SCSubgraphRef SG_ADX_mRMA   = sc.Subgraph[12];
    SCSubgraphRef SG_ADX_adxIn  = sc.Subgraph[13];
    SCSubgraphRef SG_ADX_adxRMA = sc.Subgraph[14];

    // Squeeze state internals
    SCSubgraphRef SG_SQZ_avg2   = sc.Subgraph[15];
    SCSubgraphRef SG_SQZ_val    = sc.Subgraph[16];
    SCSubgraphRef SG_SQZ_cRed   = sc.Subgraph[17];
    SCSubgraphRef SG_SQZ_cGreen = sc.Subgraph[18];
    SCSubgraphRef SG_SQZ_pos    = sc.Subgraph[19];
    SCSubgraphRef SG_SQZ_neg    = sc.Subgraph[20];

    if (sc.SetDefaults)
    {
        sc.GraphName = "Insane Oscillator";
        sc.StudyDescription =
            "Pine-exact RSI + Trampoline (TR text only) + Squeeze Relaxer v2.1. "
            "RSI per-bar color (OB=green, OS=red, else gray). Bar-close-only markers.";

        sc.AutoLoop   = 1;
        sc.UpdateAlways = 0;
        sc.GraphRegion = 1;
        sc.DrawZeros   = 0;

        // ---- Inputs ----
        In_RsiLength.Name = "RSI Length";
        In_RsiLength.SetInt(14);
        In_RsiLength.SetIntLimits(1, 2000);

        In_RsiOB.Name = "RSI Overbought";
        In_RsiOB.SetFloat(70.0f);

        In_RsiOS.Name = "RSI Oversold";
        In_RsiOS.SetFloat(30.0f);

        In_ShowTramp.Name = "Show Trampoline";
        In_ShowTramp.SetYesNo(1);

        In_BBWidthThreshold.Name = "Trampoline: BB Width Threshold (iBBThreshold)";
        In_BBWidthThreshold.SetFloat(0.0015f);

        In_TrampRsiLow.Name = "Trampoline: RSI Lower Threshold (RSIThreshold)";
        In_TrampRsiLow.SetInt(25);

        In_TrampRsiHigh.Name = "Trampoline: RSI Upper Threshold (RSIDown)";
        In_TrampRsiHigh.SetInt(72);

        In_BBLength.Name = "Trampoline: BB Length";
        In_BBLength.SetInt(20);
        In_BBLength.SetIntLimits(1, 500);

        In_BBStdDev.Name = "Trampoline: BB StdDev";
        In_BBStdDev.SetFloat(2.0f);

        In_ShowTRText.Name = "Show TR text markers (Region 1)";
        In_ShowTRText.SetYesNo(1);

        In_TRFontSize.Name = "TR font size";
        In_TRFontSize.SetInt(9);
        In_TRFontSize.SetIntLimits(6, 24);

        In_TROffset.Name = "TR vertical offset (RSI points)";
        In_TROffset.SetFloat(0.0f);

        In_ShowSqueeze.Name = "Show Squeeze Dot";
        In_ShowSqueeze.SetYesNo(1);

        In_SqTolerance.Name = "Squeeze Tolerance";
        In_SqTolerance.SetInt(2);
        In_SqTolerance.SetIntLimits(0, 100);

        In_AdxSqueeze.Name = "ADX Threshold";
        In_AdxSqueeze.SetInt(21);
        In_AdxSqueeze.SetIntLimits(0, 100);

        In_AdxLen.Name = "ADX Smoothing";
        In_AdxLen.SetInt(14);
        In_AdxLen.SetIntLimits(1, 500);

        In_DiLen.Name = "DI Length";
        In_DiLen.SetInt(14);
        In_DiLen.SetIntLimits(1, 500);

        // ---- Subgraphs ----
        SG_RSI_Pine.Name = "RSI (Pine-exact)";
        SG_RSI_Pine.DrawStyle = DRAWSTYLE_LINE;
        SG_RSI_Pine.PrimaryColor = RGB(180, 180, 180);
        SG_RSI_Pine.LineWidth = 2;
        SG_RSI_Pine.DrawZeros = 0;
        SG_RSI_Pine.SecondaryColorUsed = 1;

        SG_Midline.Name = "Midline (50)";
        SG_Midline.DrawStyle = DRAWSTYLE_LINE;
        SG_Midline.PrimaryColor = RGB(120, 120, 120);
        SG_Midline.LineWidth = 1;
        SG_Midline.DrawZeros = 0;

        SG_UpRMA.Name = "UpRMA (internal)";
        SG_UpRMA.DrawStyle = DRAWSTYLE_IGNORE;
        SG_UpRMA.DrawZeros = 0;

        SG_DownRMA.Name = "DownRMA (internal)";
        SG_DownRMA.DrawStyle = DRAWSTYLE_IGNORE;
        SG_DownRMA.DrawZeros = 0;

        SG_TrampUp.Name = "Trampoline Up (internal)";
        SG_TrampUp.DrawStyle = DRAWSTYLE_IGNORE;
        SG_TrampUp.DrawZeros = 0;

        SG_TrampDn.Name = "Trampoline Down (internal)";
        SG_TrampDn.DrawStyle = DRAWSTYLE_IGNORE;
        SG_TrampDn.DrawZeros = 0;

        SG_BB.Name = "BB (internal)";
        SG_BB.DrawStyle = DRAWSTYLE_IGNORE;
        SG_BB.DrawZeros = 0;

        SG_RSI_Eng.Name = "RSI Engine (internal)";
        SG_RSI_Eng.DrawStyle = DRAWSTYLE_IGNORE;
        SG_RSI_Eng.DrawZeros = 0;

        SG_SQZ_Buy.Name = "Squeeze Buy";
        SG_SQZ_Buy.DrawStyle = DRAWSTYLE_DIAMOND;
        SG_SQZ_Buy.PrimaryColor = RGB(255, 255, 0);
        SG_SQZ_Buy.LineWidth = 6;
        SG_SQZ_Buy.DrawZeros = 0;

        SG_SQZ_Sell.Name = "Squeeze Sell";
        SG_SQZ_Sell.DrawStyle = DRAWSTYLE_DIAMOND;
        SG_SQZ_Sell.PrimaryColor = RGB(255, 255, 0);
        SG_SQZ_Sell.LineWidth = 6;
        SG_SQZ_Sell.DrawZeros = 0;

        SG_ADX_trRMA.Name  = "ADX trRMA (internal)";
        SG_ADX_trRMA.DrawStyle = DRAWSTYLE_IGNORE;
        SG_ADX_trRMA.DrawZeros = 0;

        SG_ADX_pRMA.Name   = "ADX plusRMA (internal)";
        SG_ADX_pRMA.DrawStyle = DRAWSTYLE_IGNORE;
        SG_ADX_pRMA.DrawZeros = 0;

        SG_ADX_mRMA.Name   = "ADX minusRMA (internal)";
        SG_ADX_mRMA.DrawStyle = DRAWSTYLE_IGNORE;
        SG_ADX_mRMA.DrawZeros = 0;

        SG_ADX_adxIn.Name  = "ADX input (internal)";
        SG_ADX_adxIn.DrawStyle = DRAWSTYLE_IGNORE;
        SG_ADX_adxIn.DrawZeros = 0;

        SG_ADX_adxRMA.Name = "ADX RMA (internal)";
        SG_ADX_adxRMA.DrawStyle = DRAWSTYLE_IGNORE;
        SG_ADX_adxRMA.DrawZeros = 0;

        SG_SQZ_avg2.Name   = "SQZ avg2 (internal)";
        SG_SQZ_avg2.DrawStyle = DRAWSTYLE_IGNORE;
        SG_SQZ_avg2.DrawZeros = 0;

        SG_SQZ_val.Name    = "SQZ val (internal)";
        SG_SQZ_val.DrawStyle = DRAWSTYLE_IGNORE;
        SG_SQZ_val.DrawZeros = 0;

        SG_SQZ_cRed.Name   = "SQZ cRed (internal)";
        SG_SQZ_cRed.DrawStyle = DRAWSTYLE_IGNORE;
        SG_SQZ_cRed.DrawZeros = 0;

        SG_SQZ_cGreen.Name = "SQZ cGreen (internal)";
        SG_SQZ_cGreen.DrawStyle = DRAWSTYLE_IGNORE;
        SG_SQZ_cGreen.DrawZeros = 0;

        SG_SQZ_pos.Name    = "SQZ pos (internal)";
        SG_SQZ_pos.DrawStyle = DRAWSTYLE_IGNORE;
        SG_SQZ_pos.DrawZeros = 0;

        SG_SQZ_neg.Name    = "SQZ neg (internal)";
        SG_SQZ_neg.DrawStyle = DRAWSTYLE_IGNORE;
        SG_SQZ_neg.DrawZeros = 0;

        return;
    }

    const int i = sc.Index;
    const bool barClosed = (i < sc.ArraySize - 1);

    // ===== Helper: draw "TR" text in Region 1 =====
    auto DrawTR_Region1 = [&](int BarIndex, float Value)
    {
        s_UseTool Tool;
        Tool.Clear();
        Tool.ChartNumber  = sc.ChartNumber;
        Tool.DrawingType  = DRAWING_TEXT;
        Tool.AddMethod    = UTAM_ADD_OR_ADJUST;
        const int side    = (Value < 50.0f) ? 1 : 2;
        // LineNumber encoding: StudyID * 1e6 + BarIndex * 10 + side(1=up,2=dn)
        const int LN_INSTANCE_STRIDE = 1000000;
        const int LN_BAR_STRIDE      = 10;
        Tool.LineNumber   = sc.StudyGraphInstanceID * LN_INSTANCE_STRIDE + BarIndex * LN_BAR_STRIDE + side;
        Tool.BeginIndex   = BarIndex;
        Tool.Region       = sc.GraphRegion;
        Tool.BeginValue   = Value + In_TROffset.GetFloat();
        Tool.TextAlignment = DT_CENTER | DT_VCENTER;
        Tool.Color        = RGB(255, 255, 255);
        Tool.FontSize     = In_TRFontSize.GetInt();
        Tool.FontBold     = 1;
        Tool.FontBackColor = RGB(0, 0, 0);
        Tool.TransparentLabelBackground = 1;
        Tool.Text = "TR";
        sc.UseTool(Tool);
    };

    // Clear marker subgraphs each bar
    SG_SQZ_Buy[i]  = 0.0f;
    SG_SQZ_Sell[i] = 0.0f;

    // ===== Pine-exact RSI =====
    const int   Len = In_RsiLength.GetInt();
    const float OB  = In_RsiOB.GetFloat();
    const float OS  = In_RsiOS.GetFloat();

    SG_Midline[i] = 50.0f;

    if (i == 0)
    {
        SG_RSI_Pine[i] = 50.0f;
        SG_RSI_Pine.DataColor[i] = RGB(180, 180, 180);
        SG_UpRMA[i]   = 0.0f;
        SG_DownRMA[i] = 0.0f;
        return;
    }

    const float change = (float)(sc.Close[i] - sc.Close[i - 1]);
    const float up     = (change > 0.0f) ? change : 0.0f;
    const float down   = (change < 0.0f) ? -change : 0.0f;

    if (i < Len)
    {
        SG_UpRMA[i]   = 0.0f;
        SG_DownRMA[i] = 0.0f;
        SG_RSI_Pine[i] = 50.0f;
        SG_RSI_Pine.DataColor[i] = RGB(180, 180, 180);
        return;
    }

    if (i == Len)
    {
        float sumUp = 0.0f, sumDown = 0.0f;
        for (int k = 1; k <= Len; ++k)
        {
            const float ch = (float)(sc.Close[k] - sc.Close[k - 1]);
            sumUp   += (ch > 0.0f) ? ch : 0.0f;
            sumDown += (ch < 0.0f) ? -ch : 0.0f;
        }
        SG_UpRMA[i]   = sumUp   / (float)Len;
        SG_DownRMA[i] = sumDown / (float)Len;
    }
    else
    {
        SG_UpRMA[i]   = (SG_UpRMA[i - 1]   * (Len - 1) + up)   / (float)Len;
        SG_DownRMA[i] = (SG_DownRMA[i - 1] * (Len - 1) + down) / (float)Len;
    }

    const float upRma   = SG_UpRMA[i];
    const float downRma = SG_DownRMA[i];

    float rsiP;
    if      (downRma == 0.0f) rsiP = 100.0f;
    else if (upRma   == 0.0f) rsiP = 0.0f;
    else                      rsiP = 100.0f - (100.0f / (1.0f + (upRma / downRma)));

    if (rsiP < 0.0f)   rsiP = 0.0f;
    if (rsiP > 100.0f) rsiP = 100.0f;
    SG_RSI_Pine[i] = rsiP;

    if      (rsiP > OB) SG_RSI_Pine.DataColor[i] = RGB(0, 200, 0);
    else if (rsiP < OS) SG_RSI_Pine.DataColor[i] = RGB(220, 20, 60);
    else                SG_RSI_Pine.DataColor[i] = RGB(180, 180, 180);

    // ===== Engine RSI for Trampoline qualification =====
    sc.RSI(sc.Close, SG_RSI_Eng, MOVAVGTYPE_SIMPLE, Len);

    // ===== Trampoline — TR text only, no dot markers =====
    if (In_ShowTramp.GetYesNo() && barClosed && i >= 10)
    {
        const int   bbLen      = In_BBLength.GetInt();
        const float bbMult     = In_BBStdDev.GetFloat();
        const float bbThresh   = In_BBWidthThreshold.GetFloat();
        const float rsiLow     = (float)In_TrampRsiLow.GetInt();
        const float rsiHigh    = (float)In_TrampRsiHigh.GetInt();

        sc.BollingerBands(sc.Close, SG_BB, bbLen, bbMult, MOVAVGTYPE_SIMPLE);

        auto IsRed   = [&](int idx) { return sc.Close[idx] < sc.Open[idx]; };
        auto IsGreen = [&](int idx) { return sc.Close[idx] > sc.Open[idx]; };

        auto BBWAt = [&](int idx) -> float
        {
            const float b = SG_BB.Arrays[2][idx];
            return (b == 0.0f) ? 0.0f
                               : (SG_BB.Arrays[0][idx] - SG_BB.Arrays[1][idx]) / b;
        };

        auto BackAt = [&](int idx) -> bool
        {
            return IsRed(idx)
                && SG_RSI_Eng[idx] <= rsiLow
                && sc.Close[idx] < SG_BB.Arrays[1][idx]
                && BBWAt(idx) > bbThresh;
        };

        auto ForAt = [&](int idx) -> bool
        {
            return IsGreen(idx)
                && SG_RSI_Eng[idx] >= rsiHigh
                && sc.Close[idx] > SG_BB.Arrays[0][idx]
                && BBWAt(idx) > bbThresh;
        };

        auto WeGoUpAt = [&](int idx) -> bool
        {
            if (idx < 10) return false;
            return IsGreen(idx)
                && (BackAt(idx-1) || BackAt(idx-2) || BackAt(idx-3) || BackAt(idx-4) || BackAt(idx-5))
                && sc.High[idx] > sc.High[idx-1];
        };

        auto WeGoDownAt = [&](int idx) -> bool
        {
            if (idx < 10) return false;
            return IsRed(idx)
                && (ForAt(idx-1) || ForAt(idx-2) || ForAt(idx-3) || ForAt(idx-4) || ForAt(idx-5))
                && sc.Low[idx] < sc.Low[idx-1];
        };

        const bool weGoUp   = WeGoUpAt(i);
        const bool weGoDown = WeGoDownAt(i);

        const bool upThrust =
            weGoUp   && !WeGoUpAt(i-1)   && !WeGoUpAt(i-2)   && !WeGoUpAt(i-3)   && !WeGoUpAt(i-4);
        const bool downThrust =
            weGoDown && !WeGoDownAt(i-1) && !WeGoDownAt(i-2) && !WeGoDownAt(i-3) && !WeGoDownAt(i-4);

        if (upThrust   && In_ShowTRText.GetYesNo()) DrawTR_Region1(i, 25.0f);
        if (downThrust && In_ShowTRText.GetYesNo()) DrawTR_Region1(i, 75.0f);
    }

    // ===== Squeeze Relaxer v2.1 =====
    // Constants matching Pine defaults
    const int   sqLength = 20;
    const int   kcLength = 20;
    const float kcMult   = 1.5f;

    const int   diLen       = In_DiLen.GetInt();
    const int   adxLen      = In_AdxLen.GetInt();
    const int   sqTol       = In_SqTolerance.GetInt();
    const float adxThresh   = (float)In_AdxSqueeze.GetInt();

    // --- ADX/DI via Wilder RMA ---
    // Compute per-bar TR, plusDM, minusDM
    float tr_i = 0.0f, plusDM_i = 0.0f, minusDM_i = 0.0f;
    if (i > 0)
    {
        const float h  = sc.High[i];
        const float l  = sc.Low[i];
        const float pc = sc.Close[i - 1];
        const float r1 = h - l;
        const float r2 = (h >= pc) ? (h - pc) : (pc - h);
        const float r3 = (l >= pc) ? (l - pc) : (pc - l);
        tr_i = (r1 >= r2) ? ((r1 >= r3) ? r1 : r3) : ((r2 >= r3) ? r2 : r3);

        const float up5   = sc.High[i]   - sc.High[i - 1];
        const float down5 = sc.Low[i - 1] - sc.Low[i];
        plusDM_i  = (up5   > down5 && up5   > 0.0f) ? up5   : 0.0f;
        minusDM_i = (down5 > up5   && down5 > 0.0f) ? down5 : 0.0f;
    }

    if (i < diLen)
    {
        SG_ADX_trRMA[i]  = 0.0f;
        SG_ADX_pRMA[i]   = 0.0f;
        SG_ADX_mRMA[i]   = 0.0f;
        SG_ADX_adxIn[i]  = 0.0f;
        SG_ADX_adxRMA[i] = 0.0f;
    }
    else if (i == diLen)
    {
        // Seed with SMA of first diLen values (bars 1..diLen)
        float sumTR = 0.0f, sumP = 0.0f, sumM = 0.0f;
        for (int k = 1; k <= diLen; ++k)
        {
            const float hk  = sc.High[k];
            const float lk  = sc.Low[k];
            const float pck = sc.Close[k - 1];
            const float r1k = hk - lk;
            const float r2k = (hk >= pck) ? (hk - pck) : (pck - hk);
            const float r3k = (lk >= pck) ? (lk - pck) : (pck - lk);
            sumTR += (r1k >= r2k) ? ((r1k >= r3k) ? r1k : r3k) : ((r2k >= r3k) ? r2k : r3k);
            const float uk = sc.High[k]   - sc.High[k - 1];
            const float dk = sc.Low[k - 1] - sc.Low[k];
            sumP += (uk > dk && uk > 0.0f) ? uk : 0.0f;
            sumM += (dk > uk && dk > 0.0f) ? dk : 0.0f;
        }
        SG_ADX_trRMA[i] = sumTR / diLen;
        SG_ADX_pRMA[i]  = sumP  / diLen;
        SG_ADX_mRMA[i]  = sumM  / diLen;

        const float trR    = SG_ADX_trRMA[i];
        const float plus_  = (trR != 0.0f) ? (100.0f * SG_ADX_pRMA[i] / trR) : 0.0f;
        const float minus_ = (trR != 0.0f) ? (100.0f * SG_ADX_mRMA[i] / trR) : 0.0f;
        const float sumDI  = plus_ + minus_;
        const float absDI  = (plus_ >= minus_) ? (plus_ - minus_) : (minus_ - plus_);
        SG_ADX_adxIn[i]  = absDI / (sumDI == 0.0f ? 1.0f : sumDI);
        SG_ADX_adxRMA[i] = 0.0f;
    }
    else
    {
        // Wilder recursion
        SG_ADX_trRMA[i] = (SG_ADX_trRMA[i-1] * (diLen - 1) + tr_i)      / (float)diLen;
        SG_ADX_pRMA[i]  = (SG_ADX_pRMA[i-1]  * (diLen - 1) + plusDM_i)  / (float)diLen;
        SG_ADX_mRMA[i]  = (SG_ADX_mRMA[i-1]  * (diLen - 1) + minusDM_i) / (float)diLen;

        const float trR    = SG_ADX_trRMA[i];
        const float plus_  = (trR != 0.0f) ? (100.0f * SG_ADX_pRMA[i] / trR) : 0.0f;
        const float minus_ = (trR != 0.0f) ? (100.0f * SG_ADX_mRMA[i] / trR) : 0.0f;
        const float sumDI  = plus_ + minus_;
        const float absDI  = (plus_ >= minus_) ? (plus_ - minus_) : (minus_ - plus_);
        SG_ADX_adxIn[i]  = absDI / (sumDI == 0.0f ? 1.0f : sumDI);

        const int seedADX = diLen + adxLen - 1;
        if (i == seedADX)
        {
            float sumADX = 0.0f;
            for (int k = diLen; k <= seedADX; ++k)
                sumADX += SG_ADX_adxIn[k];
            SG_ADX_adxRMA[i] = sumADX / (float)adxLen;
        }
        else if (i > seedADX)
        {
            SG_ADX_adxRMA[i] = (SG_ADX_adxRMA[i-1] * (adxLen - 1) + SG_ADX_adxIn[i]) / (float)adxLen;
        }
        else
        {
            SG_ADX_adxRMA[i] = 0.0f;
        }
    }

    const float adxValue  = (i >= diLen + adxLen - 1) ? (100.0f * SG_ADX_adxRMA[i]) : 0.0f;
    const bool  sigabove19 = adxValue > adxThresh;

    // --- sqzOn: BB vs KC ---
    bool sqzOn = false;
    if (i >= sqLength - 1)
    {
        // BB: basis = sma(close, sqLength), dev1 = kcMult * stdev(close, sqLength)
        float sumC = 0.0f;
        for (int k = 0; k < sqLength; ++k)
            sumC += sc.Close[i - k];
        const float basis = sumC / (float)sqLength;

        float varC = 0.0f;
        for (int k = 0; k < sqLength; ++k)
        {
            const float d = sc.Close[i - k] - basis;
            varC += d * d;
        }
        const float stdC  = sqrtf(varC / (float)sqLength);
        const float dev1  = kcMult * stdC;
        const float upperBBsq = basis + dev1;
        const float lowerBBsq = basis - dev1;

        // KC: ma = sma(close, kcLength), rangema = sma(high-low, kcLength)
        float sumCkc = 0.0f, sumRng = 0.0f;
        for (int k = 0; k < kcLength; ++k)
        {
            sumCkc += sc.Close[i - k];
            sumRng += sc.High[i - k] - sc.Low[i - k];
        }
        const float ma_kc  = sumCkc / (float)kcLength;
        const float rngma  = sumRng / (float)kcLength;
        const float upperKC = ma_kc + rngma * kcMult;
        const float lowerKC = ma_kc - rngma * kcMult;

        sqzOn = (lowerBBsq > lowerKC) && (upperBBsq < upperKC);
    }

    // --- avg2 per bar (needed as linreg source) ---
    if (i >= kcLength - 1)
    {
        float highH = sc.High[i], lowL = sc.Low[i], sumCkc2 = 0.0f;
        for (int k = 0; k < kcLength; ++k)
        {
            if (sc.High[i - k] > highH) highH = sc.High[i - k];
            if (sc.Low[i - k]  < lowL)  lowL  = sc.Low[i - k];
            sumCkc2 += sc.Close[i - k];
        }
        const float avg1  = (highH + lowL)               * 0.5f;
        const float smaCl = sumCkc2 / (float)kcLength;
        SG_SQZ_avg2[i] = (avg1 + smaCl) * 0.5f;
    }
    else
    {
        SG_SQZ_avg2[i] = sc.Close[i];
    }

    // --- linreg val = ta.linreg(close - avg2, kcLength, 0) ---
    SG_SQZ_val[i] = 0.0f;
    if (i >= 2 * kcLength - 2)
    {
        const int n = kcLength;
        float sum_x = 0.0f, sum_x2 = 0.0f, sum_y = 0.0f, sum_xy = 0.0f;
        for (int k = 0; k < n; ++k)
        {
            const int   j  = i - (n - 1) + k;           // k=0 oldest, k=n-1 newest
            const float y  = sc.Close[j] - SG_SQZ_avg2[j];
            const float fk = (float)k;
            sum_x  += fk;
            sum_x2 += fk * fk;
            sum_y  += y;
            sum_xy += fk * y;
        }
        const float denom = (float)n * sum_x2 - sum_x * sum_x;
        const float slope = (denom != 0.0f) ? ((float)n * sum_xy - sum_x * sum_y) / denom : 0.0f;
        const float intercept = (sum_y - slope * sum_x) / (float)n;
        SG_SQZ_val[i] = intercept + slope * (float)(n - 1);
    }

    // --- Squeeze state tracking (Pine: var cRed, var cGreen, var pos, var neg) ---
    const float val_cur  = SG_SQZ_val[i];
    const float val_prev = (i > 0) ? SG_SQZ_val[i - 1] : 0.0f;

    float cRed_cur   = (i > 0) ? SG_SQZ_cRed[i - 1]   : 0.0f;
    float cGreen_cur = (i > 0) ? SG_SQZ_cGreen[i - 1] : 0.0f;
    const bool pos_prev = (i > 0) && (SG_SQZ_pos[i - 1] != 0.0f);
    const bool neg_prev = (i > 0) && (SG_SQZ_neg[i - 1] != 0.0f);

    bool pos_cur = false;
    bool neg_cur = false;

    // SQZ_VAL_MID: Pine hardcodes 5.0 as the histogram midline separator
    // (bright red = val < 5, dim = val approaching 0 from below)
    const float SQZ_VAL_MID = 5.0f;

    if (i >= 2 * kcLength - 2)
    {
        // Increment cRed: val decreasing, val < midline, not sqzOn
        if (val_cur < val_prev && val_cur < SQZ_VAL_MID && !sqzOn)
            cRed_cur += 1.0f;

        // Increment cGreen: val increasing, val > midline, not sqzOn
        if (val_cur > val_prev && val_cur > SQZ_VAL_MID && !sqzOn)
            cGreen_cur += 1.0f;

        // Trigger pos: val increasing, cRed > tolerance, val < midline, not prev pos, ADX ok
        if (val_cur > val_prev && cRed_cur > (float)sqTol && val_cur < SQZ_VAL_MID && !pos_prev && sigabove19)
        {
            cRed_cur = 0.0f;
            pos_cur  = true;
        }

        // Trigger neg: val decreasing, cGreen > tolerance, val > midline, not prev neg, ADX ok
        if (val_cur < val_prev && cGreen_cur > (float)sqTol && val_cur > SQZ_VAL_MID && !neg_prev && sigabove19)
        {
            cGreen_cur = 0.0f;
            neg_cur    = true;
        }
    }

    SG_SQZ_cRed[i]   = cRed_cur;
    SG_SQZ_cGreen[i] = cGreen_cur;
    SG_SQZ_pos[i]    = pos_cur ? 1.0f : 0.0f;
    SG_SQZ_neg[i]    = neg_cur ? 1.0f : 0.0f;

    // --- Squeeze markers (yellow diamonds, bar-close-only) ---
    if (In_ShowSqueeze.GetYesNo() && barClosed)
    {
        if (pos_cur) SG_SQZ_Buy[i]  = 25.0f;
        if (neg_cur) SG_SQZ_Sell[i] = 75.0f;
    }
}
