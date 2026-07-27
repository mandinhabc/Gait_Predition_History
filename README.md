# Gait Prediction History - Análise de Marcha com IMU

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
![Language: Python](https://img.shields.io/badge/Language-Python-blue)
![Language: C++](https://img.shields.io/badge/Language-C++-green)
![Language: C#](https://img.shields.io/badge/Language-C%23-purple)

Repositório da Iniciação Científica (IC) contendo dois anos de pesquisa sobre **coleta, análise e predição de padrões de marcha humana** utilizando sensores IMU (MPU6050), Rasberry Pi 5 e modelos de Machine Learning.

Os arquivos para esp32 e MQTT, não foram proveitosos no decorrer do projeto.

## 📋 Visão Geral

Este projeto implementa um sistema completo de análise de marcha:
- **Aquisição de dados**: Sensores IMU em tempo real via ESP32
- **Processamento**: Python com notebooks Jupyter para análise exploratória
- **Modelagem**: Random Forest e XGBoost para classificação de fases de marcha
- **Visualização**: Unity para display em tempo real e MATLAB para gráficos estatísticos

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

relatorio.md                   # Documentação detalhada do projeto
```

Veja [relatorio.md](relatorio.md) para descrição completa de cada diretório.

---

## 🚀 Quick Start

### Pré-requisitos

- **Python 3.8+**
- **Git**
- Hardware opcional:
  - ESP32 com sensor MPU6050
  - Raspberry Pi (para coleta de dados)
  - MQTT Broker (teste: Mosquitto ou HiveMQ)

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

#### 3. (Opcional) Configurar ESP32
```bash
# Instalar Arduino IDE: https://www.arduino.cc/en/software
# Adicionar board ESP32: https://docs.espressif.com/projects/arduino-esp32/

# Carregar firmware em: Gait Predict History/04_codigo/esp32_mqtt/
```

---

## 📊 Usando os Modelos Principais

### Análise de Dados com Jupyter

```bash
cd "Gait Predict History/06_resultados/resultados_pi/"

# Iniciar servidor Jupyter
jupyter notebook

# Abrir: Rotulagem\ e\ análise\ dos\ dados\ captados.ipynb
```

Este notebook contém:
- Leitura de arquivos CSV de sensores
- Rotulagem manual de fases de marcha
- Visualização exploratória dos dados

### Treinar/Usar Modelos

```python
import joblib
import pandas as pd

# Carregar modelo treinado
modelo = joblib.load('Gait Predict History/06_resultados/resultados_pi/modelos/modelo_xgboost.pkl')

# Carregar dados de teste
dados = pd.read_csv('Gait Predict History/06_resultados/resultados_pi/rotulados_testes/dados.csv')

# Fazer predições
predicoes = modelo.predict(dados.drop('fase', axis=1))
print(predicoes)
```

### Executar Leitor em Tempo Real (Raspberry Pi)

```bash
cd "Gait Predict History/06_resultados/resultados_pi/"

# Compilar
g++ -o gait_reader gait_reader.cpp

# Executar
./gait_reader --mqtt-broker localhost --mqtt-port 1883
```

---

## 🔧 Stack Técnico

| Componente | Tecnologia | Uso |
|-----------|-----------|-----|
| **Sensores** | MPU6050 (IMU) | Aceleração e giroscópio |
| **Microcontrolador** | ESP32 | Leitura de sensores e transmissão MQTT |
| **Comunicação** | MQTT | Envio de dados em tempo real |
| **Backend** | Python 3.8+ | Análise e treinamento |
| **ML** | Scikit-learn, XGBoost | Classificação de fases |
| **Notebooks** | Jupyter | Análise exploratória |
| **Visualização** | Unity 2017.1+ | Display em tempo real |
| **Coleta** | C++ | Leitor em Raspberry Pi |
| **Gráficos** | MATLAB | Análise estatística |

### Linguagens Utilizadas

- **Python**: Leitura serial, análise CSV, notebooks Jupyter, treinamento/análise de modelos
- **C/C++**: Leitores/runtime de marcha e binários compilados
- **Arduino/ESP32**: Projeto `.ino` com Wi-Fi, MQTT e sensor MPU6050
- **C#/Unity**: Projeto `M2MqttUnity`, scripts MQTT e cenas Unity
- **MATLAB**: Scripts de plotagem e utilidades

---

## 📈 Fluxo da Pesquisa

```
Coleta de Dados
    ↓
Sensores IMU (ESP32) → MQTT → Raspberry Pi
    ↓
Processamento em Python
    ↓
Rotulagem Manual de Fases
    ↓
Treinamento de Modelos (RF, XGBoost)
    ↓
Validação e Testes
    ↓
Implementação em Tempo Real (gait_reader.cpp)
    ↓
Visualização (Unity + MATLAB)
```

---

## 📁 Arquivo Prioritário

### `06_resultados/resultados_pi/`

Este é o diretório mais importante do projeto, contendo:

- **`modelos/`**: Arquivos `.pkl` com modelos Random Forest e XGBoost treinados
- **`logs/`**: Dados de execução em tempo real (`log_runtime_*.csv`)
- **`imagens/`**: Gráficos gerados e fotos do protótipo final
- **`rotulados_testes/`**: Dados manualmente rotulados com fases de marcha
- **`Rotulagem e análise dos dados captados.ipynb`**: Notebook para análise de dados
- **`gait_reader.cpp`**: Código C++ para aquisição em Raspberry Pi

Os demais diretórios servem como comparativos e histórico da pesquisa.

---

## 📊 Modelos de Machine Learning

### Random Forest
- **Arquivo**: `resultados_pi/modelos/modelo_rf.pkl`
- **Features**: 6-12 (aceleração e giroscópio nos 3 eixos)
- **Classes**: Fases da marcha (apoio, balanço, etc.)
- **Performance**: Vide logs de treinamento

### XGBoost
- **Arquivo**: `resultados_pi/modelos/modelo_xgboost.pkl`
- **Otimização**: Hiperparâmetros ajustados para tempo real
- **Exportado para**: Formato `.h` para implementação em C++

---

## 🔌 Hardware e Sensores

### Configuração do ESP32

```cpp
// Conexão MPU6050
SDA → GPIO 21
SCL → GPIO 22
VCC → 3.3V
GND → GND

// Conexão Wi-Fi/MQTT
SSID: [Configurar]
Broker MQTT: [IP do broker]
Porta: 1883 (padrão)
Tópicos: /gait/sensor, /gait/prediction
```

### Dados do Sensor

**Frequência**: 50-100 Hz  
**Resolução**: 16-bit  
**Range**: ±2g (aceleração), ±250°/s (giroscópio)  

---

## 📚 Referências e Documentação

- Documentos acadêmicos: `01_documentacao/`
- Papers e materiais: `02_referencias/`
- Relatório detalhado: [relatorio.md](relatorio.md)

---

## 🛠️ Troubleshooting

### Problema: Conexão MQTT não funciona
```bash
# Verificar se broker está rodando
mosquitto -v

# Testar conexão
mosquitto_sub -h localhost -t "gait/#"
```

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

### Para Reproduzir Resultados
1. Acesse `06_resultados/resultados_pi/`
2. Execute o notebook `Rotulagem e análise dos dados captados.ipynb`
3. Carregue modelos em `modelos/`
4. Valide com dados em `rotulados_testes/`

### Para Coletar Novos Dados
1. Configure ESP32 com código em `04_codigo/esp32_mqtt/`
2. Execute `gait_reader.cpp` em Raspberry Pi
3. Salve logs em `03_dados/`

### Para Desenvolver Novas Funcionalidades
1. Explore dados em `03_dados/`
2. Analise em notebooks Jupyter
3. Documente em `01_documentacao/`
4. Implemente em `04_codigo/`

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
Data de conclusão: 26/05/2026

---

## 🔗 Links Úteis

- [Relatório Completo](relatorio.md)
- [Documentação Técnica](Gait%20Predict%20History/01_documentacao)
- [Modelos Treinados](Gait%20Predict%20History/06_resultados/resultados_pi/modelos)
- [Dados de Teste](Gait%20Predict%20History/06_resultados/resultados_pi/rotulados_testes)

---

## ❓ Dúvidas?

Consulte:
1. [relatorio.md](relatorio.md) - Documentação detalhada
2. [IC/README_ORGANIZACAO.md](Gait%20Predict%20History/IC/README_ORGANIZACAO.md) - Mapa de organização
3. Notebooks em `06_resultados/resultados_pi/` - Exemplos práticos

---

**Última atualização**: 27/07/2026
