//ID del sensor
//-----------------------------------------------------------------
//Formato: Tres Mayúsculas y 2 números. Obs: .equals() es case sensitive
String StringId = "p01-04"; 

bool e32config = true; // configure e32
bool autocalibration = true; // whether to auto calibrate
bool mhz19lib = true; // whether to use external MHZ19 library
int samplingTime_seg = 15; //en seg
int tHeat = 180; //en seg
int ppmAlerta = 700; // Umbral de alerta
