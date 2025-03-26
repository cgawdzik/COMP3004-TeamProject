#ifndef BOLUS_MANAGER_H
#define BOLUS_MANAGER_H

class BolusManager {
public:
    BolusManager();

    double calculateSuggestedBolus(double bg, double carbs);
    void deliverBolus(double units);

private:
    double lastDeliveredBolus;

    // Default hardcoded parameters
    double carbRatio = 10.0;        // grams per unit
    double correctionFactor = 2.0;  // mmol/L per unit
    double targetBG = 5.5;          // mmol/L
};

#endif // BOLUS_MANAGER_H
