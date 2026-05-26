# Organizacao da pasta IC

Esta pasta foi reorganizada em uma sequencia numerada para refletir melhor o fluxo da pesquisa:

1. `01_documentacao/` - documentos academicos, comprovantes, relatorios, resumos e apresentacoes.
2. `02_referencias/` - papers, figuras de artigos e materiais usados como referencia.
3. `03_dados/` - dados brutos de coleta e logs originais.
4. `04_codigo/` - codigos de aquisicao, analise, ESP32, Unity/MQTT e MATLAB.
5. `05_modelos/` - modelos treinados e resultados intermediarios de treinamento fora da pasta principal de resultados.
6. `06_resultados/` - resultados priorizados do projeto. A antiga pasta `Resultados Pi` esta aqui como `resultados_pi/`.
7. `07_ferramentas/` - instaladores e ferramentas externas usadas ao longo da pesquisa.

Regra importante: `06_resultados/resultados_pi/` foi mantida como prioridade no `.gitignore`, entao todos os arquivos dessa pasta devem ficar visiveis para versionamento.
