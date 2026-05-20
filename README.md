# System Simulation Project: LoRaWAN Scenarios

This repository contains the OMNeT++ project files, C++ source code, NED topologies, and simulation results for analyzing LoRaWAN network scenarios.

## 🛠️ Prerequisites
To run these simulations, your OMNeT++ environment must have the following frameworks compiled and installed:
* **INET Framework:** Version 4.4
* **Flora Framework:** (Must be installed and referenced in your OMNeT++ workspace)

## 🚀 Base Setup Instructions
1. Clone this repository to your local machine.
2. Open the OMNeT++ IDE and select **File -> Import -> Existing Projects into Workspace** to import this folder.
3. Ensure the project is properly referencing your local INET 4.4 and Flora installations.
4. Copy the custom `LoRa` and `LoRaApp` folders from this repository's `src/` directory into your active Flora framework `src/` directory.
5. Copy the baseline `n100-gw1.ini` file into the `flora/simulations/examples/` directory.

---

## 📊 Running Specific Scenarios

This project is divided into specialized test cases. If you want to simulate the **Emergency Scenario** or the **Sensor Increase Scenario**, follow the specific instructions below.

### Scenario A: Emergency Case (Processing Delay)
This scenario tests how the network handles a massive spike in packet processing time at the server level.

1. Locate the `emergency.txt` file in this repository.
2. Copy its contents and overwrite your active `n100-gw1.ini` file.
3. Open the C++ source code handling the server processing timer. 
4. Locate the following line in `PacketForwarder.cc` located in  flora/src/LoRa (which defaults to a 0.5-second processing delay):
   ```cpp
   scheduleAt(simTime() + 0.5, serviceTimer);
Change it to simulate a severe processing bottleneck (20.0 seconds):

scheduleAt(simTime() + 20.0, serviceTimer);
Rebuild the project and run the .ini file.

### Scenario B: Sensor Increase (Network Load)
This scenario evaluates network congestion and packet delivery under increased node density.

Locate the sensorincrease.txt file in this repository.

Copy its contents and overwrite your active `n100-gw1.ini` file.

Run the `n100-gw1.ini` file.

⚠️ Developer Note Regarding Simulation Scale: > The initial project proposal promised testing this scenario using 100 sensors across 20 distinct simulation runs. However, during initial testing, the computational overhead was too immense, resulting in 40 minutes of real-time execution per single simulation run. To make the experiment feasible while still generating valid congestion data, the parameters were carefully scaled down to 40 sensors across 3 runs.

⚠️ Additionally, make sure you have enough space in your computer because each simulation run produces a .vec file which can go up to 800 MB of data. In default 20 smiulations were done, so if you are short on space make sure you decrease simulation runs (repeat = 3 or some other low number) or search for "vector-recording = true" statements and change them to "vector-recording = false". Both of these parametes can be found in `n100-gw1.ini` file in flora/simulations/examples/ path.
