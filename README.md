# Tandem t:slim X2 Insulin Pump Simulator

This project is a simulation of the **Tandem t:slim X2 Insulin Pump** developed using **C++ and the Qt Framework** as part of our COMP3004 final project. It emulates key functionalities of the real-world device to provide a safe and interactive environment for understanding insulin delivery, battery monitoring, and glucose management.

---

## Features

- **Home Screen**
  - Real-time clock (12-hour format) with AM/PM
  - Dynamic date display (`Month Day, Year`)
  - Battery indicator with live percentage and progressive drain
- **Bolus Function**
  - Interface to input carbohydrate and glucose values
  - Bolus calculation simulation
  - Return navigation to the home screen
- **Options Menu**
  - Access to simulated settings and additional features

---

## Navigation

- **Bolus** and **Options** buttons navigate through the `QStackedWidget` pages.
- **Back (<-)** buttons allow return to the Home screen from within submenus.

---

## Team Members

- Christopher Gawdzik
- Yuki Nakashima  
- Louis-Philippe Baril  
- Colter Harkins  

---

### Notes
Some members' VMs did not have the QtCharts module installed to allow the chart to be built for CGM monitoring. If a similar issue arises, the command below can be used to resolve the issue in Linux Ubuntu.

  sudo apt-get install libqt5charts5-dev
