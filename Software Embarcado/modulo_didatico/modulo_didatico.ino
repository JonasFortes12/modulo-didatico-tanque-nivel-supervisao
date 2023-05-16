#define botaoMais 11
#define botaoMenos 12
#define botaoOk 10
#define botaoVolta 13
#define saida 9
#define sensor A1

#include <LiquidCrystal.h> // Adiciona a biblioteca "LiquidCrystal" ao projeto

LiquidCrystal lcd(2, 3, 4, 5, 6, 7); // Pinagem do LCD

//VARIÁVEIS
///////////////////////////////////////////////////////////////////////////////////////

int menu = 1; //controle de seleção no menu (1:calibração; 2: malha_aberta; 3: malha_fechada; 4: resp_transitória)

float tensaoBomba = 0; //em volts de 0 a 12

//Variavéis de controle
float setPoint = 0; //nivel desejado
float feedBack = 0; //nivel obtido na saída
float erro = 0; //diferença entre valor desejado e valor obtido de nível

float Kff = 0; //Ganho de relação nível-tensão
float Vff = 0; //Tensão necessária para manter em nível de linearização
float Kp = 0; //Ganho proporcional
float Ki = 0; //Ganho integral
float antiWindup = 0; //Limita o acúmulo excessivo no termo integrativo

float P = 0; //Termo proporcional V/cm
float I = 0; //Termo integrativo V/(cm.s)
float PI_ff = 0; //Controlador final (P + I + Vff)

int pwm = 0; //porcentagem de duty cicle do pwm na bomba em bytes (0-256)

int degrau = 0; //variação de nível em relação ao nível estabilizado (0-4 cm)

//Variáveis para controle de tempo (amostragem)

float processoAnterior; //tempo de execução até a último amostra
float dt; //tempo de execução de uma amostra

//Especificações reais da planta

float Kb = 9; //Constante da bomba cm^3/s*V
float h0 = 0; //Nivel de linearização (referencia) cm
float At = 69.3978; //área de entrada do tanque cm^2 (diâmetro 9.4cm)
float K = 10.04; // Constante de fluxo da valvula cm^3/s*cm^0.5

//Ganho e constante de tempo do modelo linearizado

float Kdc = 0; //Ganho planta linearizada cm/V
float tal = 0; //Constante de tempo planta linearizada segundos

//Parametros módulo transitório

float PO = 0; //Percentual de overshoot (sobressinal)
float t_acom = 0; //Tempo de acomodação em segundos

float z = 0; //taxa de amortecimento (zeta)
float wn = 0; //Frequencia natural

// Variáveis para função de calibração da leitura de nível

int acumIni; //Soma as 5 primeiras leituras do sensor
int contIni = 0; //Contador 5 primeiras leituras
float medIni = 0; //Médias das 5 primeiras leituras

int acumFin[3] = {0, 0, 0}; //Vetor que armazena as 3 últimas leituras do sensor
int contFin = 0; //Contador das 3 últimas leituras
float medFin = 0; //Média das 3 últimas leituras do sensor

float bit_cent = 0; //bites por centimetro de nível

float auxTime = 0; //Variável auxiliar para marcação do tempo inicial de acionamento da bomba


//FUNÇÕES
//////////////////////////////////////////////////////////////////////////////

//FUNÇÕES ANIMAÇÃO E MENU

//Animação inicial
void animacao(){
  while(millis()/1000 < 4){ //Apresenta animação inicial por 4 segundos
    
    lcd.display();
    delay(900);
    lcd.noDisplay();
    delay(300);
    lcd.setCursor(1, 0);
    lcd.print("Ctrl de nivel");
    lcd.setCursor(5, 1);      
    lcd.print("PI_ff"); 
  }
  
  lcd.clear();
  lcd.display();
  
  while(!digitalRead(botaoOk) && !digitalRead(botaoVolta)
       && !digitalRead(botaoMais) && !digitalRead(botaoMenos)){ //Enquanto usuário não apertar qualquer botao
    
    lcd.setCursor(0, 0);
    lcd.print("Vcc Fonte: 13.4V"); //Tensão necessário na fonte de alimentação para correto funcionamento do sistema
    lcd.setCursor(0, 1);      
    lcd.print("Press any button"); 
  }
  
  lcd.clear();
  lcd.display();
  delay(300);
}

