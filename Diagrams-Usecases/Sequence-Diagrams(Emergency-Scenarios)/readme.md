# t:slim X2 Insulin Pump Simulator – Sequence Diagrams

##1. CGM-Triggered Insulin Suspension

**Description:**  
Handles the scenario where the CGM (Continuous Glucose Monitor) detects a low glucose event, triggering automatic insulin suspension to prevent hypoglycemia.

**Sequence Overview:**
- `CGMSimulator` sends `newGlucoseReading(low_glucose)` to `MainWindow`.
- `MainWindow`:
  - Processes it with `handleCGM()`.
  - Emits `suspendInsulin(flag = 0)` to `ControlIQManager`.
  - Disables the Confirm button.
  - Updates UI to reflect insulin suspension.
  - Displays `QMessageBox.information("Insulin Suspended", ...)`.

---

##2. Hyperglycemia Detection

**Description:**  
Triggered when CGM detects a high glucose level (≥ 13.0 mmol/L). A warning is issued and adjustments are made if appropriate.

**Sequence Overview:**
- `CGMSimulator` sends `newGlucoseReading(glucose)` to `MainWindow`.
- If `glucose >= 13.0`:
  - A warning is shown using `QMessageBox`.
- Then:
  - `MainWindow` calls `handleCGM(glucose)` via `ControlIQManager`.
  - If `handleCGM` returns `true`, basal insulin is reduced.
  - Updated glucose is added to `GlucoseGraphWidget`.

---

##3. Low Battery Handling
**Description:**  
Describes the automatic system behavior when the battery level becomes low or is fully drained.

**Sequence Overview:**
- Battery level decreases via `updateBattery()` (timer-based).
- If `batteryLevel <= 20`:
  - UI changes to red.
  - Warning message: `"Low Battery"`.
- If `batteryLevel == 0`:
  - Confirm button is disabled.
  - UI displays shutdown message.
  - Critical alert: `"Battery has fully drained"`.
  - Timers stop.
- On Recharge button click:
  - Battery is reset to 100%.
  - UI updates.
  - Timers restart.

---

##4. Low Insulin Sequence

**Description:**  
Triggered when the user confirms a bolus and insulin levels drop below a safe limit or become depleted.

**Sequence Overview:**
- User clicks Confirm.
- `MainWindow`:
  - Calls `calculateSuggestedBolus(bg, carbs)` from `BolusManager`.
  - Delivers bolus via `deliverBolus(units)`.
  - Decreases insulinRemaining and updates UI.
- If `insulinRemaining < 30.0`:
  - Shows `"Low Insulin"` warning.
- If `insulinRemaining == 0.0`:
  - Disables Confirm button.
  - Shows `"Pump stopped — insulin depleted."`.
  - Displays critical alert.
