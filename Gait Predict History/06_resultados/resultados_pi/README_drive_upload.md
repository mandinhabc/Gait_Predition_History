# Salvamento dos CSVs no Google Drive

O `gait_reader10.cpp` cria um CSV novo em cada execução, no diretório de onde o
programa é iniciado. O nome segue o formato
`log_v5_3_AAAAMMDD_HHMMSS_mmm_PID.csv`. Depois que a coleta termina por tecla
ou Ctrl+C, o programa fecha o CSV e envia esse mesmo arquivo ao Google Drive.
O envio acontece após a coleta para que o arquivo no Drive esteja completo.

## Configuração no Raspberry Pi

1. Instale o `rclone` e configure um remoto do tipo Google Drive com o nome
   `gdrive` executando `rclone config`. A configuração inicial exige autorização
   da conta Google; em um Pi sem navegador, siga o fluxo de autorização remota
   exibido pelo próprio `rclone config`. As instruções oficiais estão em
   https://rclone.org/drive/ . Para novas configurações, use seu próprio client ID
   OAuth do Google, conforme as instruções atuais do rclone.
2. Escolha a pasta de destino e configure-a no ambiente do processo:

   ```bash
   export GAIT_DRIVE_DEST='gdrive:Experimentos/Gait'
   rclone mkdir "$GAIT_DRIVE_DEST"
   ```

   Substitua `Experimentos/Gait` pela pasta desejada. Também é possível usar
   apenas `gdrive:` para a raiz do Drive. Se o programa for iniciado por um
   serviço, defina essa variável no ambiente do serviço, usando o mesmo usuário
   que configurou o rclone.
3. Compile a partir da raiz do projeto no Raspberry Pi:

   ```bash
   g++ -O3 -march=native -ffast-math \
     -I 06_resultados/resultados_pi/modelos/New_models \
     06_resultados/resultados_pi/gait_reader10.cpp -o gait_reader10 -lm
   ```
4. Inicie `./gait_reader10` e encerre a coleta com uma tecla ou Ctrl+C. O
   programa informa o nome do CSV local e confirma o destino após o envio.

O envio usa `rclone copy --immutable`, que recusa substituir um arquivo
diferente com o mesmo nome no Drive. Se não houver configuração ou ocorrer erro
de rede, o CSV local é preservado e o programa retorna código 2. Para reenviar
um arquivo depois de corrigir a conexão:

```bash
arquivo='log_v5_3_AAAAMMDD_HHMMSS_mmm_PID.csv'
rclone copy --immutable --include "/$arquivo" . "$GAIT_DRIVE_DEST"
```

Confira o nome real informado pelo programa antes de executar o reenvio.