//Função menu
void executaMenu(){
  
  if(digitalRead(botaoMenos)){
    delay(200);
    menu -= 1;
    if(menu < 1){
      menu = 1;
    }
  }
  
  if(digitalRead(botaoMais)){
    delay(200);
    menu += 1;
    if(menu > 4){
      menu = 4;
    }
  }
  
  switch(menu){
    case 1:
    lcd.setCursor(0, 0);
    lcd.print("calib(x)");
    lcd.setCursor(9, 0);      
    lcd.print("abrt( )"); 
    lcd.setCursor(0, 1);      
    lcd.print("trans( )");
    lcd.setCursor(9, 1);      
    lcd.print("fchd( )");
    delay(100);
    break;
    
    case 2:
    lcd.setCursor(0, 0);
    lcd.print("calib( )");
    lcd.setCursor(9, 0);      
    lcd.print("abrt(x)"); 
    lcd.setCursor(0, 1);      
    lcd.print("trans( )");
    lcd.setCursor(9, 1);      
    lcd.print("fchd( )");
    delay(100);
    break;
    
    case 3:
    lcd.setCursor(0, 0);
    lcd.print("calib( )");
    lcd.setCursor(9, 0);      
    lcd.print("abrt( )"); 
    lcd.setCursor(0, 1);      
    lcd.print("trans( )");
    lcd.setCursor(9, 1);      
    lcd.print("fchd(x)");
    delay(100);
    break;
    
    case 4:
    lcd.setCursor(0, 0);
    lcd.print("calib( )");
    lcd.setCursor(9, 0);      
    lcd.print("abrt( )"); 
    lcd.setCursor(0, 1);      
    lcd.print("trans(x)");
    lcd.setCursor(9, 1);      
    lcd.print("fchd( )");
    delay(100);
    break;
    
   }
  
  delay(100);
}

//FUNÇÃO CALIBRAÇÃO "calib()"

void calibracao(){

  lcd.clear();

  while(!digitalRead(botaoVolta)){

    //Mensagem de preparação para execução
    lcd.setCursor(1, 0);
    lcd.print("Fechar valvula");
    lcd.setCursor(2, 1);
    lcd.print("OK to Start");
        
    if(digitalRead(botaoOk)){

      lcd.clear();

      while(!digitalRead(botaoVolta) && (medFin < 400)){ //Bomba desliga sozinha caso usuário não aperta "voltar"
      
        //Aciona a bomba com 5 Volts
        analogWrite(saida, int(9.3*5 + pow(5,2)));
  
        //Média das primeiras 5 leituras do nível em bits
        if(contIni < 5){
          acumIni += analogRead(sensor);
          medIni = acumIni/5;
        }
        contIni += 1;
  
        //Média das 3 últimas leituras
        if(contFin < 3){
          acumFin[contFin] = analogRead(sensor);
          contFin += 1;
          medFin = float(acumFin[0] + acumFin[1] + acumFin[2])/float(3);
        }else{
          contFin = 0;
        }
  
        //Mensagem de instrução para parada
        lcd.setCursor(2, 0);
        lcd.print("Press VOLTAR");
        lcd.setCursor(1, 1);
        lcd.print("no nivel 35 cm"); 
  
        //Taxa de amostragem de uma leitura a cada 0,1 segundo
        delay(100);
      }
       
    }
    
  }

  lcd.clear();

  //Desliga a bomba após botao "volta"
  analogWrite(saida, 0);

  bit_cent = (float(medFin - medIni))/(float(35 - 2.3)); //Valor de bits para variação de 1 centimetro

  lcd.clear();
    
  delay(300);
}

//FUNÇÃO MALHA ABERTA "abrt()"

