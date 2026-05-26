#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <deque>
#include <array>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <algorithm>
#include <numeric>
#include <termios.h>
#include <vector>
#include <map>

// SEU MODELO LOCAL
#include "rf_embedded_small_FIXED.h" 

using namespace std;


class Kalman {
private:
    // Parâmetros de Ajuste
    double Q; // Ruído do Processo (Inércia/Mudança real esperada)
    double R; // Ruído da Medição (Erro do Sensor)
    
    // Estado do Filtro
    double P; // Covariância do erro da estimativa (Incerteza atual)
    double x; // Estado estimado (O valor "limpo")
    double K; // Ganho de Kalman

public:
    // Construtor: Inicializa com R (sensor), Q (movimento) e valor inicial
    Kalman(double process_noise, double sensor_noise, double estimated_error, double initial_value) {
        this->Q = process_noise;
        this->R = sensor_noise;
        this->P = estimated_error;
        this->x = initial_value; 
    }

    // O coração do algoritmo: Atualiza a estimativa com uma nova leitura
    double update(double measurement) {
        // 1. Predição (Time Update)
        // P = P + Q (A incerteza aumenta um pouco com o tempo)
        P = P + Q;

        // 2. Correção (Measurement Update)
        // K = P / (P + R) (Calcula o peso: confio mais na predição ou no sensor?)
        K = P / (P + R);
        
        // x = x + K * (medida - x) (Atualiza o valor estimado)
        x = x + K * (measurement - x);
        
        // P = (1 - K) * P (Reduz a incerteza após a correção)
        P = (1 - K) * P;

        return x; // Retorna o dado limpo
    }
};

// The model was trained on 42 features (6 axes * 7 features/axis)
const int FEATURE_COUNT = 42; 

// ========================= TERMINAL HANDLING =========================
struct termios original_termios;

