// Implement bar-close-only signal evaluation equivalent to Pine barstate.isconfirmed.
// Add proper ACSIL study scaffolding.
// Replace placeholder RSI logic with Pine-style RMA-based RSI.

#include <acsil.h>

// Define study properties
SStudyInfo studyInfo;

// Initialize study
void studyInitialize() {
    studyInfo.SetName("Insane Oscillator");
    studyInfo.SetStudyType(STUDY_TYPE_INDICATOR);
    studyInfo.AddParameter("Length", 14);
}

// Main calculation for the oscillator
void studyCalculate(int startIndex, int endIndex) {
    for(int i = startIndex; i <= endIndex; i++) {
        // Bar-close only condition
        if (IsBarValueConfirmed(i)) {
            // RMA-based RSI logic
            double inputValue = GetInputValue(i);
            double rma = CalculateRMA(inputValue, 14); // Example length
            // Further processing with rma
        }
    }
}

// Check if the bar is confirmed
bool IsBarValueConfirmed(int index) {
    // Implementation for bar confirmation
    return true; // Placeholder; implement actual logic
}

// Calculate RMA
double CalculateRMA(double value, int length) {
    // RMA calculation logic
    return value; // Placeholder; implement actual logic
}