void malhaAberta(){
  
  lcd.clear();
  //float auxTime = 0; //Variavel local que guarda o tempo do inicio do acionamento da bomba
  float stepTime = 0; //Variável local que guarda o tempo de aplicação do degrau
  
  while(!digitalRead(botaoVolta)){

    //Incremento de tensão (4.5V - 6V)
    if(digitalRead(botaoMais)){
      tensaoBomba += 0.10;
    }
    
    if(digitalRead(botaoMenos)){
      tensaoBomba -= 0.10;
    }
    
    if(tensaoBomba > 6){ //Tensão máxima para que o sistema não ultrapasse a região linear de leitura do sensor (2.5cm - 35.0cm)
      tensaoBomba = 6;

      lcd.setCursor(0, 0);
      lcd.print("Tensao bomba:");
      lcd.setCursor(0, 1);
      lcd.print(tensaoBomba);
      lcd.setCursor(6, 1);
      lcd.print("V max");
      delay(300);
      lcd.clear();
      
    }else if(tensaoBomba < 4.5){ //Tensão mínima para que a resposta em malha aberta corresponda ao modelo
      tensaoBomba = 4.5;

      lcd.setCursor(0, 0);
      lcd.print("Tensao bomba:");
      lcd.setCursor(0, 1);
      lcd.print(tensaoBomba);
      lcd.setCursor(6, 1);
      lcd.print("V min");
      delay(300);
      lcd.clear();
      
    }
    
    //Exibe valor de tensão
    lcd.setCursor(0, 0);
    lcd.print("Tensao bomba:");
    lcd.setCursor(0, 1);
    lcd.print(tensaoBomba);
    lcd.setCursor(6, 1);
    lcd.print("V");

    //Acionamento da bomba em malha aberta
    if(digitalRead(botaoOk)){

      delay(300);
      lcd.clear();
      
      while(!digitalRead(botaoVolta)){

        //Mensagem de preparação para execução
        lcd.setCursor(4, 0);
        lcd.print("Press OK");
        lcd.setCursor(4, 1);
        lcd.print("to Start");
        
        if(digitalRead(botaoOk)){

          lcd.clear();
          auxTime = millis();

          while(!digitalRead(botaoVolta)){ //Loop de execução da bomba

            //Degral de tensão após estabilização em malha aberta
            if(digitalRead(botaoMais)){
              tensaoBomba += 0.6;
              stepTime = (millis() - auxTime)/1000;
              lcd.clear();
            }

            analogWrite(saida, int(9.3*tensaoBomba + pow(tensaoBomba,2))); //Conversão de volts (0-12) para bytes (0-256) (porcentagem de duty-cicle correspondente)
            feedBack = ((float(analogRead(sensor)-medIni)/float(bit_cent)) + 2); //2 = offset inicial; bit_cent = bits para cada centímetro (valores aproximados)
            
            lcd.setCursor(0, 0);
            lcd.print("Nv:");
            lcd.setCursor(4, 0);
            lcd.print(feedBack, 1); 
            lcd.setCursor(9, 0);
            lcd.print("Vb:");
            lcd.setCursor(13,0);
            lcd.print(tensaoBomba,1);
            //Exibie mensagem para aplicação de degrau, quando o degrau é aplicado exibe o tempo da aplicação em segundos 
            if(stepTime == 0){
              lcd.setCursor(0, 1);
              lcd.print("Step: Press MAIS");
            }else{
              lcd.setCursor(0,1);
              lcd.print("Step time:");
              lcd.setCursor(11,1);
              lcd.print(int(stepTime));
              lcd.setCursor(15,1);
              lcd.print("s");
            }
            

            Serial.print(feedBack); //Exibe o nível atual no monitor serial da IDE Arduino
            Serial.print(", ");
            Serial.println(float(millis() - auxTime)/float(1000),1); //Imprime o tempo de execução no monitor serial
            
            //uma execução de loop a cada 0.1 segundo
            delay(100);
            
          }

          lcd.clear();
          stepTime = 0; //Reinicia o tempo do degrau;
          tensaoBomba = 4.5; //Tensão mínima no menu da função
          Serial.println("---------- FIM ----------");
        }
        
      }
      
      lcd.clear();
      
    }
    
    //Desliga a bomba após botao "volta"
    analogWrite(saida, 0);
    
    delay(300);
  }
  
  //Reinicia o valor de tensão e do feedBack
  tensaoBomba = 0;
  feedBack = 0;
  
  lcd.clear();
  
}


