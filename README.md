# Gait Prediction History - Análise de Marcha com IMU

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
![Language: Python](https://img.shields.io/badge/Language-Python-blue)
![Language: C++](https://img.shields.io/badge/Language-C++-green)
![Language: C#](https://img.shields.io/badge/Language-C%23-purple)

Repositório da Iniciação Científica (IC) contendo dois anos de pesquisa sobre **coleta, análise e predição de padrões de marcha humana** utilizando sensores IMU (MPU6050), Rasberry Pi 5 e modelos de Machine Learning.

Os arquivos para esp32, MQTT e Unity não foram proveitosos no decorrer do projeto.

## 📋 Visão Geral

Este projeto implementa um sistema completo de análise de marcha:
- **Aquisição de dados**: Sensores IMU em tempo real 
- **Processamento**: Rasberry Pi para análise exploratória em C++ com janelas de captação e processamento
- **Modelagem**: XGBoost para classificação de fases de marcha em tempo real
- **Visualização**: Python para vizualização grafica dos resultados posteriormente

**Status**: Pesquisa concluída com resultados prioritários em `06_resultados/resultados_pi/`

---

## 🗂️ Estrutura do Repositório

```
Gait Predict History/
├── 01_documentacao/          # Documentos acadêmicos, relatórios e apresentações
├── 02_referencias/           # Papers e materiais de referência
├── 03_dados/                 # Dados brutos de coleta e logs originais
├── 04_codigo/                # Código de aquisição, análise e aplicações
├── 05_modelos/               # Modelos treinados e resultados intermediários
├── 06_resultados/            # ⭐ PASTA PRIORITÁRIA - Resultados principais
│   └── resultados_pi/        # Resultados otimizados da pesquisa
│       ├── modelos/          # Modelos Random Forest e XGBoost
│       ├── logs/             # Dados adquiridos em tempo de execução
│       ├── imagens/          # Gráficos e fotos do projeto final
│       ├── rotulados_testes/ # Dados manualmente rotulados para treinamento
│       └── *.ipynb           # Notebooks de análise e rotulagem
└── 07_ferramentas/           # Instaladores e ferramentas externas

```


---

## 🚀 Quick Start

### Pré-requisitos

- **Python 3.8+**
- **Git**
- Hardware (em caso da reprodução do projeto):
  - sensor MPU6050
  - Raspberry Pi (para coleta de dados)

### Instalação

#### 1. Clonar o repositório
```bash
git clone https://github.com/mandinhabc/Gait_Predition_History.git
cd Gait_Predition_History
```

#### 2. Configurar ambiente Python
```bash
# Criar ambiente virtual
python -m venv venv

# Ativar (Linux/Mac)
source venv/bin/activate

# Ativar (Windows)
venv\Scripts\activate

# Instalar dependências
pip install -r requirements.txt
```

---

## 📊 Usando os Modelos Principais

### Análise de Dados com Colab

```bash
cd "Gait Predict History/06_resultados/resultados_pi/"


# Abrir: Rotulagem\ e\ análise\ dos\ dados\ captados.ipynb
```

Este notebook contém:
- Leitura de arquivos CSV de sensores
- Rotulagem manual de fases de marcha
- Visualização exploratória dos dados

### Usar Modelos

```python
# Carregar modelo treinado
modelo = joblib.load('Gait Predict History/06_resultados/resultados_pi/modelos/modelo_xgboost.pkl')

# Carregar dados de teste
dados = pd.read_csv('Gait Predict History/06_resultados/resultados_pi/rotulados_testes/dados.csv')

# Fazer predições
predicoes = modelo.predict(dados.drop('fase', axis=1))
print(predicoes)
```
*Exemplo de uso para predição artificial dos dados, sem a coleta em janela em tempo real.

### Executar Leitor em Tempo Real (Raspberry Pi)

```bash
cd "Gait Predict History/06_resultados/resultados_pi/"

# Compilar
g++ -o gait_reader gait_reader.cpp

# Executar
./gait_reader
```

---

## 🔧 Stack Técnico

| Componente | Tecnologia | Uso |
|-----------|-----------|-----|
| **Sensores** | MPU6050 (IMU) | Aceleração e giroscópio |
| **Microcontrolador** | RasberryPi5 | Leitura de sensores, processamento e predição das fases |
| **Backend** | Python 3.8+ | Análise e treinamento |
| **ML** | Scikit-learn, XGBoost | Classificação de fases |
| **Notebooks** | Colab | Análise exploratória |
| **Coleta** | C++ | Leitor em Raspberry Pi + IMU |
| **Gráficos** | Python 3.8+ | Análise estatística |

### Linguagens Utilizadas

- **Python**: Leitura serial, análise CSV, notebooks Colab, treinamento/análise de modelos
- **C/C++**: Leitores/runtime de marcha, binários compilados e predoção em tempo real.
- **MATLAB**: Scripts de plotagem e utilidades

---

## 📈 Fluxo da Pesquisa

```
Coleta de Dados
    ↓
Sensores IMU → Raspberry Pi
    ↓
Processamento em C++
    ↓
Rotulagem Manual de Fases (Python)
    ↓
Treinamento de Modelos (RF, XGBoost)
    ↓
Validação e Testes
    ↓
Implementação em Tempo Real (gait_reader.cpp)
    ↓
Visualização (MATLAB, Python)
```

---

## 📁 Arquivo Prioritário

### `06_resultados/resultados_pi/`

Este é o diretório mais importante do projeto, contendo:

- **`modelos/`**: Arquivos `.pkl` com modelos Random Forest e XGBoost treinados e convertidos para `.h`
- **`logs/`**: Dados de execução em tempo real (`log_runtime_*.csv`)
- **`imagens/`**: Gráficos gerados e fotos do protótipo final
- **`rotulados_testes/`**: Dados manualmente rotulados com fases de marcha
- **`Rotulagem e análise dos dados captados.ipynb`**: Notebook para análise de dados e rotulação
- **`gait_reader.cpp`**: Código C++ para aquisição e predição em Raspberry Pi

Os demais diretórios servem como comparativos e histórico da pesquisa.

---

## 📊 Modelos de Machine Learning

### Random Forest
- **Arquivo**: `05_modelos/modelo_rf.pkl`
- **Features**: 42 (aceleração e giroscópio nos 3 eixos, )
- **Classes**: Fases da marcha (apoio, balanço, etc.)
- **Performance**: `04_codigos/imu_serial/..` não utilizados no modelo final

### XGBoost
- **Arquivo**: `../resultados_pi/modelos/New_models/gait_xgb_v5_3_min.pkl`
- **Otimização**: Hiperparâmetros ajustados para tempo real
- **Exportado para**: Formato `.h` para implementação em C++

---

## 🔌 Hardware e Sensores

### Configuração da Rasp

```cpp
// Conexão MPU6050-RaspberryPi
SDA → GPIO 2
SCL → GPIO 3
VCC → 3.3V
GND → GND
```

### Dados do Sensor

**Frequência**: 80-100 Hz  
**Resolução**: 16-bit  
**Range**: ±2g (aceleração), ±250°/s (giroscópio)  

---

## 📚 Referências e Documentação

- Documentos acadêmicos: `01_documentacao/`
- Papers e materiais: `02_referencias/`
- Relatório detalhado: enviado através do SAGe

---

## 🛠️ Troubleshooting


### Problema: Modelos não carregam
```python
# Verificar versão do joblib
pip show joblib scikit-learn

# Reinstalar se necessário
pip install --upgrade joblib scikit-learn
```

### Problema: Dados CSV corrompidos
Veja os dados rotulados em: `resultados_pi/rotulados_testes/`

---

## 📝 Como Usar Este Repositório

### Para Coletar Novos Dados
1. Acesse `06_resultados/resultados_pi/`
2. Carregue modelos em `modelos/`
3. Carregue o arquivo `gait_reader.cpp` (sempre o arquivo mais recente)
4. Compile o arquivo em 3, gerando o executavel `gait_runtime`
5. Com o prototipo ja posicionado corretamente no pé, rode o executável e faça uma pequena caminhada andando normalmente
6. Salve o log gerado e faça as análises correspondentes

### Para reproduzir resultados
1.  Acesse `06_resultados/resultados_pi/`
2. Carregue `Gait_xgb_v5_3_usar.ipynb`
3. Carregue modelos em `modelos/`
4. Rode o notebook

---

## 📋 Dependências

Veja `requirements.txt` para lista completa:

```
pandas>=1.3.0
numpy>=1.21.0
scikit-learn>=1.0.0
xgboost>=1.5.0
matplotlib>=3.4.0
jupyter>=1.0.0
paho-mqtt>=1.6.0
pyserial>=3.5
scipy>=1.7.0
```

---

## 📄 Licença

Este projeto está sob licença MIT. Veja [LICENSE](LICENSE) para detalhes.

---

## 👤 Autor

**Iniciação Científica** - Análise de Marcha com Sensores IMU  
Data de conclusão: 01/06/2026

---

## 🔗 Links Úteis

- [Documentação Técnica](Gait%20Predict%20History/01_documentacao)
- [Modelos Treinados](Gait%20Predict%20History/06_resultados/resultados_pi/modelos)
- [Dados de Teste](Gait%20Predict%20History/06_resultados/resultados_pi/rotulados_testes)


2. [IC/README_ORGANIZACAO.md](Gait%20Predict%20History/IC/README_ORGANIZACAO.md) - Mapa de organização
3. Notebooks em `06_resultados/resultados_pi/` - Exemplos práticos

---

**Última atualização**: 27/07/2026
