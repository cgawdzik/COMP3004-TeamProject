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
 
- **Profile Management**
  - Input interface for **Profile Creation**, **Basal Scheduling**, and **Bolus Setting**.
  - Simulate profile creation, deletion, activation, and modification.
  - Simulate storing profiles containing scheduled data.

- **Glucose Monitoring**
  - Real-time glucose level **graph** displayed across relevant pages

- **Brand Integration**
  - Embedded **Tandem logo** via `resources.qrc` file

---

## Navigation

- **Bolus** and **Options** buttons navigate through the `QStackedWidget` pages.
- **Back (<-)** buttons allow return to the Home screen from within submenus.

---

## Team Members - Each Member and Their Implementations

- **Christopher Gawdzik**
  - Implemented Delivering Bolus Calculations page
    - On Bolus Calculation page took live inputs for carbs and glucose. Also contains a live display to show the bolus in units
    - As well as a view calculation button so user can see how the bolus to be delivered is calculated
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
  - Contributed to UML Use Cases and one Sequence Diagram alongside all other members.
  - Also created four seperate Sequence Diagrams for Emergency scenarios
- **Yuki Nakashima**
  - Implemented Personal Profile Management pages.
    - Developed a data structure model to display data on each table/list view.
      - Implemented an MVC architecture to synchronize across tables.
      - Automatic updating of tables via models.
    - Visually display active and select profiles automatically.
    - Built and managed **Profile List**: GUI and profile creation/deletion/modification using states.
    - Built and managed a list of **Basal Schedules** for each profile: GUI and schedule creation/deletion/modification, also using states.
    - Built and managed **Bolus Setting** for each profile: GUI and settings for each profile.
    - Added active profile toggle functionality to switch status and order of profiles in the profile list.
  - Designed overall GUI layout and navigation in collaboration with **Christopher Gawdzik**.
  - Contributed to UML Use Cases and one Sequence Diagram alongside all other members.
  - Created UML State Machine Diagram.
- **Louis-Philippe Baril**
   - Implemented a History Management Page.
     - Developed a data structure class (HistoryData) that store the full history data that can be accessed from the program.
     - Implemented the required previous/next buttons to loop across multiple instance of history states from the screen.
   - Implemented a Status Screen Page that can be accessed anytime during the program
     - Developed a system that establish a calculation of the variation between attributes so that the changes can be relevant to the user
  - Implemented multiple list data structures (**list_basal**, **list_bolus**, ...) that store the history instances by states
  - Designed the GUI layout buttons and Tandem Logo Button in collaboration with **Christopher Gawdzik**.
  - Contributed to UML Use Cases and one State Diagram alongside all other members.
  - Created UML Class Diagram.
  - Created the Video required to showcase the required Use Case 4, Access Insulin Pump History.
- **Colter Harkins**  

---

### Notes
Some members' VMs did not have the QtCharts module installed to allow the chart to be built for CGM monitoring. If a similar issue arises, the command below can be used to resolve the issue in Linux Ubuntu.

  sudo apt-get install libqt5charts5-dev