//FUNÇÕES PARA MALHA FECHADA "fchd()"

//Insere nivel desejado: setPoint
void insereSP(){
  
      if(digitalRead(botaoMais)){
        lcd.clear();
        setPoint += 0.5;
      }

      if(digitalRead(botaoMenos)){
        lcd.clear();
        setPoint -= 0.5;
      }
      
      //Limita nível 10-35 cm
      if(setPoint < 10){
        setPoint = 10;
        
        lcd.setCursor(0, 0);
        lcd.print("Nivel operacao:");
        lcd.setCursor(0, 1);
        lcd.print(setPoint,1);
        lcd.setCursor(5, 1);
        lcd.print("cm min");
        delay(300);
        lcd.clear();
        
      }else if(setPoint > 35){
        setPoint = 35;
        
        lcd.setCursor(0, 0);
        lcd.print("Nivel operacao:");
        lcd.setCursor(0, 1);
        lcd.print(setPoint,1);
        lcd.setCursor(5, 1);
        lcd.print("cm max");
        delay(300);
        lcd.clear();
        
      }

      //Exibe nível
      lcd.setCursor(0, 0);
      lcd.print("Nivel operacao:");
      lcd.setCursor(0, 1);
      lcd.print(setPoint,1);
      lcd.setCursor(5, 1);
      lcd.print("cm");
      delay(100);
}

//Insere ganho Kff
void insereKff(){
  
      if(digitalRead(botaoMais)){
        Kff += 0.03;
      }

      if(digitalRead(botaoMenos)){
        Kff -= 0.03;
      }

      //Exibe ganho
      lcd.setCursor(0, 0);
      lcd.print("Ganho Kff:");
      lcd.setCursor(0, 1);
      lcd.print(Kff);
      lcd.setCursor(5, 1);
      lcd.print("V/cm^0.5");
      delay(100);
}

//Insere ganho Kp
void insereKp(){
  
      if(digitalRead(botaoMais)){
        Kp += 0.05;
      }

      if(digitalRead(botaoMenos)){
        Kp -= 0.05;
      }

      lcd.setCursor(0, 0);
      lcd.print("Ganho Kp:");
      lcd.setCursor(0, 1);
      lcd.print(Kp);
      lcd.setCursor(6, 1);
      lcd.print("V/cm");
      delay(100);
}

//Insere ganho Ki
void insereKi(){
  
      if(digitalRead(botaoMais)){
        Ki += 0.05;
      }

      if(digitalRead(botaoMenos)){
        Ki -= 0.05;
      }

      lcd.setCursor(0, 0);
      lcd.print("Ganho Ki:");
      lcd.setCursor(0, 1);
      lcd.print(Ki);
      lcd.setCursor(6, 1);
      lcd.print("V/(cm.s)");
      delay(100);
}

//insere degrau com relação ao nível de operação (0-4 cm)
void insereDegrau(){

      
      if(digitalRead(botaoMais)){
        degrau += 1;
      }

      if(digitalRead(botaoMenos)){
        degrau -= 1;
      }

      //Limita degrau (0-5 cm)
      if(degrau < 0){
        degrau = 0;

        lcd.setCursor(0, 0);
        lcd.print("Step reg permnt:");
        lcd.setCursor(0, 1);
        lcd.print(degrau);
        lcd.setCursor(2, 1);
        lcd.print("cm min");
        delay(300);
        lcd.clear();
        
      }

      if(degrau > 5){
        degrau = 5;

        lcd.setCursor(0, 0);
        lcd.print("Step reg permnt:");
        lcd.setCursor(0, 1);
        lcd.print(degrau);
        lcd.setCursor(2, 1);
        lcd.print("cm max");
        delay(300);
        lcd.clear();
        
      }

      //Exibe nivel do degrau
      lcd.setCursor(0, 0);
      lcd.print("Step reg permnt:");
      lcd.setCursor(0, 1);
      lcd.print(degrau);
      lcd.setCursor(2, 1);
      lcd.print("cm");
      delay(100);
}

