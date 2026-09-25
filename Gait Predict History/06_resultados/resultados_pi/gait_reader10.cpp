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
#include <map>
#include <vector>
#include <deque>
#include <array>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <complex>


// COMANDO DE COMPILAÇÃO
// g++ -O3 -march=native -ffast-math gait_reader5.cpp -o gait_reader8 -lm 

// SEU MODELO LOCAL
#include "gait_xgb_v5_3_min_model.h" 

using namespace std;

double ax;
double ay;
double az;

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
    const int AXES = 8;
    const int FEATURES = 23;

vector<complex<double>> dft(const vector<double>& signal) {
    int N = signal.size();
    vector<complex<double>> X(N);

    for (int k = 0; k < N; k++) {
        complex<double> sum(0.0, 0.0);

        for (int n = 0; n < N; n++) {
            double angle = -2.0 * M_PI * k * n / N;
            sum += signal[n] * complex<double>(cos(angle), sin(angle));
        }

        X[k] = sum;
    }

    return X;
}

vector<double> spectral_features(const vector<double>& signal) {
    if (signal.empty()) {
        return {0.0, 0.0, 0.0};
    }

    int n = signal.size();

    double m = 0.0;
    for (double x : signal) m += x;
    m /= n;

    vector<double> centered(n);
    for (int i = 0; i < n; i++) {
        centered[i] = signal[i] - m;
    }

    vector<complex<double>> X = dft(centered);

    vector<double> spec;
    for (auto& z : X) {
        spec.push_back(abs(z));
    }

    if (spec.size() <= 1) {
        return {0.0, 0.0, 0.0};
    }

    spec.erase(spec.begin()); // remove componente DC

    double spec_sum = 0.0;
    for (double x : spec) spec_sum += x;

    double spec_energy = 0.0;
    for (double x : spec) {
        spec_energy += x * x;
    }

    spec_energy /= spec.size();

    int idx_max = max_element(spec.begin(), spec.end()) - spec.begin();
    double dom_freq = (double)idx_max / max((int)spec.size() - 1, 1);

    double entropy = 0.0;
    if (spec_sum > 1e-12) {
        for (double x : spec) {
            double p = x / spec_sum;
            if (p > 1e-12) {
                entropy -= p * log(p);
            }
        }
    }

    return {spec_energy, dom_freq, entropy};
}

vector<double> compute_features_full(const deque<array<double, 8>>& window) {
    vector<double> feats;

    const int AXES = 8;
    const int FEATURES_PER_AXIS = 25;

    feats.reserve(AXES * FEATURES_PER_AXIS);

    auto mean_vec = [](const vector<double>& v) {
        if (v.empty()) return 0.0;

        double sum = 0.0;
        for (double x : v) sum += x;

        return sum / v.size();
    };

    auto std_vec = [&](const vector<double>& v) {
        if (v.empty()) return 0.0;

        double mean = mean_vec(v);
        double var = 0.0;

        for (double x : v) {
            var += (x - mean) * (x - mean);
        }

        var /= v.size(); // ddof = 0
        return sqrt(var);
    };

    auto median = [](vector<double> v) {
        if (v.empty()) return 0.0;

        sort(v.begin(), v.end());
        int n = v.size();

        if (n % 2 == 0) {
            return (v[n / 2 - 1] + v[n / 2]) / 2.0;
        }

        return v[n / 2];
    };

    auto percentile = [](vector<double> v, double p) {
        if (v.empty()) return 0.0;

        sort(v.begin(), v.end());

        double pos = (p / 100.0) * (v.size() - 1);
        int idx = (int)pos;
        double frac = pos - idx;

        if (idx + 1 < (int)v.size()) {
            return v[idx] * (1.0 - frac) + v[idx + 1] * frac;
        }

        return v[idx];
    };

    auto zero_cross_count = [](const vector<double>& v) {
        int count = 0;

        for (int i = 1; i < (int)v.size(); i++) {
            if ((v[i - 1] >= 0 && v[i] < 0) ||
                (v[i - 1] < 0 && v[i] >= 0)) {
                count++;
            }
        }

        return count;
    };

    for (int axis = 0; axis < AXES; axis++) {
        vector<double> signal;
        signal.reserve(window.size());

        for (const auto& s : window) {
            signal.push_back(s[axis]);
        }

        int n = signal.size();

        if (n == 0) {
            for (int k = 0; k < FEATURES_PER_AXIS; k++) {
                feats.push_back(0.0);
            }
            continue;
        }

        vector<double> diff;
        diff.reserve(max(n - 1, 0));

        for (int i = 1; i < n; i++) {
            diff.push_back(signal[i] - signal[i - 1]);
        }

        double min_val = *min_element(signal.begin(), signal.end());
        double max_val = *max_element(signal.begin(), signal.end());
        double amp = max_val - min_val;

        int center = n / 2;
        int c_start = max(0, center - 2);
        int c_end = min(n, center + 3);

        vector<double> center_window(signal.begin() + c_start,
                                     signal.begin() + c_end);

        double center_mean = mean_vec(center_window);

        double mean_abs_diff = 0.0;

        if (!diff.empty()) {
            for (double d : diff) {
                mean_abs_diff += abs(d);
            }
            mean_abs_diff /= diff.size();
        }

        double rms = 0.0;
        double energy = 0.0;

        for (double x : signal) {
            rms += x * x;
            energy += x * x;
        }

        rms = sqrt(rms / n);
        energy = energy / n;

        int idx_max_abs = 0;
        for (int i = 1; i < n; i++) {
            if (abs(signal[i]) > abs(signal[idx_max_abs])) {
                idx_max_abs = i;
            }
        }

        int idx_min = min_element(signal.begin(), signal.end()) - signal.begin();

        double norm_idx_max_abs = (double)idx_max_abs / max(n - 1, 1);
        double norm_idx_min = (double)idx_min / max(n - 1, 1);

        double diff_center_mean = 0.0;

        if (!diff.empty()) {
            int dn = diff.size();
            int d_center = dn / 2;

            int d_start = max(0, d_center - 1);
            int d_end = min(dn, d_center + 2);

            vector<double> diff_center(diff.begin() + d_start,
                                       diff.begin() + d_end);

            diff_center_mean = mean_vec(diff_center);
        }

        feats.push_back(mean_vec(signal));
        feats.push_back(std_vec(signal));
        feats.push_back(median(signal));

        feats.push_back(min_val);
        feats.push_back(max_val);

        feats.push_back(percentile(signal, 10));
        feats.push_back(percentile(signal, 25));
        feats.push_back(percentile(signal, 75));
        feats.push_back(percentile(signal, 90));

        feats.push_back(signal.front());
        feats.push_back(signal.back());
        feats.push_back(signal.back() - signal.front());

        feats.push_back(center_mean);

        feats.push_back(mean_abs_diff);
        feats.push_back(std_vec(diff));

        feats.push_back(rms);
        feats.push_back(energy);

        feats.push_back(amp);

        feats.push_back(norm_idx_max_abs);
        feats.push_back(norm_idx_min);

        feats.push_back((double)zero_cross_count(signal));

        feats.push_back(diff_center_mean);

        vector<double> spec = spectral_features(signal);
        feats.insert(feats.end(), spec.begin(), spec.end());
    }

    return feats;
}

