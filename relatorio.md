# Relatorio do repositorio

Data da analise: 26/05/2026.

## Visao geral

O repositorio `Ciencia` reune cerca de dois anos de pesquisa sobre coleta e analise de marcha usando IMU/MPU6050, ESP32, MQTT, Unity, Python, MATLAB e modelos de machine learning.

A pasta `IC/` foi reorganizada para funcionar como uma unica arvore de pesquisa dentro do repositorio principal. Os repositorios Git internos que existiam em subpastas foram removidos anteriormente, preservando os arquivos como pastas comuns.

## Organizacao atual

A estrutura ficou numerada para representar um fluxo mais linear:

- `IC/01_documentacao/`: documentos academicos, comprovantes, relatorios, resumos e apresentacoes.
- `IC/02_referencias/`: papers, figuras e materiais usados como referencia.
- `IC/03_dados/`: dados brutos de coleta e logs originais.
- `IC/04_codigo/`: codigos de aquisicao, analise, ESP32, Unity/MQTT e MATLAB.
- `IC/05_modelos/`: modelos treinados e resultados intermediarios de treinamento fora da pasta principal de resultados.
- `IC/06_resultados/`: resultados priorizados do projeto. A antiga pasta `Resultados Pi` agora esta em `IC/06_resultados/resultados_pi/`.
- `IC/07_ferramentas/`: instaladores e ferramentas externas usadas ao longo da pesquisa.

Tambem foi criado `IC/README_ORGANIZACAO.md` com um mapa rapido dessa organizacao.

## Mapa de movimentacao

| Antes | Depois |
| --- | --- |
| `IC/Documentos/` | `IC/01_documentacao/documentos_academicos/` |
| `IC/Passo a passo_.docx` | `IC/01_documentacao/resumos_e_apresentacoes/Passo a passo_.docx` |
| `IC/Resumo-Amanda-xxxvii-cic-unesp.docx` | `IC/01_documentacao/resumos_e_apresentacoes/Resumo-Amanda-xxxvii-cic-unesp.docx` |
| `IC/Papers/` | `IC/02_referencias/papers/` |
| `IC/CDC_LOGS - cesar/` | `IC/03_dados/cdc_logs_cesar/` |
| `IC/Codigos/IMUSerial/` | `IC/04_codigo/imu_serial/` |
| `IC/Codigos/proj_esp_unity/` | `IC/04_codigo/esp32_mqtt/proj_esp_unity/` |
| `IC/esp32_mpu6050/` | `IC/04_codigo/esp32_mqtt/esp32_mpu6050/` |
| `IC/Codigos/MPU_ESP32.cs` | `IC/04_codigo/esp32_mqtt/MPU_ESP32.cs` |
| `IC/Codigos/Repositorio M2MQTT/M2MqttUnity/` | `IC/04_codigo/unity_mqtt/M2MqttUnity/` |
| `IC/MatLab - cesar/` | `IC/04_codigo/matlab_cesar/` |
| modelos `.pkl` e `modelo_rf.h` soltos em `IC/Codigos/` | `IC/05_modelos/modelos_treinados/` |
| `resultados_df*.csv` soltos em `IC/Codigos/` | `IC/05_modelos/resultados_treinamento/` |
| `IC/Resultados Pi/` | `IC/06_resultados/resultados_pi/` |
| `IC/MQTT/` | `IC/07_ferramentas/instaladores_mqtt/` |
| `IC/VisualStudioSetup.exe` | `IC/07_ferramentas/instaladores/VisualStudioSetup.exe` |

## Tecnologias identificadas

- Python: leitura serial, analise CSV, notebooks Jupyter e treinamento/analise de modelos.
- C/C++: leitores/runtime de marcha e possiveis binarios compilados.
- Arduino/ESP32: projeto `.ino` com Wi-Fi, MQTT e sensor MPU6050.
- C#/Unity: projeto `M2MqttUnity`, scripts MQTT e cenas Unity.
- MATLAB: scripts de plotagem e utilidades.
- Machine learning: modelos Random Forest/XGBoost em `.pkl` e modelos exportados para `.h`.
- Dados experimentais: CSV/DAT de sensores e logs de predicao.

## Prioridade dos resultados

A pasta prioritaria agora e:

`IC/06_resultados/resultados_pi/`

O `.gitignore` foi ajustado para liberar todos os arquivos dentro de `IC/06_resultados/`, mesmo quando os padroes gerais ignoram CSV, PNG, DOCX, PKL, PDF ou headers gerados.

## Pontos de atencao

- `IC/05_modelos/modelos_treinados/modelo_rf.h` tem cerca de 153 MB, acima do limite comum de 100 MB do GitHub para arquivos normais. O ideal e Git LFS, DVC ou armazenamento externo.
- `IC/07_ferramentas/` contem instaladores. O ideal, no longo prazo, e documentar links oficiais em vez de versionar `.exe`.
- `IC/01_documentacao/documentos_academicos/` contem documentos pessoais/sensiveis. Vale revisar antes de publicar o repositorio.
- `IC/04_codigo/esp32_mqtt/proj_esp_unity/proj_esp_unity.ino` contem SSID, senha e IP de broker MQTT diretamente no codigo. O melhor e mover isso para um arquivo local ignorado ou template sem segredo.
- Alguns comentarios de arquivos Python/Arduino aparecem com caracteres quebrados, sinal de possivel mistura de encoding. Padronizar em UTF-8 ajudaria.

## Gitignore atual

A regra geral do `.gitignore` e:

- Manter codigo-fonte e configuracoes de projeto versionaveis.
- Ignorar caches, ambientes locais, builds, credenciais e artefatos de IDE.
- Ignorar dados/logs/modelos/documentos/binarios fora da pasta de resultados prioritaria.
- Liberar tudo em `IC/06_resultados/`.

## Proximos passos sugeridos

1. Revisar `IC/01_documentacao/documentos_academicos/` antes de publicar.
2. Considerar Git LFS para arquivos grandes, principalmente `modelo_rf.h`.
3. Extrair credenciais do codigo ESP32.
4. Criar um `README.md` principal com objetivo, requisitos e como reproduzir os experimentos.
5. Criar `requirements.txt` ou `environment.yml` para os scripts Python/notebooks.
