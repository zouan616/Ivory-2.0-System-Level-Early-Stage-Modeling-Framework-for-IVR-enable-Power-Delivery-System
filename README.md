Ivory 2.0: System-Level Early-Stage ModelingFramework for IVR-enable Power Delivery System

Current version is written in C++. A newer Python version will be released soon.

Ivory contains two parts: static model part and dynamic model part.
Static Model part gives out efficiency, area and ripple of an integrated voltage regulator.
Dynamic Model part gives out the transient voltage waveform based on ngspice(SPICE3) circuit simulator.

main.cc is the main function. Users can choose the type of IVR and also can select to use the static model or dynamic model.
static_buck.cc is the static model for integrated buck converter.
static_switched_capacitor.cc is the static model for integrated switched capacitor converter.
Technology.csv is the technology parameters, which is used in static model.

ivr_functions.cc defines the flow of dynamic model
OnChipPDN.cir is the circuit netlist
ivr.cc defines the IVR behavior such as measure and update IVR output voltage. Users do not need to edit this file.
circuit_simulator.cc provides the interface of SPICE circuit simulator(ngspice). Users do not need to edit this file.
sharedspice.cc is the spice circuit simulator ngspice. Users do not need to edit it. 

If you have any questions please feel free to contact me.(An Zou: anzou@wustl.edu)

