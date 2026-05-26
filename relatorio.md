# Relatorio do repositorio

Data da analise: 26/05/2026.

## Visao geral

O repositorio `Ciencia` parece reunir materiais de iniciacao cientifica sobre coleta e analise de marcha usando IMU/MPU6050, ESP32, MQTT, Unity, Python, MATLAB e modelos de machine learning. Hoje o Git da raiz possui apenas `.gitattributes` versionado; quase todo o conteudo aparece como nao versionado.

Ha cerca de 381 arquivos fora das pastas `.git`, somando aproximadamente 620 MB. A maior parte do volume esta em dados CSV/DAT, modelos treinados, headers gerados por modelos, instaladores e documentos binarios.

Atualizacao: a pasta `IC/` deve funcionar como uma unica arvore dentro do repositorio principal. Os repositorios Git internos encontrados em subpastas foram removidos, preservando os arquivos como pastas comuns.

## Estrutura encontrada

- `.gitattributes`: normalizacao de fim de linha (`* text=auto`).
- `.gitignore`: foi ajustado para manter codigo-fonte versionavel e ignorar artefatos pesados/gerados.
- `IC/`: pasta principal do projeto.
- `IC/CDC_LOGS - cesar/`: logs brutos de coletas XSens/ATIMX/EposEXO/MarkovMao em CSV e DAT, organizados por testes `T1` a `T8`.
- `IC/Códigos/`: codigos e artefatos de desenvolvimento, incluindo Python, Arduino/ESP32, C#, modelos `.pkl`, headers de modelos e um repositorio Unity/M2MQTT.
- `IC/Documentos/`: documentos administrativos e relatorios em DOCX/PDF, alem de imagem pessoal e atalho do Windows.
- `IC/esp32_mpu6050/`: arquivos de web server/dados para ESP32/MPU6050.
- `IC/MatLab - cesar/`: scripts MATLAB de utilidades e exemplos.
- `IC/MQTT/`: instaladores de Mosquitto, MQTTBox e pacotes .NET.
- `IC/Papers/`: artigos, PDFs, imagens e arquivos auxiliares.
- `IC/Resultados Pi/`: notebooks, logs, graficos, modelos, codigos C++ e resultados de validacao/classificacao. Esta pasta foi marcada como prioridade no `.gitignore`, entao seus arquivos devem ficar versionaveis mesmo quando forem CSV, PNG, DOCX, PKL ou headers gerados.

Observacao: existem nomes com acentos e espacos. Isso funciona no Git, mas pode atrapalhar automacoes, scripts e ambientes Linux. Para codigo, o ideal e padronizar nomes sem acento e com `_` ou `-`.

## Tecnologias identificadas

- Python: leitura serial, analise CSV, notebooks Jupyter e treinamento/analise de modelos.
- C/C++: leitores/runtime de marcha e possiveis binarios compilados.
- Arduino/ESP32: projeto `.ino` com Wi-Fi, MQTT e sensor MPU6050.
- C#/Unity: projeto `M2MqttUnity`, scripts MQTT e cenas Unity.
- MATLAB: scripts de plotagem e utilidades.
- Machine learning: modelos Random Forest/XGBoost em `.pkl` e modelos exportados para `.h`.
- Dados experimentais: CSV/DAT de sensores e logs de predicao.

## Contagem por tipo de arquivo

| Tipo | Quantidade | Tamanho aproximado |
| --- | ---: | ---: |
| `.csv` | 117 | 133,1 MB |
| `.meta` | 69 | 0,0 MB |
| `.cs` | 50 | 0,3 MB |
| `.png` | 30 | 7,2 MB |
| `.dat` | 16 | 26,0 MB |
| `.asset` | 15 | 0,0 MB |
| `.h` | 12 | 206,5 MB |
| `.docx` | 10 | 5,7 MB |
| `.pdf` | 8 | 10,4 MB |
| `.pkl` | 7 | 75,1 MB |
| `.exe` | 6 | 152,1 MB |
| `.m` | 6 | 0,0 MB |
| `.ipynb` | 4 | 1,7 MB |
| `.py` | 3 | 0,0 MB |
| `.cpp` | 3 | 0,0 MB |

## Maiores arquivos

| Tamanho | Arquivo |
| ---: | --- |
| 153,1 MB | `IC/Códigos/modelo_rf.h` |
| 104,6 MB | `IC/MQTT/NDP472-DevPack-ENU.exe` |
| 92,7 MB | `IC/Códigos/IMUSerial/IMU-Serial---IC/all_Leo_n_Gait_n_Read_ALL_Concatenated_NaN_filled.csv` |
| 68,5 MB | `IC/Códigos/IMUSerial/IMU-Serial---IC/modelo_randomforest.pkl` |
| 27,2 MB | `IC/MQTT/mosquitto-2.0.21-install-windows-x64.exe` |
| 13,7 MB | `IC/MQTT/NDP472-DevPack-PTB.exe` |
| 12,9 MB | `IC/Resultados Pi/modelos/modelx_F3.h` |

