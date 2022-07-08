/*
   Este código trata-se de um gerenciador de tarefas baseado num    
   kernel cooperativo (não preemptivo.)
   Para isso, utilizou-se um buffer estático para armazenar as tarefas;
   As tarefas são escalonadas de acordo com a interrupção do Timer1.     
   Este verifica o tempo restante para cada tarefa ser executada.
   A tarefa que atingir o tempo primeiro, será executada.
   As "prioridades" das tarefas é por ordem de adição no buffer.
   Autor: Caio Moraes (Embarcados.com.br, 2017)
   Modificação: Luis Claudio (2018) e Mateus Oliveira (2022)
*/ 

#include "avr/wdt.h"
#include "TimerOne.h" // Se for simular no TinkerCad, remover e usar registradores

#define NUMBER_OF_TASKS  4
#define TEMPO_MAXIMO_EXECUCAO 100   // 100ms
#define SUCCESS 1
#define FAIL    0
#define SIM 1
#define NAO 0

int buzzer = 10;
int button = 11;

byte trig = 3; // transmissao
byte echo = 2; // recepcao

int ledRed = 4;
int ledGreen = 7;
int ledBlue = 8;

int valueLed = 1;
int valueTone = 0;
int valueSonic = 0;

float cm=0.0, tempo, distancia; // comprimento da onda

typedef void(*ptrFunc)();  // Definição ponteiro de função

// Definição da estrutura que contem as informações das tarefas
typedef struct{
  ptrFunc Function;
  uint32_t period;
  bool enableTask; 
} TaskHandle;

// Definição do buffer para armazenar as tarefas
TaskHandle* buffer[NUMBER_OF_TASKS]; 

// Variáveis globais do kernel
volatile uint32_t taskCounter[NUMBER_OF_TASKS];
volatile int16_t TempoEmExecucao;
volatile uint32_t sysTickCounter = 0;
volatile bool TemporizadorEstourou;
volatile bool TarefaSendoExecutada;
int contador = 0;
int ordem = 0;
int estado = 0;



//---------------------------------------------------------------------------------------------------------------------
// Função vKernelInit()
// Descrição: Inicializa as variáveis utilizadas pelo kernel, e o temporizador resposável pelo tick
// Parâmetros: nenhum
// Saida: nenhuma
//---------------------------------------------------------------------------------------------------------------------
char KernelInit()
{
  memset(buffer, NULL, sizeof(buffer));    // Inicializa o buffer para funções
  memset(taskCounter, 0, sizeof(taskCounter)); 

  TemporizadorEstourou = NAO;
  TarefaSendoExecutada = NAO;

  // Base de tempo para o escalonador
  Timer1.initialize(10000);             // 10ms
  Timer1.attachInterrupt(IsrTimer);    // chama vIsrTimer() quando o timer estoura

  return SUCCESS;
}//end vKernelInit


//---------------------------------------------------------------------------------------------------------------------
// Função KernelAdd()
// Descrição: Adiciona uma nova Tarefa ao pool
// Parâmetros: funcao da tareda, nome, periodo, habilita e estrutura para guardar as informações da tarefa
// Saida: nenhuma
//---------------------------------------------------------------------------------------------------------------------
char KernelAdd(TaskHandle* task)
{
  int i;
 
  // Verifica se já existe a tarefa no buffer
  for(i = 0; i < NUMBER_OF_TASKS; i++)
  {
    if((buffer[i]!=NULL) && (buffer[i] == task))
       return SUCCESS;
  }

  // Adiciona a tarefa em um slot vazio
  for(i = 0; i < NUMBER_OF_TASKS; i++)
  {
    if(!buffer[i])
    {
      buffer[i] = task;
      return SUCCESS;
    }
  }
  return FAIL;
}//end vKernelAdd


//---------------------------------------------------------------------------------------------------------------------
// Função KernelRemoveTask()
// Descrição: de forma contrária a função KernelAddTask, esta função remove uma Tarefa do buffer circular
// Parâmetros: Nenhum
// Saída: Nenhuma
//---------------------------------------------------------------------------------------------------------------------
char KernelRemoveTask(TaskHandle* task)
{
  int i;
  for(i=0; i<NUMBER_OF_TASKS; i++)
  {
     if(buffer[i] == task)
     {
        buffer[i] = NULL;
        return SUCCESS; 
     }
  }
  return FAIL;

}//end vKernelRemoveTask


//---------------------------------------------------------------------------------------------------------------------
// Função KernelLoop()
// Descrição: função responsável por escalonar as tarefas de acordo com a resposta da interrupção do Timer 1
// Parâmetros: Nenhum
// Saída: Nenhuma
//---------------------------------------------------------------------------------------------------------------------
void KernelLoop()
{
  int i;

  for (;;)
  {
    if (TemporizadorEstourou == SIM)
    {
      for (i = 0; i < NUMBER_OF_TASKS; i++)
      {
        if (buffer[i] != 0)
        {
          if (((sysTickCounter - taskCounter[i])>buffer[i]->period) && (buffer[i]->enableTask == SIM))
          {
            TarefaSendoExecutada = SIM;
            TempoEmExecucao = TEMPO_MAXIMO_EXECUCAO;
            buffer[i]->Function();
            TarefaSendoExecutada = NAO;
            taskCounter[i] = sysTickCounter;
          }
        }
      }
    }
  }
}//end vKernelLoop


//---------------------------------------------------------------------------------------------------------------------
// Trata a Interrupção do timer 1
// Decrementa o tempo para executar de cada tarefa
// Se uma tafera estiver em execução, decrementa o tempo máximo de execução para reiniciar o MCU caso ocorra
// algum travamento
//---------------------------------------------------------------------------------------------------------------------
void IsrTimer()
{
  int i;
  TemporizadorEstourou = SIM;

  sysTickCounter++;
  
  // Conta o tempo em que uma tarefa está em execução
  if (TarefaSendoExecutada == SIM)
  {
    TempoEmExecucao--;
    if (!TempoEmExecucao)
    {
      // possivelmente a tarefa travou, então, ativa o WDT para reiniciar o micro
      wdt_enable(WDTO_15MS);
      while (1);
    }
  }
}//end vIsrTimer


// Aqui embarcamos o resto do código: processos, funções auxiliares, void setup() e void loop() "vazio".

char tarefa1() {
  Serial.println("Led");
  if(valueLed == 0){
    digitalWrite(ledRed, LOW);
    valueLed = 1;
  }else{
    digitalWrite(ledRed, HIGH);
    valueLed = 0;
  }
}

char tarefa2() {
  Serial.println("Buzzer");
  if(valueTone == 0){
    tone(buzzer, 2000);
    valueTone = 1;
  }else{
    noTone(buzzer);
    valueTone = 0;
  }
}

char tarefa3() {
  Serial.println("Sensor");  
  digitalWrite(trig, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  tempo = pulseIn(echo, HIGH, 23529);
  distancia = tempo/58.0;
  Serial.println(distancia);
}

void setup() {
  Serial.begin(9600);

  pinMode(ledRed, OUTPUT);
  pinMode(buzzer, OUTPUT);

  pinMode(trig, OUTPUT); 
  pinMode(echo, INPUT);

  TaskHandle task1 = {tarefa1, 100, SIM}; 
  TaskHandle task2 = {tarefa2, 200, SIM};
  TaskHandle task3 = {tarefa3, 500, SIM};
  KernelInit();
  KernelAdd(&task1);
  KernelAdd(&task2);
  KernelAdd(&task3);
  KernelLoop();
}


void loop() {}