class StabilityFilter {
private:
    std::vector<int> history;
    size_t window_size;
    size_t index;

public:
    // window_size: 10 a 20 costuma ser otimo para 100Hz
    StabilityFilter(size_t size) : window_size(size), index(0) {
        history.resize(window_size, 0); // Inicia com classe 0
    }

    int filter(int raw_prediction) {
        // 1. Adiciona a nova predicao ao historico circular
        history[index] = raw_prediction;
        index = (index + 1) % window_size;

        // 2. Conta qual classe aparece mais (Voto Majoritario)
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

    const int WINDOW_SIZE = 30;
    deque<array<double,8>> window;
    ofstream csv;
    string filename = "log_v5_3.csv";
    csv.open(filename);

    // Adicionei colunas do Kalman no Header
    csv << "timestamp,ax,ay,az,gx,gy,gz,roll,pitch,delay_ms,classe_prevista\n";

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
    StabilityFilter stability_filter(30); // Janela de 10 predicoes para suavizar a saida   
           
    // Variáveis para guardar o erro de montagem do sensor hoje
    double gyro_offset_x = 0, gyro_offset_y = 0, gyro_offset_z = 0;
    double pitch_offset = 0, roll_offset = 0;

    cout << "CALIBRANDO... COLOQUE O TÊNIS NO CHÃO E FIQUE TOTALMENTE PARADO POR 2 SEGUNDOS." << endl;

    for (int i = 0; i < 200; i++) {
        int16_t ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw;
        // Use a sua função real de leitura aqui:
        ax_raw = read_word_2c(fd, 0x3B) / 8192.0; // ax
        ay_raw = read_word_2c(fd, 0x3D) / 8192.0; // ay
        az_raw = read_word_2c(fd, 0x3F) / 8192.0; // az
        gx_raw = read_word_2c(fd, 0x43) ;  // gx (Mantive padrao 250dps)
        gy_raw = read_word_2c(fd, 0x45)  ; // gy
        gz_raw = read_word_2c(fd, 0x47) ; // gz

        // Acumula os erros do giroscópio
        gyro_offset_x += (gx_raw / 131.0);
        gyro_offset_y += (gy_raw / 131.0);
        gyro_offset_z += (gz_raw / 131.0);

        // Acumula os ângulos em que o tênis está parado agora
        roll_offset += atan2(ay_raw, az_raw) * 180.0 / M_PI;
        pitch_offset += atan2(-ax_raw, sqrt(ay_raw * ay_raw + az_raw * az_raw)) * 180.0 / M_PI;

        usleep(10000); // Espera 10ms
    }

    // Tira a média de tudo
    gyro_offset_x /= 200.0; 
    gyro_offset_y /= 200.0; 
    gyro_offset_z /= 200.0;
    roll_offset /= 200.0; 
    pitch_offset /= 200.0;

    cout << "Calibrado com sucesso! Pode começar a andar." << endl;


    // --- Initialize the local model object ---
    Eloquent::ML::Port::GaitXGB model;

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
         double R_accel = 0.3; 
         double R_gyro = 0.3;  

         // Q (Process Noise): Ajuste fino (0.001 = muito liso/lag, 0.1 = rápido/ruidoso)
         double Q_gait = 0.001; 
         
         Kalman kalmanRoll(0.01, 0.1, 1.0, 0.0);
         Kalman kalmanPitch(0.01, 0.1, 1.0, 0.0);

         // --- 2. INSTANCIAÇÃO DOS 6 FILTROS ---
         // Sintaxe: KalmanFilter(Q, R, ErroInicial, ValorInicial)
         Kalman kf_ax(Q_gait, R_accel, 1.0, 0.0);
         Kalman kf_ay(Q_gait, R_accel, 1.0, 0.0);
         Kalman kf_az(Q_gait, R_accel, 1.0, 0.0); // Z geralmente tem a gravidade (~9.8 ou 1g)

         Kalman kf_gx(Q_gait, R_gyro, 1.0, 0.0);
         Kalman kf_gy(Q_gait, R_gyro, 1.0, 0.0);
         Kalman kf_gz(Q_gait, R_gyro, 1.0, 0.0);
         
         auto last_time = chrono::steady_clock::now();
         array<double,8> clean_data{}; // Para armazenar os dados filtrados
      
    while (keep_running) {
        if (is_key_pressed()) {
            cout << "\n[INFO] Key pressed. Stopping loop." << endl;
            keep_running = false;
            break; 
        }

        auto t1 = chrono::steady_clock::now();
        

        array<double,8> sample{};
        
        // --- LEITURA AJUSTADA PARA +/- 4G (DIVIDIR POR 8192.0) ---
        sample[0] = read_word_2c(fd, 0x3B) / 8192.0; // ax
        sample[1] = read_word_2c(fd, 0x3D) / 8192.0; // ay
        sample[2] = read_word_2c(fd, 0x3F) / 8192.0; // az
        sample[3] = read_word_2c(fd, 0x43) / 131.0;  // gx (Mantive padrao 250dps)
        sample[4] = read_word_2c(fd, 0x45) / 131.0;  // gy
        sample[5] = read_word_2c(fd, 0x47) / 131.0;  // gz
        
        ax = sample[0];
        ay = sample[1];
        az = sample[2]; 
        
        // Calcular DT para o Kalman
        double dt = chrono::duration<double>(t1 - last_time).count();
        last_time = t1;
        double raw_roll = atan2(ay, az) * 180.0 / M_PI;
        double raw_pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / M_PI;

        // 3. Passa os ângulos brutos pelo Filtro de Kalman
        double roll_k = kalmanRoll.update(raw_roll);
        double pitch_k = kalmanPitch.update(raw_pitch); 
        
        sample[6] = roll_k;
        sample[7] = pitch_k;
        
        // --- 4. FILTRAGEM (O "Grosso" do trabalho) ---
        clean_data[0] = kf_ax.update(sample[0]);
        clean_data[1] = kf_ay.update(sample[1]);
        clean_data[2] = kf_az.update(sample[2]);
        clean_data[3] = kf_gx.update(sample[3]) * 0.01;
        clean_data[4] = kf_gy.update(sample[4]) * 0.01;
        clean_data[5] = kf_gz.update(sample[5]) * 0.01;
        

        window.push_back(sample);
        if ((int)window.size() > WINDOW_SIZE)
            window.pop_front();

        int predicted = -1;
        if ((int)window.size() == WINDOW_SIZE) {
            vector<double> feats = compute_features_full(window);
            
            // --- CALL LOCAL C++ MODEL ---
            if (feats.size() == (AXES * FEATURES)) {
                float input_features[AXES * FEATURES]; 
                for (int i = 0; i < AXES * FEATURES; ++i) {
                    input_features[i] = (float)feats[i]; 
                }
                
                predicted = model.predict(input_features); 
                predicted = stability_filter.filter(predicted); // Aplica o filtro de instabilidade
            }
        }

        auto t2 = chrono::steady_clock::now();
        double delay_ms = chrono::duration<double, milli>(t2 - t1).count();

        auto now_ms = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now().time_since_epoch()).count();

        csv << now_ms;
        for (int i = 0; i < 8; ++i) csv << "," << clean_data[i];
           // Salvando Kalman no CSV
        
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


