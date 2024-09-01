#include <avr/io.h>
#include <util/delay.h>

uint8_t TOP = 255; // Se for usar Timer1 isso deve ser = a ICR1 e tem que ser 16bit

// Funçãozinha marota só pra ficar elegante lá embaixo
float limitasValores(float valor, float minimo, float maximo){
  if (valor < minimo){
    return minimo;
  }
  else if (valor > maximo){
    return maximo;
  }
  else{
    return valor;
  }
}

// Boas anotações pra daqui a 3 meses quando eu revisitar esse código (ass: Taylas)
// =============================
  // PARA O TIMER1
  // Frequências possíveis com Fast PWM (N=Prescale)
  // f_pwm = f_clk/(N*256)
  //       = 16mHz/256N
  // Como mudar o prescale:
  // CS12 CS11 CS10 -> Prescaler -> f_pwm (considerando 16MHz de entrada)
  //   0   0    0   -> Inativo   -> -
  //   0   0    1   ->   1       -> 16 MHz
  //   0   1    0   ->   8       -> 2 MHz
  //   0   1    1   ->   64      -> 250 KHz
  //   1   0    0   ->   256     -> 62,5 KHz
  //   1   0    1   ->   1024    -> 15,625 KHz
  // Essa config trás a FREQUÊNCIA BASE do PWM, então dá pra alterar com o TOP qual vai ser
  // A freq final do pwm
  // Lembra sempre de conferir o WGM1[x] para ver qual o modo do PWM

  // O TOP do PWM (IMPORTANTE ISSO HEIN) pode ser calculado como:
  // TOP = (f_clk/(N*f_pwm)) - 1
  // Dessa vez, o f_pwm vai ser a frequência final do PWM, a que vai ser gerada
  // O valor do ICR1 deve ser definido com o valor do TOP
  // Ex: Freq de 10416Hz: 
  // Seleciona-se o Prescale de 256, no fim o cálculo de TOP entregará TOP=5
  // Equivalendo a 10,416 KHz


  // =============================
  // PARA O TIMER0
  // f_pwm = f_clk/(N*256)
  //       = 16mHz/256N
  // Como mudar o prescale:
  // CS12 CS11 CS10 -> Prescaler -> f_pwm (considerando 16MHz de entrada)
  //   0   0    0   -> Inativo   -> -
  //   0   0    1   ->   1       -> 62,5 kHz
  //   0   1    0   ->   8       -> 7,8125 kHz
  //   0   1    1   ->   64      -> 976,5625 Hz
  //   1   0    0   ->   256     -> 244,1406 Hz
  //   1   0    1   ->   1024    -> 61,035 Hz

  // No Timer0 o TOP é fixo em 255 (de 0 a 255), então a freq é sempre baseada nesse prescaler


void setup_fastPWM(){
  // Pinos D5 e D6
  DDRD |= (1 << DDD5 | 1 << DDD6);

  // Configurando o Timer0 para Fast PWM não-invertido
  TCCR0A |= (1 << WGM00 | 1 << WGM01 | 1 << COM0A1 | 1 << COM0B1);
  TCCR0B |= (1 << CS11 | 1 << CS10); // Prescalers

  // Iniciando com o Duty Cycle zerado
  OCR0A = 0; 
  OCR0B = 0;
}

uint8_t setDutyCycle(float porcentPWM){
  // porcentPWM deve estar entre 0-100
  // Essa função lê valores de 0-100 e faz o Duty Cycle adotar esse valor

  // Isso é para limitar o valor mínimo do Duty Cycle
  porcentPWM = limitasValores(porcentPWM, 0, 100);

  uint8_t dutyCycle = (uint8_t)((porcentPWM/100) * TOP);
  return dutyCycle;
}


int main(void){
  setup_fastPWM();

  while (1){

    OCR0B = setDutyCycle(50);
    OCR0A = setDutyCycle(50);

    _delay_ms(1000);

  }
}

