import tkinter as tk
from tkinter import Canvas, Frame, ttk
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import serial as sr

# janelas
root1 = tk.Tk()  # root1 global
root2 = tk.Tk()  # root2 global

# Instância da porta serial lida
serialPort = sr.Serial('COM9', 9600)
serialPort.reset_input_buffer()

# variáveis do intervalo dos graficos (eixo x)
a = 0
b = 5

# parâmetros iniciais
i = 0  # eixo x(tempo)
activation = False  # ativar/desativar o gráfico

# dados de plotagem
xData = []  # valores de tempo
yData = []  # valores de sinal
levelValue = 0  # valor do nivel do tanque p/ grafico o grafico de nivel


class Grafic():

    def __init__(self):
        # criando uma figura para o grafico:
        # Define as dimensões da figura
        self.figure1 = plt.figure(figsize=(10, 6), dpi=70)
        # Coloca a figura dentro da variavel grafico
        self.g1 = self.figure1.add_subplot(111)
        self.g1.set_title('Serial Data')
        self.g1.set_xlabel('Time')
        self.g1.set_ylabel('Signal')
        self.g1.set_xlim(a, b)
        self.g1.set_ylim(-1, 55)

        self.lines1 = self.g1.plot([], [])[0]  # (Explicar)

        # Instancia a figura dentro da janela
        self.canva1 = FigureCanvasTkAgg(self.figure1, master=root2)
        self.canva1.get_tk_widget().place(relx=0.15, rely=0.05,
                                          relwidth=0.60, relheight=0.70)  # Tamnaho e posição na tela

        self.canva1.draw()

        self.canva2 = Canvas(master=root2, bg='#087E8B')
        self.canva2.place(relx=0.75, rely=0.05, relwidth=0.1, relheight=0.35)

        # Label Tank Level
        self.lbTankLevel = tk.Label(self.canva2,
                                    text='Tank Level', font=('calbiri', 12))
        self.lbTankLevel.place(relx=0.1, rely=0.05, relwidth=0.8)

        self.reportInit()

        self.plot_data()

    #Criação do arquivo de relatório
    def reportInit(self):
        with open('Relatório.txt', 'w') as report:
            report.write('Relatório - Supervisão do Tanque de Nível\n\n\n')

    def update_data(self):
        global i, a, b, levelValue

        # Armazena os pares de dados - sinal , tempo
        self.data = str(serialPort.readline().decode()).split(",")
        print(f"Level:{self.data[0]}  Time: {self.data[1]} ")

        # Insere dados de leitura no relatório
        report = open('Relatório.txt', 'a')
        report.write(f"Tensão(Nível):{self.data[0]} | Tempo:{self.data[1]}\n")
        report.close()

        # atualizacao dos dados atuais de tempo
        i = float(self.data[1])
        xData.append(i)

        # atualizacao dos dados atuais de sinal
        try:
            yData.append(float(self.data[0]))  # define o gráfico de linha
            levelValue = float(self.data[0])  # define o gráfico de nivel
        except:
            yData.append(0)

        # Ajuste do intervalo do grafico em tempo de execucao
        if (i > b):
            a = a + 0.1
            b = b + 0.1
            self.g1.set_xlim(a, b)

    def plot_data(self):
        if (activation == True):  # testa se o botão start foi acionado
            self.update_data()  # atualiza os dados

            self.lines1.set_xdata(xData)  # define o valor de tempo no grafico
            self.lines1.set_ydata(yData)  # define o valor de sinal no grafico
            self.canva1.draw()  # re-desenha o grafio na janela

            self.setLevelTank()  # define o valor de sinal no grafico de nivel

            # acionamento continuo da funcao 'plot_data'
            root2.after(1, self.plot_data)
        else:
            # acionamento continuo da funcao 'plot_data'
            root2.after(1, self.plot_data)

    # define o nivel do grafico de nivel - 10 niveis
    # pensar em um modo mais elegante de testar o nivel!
    def setLevelTank(self):
        global levelValue
        self.canva2.delete("level")

        if (levelValue > 0 and levelValue <= 5):
            self.canva2.create_line(
                10, 240, 100, 240, fill="white", width=15, tags="level")

        elif (levelValue > 5 and levelValue <= 10):
            for i in range(240, 200, -20):
                self.canva2.create_line(
                    10, i, 100, i, fill="white", width=15, tags="level")

        elif (levelValue > 10 and levelValue <= 15):
            for i in range(240, 180, -20):
                self.canva2.create_line(
                    10, i, 100, i, fill="white", width=15, tags="level")

        elif (levelValue > 15 and levelValue <= 20):
            for i in range(240, 160, -20):
                self.canva2.create_line(
                    10, i, 100, i, fill="white", width=15, tags="level")

        elif (levelValue > 20 and levelValue <= 25):
            for i in range(240, 140, -20):
                self.canva2.create_line(
                    10, i, 100, i, fill="white", width=15, tags="level")

        elif (levelValue > 25 and levelValue <= 30):
            for i in range(240, 120, -20):
                self.canva2.create_line(
                    10, i, 100, i, fill="white", width=15, tags="level")

        elif (levelValue > 30 and levelValue <= 35):
            for i in range(240, 100, -20):
                self.canva2.create_line(
                    10, i, 100, i, fill="white", width=15, tags="level")

        elif (levelValue > 35 and levelValue <= 40):
            for i in range(240, 80, -20):
                self.canva2.create_line(
                    10, i, 100, i, fill="white", width=15, tags="level")

        elif (levelValue > 40 and levelValue <= 45):
            for i in range(240, 60, -20):
                self.canva2.create_line(
                    10, i, 100, i, fill="white", width=15, tags="level")

        elif (levelValue > 45):
            for i in range(240, 40, -20):
                self.canva2.create_line(
                    10, i, 100, i, fill="white", width=15, tags="level")

    # mudar o estado de ativacao dos graficos - botao 'start'
    def changeActivation(self):
        global activation
        activation = not activation

    # funcao p/ resetar os valores dos graficos - reinicia os graficos
    def reset(self):
        global xData, yData, i, a, b
        xData = []
        yData = []
        i = 1  # eixo x
        a = 0
        b = 100

        self.g1.set_xlim(a, b)
        self.plot_data()