// Sets terminal to raw mode (no buffering, no echo)
void setup_terminal() {
    tcgetattr(STDIN_FILENO, &original_termios);
    struct termios raw = original_termios;
    
    // Disable canonical mode (line buffering) and echo
    raw.c_lflag &= ~(ICANON | ECHO); 
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// Restores terminal settings to normal (canonical) mode
void restore_terminal() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

// Checks if a key has been pressed without blocking
bool is_key_pressed() {
    char buf[1];
    return (read(STDIN_FILENO, buf, 1) > 0);
}

// ========================= MPU6050 CONFIG =========================
#define MPU_ADDR 0x68

int open_i2c() {
    int file = open("/dev/i2c-1", O_RDWR);
    if (file < 0) {
        perror("[ERROR] Failed to open /dev/i2c-1");
        return -1;
    }
    if (ioctl(file, I2C_SLAVE, MPU_ADDR) < 0) {
        perror("[ERROR] Failed to set I2C address (0x68)");
        return -1;
    }
    return file;
}

void init_mpu6050(int file) {
    char config[2];
    config[0] = 0x6B; // Power management
    config[1] = 0x00; // Wake up MPU6050
    write(file, config, 2);
    
    // 2. Configurar +/- 4g (Registro 0x1C recebe 0x08)
    char acc_conf[2] = {0x1C, 0x08}; 
    write(file, acc_conf, 2);
    
    usleep(10000);
}

short read_word_2c(int file, int addr) {
    char reg[1] = { (char)addr };
    write(file, reg, 1);
    char data[2];
    read(file, data, 2);
    short val = (data[0] << 8) | data[1];
    if (val >= 0x8000)
        val = -((65535 - val) + 1);
    return val;
}

// ========================= SIGNAL HANDLER (FOR CTRL+C FALLBACK) =========================
bool keep_running = true;
void sigint_handler(int) {
    keep_running = false;
}

// ========================= FEATURE EXTRACTION =========================
// This function generates 42 features (7 features for each of the 6 axes)
vector<double> compute_features(const deque<array<double,6>> &window) {
    vector<double> feats;
    feats.reserve(FEATURE_COUNT);

    for (int i = 0; i < 6; ++i) {
        vector<double> v;
        for (auto &s : window) v.push_back(s[i]);

        double sum = 0.0;
        for (auto x : v) sum += x;
        double mean = sum / v.size();

        double var = 0.0;
        for (auto x : v) var += (x - mean) * (x - mean);
        var /= v.size();

        // 7 Features per axis: Mean, StdDev, Front, Back, Min, Max, Range
        feats.push_back(mean);
        feats.push_back(sqrt(var));
        feats.push_back(v.front());
        feats.push_back(v.back());
        feats.push_back(*min_element(v.begin(), v.end()));
        feats.push_back(*max_element(v.begin(), v.end()));
        feats.push_back(v.back() - v.front());
    }
    return feats;
}

class StabilityFilter {
private:
    std::vector<int> history;
    size_t window_size;
    size_t index;

public:
    // window_size: 10 a 20 costuma ser ótimo para 100Hz
    StabilityFilter(size_t size) : window_size(size), index(0) {
        history.resize(window_size, 0); // Inicia com classe 0
    }

    int filter(int raw_prediction) {
        // 1. Adiciona a nova predição ao histórico circular
        history[index] = raw_prediction;
        index = (index + 1) % window_size;

        // 2. Conta qual classe aparece mais (Voto Majoritário)
        std::map<int, int> counts;
        int max_count = 0;
        int mode = raw_prediction;

        for (int val : history) {
            counts[val]++;
            if (counts[val] > max_count) {
                max_count = counts[val];
                mode = val;
            }
        }
        
        return mode; // Retorna a classe vencedora
    }
};

// ========================= MAIN LOOP =========================
int main() {
    signal(SIGINT, sigint_handler);
    setup_terminal(); // Enable non-canonical mode

    const int WINDOW_SIZE = 10;
    deque<array<double,6>> window;
    ofstream csv;
    string filename = "log_kalman_teste1.csv";
    csv.open(filename);
    

    // Adicionei colunas do Kalman no Header
    csv << "timestamp,ax,ay,az,gx,gy,gz,roll_k,pitch_k,delay_ms,classe_prevista\n";

    cout << "Starting real-time gait reader (100Hz) WITH KALMAN." << endl;
    cout << "Press any key to stop." << endl;

    // --- Initialize MPU6050 ---
    int fd = open_i2c();
    if (fd < 0) {
        restore_terminal(); 
        return 1;
    }
    init_mpu6050(fd);
    cout << "[OK] MPU6050 initialized on /dev/i2c-1 (0x68)" << endl;
    StabilityFilter stability_filter(10); // Janela de 10 predições para suavizar a saída   

    // --- Initialize the local model object ---
    Eloquent::ML::Port::RandomForest model;

    while (keep_running) {
        // --- TERMINATION CHECK (Any key press) ---
        if (is_key_pressed()) {
            cout << "\n[INFO] Key pressed. Stopping loop." << endl;
            keep_running = false;
            break; 
        }
        // ------------------------------------------
        // --- 1. CONFIGURAÇÃO (TUNING) ---
         // R (Sensor Noise): Calcule a variância com o sensor parado
         double R_accel = 0.5; 
         double R_gyro = 0.3;  

         // Q (Process Noise): Ajuste fino (0.001 = muito liso/lag, 0.1 = rápido/ruidoso)
         double Q_gait = 10000; 

         // --- 2. INSTANCIAÇÃO DOS 6 FILTROS ---
         // Sintaxe: KalmanFilter(Q, R, ErroInicial, ValorInicial)
         Kalman kf_ax(Q_gait, R_accel, 1.0, 0.0);
         Kalman kf_ay(Q_gait, R_accel, 1.0, 0.0);
         Kalman kf_az(Q_gait, R_accel, 1.0, 0.0); // Z geralmente tem a gravidade (~9.8 ou 1g)

         Kalman kf_gx(Q_gait, R_gyro, 1.0, 0.0);
         Kalman kf_gy(Q_gait, R_gyro, 1.0, 0.0);
         Kalman kf_gz(Q_gait, R_gyro, 1.0, 0.0);

         auto last_time = chrono::steady_clock::now();
         array<double,6> clean_data{}; // Para armazenar os dados filtrados
         double roll_k = 0.0;
         double pitch_k = 0.0;



    while (keep_running) {
        if (is_key_pressed()) {
            cout << "\n[INFO] Key pressed. Stopping loop." << endl;
            keep_running = false;
            break; 
        }

        auto t1 = chrono::steady_clock::now();
        
        // Calcular DT para o Kalman
        double dt = chrono::duration<double>(t1 - last_time).count();
        last_time = t1;

        array<double,6> sample{};
        
        // --- LEITURA AJUSTADA PARA +/- 4G (DIVIDIR POR 8192.0) ---
        sample[0] = read_word_2c(fd, 0x3B) / 8192.0; // ax
        sample[1] = read_word_2c(fd, 0x3D) / 8192.0; // ay
        sample[2] = read_word_2c(fd, 0x3F) / 8192.0; // az
        sample[3] = read_word_2c(fd, 0x43) / 131.0;  // gx (Mantive padrao 250dps)
        sample[4] = read_word_2c(fd, 0x45) / 131.0;  // gy
        sample[5] = read_word_2c(fd, 0x47) / 131.0;  // gz
        
        // --- 4. FILTRAGEM (O "Grosso" do trabalho) ---
        clean_data[0] = kf_ax.update(sample[0]);
        clean_data[1] = kf_ay.update(sample[1]);
        clean_data[2] = kf_az.update(sample[2]);
        clean_data[3] = kf_gx.update(sample[3]);
        clean_data[4] = kf_gy.update(sample[4]);
        clean_data[5] = kf_gz.update(sample[5]);
        

        window.push_back(sample);
        if ((int)window.size() > WINDOW_SIZE)
            window.pop_front();

        int predicted = -1;
        if ((int)window.size() == WINDOW_SIZE) {
            vector<double> feats = compute_features(window);
            
            // --- CALL LOCAL C++ MODEL ---
            if (feats.size() == FEATURE_COUNT) {
                float input_features[FEATURE_COUNT]; 
                for (int i = 0; i < FEATURE_COUNT; ++i) {
                    input_features[i] = (float)feats[i]; 
                }
                
                // Add 1 to the zero-indexed output (0-4) to get the 1-indexed class (1-5)
                predicted = model.predict(input_features) + 1; 
                predicted = stability_filter.filter(predicted); // Aplica o filtro de estabilidade
            }
        }

        auto t2 = chrono::steady_clock::now();
        double delay_ms = chrono::duration<double, milli>(t2 - t1).count();

        auto now_ms = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now().time_since_epoch()).count();

        csv << now_ms;
        for (int i = 0; i < 6; ++i) csv << "," << clean_data[i];
           // Salvando Kalman no CSV
        csv << "," << roll_k << "," << pitch_k;
        
        csv << "," << delay_ms << "," << predicted << "\n";

        cout << "ax:" << clean_data[0]
             << " ay:" << clean_data[1]
             << " az:" << clean_data[2]
             << " gx:" << clean_data[3]
             << " gy:" << clean_data[4]
             << " gz:" << clean_data[5]
             << " | Class: " << predicted
             << " | Delay: " << delay_ms << " ms" << endl;
        

        // Exibindo Kalman na tela
        cout << " | Dly: " << delay_ms << "ms\r" << flush; 

        usleep(10000); // 10 ms between reads (100 Hz)
    } // CLOSES while (keep_running)

    // --- CLEANUP ---
    close(fd);
    csv.close();
    restore_terminal(); // Restore terminal settings
    cout << "[INFO] Stopped gracefully. Terminal restored.\n";
    return 0;
}}
