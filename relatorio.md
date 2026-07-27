## Relatório do Repositório

Data da análise: 26/05/2026.

## Visão geral

O repositório `Gait_Predition_History` reúne cerca de dois anos de pesquisa sobre coleta e análise de marcha utilizando IMU/MPU6050, ESP32, MQTT, Unity, Python, MATLAB e modelos de machine learning.

A pasta `Gait_Predition_History/` foi reorganizada para funcionar como uma única árvore de pesquisa dentro do repositório principal.

## Organização atual

A estrutura foi numerada para representar o fluxo de trabalho:

-`01_documentacao/`: documentos acadêmicos, comprovantes, relatórios, resumos e apresentações.
-`02_referencias/`: artigos (papers), figuras e materiais utilizados como referência.
-`03_dados/`: dados brutos de coleta e logs originais.
-`04_codigo/`: códigos de aquisição, análise, ESP32, Unity/MQTT e MATLAB.
-`05_modelos/`: modelos treinados e resultados intermediários de treinamento, armazenados fora da pasta principal de resultados.
-`06_resultados/`: resultados priorizados do projeto.
-`07_ferramentas/`: instaladores e ferramentas externas utilizadas ao longo da pesquisa.


## Linguagens utilizadas

- Python: leitura serial, análise de arquivos CSV, notebooks Jupyter e treinamento/análise de modelos.
- C/C++: leitores (runtime) de marcha e possíveis binários compilados.
- Arduino/ESP32: projetos .ino com Wi-Fi, MQTT e sensor MPU6050.
- C#/Unity: projeto M2MqttUnity, scripts MQTT e cenas Unity.
- MATLAB: scripts de plotagem e utilitários.
- Machine Learning: modelos Random Forest e XGBoost em arquivos .pkl e modelos exportados para .h.
- Dados experimentais: arquivos CSV/DAT de sensores e logs de predição.

## Resultados Principais

A pasta prioritária do projeto é:

``Gait_Predition_History/06_resultados/resultados_pi/``

Os demais dados adquiridos e os demais resultados gerados serviram apenas para comparação entre diferentes abordagens e não contribuíram para os melhores resultados obtidos na pesquisa.

## Como usar

Como mencionado anteriormente, a pasta principal é ``Gait_Predition_History/06_resultados/resultados_pi/``, onde se encontram todos os processos relevantes para o resultado atual da pesquisa. Seu conteúdo está organizado da seguinte forma:

- ``../resultados_pi/modelos/``: modelos Random Forest e XGBoost treinados com diferentes abordagens para aplicação no projeto.
- ``../resultados_pi/logs/``: dados adquiridos em algumas variações dos modelos, sendo os arquivos log_runtime... os mais recentes.
- ``../resultados_pi/imagens/``: gráficos correspondentes aos arquivos log_runtime... e fotografias do projeto em sua versão final.
- ``../resultados_pi/rotulados_testes/``: dados correspondentes às aquisições realizadas, contendo a separação manual das fases da marcha. Esses dados foram utilizados no treinamento dos modelos.
- ``../resultados_pi/Rotulagem e análise dos dados captados.ipynb``: notebook utilizado para a análise preliminar dos dados e para a rotulagem/separação das fases da marcha.
- ``../resultados_pi/gait_reader.cpp``: código utilizado na Raspberry Pi para aquisição de dados, tanto para a etapa de rotulagem quanto para os testes finais.

As demais pastas e arquivos tiveram como finalidade apenas a comparação entre diferentes abordagens, permitindo avaliar a eficiência de cada uma durante o desenvolvimento do projeto.
