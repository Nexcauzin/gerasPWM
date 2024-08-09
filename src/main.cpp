#include <avr/io.h>
#include <util/delay.h>

uint16_t TOP;

// Configura o PWM 
void setupFastPWM(){
  // Configurado para gerar no pino Digital 9 (OCR1A)
  // Está no Port B, bit PB1
  // Configurando o pino OC1A (PB1) como saída
  DDRB |= (1 << DDB1);
  // Mesma coisa para o OC1B (PB2), que é o Digital 10
  DDRB |= (1 << DDB2);

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

  // Configurando o Timer/Counter1 para Fast PWM
  TCCR1A |= (1 << WGM11 | 1 << COM1A1 | 1 << COM1B1);
  // Para colocar no modo não invertido, tem que setar o bit COM1A1 e COM1B1
  TCCR1B |= (1 << WGM13 | 1 << WGM12 | 1 << CS11);
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

  // Deve ser sempre um valor INT
  ICR1 = 199; 
  TOP = ICR1; // Pra usar na função que altera o Duty Cycle

  // Iniciando com o Duty Cycle zerado
  OCR1A = 0; 
  OCR1B = 0; 
}

// Atualiza o valor do Duty Cycle
uint16_t setDutyCycle(int porcentPWM, int topValue){
  // Essa função lê valores de 0-100 e faz o Duty Cycle adotar esse valor

  // Isso é para limitar o valor mínimo do Duty Cycle
  if (porcentPWM < 0){ 
    porcentPWM = 0;
  }

  // Isso é para limitar o vlaor máximo do Duty Cycle
  if (porcentPWM > 100){
    porcentPWM = 100;
  }

  uint16_t dutyCycle = ((porcentPWM * (topValue + 1))/100) - 1; // Cálculo do ICR para % do Duty
  return dutyCycle;
}



int main(void){

  setupFastPWM();

  while (1){

    // Atualiza para o MOTOR DIREITO
    OCR1A = setDutyCycle(75, TOP);
    // Atualiza para o MOTOR ESQUERDO
    OCR1B = setDutyCycle(0, TOP);

    _delay_ms(2000);

  }
}