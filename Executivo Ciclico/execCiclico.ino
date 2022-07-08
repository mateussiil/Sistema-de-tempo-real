#define LED  7
#define BUZZER 9 

byte trig = 3; // transmissao
byte echo = 2; // recepcao

float cm=0.0, tempo, distancia; // comprimento da onda

class timer
{
private:
    uint32_t    step;
    uint32_t    timeOut;
    uint32_t    timeNow;
    bool        mEnabled;
public:
    timer(bool enabled)
    {
        timeNow = 0; //deadline relativo
        timeOut = 0;  //deadline absoluto
        step = 0; //deadline
        mEnabled = enabled;
    }    
  
  	void setTimeOut(uint32_t seg)
    {
        mEnabled = true;
        step = seg;  //deadline 
        timeNow = millis(); //deadline relativo
        timeOut = timeNow + seg;  //deadline absoluto
    }
  
    bool isTimeOut(bool stop)
    {
        bool ret;

        uint32_t time = millis();

        if (timeOut >= timeNow)
        {
            //condicao normal
            ret = (time >= timeOut) || (time < timeNow);
        }

        if(ret && stop) setTimeOut(step);

        return ret;
    }
};
 
timer  timerLed(true);
timer  timerBuzzer(true);
timer  timerSensor(true);
 
// Defina aqui os tempos em ms para cada LED piscar:
#define   TEMPO_0   250
#define   TEMPO_1   2000
#define   TEMPO_2   715

bool    gLed0 = true;
bool    gTone = true;
 
void setup() {
  Serial.begin(9600);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, 0);
  
  pinMode(trig, OUTPUT); 
  pinMode(echo, INPUT);
  
  pinMode(BUZZER,OUTPUT);
 
  timerLed.setTimeOut(TEMPO_0);
  timerBuzzer.setTimeOut(TEMPO_1);
  timerSensor.setTimeOut(TEMPO_2);
}
 
 
void loop() {
  if(timerLed.isTimeOut(true))
  {
    digitalWrite(LED, gLed0);
    gLed0 = !gLed0;  
  }
 
  if(timerBuzzer.isTimeOut(true))
  {
    if(gTone){
       tone(BUZZER, 440);
    }
    else{
      noTone(BUZZER); 
    }
    gTone = !gTone;
  }
 
  if(timerSensor.isTimeOut(true))
  {
    Serial.println("Sensor");  
    digitalWrite(trig, HIGH); 
    delayMicroseconds(10);
    digitalWrite(trig, LOW);
    tempo = pulseIn(echo, HIGH, 23529);
    distancia = tempo/58.0;
    Serial.println(distancia);
  }
}