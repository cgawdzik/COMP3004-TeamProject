#include "control_iq_manager.h"

ControlIQManager::ControlIQManager(QObject *parent)
    : QObject(parent), basalRate(0) {}

bool ControlIQManager::handleCGM(double glucose) {
   double maxBasal = 2.0;

   //  If glucose is not too high
   if (glucose < 8.9 && basalRate > 0.1) {
       // Glucose is predicted to be at or below minimum (3.9mmol/L), hypoglycemic
       if (glucose <= 3.9 && !suspended) {
           suspended = true;
           emit suspendInsulin(0);
       } else if (glucose <= 6.25) { // Glucose is predicted to be slightly too low
           // Decrease basal delivery
           basalRate -= 0.1;
           justIncreased = false;
       }
       else { suspended = false; }
   } else { // Glucose is predicted to be slightly too high (>= 8.9mmol/L)
       // Increase basal delivery
       basalRate += 0.1;
       justIncreased = true;
   }


   // If basal rate was just increased or at max, administer bolus
   // Glucose is predicted to be at or above maximum (10.0mmol/L), hyperglyccemic
   if (glucose >= 10.0 && (basalRate == maxBasal || justIncreased)) {
        // Administer bolus
       emit administerBolus(glucose - 6.1); // Target bg is automatically 6.1
   }

   return suspended;
}

void ControlIQManager::setBasal(double rate) {
   //Basal delivery suspended
   if (rate <= 0) {
       suspended = true;
       emit suspendInsulin(1);
   } else { //Start insulin with rate within proper bounds
       suspended = false;
       basalRate = qBound(0.5, rate, 2.0);
       emit resumeInsulin();
   }
}

double ControlIQManager::getBasal() {
   return basalRate;
}
