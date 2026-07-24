
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <complex>
#include <csignal>
#include <cstdint>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <termios.h>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

// Compile:
// g++ -O3 -march=native -ffast-math gait_runtime_v5_3.cpp -o gait_runtime_v5_3 -lm
//
// Este runtime foi alinhado com o pipeline do notebook V5.3:
// - usa o header exportado do XGBoost principal
// - gera exatamente 647 features na mesma ordem
// - usa janela de 30 amostras
// - usa roll/pitch filtrados por um Kalman simples
// - calcula sinais derivados incrementalmente para bater com o preprocess do notebook

#include "gait_xgb_v5_3_min_model.h"

using std::array;
using std::cout;
using std::cerr;
using std::deque;
using std::endl;
using std::ofstream;
using std::string;
using std::vector;

static volatile bool keep_running = true;

// ========================= TERMINAL HANDLING =========================
struct termios original_termios;

void setup_terminal() {
    tcgetattr(STDIN_FILENO, &original_termios);
    struct termios raw = original_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void restore_terminal() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

bool is_key_pressed() {
    char buf[1];
    return (read(STDIN_FILENO, buf, 1) > 0);
}

void sigint_handler(int) {
    keep_running = false;
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
        close(file);
        return -1;
    }

    return file;
}

void init_mpu6050(int file) {
    char config[2] = {0x6B, 0x00};  // Wake up MPU6050
    write(file, config, 2);

    // Accelerometer: +/-4g
    char acc_conf[2] = {0x1C, 0x08};
    write(file, acc_conf, 2);

    // Gyroscope: default +/-250 deg/s
    char gyro_conf[2] = {0x1B, 0x00};
    write(file, gyro_conf, 2);

    usleep(10000);
}

short read_word_2c(int file, int addr) {
    char reg[1] = {(char) addr};
    if (write(file, reg, 1) != 1) return 0;
    char data[2];
    if (read(file, data, 2) != 2) return 0;
    short val = (short)((data[0] << 8) | (unsigned char)data[1]);
    if (val >= 0x8000)
        val = -((65535 - val) + 1);
    return val;
}

// ========================= SIMPLE KALMAN =========================
class SimpleKalman {
public:
    SimpleKalman(double process_noise = 0.01,
                 double sensor_noise = 0.1,
                 double estimated_error = 1.0,
                 double initial_value = 0.0)
        : Q(process_noise), R(sensor_noise), P(estimated_error), x(initial_value) {}

    double update(double measurement) {
        P = P + Q;
        const double K = P / (P + R);
        x = x + K * (measurement - x);
        P = (1.0 - K) * P;
        return x;
    }

private:
    double Q;
    double R;
    double P;
    double x;
};

// ========================= MODEL CONSTANTS =========================
constexpr int WINDOW_SIZE = 30;
constexpr int RUNTIME_STEP = 5;
constexpr bool ENABLE_CLASS0_REJECT = false;   // desligado por padrão
constexpr double REJECT_GYRO_STD_MAX = 1.5;    // heurística opcional
constexpr double REJECT_ACC_STD_MAX = 0.03;    // heurística opcional

enum AxisIndex {
    IDX_AX = 0,
    IDX_AY,
    IDX_AZ,
    IDX_GX,
    IDX_GY,
    IDX_GZ,
    IDX_ROLL,
    IDX_PITCH,
    IDX_ACC_NORM,
    IDX_GYRO_NORM,
    IDX_D_AX,
    IDX_D_AY,
    IDX_D_AZ,
    IDX_D_GX,
    IDX_D_GY,
    IDX_D_GZ,
    IDX_DD_AX,
    IDX_DD_AY,
    IDX_DD_AZ,
    IDX_DD_GX,
    IDX_DD_GY,
    IDX_DD_GZ,
    IDX_JERK_ACC,
    IDX_JERK_GYRO,
    NUM_SIGNALS
};

struct Frame {
    std::int64_t timestamp_ms = 0;
    array<float, NUM_SIGNALS> v{};

    float ax() const { return v[IDX_AX]; }
    float ay() const { return v[IDX_AY]; }
    float az() const { return v[IDX_AZ]; }
    float gx() const { return v[IDX_GX]; }
    float gy() const { return v[IDX_GY]; }
    float gz() const { return v[IDX_GZ]; }
    float roll() const { return v[IDX_ROLL]; }
    float pitch() const { return v[IDX_PITCH]; }
};