//Executa controle
void executaControle(){

  /* Insere degrau de nível conforme o valor selecionado (0-5cm)
  O degrau sempre é em relação ao nivel de operação (nível de linearização)
  Exemplo: SP = 30cm, degrau = 3cm. Caso botao "mais", SP = 33cm; caso botao "menos", SP = 27cm. */
  
  if(digitalRead(botaoMais)){
    setPoint += degrau;
    if(setPoint > 35){
      setPoint = 35;
    }
  }
  
  if(digitalRead(botaoMenos)){
    setPoint -= degrau;
    if(setPoint < 10){
      setPoint = 10;
    }
  }
  
  //Tempo de amostra
  dt = float(millis() - processoAnterior)/float(1000);
  processoAnterior = millis();

  //Leitura da saída: nível em centímetros
  feedBack = ((float(analogRead(sensor)-medIni)/float(bit_cent)) + 2); //2 = offset inicial; bit_cent = bits para cada centímetro (valores aproximados)

  //Erro entre nivel desejado e nivel obtido
  erro = setPoint - feedBack;

  //Tensão Feed-forward
  Vff = sqrt(setPoint)*Kff;
  
  //Proporcional
  P = erro*Kp;

  //Integral com ação anti-windup
  I = I + (erro*Ki + antiWindup)*dt;

  //Saída do controlador
  PI_ff = (P + I + Vff);

  //Limitador do sinal de controle (máximo de 12V)
  if(PI_ff > 12){
    tensaoBomba = 12;
  }else if(PI_ff < 0){
    tensaoBomba = 0;
  }else{
    tensaoBomba = PI_ff;
  }

  //Calculo do termo de anti-windup no integrador (back calculation)
  antiWindup = (1/(Kp/Ki))*(tensaoBomba - PI_ff); //Ganho =  1/(Kp/Ki)

  //Conversão tensão para bites (duty-cicle do pwm)
  pwm = int(9.3*tensaoBomba + pow(tensaoBomba,2));

  analogWrite(saida,pwm);

  //Dados no display
  lcd.setCursor(0, 0);
  lcd.print("Sp:");
  lcd.setCursor(3, 0);
  lcd.print(setPoint,1);
  //lcd.setCursor(5, 0);
  //lcd.print("cm");
  
  lcd.setCursor(8, 0);
  lcd.print("Vb:");
  lcd.setCursor(11, 0);
  lcd.print(tensaoBomba,1);
  lcd.setCursor(15, 0);
  lcd.print("V");
  
  lcd.setCursor(0, 1);
  lcd.print("Nv:");
  lcd.setCursor(3, 1);
  lcd.print(feedBack,2);
  lcd.setCursor(7, 1);
  lcd.print("cm");

  Serial.print(setPoint);
  Serial.print(", ");
  Serial.print(feedBack); //Exibe o nível atual no monitor serial da IDE Arduino
  Serial.print(", ");
  Serial.println(float(millis() - auxTime)/float(1000),1); //Imprime o tempo de execução no monitor serial
  
  //Tempo de execução de um loop: 0.1 segundo
  delay(100);
}


