#include <avr/io.h>
#include <util/delay.h>

uint16_t TOP;

void setup_fastPWM(){
  // Configurado para gerar no pino Digital 9 (OCR1A)
  // Está no Port D, bit PD6
  // Configurando o pino OC1A (PB1) como saída
  DDRD |= (1 << DDB1);

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
  TCCR1A |= (1 << WGM11 | 1 << COM1A1);
  // Para colocar no modo não invertido, tem que setar o bit COM1A1
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
  OCR1A = 0; // Iniciando com o Duty Cycle zerado
}

void setDutyCycle(int porcentPWM){
  // porcentPWM deve estar entre 0-100
  // Essa função lê valores de 0-100 e faz o Duty Cycle adotar esse valor

  // Isso é para limitar o valor mínimo do Duty Cycle
  if (porcentPWM < 0){ 
    porcentPWM = 0;
  }

  // Isso é para limitar o vlaor máximo do Duty Cycle
  if (porcentPWM > 100){
    porcentPWM = 100;
  }

  uint16_t dutyCycle = (porcentPWM * (TOP + 1))/100; // Cálculo do ICR para % do Duty
  OCR1A = dutyCycle; // Atualizando o Duty Cycle
}

int main(void){
  setup_fastPWM();

  while (1){
    setDutyCycle(0);
    _delay_ms(2000);

    setDutyCycle(25);
    _delay_ms(2000);

    setDutyCycle(50);
    _delay_ms(2000);

    setDutyCycle(75);
    _delay_ms(2000);

    setDutyCycle(100);
    _delay_ms(2000);
  }
}