const char* class_name(int cls) {
    switch (cls) {
        case 0: return "Desconhecida";
        case 1: return "Heel-strike";
        case 2: return "Foot-flat";
        case 3: return "Heel-off";
        case 4: return "Toe-off";
        case 5: return "Swing";
        default: return "N/A";
    }
}

// ========================= STABILITY FILTER =========================
class StabilityFilter {
public:
    explicit StabilityFilter(size_t size = 9)
        : history(size, 0), index(0), filled(false) {}

    int filter(int raw_prediction) {
        history[index] = raw_prediction;
        index = (index + 1) % history.size();
        if (index == 0) filled = true;

        std::map<int, int> counts;
        int best_cls = raw_prediction;
        int best_count = -1;
        const size_t n = filled ? history.size() : index;

        for (size_t i = 0; i < n; i++) {
            counts[history[i]]++;
            if (counts[history[i]] > best_count) {
                best_count = counts[history[i]];
                best_cls = history[i];
            }
        }

        return best_cls;
    }

private:
    vector<int> history;
    size_t index;
    bool filled;
};

// ========================= FEATURE UTILS =========================
double mean_vec(const vector<double>& v) {
    if (v.empty()) return 0.0;
    return std::accumulate(v.begin(), v.end(), 0.0) / (double) v.size();
}

double std_vec(const vector<double>& v) {
    if (v.empty()) return 0.0;
    const double mu = mean_vec(v);
    double acc = 0.0;
    for (double x : v) {
        const double d = x - mu;
        acc += d * d;
    }
    acc /= (double) v.size();
    return std::sqrt(acc);
}

