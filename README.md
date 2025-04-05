# Tandem t:slim X2 Insulin Pump Simulator

This project is a simulation of the **Tandem t:slim X2 Insulin Pump** developed using **C++ and the Qt Framework** as part of our COMP3004 final project. It emulates key functionalities of the real-world device to provide a safe and interactive environment for understanding insulin delivery, battery monitoring, and glucose management.

## Features

- **Home Screen**
  - Real-time clock (12-hour format) with AM/PM
  - Dynamic date display (`Month Day, Year`)
  - Battery indicator with live percentage, progressive drain, and recharge
  - Live-updating status labels for **Insulin on Board** and **Glucose Levels**
  - Insulin remaining progress bar with **low insulin level warning**

- **Bolus Function**
  - Input interface for **carbohydrates and glucose values**
  - Simulated **bolus calculation** using **CGM and Control-IQ-inspired logic**
  - Visual **Bolus Bar** showing last insulin dose
  - **Hyperglycemia** and **low insulin** warning messages
  - New insulin cartridge replacement functionality

- **Options Menu**
  - Access to simulated settings and additional features

- **Glucose Monitoring**
  - Real-time glucose level **graph** displayed across relevant pages

- **Brand Integration**
  - Embedded **Tandem logo** via `resources.qrc` file

---

## Navigation

- **Bolus** and **Options** buttons navigate through the `QStackedWidget` pages.
- **Back (<-)** buttons allow return to the Home screen from within submenus.

---

## Team Members

- **Christopher Gawdzik**
- Implemented Delivering Bolus Calculations page
  - Developed glucose level graph visualizations for each page.
  - Implemented real-time **Time and Date** display.
  - Built and managed **Battery system**: GUI, battery drain/recharge, and low-level warning.
  - Designed overall GUI layout and navigation in collaboration with **Yuki Nakashima**.
  - Created **Insulin Remaining in Pump Progress Bar** with low insulin warning.
  - Added functionality for **New Insulin Cartridge** replacement.
  - Programmed live-updating **Status Labels** for Insulin on Board and Glucose.
  - Integrated **Tandem logo** via `resources.qrc`.
  - Implemented warnings for **Hyperglycemia** and **Insulin Too Low**.
  - Created the **Bolus Bar** to visually display the last dose.
- **Yuki Nakashima**  
- **Louis-Philippe Baril**  
- **Colter Harkins**  

---

### Notes
Some members' VMs did not have the QtCharts module installed to allow the chart to be built for CGM monitoring. If a similar issue arises, the command below can be used to resolve the issue in Linux Ubuntu.

  sudo apt-get install libqt5charts5-dev