O arquivo `IC/Códigos/modelo_rf.h` passa do limite comum de 100 MB do GitHub para arquivos normais. Se ele precisar ficar no historico, use Git LFS; se for gerado a partir de um modelo, deixe ignorado.

## Problemas e riscos

- O `.gitignore` anterior ignorava tambem codigo-fonte (`.py`, `.cpp`, `.h`, `.m`, `.ipynb`, `.json`, `.unity`). Isso deixava o repositorio "limpo", mas escondia arquivos importantes.
- Ha instaladores `.exe` dentro do repositorio. O ideal e documentar links de instalacao, nao versionar instaladores.
- Ha modelos `.pkl` e headers gerados muito grandes. Eles devem ir para Git LFS, DVC, releases ou armazenamento externo.
- Ha documentos possivelmente pessoais/sensiveis em `IC/Documentos/`, incluindo comprovantes, historico escolar, atestado e foto.
- Havia dois repositorios Git dentro de subpastas. Eles foram removidos para que `IC/` fique como uma pasta normal dentro do repositorio principal:
  - `IC/Códigos/IMUSerial/IMU-Serial---IC/.git`
  - `IC/Códigos/Repositorio M2MQTT/M2MqttUnity/.git`
- O arquivo `IC/Códigos/proj_esp_unity/proj_esp_unity.ino` contem SSID, senha e IP de broker MQTT diretamente no codigo. Mesmo sendo rede local, o melhor e mover isso para um arquivo local ignorado ou template sem segredo.
- Alguns arquivos Python aparecem com caracteres quebrados nos comentarios, indicando possivel mistura de encodings. Padronize em UTF-8.

## Recomendacoes de organizacao

- Separar codigo, dados e documentacao:
  - `src/` para Python/C++/Arduino/C#/MATLAB proprio.
  - `notebooks/` para notebooks.
  - `data/raw/` para dados brutos ignorados.
  - `data/processed/` para dados processados ignorados ou controlados por DVC.
  - `models/` para modelos ignorados ou controlados por Git LFS/DVC.
  - `docs/` para documentacao em Markdown.
- Criar um `README.md` com objetivo do projeto, requisitos, como rodar coleta serial, como reproduzir notebooks e como gerar modelos.
- Criar `requirements.txt` ou `environment.yml` para Python.
- Criar um `secrets.example.h` ou `config.example.h` para o ESP32, mantendo credenciais reais fora do Git.
- Manter o projeto M2MQTT como submodule, subtree, dependencia documentada ou pasta externa, em vez de copiar o `.git` interno.
- Evitar arquivos finais em DOCX/PDF no Git normal. Se forem essenciais para entrega academica, considerar Git LFS.

## Gitignore recomendado

O `.gitignore` recomendado foi aplicado na raiz. A ideia principal e:

- Versionar codigo-fonte: `.py`, `.cpp`, `.m`, `.ino`, `.cs`, `.ipynb`, `.json`, `.unity`, `.asset`, `.meta`.
- Versionar todos os arquivos de `IC/Resultados Pi/`, pois essa pasta foi definida como prioridade.
- Ignorar dados e logs: `.csv`, `.dat`, pastas de logs e resultados gerados.
- Ignorar binarios grandes: `.pkl`, instaladores, arquivos compactados, modelos exportados grandes e documentos binarios.
- Ignorar caches: Python, Jupyter, Unity, IDEs e sistema operacional.
- Ignorar credenciais/certificados: `.env`, `.pfx`, `.pem`, `.key`, `.crt`.

Se algum CSV, PDF, imagem ou modelo for pequeno e realmente necessario para reproducibilidade, ha duas boas opcoes:

1. Versionar por excecao no `.gitignore` com `!caminho/do/arquivo`.
2. Usar Git LFS ou DVC para arquivos pesados.

## Proximos passos sugeridos

1. Remover ou mover instaladores de `IC/MQTT/` e documentar links oficiais.
2. Tirar credenciais do arquivo `.ino`.
3. Escolher quais documentos pessoais devem ficar fora do Git.
4. Revisar a lista final de arquivos de `IC/Resultados Pi/` antes do primeiro commit, ja que essa pasta agora entra como prioridade.
5. Rodar `git status --short --untracked-files=all` apos revisar o `.gitignore` para conferir o que ficara pronto para commit.
