# Relatorio do repositorio

Data da analise: 26/05/2026.

## Visao geral

O repositorio `Gait_Predition_History` reúne cerca de dois anos de pesquisa sobre coleta e analise de marcha usando IMU/MPU6050, ESP32, MQTT, Unity, Python, MATLAB e modelos de machine learning.

A pasta `Gait_Predition_History/` foi reorganizada para funcionar como uma unica arvore de pesquisa dentro do repositorio principal.
## Organizacao atual

A estrutura ficou numerada para representar um fluxo mais linear:

- `01_documentacao/`: documentos academicos, comprovantes, relatorios, resumos e apresentacoes.
- `02_referencias/`: papers, figuras e materiais usados como referencia.
- `03_dados/`: dados brutos de coleta e logs originais.
- `04_codigo/`: codigos de aquisicao, analise, ESP32, Unity/MQTT e MATLAB.
- `05_modelos/`: modelos treinados e resultados intermediarios de treinamento fora da pasta principal de resultados.
- `06_resultados/`: resultados priorizados do projeto.
- `07_ferramentas/`: instaladores e ferramentas externas usadas ao longo da pesquisa.

Tambem foi criado `IC/README_ORGANIZACAO.md` com um mapa rapido dessa organizacao.

## Linguagens utilizadas

- Python: leitura serial, analise CSV, notebooks Jupyter e treinamento/analise de modelos.
- C/C++: leitores/runtime de marcha e possiveis binarios compilados.
- Arduino/ESP32: projeto `.ino` com Wi-Fi, MQTT e sensor MPU6050.
- C#/Unity: projeto `M2MqttUnity`, scripts MQTT e cenas Unity.
- MATLAB: scripts de plotagem e utilidades.
- Machine learning: modelos Random Forest/XGBoost em `.pkl` e modelos exportados para `.h`.
- Dados experimentais: CSV/DAT de sensores e logs de predicao.

## Prioridade dos resultados

A pasta prioritaria é:

`Gait_Predition_History/06_resultados/resultados_pi/`

O restante dos dados adquiridos e supostos resultados gerados, não foram proveitosos para o melhor resultado da pesquisa.

## Como usar:

Como dito a pasta principal é `Gait_Predition_History/06_resultados/resultados_pi/`, onde se encontram todos os processos relevantes para o resultado atual da pesquisa. Separados em: 

- `../resultados_pi/modelos/`: modelos de Random Forest e XGBoost, treinados de formas diferentes para a aplicação no projeto.
- `../resultados_pi/logs/`: dados adquiridos em algumas das variações dos modelos, os mais recentes sendo `log_runtime..`.
- `../resultados_pi/imagens/`: graficos respectivos aos `log_runtime..` e fotos do projeto em sua forma final.
- `../resultados_pi/rotulados_testes/`: rescpestivos aos dados adquiridos, mas com a separação das fases feita manualmente. Dados usados nos treinamentos dos modelos.
- `../resultados_pi/Rotulagem e análise dos dados captados.ipynb`: codigo utilizado na analise previa dos dados anteriores e para a roulagem/separação das fases.
- `../resultados_pi/gait_reader.cpp`: codigo usado na Rasberry Pi para aquisição de dados, tanto para rotulagem quanto os finais.

As outras pastas e arquivos serviram apenas de comparativo entre si para verificar a melhor eficiência do projeto.




