# Módulo Didático para Controle e Supervisão de um Tanque de Nível

<center>
<a href="" alt="Python" target="_blank">
  <img src="https://img.shields.io/badge/Python-3776AB?style=for-the-badge&logo=python&logoColor=white">
</a>
<a href="" alt="C" target="_blank">
  <img src="https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white">
</a>

<a href="" alt="Arduino" target="_blank">
  <img src="https://img.shields.io/badge/Arduino-Open%20Source-blue">
</a>
</center>


<br>
<br>

<center><h2 style="color:lightblue;">Descrição</h2></center>
<hr/>
<center>
<p>
O objetivo principal do módulo didático é proporcionar uma plataforma de aprendizado prática e interativa para estudantes da área de controle e automação. O sistema simula o funcionamento de um tanque de nível, permitindo que os usuários compreendam e experimentem os conceitos e técnicas de controle.
</p>

<p>
O módulo didático é composto por um tanque físico, uma bomba de água, um sensor de nível, um microcontrolador Arduino e uma interface de acionamento e configuração para interação e monitoramento. O microcontrolador Arduino é responsável por adquirir os dados do sensor de nível e tomar decisões de controle com base nesses dados.
</p>

<p>
Além do módulo, há um sistema supervisório responsável por coletar e monitorar dados importantes. Este sistema supervisório é composto por uma interface contendo gráficos que apresentam, em tempo real, os valores das variáveis do tanque de nível, além de possuir outras funcionalidades, tais como salvar dados coletados. A linguagem de programação utilizada para a implementação do sistema supervisório foi o Python, a qual disponibiliza várias bibliotecas de código aberto (open source). Dentre estas foram utilizadas duas: Tkinter e Matplotlib. A biblioteca Tkinter dispõe de um kit de ferramentas para a criação de interfaces gráficas, e foi utilizada para criar a interface do sistema de supervisão. Já a biblioteca Matplotlib oferece ferramentas para representar os dados por meio de gráficos, o que permite a visualização da variação, no tempo, dos dados. A integração entre essas duas bibliotecas ofereceu as ferramentas necessárias para a criação do sistema supervisório proposto.
</p>
</center>
<hr/>

<center>
<figure>
  <img style="width:300px;" src=".\Firmware do Módulo\imgs\system.gif" alt="Minha Figura">
  <figcaption>
  Tanque de Nível + Sistema Supervisório</figcaption>
</figure>
</center>