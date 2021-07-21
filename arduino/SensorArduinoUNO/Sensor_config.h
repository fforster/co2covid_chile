//ID del sensor
//-----------------------------------------------------------------
//Formato: Tres Mayúsculas y 2 números. Obs: .equals() es case sensitive
String StringId = "ffb-02"; 

bool activebuzzer = true; // whether we are using an active or passive buzzer
bool e32config = true; // configure e32
bool autocalibration = false; // whether to auto calibrate
bool zerocalibration = true; // whether to zero calibrate (only if autocalibration=false)
bool mhz19lib = true; // whether to use external MHZ19 library
int samplingTime_seg = 15; //en seg
int tHeat = 180; //en seg
bool visualalarm = false;
bool soundalarm = false;
int ppmAlerta = 700; // Umbral de alerta
