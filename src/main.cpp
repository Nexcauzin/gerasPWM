#include <avr/io.h>
#include <util/delay.h>

uint8_t TOP = 255; // Se for usar Timer1 isso deve ser = a ICR1 e tem que ser 16bit
uint16_t inputThrottle, inputLateral, inputFreio;

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
  // Pinos D5 e D6 (configurando como output)
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
  porcentPWM = limitasValores(porcentPWM, 0, 1);

  uint8_t dutyCycle = (uint8_t)((porcentPWM) * TOP);
  return dutyCycle;
}


int main(void){
  setup_fastPWM();
  
  // BASEADO NA PONTE H L298N
  // TABELA VERDADE:
  // IN1   IN2   -> SENTIDO
  //  0     1    -> FREIO DE MÃO
  //  1     0    -> Frente
  //  0     1    -> Ré
  //  1     1    -> FREIO DE MÃO

  // PINOS UTILIZADOS PRA DIREÇÃO (configurando como input)
  // Motor direito:
  DDRD &= ~(1 << DDD4 | 1 << DDD7); // D4 e D7 
  // IN1 = D4, IN2 = D7

  // Motor esquerdo:
  DDRB &= ~(1 << DDB4 | 1 << DDB5); // D12 e D13
  // IN3 = D12, IN4 = D13

  // Inicializando ambos freados
  PORTD &= ~(1 << PD4 | 1 << PD7);
  PORTB &= ~(1 << PB4 | 1 << PB5);

  while (1){
    
    // Aqui configura uma chave de ativação no controle, quando ela for ativa vai ter um valor x (botei >=1800 ficticio), e engatilha a operação
    // em us, e a partir daí faz a operação de frenagem do robô (a ser pensada)
    if (inputFreio >= 1800){
      // por enquanto tá só travando os motores
      PORTD &= ~(1 << PD4 | 1 << PD7);
      PORTB &= ~(1 << PB4 | 1 << PB5);
      // Se MADS passar algo gradual pra frenagem, cria um Flag pra ser acionado enquanto o gatilho estiver ativado
      // Faz a frenagem gradual dos motores, e mantem o estado 0 até que o Flag seja liberado (gatilho desativado)
    }

    // Se não ativar a chave:
    else{
      // Essa etapa leva em consideração valor entre 1000 e 2000 us
      // PROCESSAMENTO DO COMANDO FRENTE/TRÁS
      // Aqui entra o valor lido do radiocontrole (1000 a 2000)
      if (inputThrottle >= 1500){
        inputThrottle = limitasValores((inputThrottle - 1500) * 0.002, 0, 1); // Traz o valor para o range 0 a 1 (float)
        // ASSUMINDO AS ROTAÇÕES AÍ
        // Motor DIREITO
        PORTD &= ~(1 << PD4);
        PORTD |= (1 << PD7);

        // Motor ESQUERDO
        PORTD &= ~(1 << PB5);
        PORTD |= (1 << PD7);
      } 

      else if (inputThrottle < 1500){
        inputThrottle = limitasValores((1500 - inputThrottle) * 0.002, 0, 1);
        // ASSUMINDO AS ROTAÇÕES AÍ
        // Motor DIREITO
        PORTD &= ~(1 << PD7);
        PORTD |= (1 << PD4);

        // Motor ESQUERDO
        PORTD &= ~(1 << PB7);
        PORTD |= (1 << PD5);
      }
      

      // Tô levando em consideração que:
      // OCR0A -> Motor DIREITO
      // OCR0B -> Motor ESQUERDO

      // PROCESSAMENTO DO COMANDO LATERAL
      // DIREITA
      if (inputLateral >= 1500){
        inputLateral = limitasValores((inputLateral - 1500) * 0.002, 0, 1);
        OCR0A = setDutyCycle(inputThrottle - inputLateral);
        OCR0B = setDutyCycle(inputThrottle);
      }
      // ESQUERDA
      else if (inputLateral < 1500){
        inputLateral = limitasValores((1500 - inputLateral) * 0.002, 0, 1);
        OCR0A = setDutyCycle(inputThrottle);
        OCR0B = setDutyCycle(inputThrottle - inputLateral);
      }
    }
  }
}

