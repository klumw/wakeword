# Keyword/Wakeword Detection on ESP32-S3: Your Voice-Activated Project Starter! 🚀

Unlock the power of voice with this ready-to-go template for keyword and wakeword detection using the ESP32-S3-DevKitC-1 board! This project provides a solid foundation for building your own voice-controlled applications, from smart assistants to interactive devices.

## ✨ Project Highlights

* **Ready-to-Use ML Model:** Jumpstart your project with our publicly available model on [Wakeword-Computer](https://studio.edgeimpulse.com/public/277934/live). Feel free to clone and customize it with your own unique keyword or start from scratch and learn [how to build your own model](https://docs.edgeimpulse.com/docs/tutorials/end-to-end-tutorials/audio/responding-to-your-voice).
* **Powered by Edge Impulse:** The included machine learning audio model is expertly crafted using [Edge Impulse](https://edgeimpulse.com/), making edge AI accessible to everyone.
* **Learn by Doing:** This project serves as the perfect educational template to understand the end-to-end process of programming and training your own custom wakeword or keyword detection system.
* **'Computer' is Listening:** The default configuration is set to recognize the keyword 'computer', allowing you to test the functionality immediately.
* **Optimized Performance:** Leverages the efficient `esp-nn` neural network library, specifically tailored for Espressif devices, ensuring fast and accurate keyword detection.
* **Crystal-Clear Audio:** Utilizes an I2S microphone for high-quality audio input, enhanced by software Automatic Gain Control (AGC) for robust performance in varying acoustic environments.
* **Effortless Model Updates:** The standardized C++ library design enables seamless swapping and integration of different machine learning models.
* **Visual Confirmation:** An on-board LED provides instant feedback, turning red whenever the keyword 'computer' is successfully detected.

## 🧠 Why ESP32-S3 for AI?

The Espressif [esp32-s3](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf) microcontroller is a powerhouse for edge AI applications, offering:

* **Dual-Core Muscle:** A robust dual-core processor clocked at 240 MHz delivers ample processing power for demanding ML tasks.
* **Dedicated AI Acceleration:** An integrated Neural Network Accelerator (NNA) significantly speeds up the inference of your machine learning models.
* **Versatile Connectivity:** Supports a wide array of interfaces and sensors, making it adaptable to various project requirements.

## 🛠️ Prerequisites

Before you dive in, ensure you have the following set up:

* **ESP-IDF:** Version >= v5.3.2 of the Espressif IoT Development Framework.
* **Hardware:**
    * [ESP-32-S3-DevKitC-1](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/hw-reference/esp32s3/user-guide-devkitc-1.html) or a similar ESP32-S3 development board.
    * I2S Microphone (e.g., [SKU 107990153](https://www.seeedstudio.com/Sipeed-I2S-Mic-for-MAIX-Dev-Boards-p-2887.html)).

### 🔌 Circuit Diagram

Connect your I2S microphone to the ESP32-S3-DevKitC-1 as shown below:
<img src="./docs/circuit_esp32-s3-devkitc-1.svg" width="500" height="400">

### Clone this repo with submodules

git clone --recurse-submodules https://github.com/klumw/wakeword.git

### Build with idf.py
idf.py set-target esp32s3  
idf.py build