//Executa malha fechada com SP e ganhos Kff, Kp e Ki manuais
void malhaFechada(){
  
  lcd.clear();
  
  while(!digitalRead(botaoVolta)){    
    
    insereSP();
    
    if(digitalRead(botaoOk)){
      delay(300);
      lcd.clear();
      
      while(!digitalRead(botaoVolta)){
        
        insereKff();
        
        if(digitalRead(botaoOk)){
          delay(300);
          lcd.clear();
          
          while(!digitalRead(botaoVolta)){
            
            insereKp();
            
            if(digitalRead(botaoOk)){
              delay(300);
              lcd.clear();
              
              while(!digitalRead(botaoVolta)){
                
                insereKi();
                
                if(digitalRead(botaoOk)){
                  
                  delay(300);
                  lcd.clear();
                  
                  while(!digitalRead(botaoVolta)){
                    
                    insereDegrau();

                    if(digitalRead(botaoOk)){
                  
                      delay(300);
                      lcd.clear();
                      
                      while(!digitalRead(botaoVolta)){

                        //Mensagem de preparação para execução
                        lcd.setCursor(4, 0);
                        lcd.print("Press OK");
                        lcd.setCursor(4, 1);
                        lcd.print("to Start");

                        if(digitalRead(botaoOk)){

                          delay(300);
                          lcd.clear();

                          auxTime = millis(); //Marca o tempo inicial do acionamento da bomba

                          while(!digitalRead(botaoVolta)){
                            
                            executaControle();
                            
                          }

                          lcd.clear();
                          analogWrite(saida,0);

                          Serial.println("---------- FIM ----------");
                          
                        }
                        
                      }
                      
                      lcd.clear();
                      delay(300);
                    }
                    
                  }
                  
                  lcd.clear();
                  delay(300);
                }
                
              }
              lcd.clear();
              delay(300);
            }
            
          }
          lcd.clear();
          delay(300);
        }
        
      }
      lcd.clear();
      delay(300);
    }
  }
  lcd.clear();
  
  //Reinicia valores de SetPoint, degrau, Kff, Ki e Kp
  setPoint = 0;
  Kff = 0;
  Ki = 0;
  Kp = 0;
  degrau = 0;
}


//FUNÇÃO PARA VARIÁVEIS DE MÓDULO TRANSITÓRIO (Sobresinal e Tempo de acomodação) "trans()"

//Insere porcentagem de sobressinal
void inserePO(){
  
      if(digitalRead(botaoMais)){
        lcd.clear();
        PO += 1;
      }

      if(digitalRead(botaoMenos)){
        lcd.clear();
        PO -= 1;
      }
      
      //Limita porcentagem 0-30%
      if(PO < 0){
        PO = 0;
        
        lcd.setCursor(0, 0);
        lcd.print("Sobressinal:");
        lcd.setCursor(0, 1);
        lcd.print(int(PO));
        lcd.setCursor(3, 1);
        lcd.print("% min");
        delay(300);
        lcd.clear();
        
      }else if(PO > 30){
        PO = 30;
        
        lcd.setCursor(0, 0);
        lcd.print("Sobressinal:");
        lcd.setCursor(0, 1);
        lcd.print(int(PO));
        lcd.setCursor(3, 1);
        lcd.print("% max");
        delay(300);
        lcd.clear();
        
      }

      //Exibe sobressinal
      lcd.setCursor(0, 0);
      lcd.print("Sobressinal:");
      lcd.setCursor(0, 1);
      lcd.print(int(PO));
      lcd.setCursor(3, 1);
      lcd.print("%");
      delay(100);
}

//Insere tempo de acomodação em segundos
void insereTA(){
  
      if(digitalRead(botaoMais)){
        lcd.clear();
        t_acom += 1;
      }

      if(digitalRead(botaoMenos)){
        lcd.clear();
        t_acom -= 1;
      }
      
      //Limita valor de tempo de acomodação 0-35 segundos
      if(t_acom < 0){
        t_acom = 0;
        
        lcd.setCursor(0, 0);
        lcd.print("Tempo acomod:");
        lcd.setCursor(0, 1);
        lcd.print(int(t_acom));
        lcd.setCursor(3, 1);
        lcd.print("seg min");
        delay(300);
        lcd.clear();
        
      }else if(t_acom > 35){
        t_acom = 35;
        
        lcd.setCursor(0, 0);
        lcd.print("Tempo acomod:");
        lcd.setCursor(0, 1);
        lcd.print(int(t_acom));
        lcd.setCursor(3, 1);
        lcd.print("seg max");
        delay(300);
        lcd.clear();
        
      }

      //Exibe sobressinal
      lcd.setCursor(0, 0);
      lcd.print("Tempo acomod:");
      lcd.setCursor(0, 1);
      lcd.print(int(t_acom));
      lcd.setCursor(3, 1);
      lcd.print("seg");
      delay(100);
}



