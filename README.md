# Pressure-Controlled Perfusion System

A modular, open-source pressure-controlled perfusion system designed for the biomechanical characterization of corneal tissue. This system provides precise control over retrocorneal pressure and perfusion flow rate for *ex vivo* experiments.

## 🚀 Quick Start

1.  **Hardware:** Assemble the mechanical components and connect the electronics as per the `hardware/` documentation.
2.  **Firmware:** Flash the Arduino with the appropriate controller firmware from the `Controller/` directory.
3.  **Software:** Install Python dependencies and run the recommended dashboard.
    ```bash
    pip install -r software/Dashboard_2/requirements.txt
    python software/Dashboard_2/app.py
    ```

## 📁 Repository Structure & Component Guide

Here is a breakdown of the key software components and their intended use:

| Component | Purpose | Recommendation |
| :--- | :--- | :--- |
| **`Controller/Controller_1`** | Firmware configured for the **Variohm CIT3100** pressure sensor. | Use if your hardware uses the CIT3100 sensor. |
| **`Controller/Controller_2`** | Firmware configured for the **Ceraphant PTC31B** pressure sensor. | **Recommended** for systems with the PTC31B sensor. |
| **`software/Dashboard_1`** | Legacy version of the Python control dashboard. | For reference or specific use cases. |
| **`software/Dashboard_2`** | Enhanced version with improved data visualization and control. | **Recommended** for all new installations. |

### 🔧 Hardware Specifications

Key components used in this system include:

*   **Stepper Motor Driver:** The TMC2209 driver module controls the NEMA 17 stepper motor, providing silent operation and advanced features like stealthChop2 for noiseless motion and spreadCycle for high-speed performance.
*   **Pressure Transducer:** The Ceraphant PTC31B is a pressure switch with a ceramic, vacuum-tight measuring cell, used for safe monitoring of absolute and gauge pressure. It is suitable for measurements in gases, vapors, and liquids.

## ⚙️ How to Use This Repository

### Downloading the Project

To get a copy of the project, you can **clone** the repository or **download it as a ZIP** file.

*   **Clone (recommended for development):**
    ```bash
    git clone https://github.com/RamzanJamali/Perfusion_System.git
    ```
*   **Download ZIP:**
    Click the `Code` button on the GitHub repository page and select "Download ZIP".

### Navigating the Code

- The `Controller/` directory contains the Arduino firmware. Select the version that matches your pressure sensor.
- The `software/` directory contains the Python-based dashboards. Use `Dashboard_2` for the best experience.
- Refer to the `hardware/` directory for assembly guides, CAD files, and the bill of materials.