double median_vec(vector<double> v) {
    if (v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    const int n = (int) v.size();
    if (n % 2 == 0) return 0.5 * (v[n / 2 - 1] + v[n / 2]);
    return v[n / 2];
}

double percentile_linear(vector<double> v, double p) {
    if (v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    const double pos = (p / 100.0) * (double)(v.size() - 1);
    const int idx = (int) std::floor(pos);
    const double frac = pos - (double) idx;
    if (idx + 1 < (int) v.size())
        return v[idx] * (1.0 - frac) + v[idx + 1] * frac;
    return v[idx];
}

double zero_cross_count(const vector<double>& signal) {
    if (signal.size() < 2) return 0.0;
    int count = 0;
    for (size_t i = 1; i < signal.size(); i++) {
        const double a = signal[i - 1];
        const double b = signal[i];
        if ((a >= 0.0 && b < 0.0) || (a < 0.0 && b >= 0.0))
            count++;
    }
    return (double) count;
}

double safe_corr(const vector<double>& a, const vector<double>& b) {
    if (a.size() != b.size() || a.size() < 2) return 0.0;
    const double sa = std_vec(a);
    const double sb = std_vec(b);
    if (sa < 1e-12 || sb < 1e-12) return 0.0;

    const double ma = mean_vec(a);
    const double mb = mean_vec(b);

    double cov = 0.0;
    for (size_t i = 0; i < a.size(); i++)
        cov += (a[i] - ma) * (b[i] - mb);

    cov /= (double) a.size();
    return cov / (sa * sb);
}

double peak_position_ratio(const vector<double>& signal, bool absolute_value) {
    if (signal.empty()) return 0.0;
    int idx = 0;
    double best = absolute_value ? std::fabs(signal[0]) : signal[0];
    for (int i = 1; i < (int) signal.size(); i++) {
        const double cur = absolute_value ? std::fabs(signal[i]) : signal[i];
        if (cur > best) {
            best = cur;
            idx = i;
        }
    }
    return (double) idx / std::max((int) signal.size() - 1, 1);
}

double time_since_last_peak_ratio(const vector<double>& signal) {
    const int n = (int) signal.size();
    if (n < 3) return 0.0;
    int last_peak = -1;
    for (int i = 1; i < n - 1; i++) {
        if (signal[i] > signal[i - 1] && signal[i] >= signal[i + 1])
            last_peak = i;
    }
    if (last_peak < 0) return 1.0;
    return (double) ((n - 1) - last_peak) / std::max(n - 1, 1);
}

double center_slope(const vector<double>& signal, int radius = 2) {
    const int n = (int) signal.size();
    if (n < 2) return 0.0;
    const int c = n / 2;
    const int left = std::max(0, c - radius);
    const int right = std::min(n - 1, c + radius);
    if (right == left) return 0.0;
    return (signal[right] - signal[left]) / std::max(right - left, 1);
}

double local_amplitude(const vector<double>& signal, int radius = 3) {
    if (signal.empty()) return 0.0;
    const int n = (int) signal.size();
    const int c = n / 2;
    const int start = std::max(0, c - radius);
    const int end = std::min(n, c + radius + 1);
    auto minmax = std::minmax_element(signal.begin() + start, signal.begin() + end);
    return *minmax.second - *minmax.first;
}

double half_ratio(const vector<double>& signal) {
    const int n = (int) signal.size();
    const int half = n / 2;
    if (half == 0 || half == n) return 0.0;
    vector<double> first(signal.begin(), signal.begin() + half);
    vector<double> second(signal.begin() + half, signal.end());
    return mean_vec(second) / (std::fabs(mean_vec(first)) + 1e-6);
}

vector<double> spectral_features(const vector<double>& signal) {
    const int n = (int) signal.size();
    if (n == 0) return {0.0, 0.0, 0.0};

    const double mu = mean_vec(signal);
    vector<double> centered(n);
    for (int i = 0; i < n; i++) centered[i] = signal[i] - mu;

    const int max_k = n / 2;  // rfft bins
    vector<double> mag;
    mag.reserve(max_k + 1);

    for (int k = 0; k <= max_k; k++) {
        std::complex<double> sum(0.0, 0.0);
        for (int t = 0; t < n; t++) {
            const double angle = -2.0 * M_PI * (double) k * (double) t / (double) n;
            sum += centered[t] * std::complex<double>(std::cos(angle), std::sin(angle));
        }
        mag.push_back(std::abs(sum));
    }

    if (mag.size() <= 1) return {0.0, 0.0, 0.0};

    vector<double> spec(mag.begin() + 1, mag.end());  // remove DC
    double total = std::accumulate(spec.begin(), spec.end(), 0.0);
    if (total <= 1e-12) return {0.0, 0.0, 0.0};

    const int idx = (int) std::distance(spec.begin(), std::max_element(spec.begin(), spec.end()));
    const double dom_bin = (double) (idx + 1);
    const double dom_ratio = *std::max_element(spec.begin(), spec.end()) / (total + 1e-12);

    double entropy = 0.0;
    for (double x : spec) {
        const double p = x / (total + 1e-12);
        if (p > 1e-12)
            entropy -= p * std::log(p + 1e-12);
    }
    entropy /= std::log((double) spec.size() + 1e-12);

    return {dom_bin, dom_ratio, entropy};
}

vector<float> extract_signal_features(const vector<double>& signal) {
    vector<float> feats;
    feats.reserve(25);

    const int n = (int) signal.size();
    if (n == 0) {
        feats.assign(25, 0.0f);
        return feats;
    }

    vector<double> diff;
    if (n > 1) {
        diff.reserve(n - 1);
        for (int i = 1; i < n; i++)
            diff.push_back(signal[i] - signal[i - 1]);
    }
    else {
        diff.push_back(0.0);
    }

    const int center = n / 2;
    const int c0 = std::max(0, center - 2);
    const int c1 = std::min(n, center + 3);
    vector<double> center_slice(signal.begin() + c0, signal.begin() + c1);

    const double min_val = *std::min_element(signal.begin(), signal.end());
    const double max_val = *std::max_element(signal.begin(), signal.end());
    const double amp = max_val - min_val;

    const int diff_center = (int) diff.size() / 2;
    const int d0 = std::max(0, diff_center - 1);
    const int d1 = std::min((int) diff.size(), diff_center + 2);
    vector<double> diff_center_slice(diff.begin() + d0, diff.begin() + d1);

    double rms = 0.0;
    double energy = 0.0;
    for (double x : signal) {
        rms += x * x;
        energy += x * x;
    }
    rms = std::sqrt(rms / (double) n);
    energy /= (double) n;

    int idx_abs_max = 0;
    for (int i = 1; i < n; i++) {
        if (std::fabs(signal[i]) > std::fabs(signal[idx_abs_max]))
            idx_abs_max = i;
    }
    const int idx_min = (int) std::distance(signal.begin(), std::min_element(signal.begin(), signal.end()));

    feats.push_back((float) mean_vec(signal));
    feats.push_back((float) std_vec(signal));
    feats.push_back((float) median_vec(signal));
    feats.push_back((float) min_val);
    feats.push_back((float) max_val);
    feats.push_back((float) percentile_linear(signal, 10.0));
    feats.push_back((float) percentile_linear(signal, 25.0));
    feats.push_back((float) percentile_linear(signal, 75.0));
    feats.push_back((float) percentile_linear(signal, 90.0));
    feats.push_back((float) signal.front());
    feats.push_back((float) signal.back());
    feats.push_back((float) (signal.back() - signal.front()));
    feats.push_back((float) mean_vec(center_slice));

    double mean_abs_diff = 0.0;
    for (double d : diff) mean_abs_diff += std::fabs(d);
    mean_abs_diff /= (double) diff.size();
    feats.push_back((float) mean_abs_diff);
    feats.push_back((float) std_vec(diff));

    feats.push_back((float) rms);
    feats.push_back((float) energy);
    feats.push_back((float) amp);
    feats.push_back((float) ((double) idx_abs_max / std::max(n - 1, 1)));
    feats.push_back((float) ((double) idx_min / std::max(n - 1, 1)));
    feats.push_back((float) zero_cross_count(signal));
    feats.push_back((float) mean_vec(diff_center_slice));

    vector<double> spec = spectral_features(signal);
    feats.push_back((float) spec[0]);
    feats.push_back((float) spec[1]);
    feats.push_back((float) spec[2]);

    return feats;
}

vector<double> get_axis_signal(const deque<Frame>& window, int axis_idx) {
    vector<double> s;
    s.reserve(window.size());
    for (const auto& f : window)
        s.push_back((double) f.v[axis_idx]);
    return s;
}

vector<float> build_feature_vector(const deque<Frame>& window) {
    vector<float> features;
    features.reserve(Eloquent::ML::Port::GaitXGB::NUM_FEATURES);

    const int axis_order[] = {
        IDX_AX, IDX_AY, IDX_AZ,
        IDX_GX, IDX_GY, IDX_GZ,
        IDX_ROLL, IDX_PITCH,
        IDX_ACC_NORM, IDX_GYRO_NORM,
        IDX_D_AX, IDX_D_AY, IDX_D_AZ,
        IDX_D_GX, IDX_D_GY, IDX_D_GZ,
        IDX_DD_AX, IDX_DD_AY, IDX_DD_AZ,
        IDX_DD_GX, IDX_DD_GY, IDX_DD_GZ,
        IDX_JERK_ACC, IDX_JERK_GYRO
    };

    for (int axis : axis_order) {
        vector<double> sig = get_axis_signal(window, axis);
        vector<float> f = extract_signal_features(sig);
        features.insert(features.end(), f.begin(), f.end());
    }

    // Pairwise correlations
    const std::pair<int, int> corr_pairs[] = {
        {IDX_AX, IDX_AY}, {IDX_AX, IDX_AZ}, {IDX_AY, IDX_AZ},
        {IDX_GX, IDX_GY}, {IDX_GX, IDX_GZ}, {IDX_GY, IDX_GZ},
        {IDX_ACC_NORM, IDX_GYRO_NORM},
        {IDX_ROLL, IDX_PITCH}
    };

    for (auto p : corr_pairs) {
        vector<double> a = get_axis_signal(window, p.first);
        vector<double> b = get_axis_signal(window, p.second);
        features.push_back((float) safe_corr(a, b));
    }

    // Half-comparison features
    const int half_axes[] = {IDX_ACC_NORM, IDX_GYRO_NORM, IDX_ROLL, IDX_PITCH, IDX_D_AX, IDX_D_GZ};
    const int half = (int) window.size() / 2;

    for (int axis : half_axes) {
        vector<double> sig = get_axis_signal(window, axis);
        vector<double> first(sig.begin(), sig.begin() + half);
        vector<double> second(sig.begin() + half, sig.end());

        features.push_back((float) (mean_vec(second) - mean_vec(first)));
        features.push_back((float) (std_vec(second) - std_vec(first)));
        features.push_back((float) (mean_vec(second) / (std::fabs(mean_vec(first)) + 1e-6)));
    }

    // Transition-focus features
    const int trans_axes[] = {IDX_GX, IDX_GZ, IDX_PITCH, IDX_ROLL};
    for (int axis : trans_axes) {
        vector<double> sig = get_axis_signal(window, axis);
        const bool use_abs_peak = (axis == IDX_GX || axis == IDX_GZ);
        features.push_back((float) center_slope(sig));
        features.push_back((float) peak_position_ratio(sig, use_abs_peak));
        features.push_back((float) time_since_last_peak_ratio(sig));
        features.push_back((float) half_ratio(sig));
    }

    vector<double> pitch_sig = get_axis_signal(window, IDX_PITCH);
    vector<double> roll_sig = get_axis_signal(window, IDX_ROLL);
    vector<double> gz_sig = get_axis_signal(window, IDX_GZ);

    features.push_back((float) local_amplitude(pitch_sig));
    features.push_back((float) local_amplitude(roll_sig));
    features.push_back((float) peak_position_ratio(gz_sig, true));
    features.push_back((float) center_slope(gz_sig));
    features.push_back((float) local_amplitude(gz_sig));

    return features;
}

bool should_reject_as_unknown(const deque<Frame>& window) {
    if (!ENABLE_CLASS0_REJECT) return false;

    vector<double> acc_norm = get_axis_signal(window, IDX_ACC_NORM);
    vector<double> gyro_norm = get_axis_signal(window, IDX_GYRO_NORM);

    const double acc_std = std_vec(acc_norm);
    const double gyro_std = std_vec(gyro_norm);

    return (acc_std < REJECT_ACC_STD_MAX && gyro_std < REJECT_GYRO_STD_MAX);
}

// ========================= STREAM PREPROCESS =========================
class StreamPreprocessor {
public:
    StreamPreprocessor() : kalman_roll(), kalman_pitch(), initialized(false) {
        prev_base.fill(0.0f);
        prev_d.fill(0.0f);
    }

    Frame process(float ax, float ay, float az, float gx, float gy, float gz, std::int64_t timestamp_ms) {
        Frame f;
        f.timestamp_ms = timestamp_ms;

        // Base channels
        f.v[IDX_AX] = ax;
        f.v[IDX_AY] = ay;
        f.v[IDX_AZ] = az;
        f.v[IDX_GX] = gx;
        f.v[IDX_GY] = gy;
        f.v[IDX_GZ] = gz;

        // Orientation + norms
        const double raw_roll = std::atan2((double) ay, (double) az) * 180.0 / M_PI;
        const double raw_pitch = std::atan2(-(double) ax, std::sqrt((double) ay * ay + (double) az * az)) * 180.0 / M_PI;
        f.v[IDX_ROLL] = (float) kalman_roll.update(raw_roll);
        f.v[IDX_PITCH] = (float) kalman_pitch.update(raw_pitch);

        const double acc_norm = std::sqrt((double) ax * ax + (double) ay * ay + (double) az * az);
        const double gyro_norm = std::sqrt((double) gx * gx + (double) gy * gy + (double) gz * gz);
        f.v[IDX_ACC_NORM] = (float) acc_norm;
        f.v[IDX_GYRO_NORM] = (float) gyro_norm;

        // Incremental derivatives aligned with notebook preprocess
        const array<float, 8> cur_base = {
            f.v[IDX_AX], f.v[IDX_AY], f.v[IDX_AZ],
            f.v[IDX_GX], f.v[IDX_GY], f.v[IDX_GZ],
            f.v[IDX_ACC_NORM], f.v[IDX_GYRO_NORM]
        };

        array<float, 8> cur_d{};
        if (!initialized) {
            cur_d.fill(0.0f);
        }
        else {
            for (int i = 0; i < 8; i++)
                cur_d[i] = cur_base[i] - prev_base[i];
        }

        array<float, 6> cur_dd{};
        if (!initialized) {
            cur_dd.fill(0.0f);
        }
        else {
            for (int i = 0; i < 6; i++)
                cur_dd[i] = cur_d[i] - prev_d[i];
        }

        f.v[IDX_D_AX] = cur_d[0];
        f.v[IDX_D_AY] = cur_d[1];
        f.v[IDX_D_AZ] = cur_d[2];
        f.v[IDX_D_GX] = cur_d[3];
        f.v[IDX_D_GY] = cur_d[4];
        f.v[IDX_D_GZ] = cur_d[5];

        f.v[IDX_DD_AX] = cur_dd[0];
        f.v[IDX_DD_AY] = cur_dd[1];
        f.v[IDX_DD_AZ] = cur_dd[2];
        f.v[IDX_DD_GX] = cur_dd[3];
        f.v[IDX_DD_GY] = cur_dd[4];
        f.v[IDX_DD_GZ] = cur_dd[5];

        f.v[IDX_JERK_ACC] = cur_d[6];
        f.v[IDX_JERK_GYRO] = cur_d[7];

        prev_base = cur_base;
        for (int i = 0; i < 6; i++) prev_d[i] = cur_d[i];
        initialized = true;

        return f;
    }

private:
    SimpleKalman kalman_roll;
    SimpleKalman kalman_pitch;
    bool initialized;
    array<float, 8> prev_base;
    array<float, 6> prev_d;
};

// ========================= MAIN =========================
int main() {
    signal(SIGINT, sigint_handler);
    setup_terminal();

    const string filename = "log_v5_3_runtime.csv";
    ofstream csv(filename);
    csv << "timestamp,ax,ay,az,gx,gy,gz,roll,pitch_k,acc_norm,gyro_norm,classe_prevista_raw,classe_prevista,delay_ms\n";

    int fd = open_i2c();
    if (fd < 0) {
        restore_terminal();
        return 1;
    }

    init_mpu6050(fd);

    cout << "Starting gait runtime aligned with V5.3" << endl;
    cout << "Model expects " << Eloquent::ML::Port::GaitXGB::NUM_FEATURES << " features." << endl;
    cout << "Press any key to stop." << endl;

    Eloquent::ML::Port::GaitXGB model;
    StreamPreprocessor pre;
    StabilityFilter stability_filter(9);

    deque<Frame> window;
    int last_raw_prediction = 0;
    int last_smooth_prediction = 0;
    int sample_counter = 0;

    while (keep_running) {
        if (is_key_pressed()) {
            cout << "\n[INFO] Key pressed. Stopping loop." << endl;
            break;
        }

        const auto t1 = std::chrono::steady_clock::now();

        const float ax = (float) read_word_2c(fd, 0x3B) / 8192.0f;
        const float ay = (float) read_word_2c(fd, 0x3D) / 8192.0f;
        const float az = (float) read_word_2c(fd, 0x3F) / 8192.0f;

        // Notebook uses gx/gy/gz directly in deg/s scale; no extra *0.01
        const float gx = (float) read_word_2c(fd, 0x43) / 131.0f;
        const float gy = (float) read_word_2c(fd, 0x45) / 131.0f;
        const float gz = (float) read_word_2c(fd, 0x47) / 131.0f;

        const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        Frame f = pre.process(ax, ay, az, gx, gy, gz, now_ms);
        window.push_back(f);
        if ((int) window.size() > WINDOW_SIZE)
            window.pop_front();

        sample_counter++;
        if ((int) window.size() == WINDOW_SIZE && (sample_counter % RUNTIME_STEP == 0)) {
            if (should_reject_as_unknown(window)) {
                last_raw_prediction = 0;
                last_smooth_prediction = 0;
            }
            else {
                vector<float> feats = build_feature_vector(window);

                if ((int) feats.size() != (int) Eloquent::ML::Port::GaitXGB::NUM_FEATURES) {
                    cerr << "[ERROR] Feature count mismatch: got " << feats.size()
                         << " expected " << Eloquent::ML::Port::GaitXGB::NUM_FEATURES << endl;
                    last_raw_prediction = -1;
                    last_smooth_prediction = -1;
                }
                else {
                    last_raw_prediction = model.predict(feats.data());
                    last_smooth_prediction = stability_filter.filter(last_raw_prediction);
                }
            }
        }

        const auto t2 = std::chrono::steady_clock::now();
        const double delay_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();

        csv << f.timestamp_ms << ","
            << f.v[IDX_AX] << "," << f.v[IDX_AY] << "," << f.v[IDX_AZ] << ","
            << f.v[IDX_GX] << "," << f.v[IDX_GY] << "," << f.v[IDX_GZ] << ","
            << f.v[IDX_ROLL] << "," << f.v[IDX_PITCH] << ","
            << f.v[IDX_ACC_NORM] << "," << f.v[IDX_GYRO_NORM] << ","
            << last_raw_prediction << "," << last_smooth_prediction << ","
            << std::fixed << std::setprecision(3) << delay_ms << "\n";

        cout << "\rax:" << std::fixed << std::setprecision(3) << f.v[IDX_AX]
             << " ay:" << f.v[IDX_AY]
             << " az:" << f.v[IDX_AZ]
             << " gx:" << f.v[IDX_GX]
             << " gy:" << f.v[IDX_GY]
             << " gz:" << f.v[IDX_GZ]
             << " | raw:" << last_raw_prediction << " (" << class_name(last_raw_prediction) << ")"
             << " | smooth:" << last_smooth_prediction << " (" << class_name(last_smooth_prediction) << ")"
             << " | delay:" << delay_ms << " ms"
             << std::flush;

        usleep(10000);  // ~100 Hz
    }

    cout << endl;
    close(fd);
    csv.close();
    restore_terminal();
    cout << "[INFO] Stopped gracefully. CSV saved to " << filename << endl;
    return 0;
}