//Executa malha fechada com ganhos Kff, Ki, e Kp calculados automaticamente a partir de valores de sobressinal (%) e tempo de acomodação (seg)
void transitorio(){

  lcd.clear();
  
  while(!digitalRead(botaoVolta)){
    
    insereSP();

    if(digitalRead(botaoOk)){

      delay(300);
      lcd.clear();

      while(!digitalRead(botaoVolta)){

        inserePO();

        if(digitalRead(botaoOk)){
          
          delay(300);
          lcd.clear();

          while(!digitalRead(botaoVolta)){

            insereTA();

            if(digitalRead(botaoOk)){
                  
              delay(300);
              lcd.clear();
                  
              while(!digitalRead(botaoVolta)){
                    
                insereDegrau();

                if(digitalRead(botaoOk)){

                  //Calcula variavéis de linearização e de controle
                  h0 = setPoint;
                  Kdc = (2*Kb*sqrt(h0))/(K);
                  tal = (2*At*sqrt(h0))/(K);
                  z = ( abs(log(PO/100)) ) / (sqrt( pow(log(PO/100),2) + pow(3.14,2) ));
                  wn = 4/(z*t_acom);
                  
                  Kff = (K)/(Kb);
                  Kp = (2*z*wn*tal - 1)/(Kdc);
                  Ki = ((pow(wn,2))*tal)/(Kdc); 
                  
                  delay(300);
                  lcd.clear();
                      
                  while(!digitalRead(botaoVolta)){

                    //Mensagem de preparação para execução
                    lcd.setCursor(4, 0);
                    lcd.print("Press OK");
                    lcd.setCursor(4, 1);
                    lcd.print("to Start");

                    if(digitalRead(botaoOk)){

                      delay(300);
                      lcd.clear();

                      auxTime = millis(); //Marca o tempo inicial do acionamento da bomba

                      while(!digitalRead(botaoVolta)){
                            
                        executaControle();
                            
                      }

                      lcd.clear();
                      analogWrite(saida,0); //Desliga bomba
                      Serial.println("---------- FIM ----------");
                          
                    }
                        
                  }
                      
                  lcd.clear();
                  delay(300);
                }
                    
              }
                  
              lcd.clear();
              delay(300);
            }
            
          }
          delay(300);
          lcd.clear();
        }
        
      }
      delay(300);
      lcd.clear();
    }
    
  }
  
  lcd.clear();

  //Reinicia valores de SetPoint, degrau, Kff, Ki e Kp
  setPoint = 0;
  Kff = 0;
  Ki = 0;
  Kp = 0;
  degrau = 0;

  h0 = 0;
  Kdc = 0;
  tal = 0;
  z = 0;
  wn = 0;
}


//Função setup
void setup()
{
  lcd.begin(16, 2); // Inicia o lcd de 16x2
  Serial.begin(9600);
  pinMode(botaoMais, INPUT);
  pinMode(botaoMenos, INPUT);
  pinMode(botaoOk, INPUT);
  pinMode(botaoVolta, INPUT);
  pinMode(saida, OUTPUT);
  pinMode(sensor, INPUT);
  
}

//Loop principal do programa
void loop(){
  
  animacao();

  while(true){
    executaMenu();
      
    if(menu == 1 && digitalRead(botaoOk)){
       delay(500);
       calibracao();
    }
    
    if(menu == 2 && digitalRead(botaoOk)){
      delay(500);
      malhaAberta();
    }
  
    if(menu == 3 && digitalRead(botaoOk)){
      delay(500);
      malhaFechada();
    }
    
    if(menu == 4 && digitalRead(botaoOk)){
      delay(500);
      transitorio();
    }   
  }  
}