class Application(Grafic):

    def __init__(self):
        self.root1 = root1  # pega o root1 global
        self.root2 = root2  # pega o root2 global
        self.screen1()  # inicia a janela 1
        self.widgetsScreen1()  # inicia os componentes da janela 1
        root1.mainloop()  # coloca a janela em loop. Continuar visivel

    def screen1(self):
        self.root1.title("Configurações")
        self.root1.configure(background='#011936')
        self.root1.geometry("900x500")
        self.root1.resizable(False, False)

    def screen2(self):
        self.root2.title("Dados")
        self.root2.configure(background='#011936')
        self.root2.resizable(True, True)  # Responsividade
        self.root2.geometry("1100x700")

    def widgetsScreen1(self):

        # funcao ao acionar botao confirmar
        def confirmButtonAction():

            self.bitSec = self.cbBitSec.get()
            self.bitData = self.cbBitData.get()
            self.bitStop = self.cbBitStop.get()
            self.port = self.entryPort.get()
            self.parity = self.cbParity.get()
            self.fluxContr = self.cbFluxContr.get()

            self.root1.destroy()  # destroi a janela 1
            self.screen2()  # inicia a janela 2
            self.widgetsScreen2()  # inicia os componentes da janela 2
            self.frameScreen2()  # plota o frame da tela 2
            self.infoConfig()  # plota as informacoes inseridas pelo usuario na janela 2

        def cancelButtonAction():
            self.root1.destroy()

        # Botão Confirmar
        self.confirmButton = tk.Button(self.root1, text="Confirm", font=('calbiri', 15),
                                       command=confirmButtonAction)
        self.confirmButton.place(relx=0.73, rely=0.85,
                                 relwidth=0.15, relheight=0.08)

        # Botão Cancelar
        self.cancelButton = tk.Button(self.root1, text="Cancel", font=('calbiri', 15),
                                      command=cancelButtonAction)
        self.cancelButton.place(relx=0.15, rely=0.85,
                                relwidth=0.15, relheight=0.08)

        # criando labels e entradas
        bitSecValues = ["9600", "700", "580"]
        bitDataValues = ["10", "9", "8", "7", "6", "5", "4"]
        parityValues = ["Nenhum", "12", "Padrão"]
        bitsStopValues = ["1", "2", "4"]
        fluxContrValues = ["Nenhum", "Padrão", "Automático"]

        self.lbBitSec = tk.Label(
            self.root1, text="Bits por segundo", font=('calbiri', 15))
        self.lbBitSec.place(relx=0.20, rely=0.30, relwidth=0.18)
        self.cbBitSec = ttk.Combobox(
            self.root1, values=bitSecValues, font=('calbiri', 15))
        self.cbBitSec.place(relx=0.20, rely=0.35, relwidth=0.18)
        self.cbBitSec.set("9600")

        self.lbBitData = tk.Label(
            self.root1, text="Bits de dados", font=('calbiri', 15))
        self.lbBitData.place(relx=0.20, rely=0.42, relwidth=0.18)
        self.cbBitData = ttk.Combobox(
            self.root1, values=bitDataValues, font=('calbiri', 15))
        self.cbBitData.place(relx=0.20, rely=0.47, relwidth=0.18)
        self.cbBitData.set("8")

        self.lbBitStop = tk.Label(
            self.root1, text="Bits de Parada", font=('calbiri', 15))
        self.lbBitStop.place(relx=0.20, rely=0.54, relwidth=0.18)
        self.cbBitStop = ttk.Combobox(
            self.root1, values=bitsStopValues, font=('calbiri', 15))
        self.cbBitStop.place(relx=0.20, rely=0.59, relwidth=0.18)
        self.cbBitStop.set("1")

        self.lbPort = tk.Label(self.root1, text="Porta", font=('calbiri', 15))
        self.lbPort.place(relx=0.65, rely=0.30, relwidth=0.18)
        self.entryPort = tk.Entry(self.root1, font=('calbiri', 15))
        self.entryPort.place(relx=0.65, rely=0.35, relwidth=0.18)

        self.lbParity = tk.Label(
            self.root1, text="Paridade", font=('calbiri', 15))
        self.lbParity.place(relx=0.65, rely=0.42, relwidth=0.18)
        self.cbParity = ttk.Combobox(
            self.root1, values=parityValues, font=('calbiri', 15))
        self.cbParity.place(relx=0.65, rely=0.47, relwidth=0.18)
        self.cbParity.set("Nenhum")

        self.lbFluxContr = tk.Label(
            self.root1, text="Controle de fluxo", font=('calbiri', 15))
        self.lbFluxContr.place(relx=0.65, rely=0.54, relwidth=0.18)
        self.cbFluxContr = ttk.Combobox(
            self.root1, values=fluxContrValues, font=('calbiri', 15))
        self.cbFluxContr.place(relx=0.65, rely=0.59, relwidth=0.18)
        self.cbFluxContr.set("Nenhum")

    def widgetsScreen2(self):

        btStart = tk.Button(root2, text="Start/Stop", font=('calbiri', 15),
                            command=lambda: self.changeActivation())
        btStart.place(relx=0.54, rely=0.77, relwidth=0.12, relheight=0.07)

        btReset = tk.Button(root2, text="Reset", font=('calbiri', 15),
                            command=lambda: self.reset())
        btReset.place(relx=0.34, rely=0.77, relwidth=0.12, relheight=0.07)

        Grafic()

    def frameScreen2(self):
        self.frame1 = Frame(self.root2, bg='#F7FFF7',
                            highlightbackground='#1e372c', highlightthickness=2)
        self.frame1.place(relx=0.05, rely=0.87, relwidth=0.9, relheight=0.12)

    def infoConfig(self):
        # Label info bits por segundo
        self.lbInfoBitSec = tk.Label(self.frame1,
                                     text='Bits por Segundo: ' + str(self.bitSec), font=('calbiri', 12))
        self.lbInfoBitSec.place(relx=0.03, rely=0.1, relwidth=0.3)

        # Label info bits de dados
        self.lbInfoBitData = tk.Label(self.frame1,
                                      text='Bits de Dados: ' + str(self.bitData), font=('calbiri', 12))
        self.lbInfoBitData.place(relx=0.36, rely=0.1, relwidth=0.3)

        # Label info bit de parada
        self.lbInfoBitStop = tk.Label(self.frame1,
                                      text='Bit de Parada: ' + str(self.bitStop), font=('calbiri', 12))
        self.lbInfoBitStop.place(relx=0.69, rely=0.1, relwidth=0.3)

        # Label info porta
        self.lbInfoPort = tk.Label(self.frame1,
                                   text='Porta: ' + str(self.port), font=('calbiri', 12))
        self.lbInfoPort.place(relx=0.03, rely=0.6, relwidth=0.3)

        # Label info paridade
        self.lbInfoParity = tk.Label(self.frame1,
                                     text='Paridade: ' + str(self.parity), font=('calbiri', 12))
        self.lbInfoParity.place(relx=0.36, rely=0.6, relwidth=0.3)

        # Label info paridade
        self.lbInfoFluxContr = tk.Label(self.frame1,
                                        text='Controle de Fluxo: ' + str(self.fluxContr), font=('calbiri', 12))
        self.lbInfoFluxContr.place(relx=0.69, rely=0.6, relwidth=0.3)


Application()
