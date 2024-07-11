#include <vector>
#include <chrono>
#include <random>
#include "avxfloat.hpp"
#include "scalar.hpp"
#include <iomanip>
#include <iostream>

const int NUM_VERTICES = 80000;
const int NUM_ITERATIONS = 10000;
const int NUM_WARMUP = 100;

// Generate random transformation matrix
Mat4 generateRandomTransform(std::mt19937& gen) {
    std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
    Mat4 mat{};
    for (auto & i : mat.data) {
        for (float & j : i) {
            j = dis(gen);
        }
    }
    return mat;
}

template<typename T>
void preventOptimization(const T& value) {
    volatile auto temp = value;
    (void)temp;  // Suppress unused variable warning
}

int main() {
    printf("Initializing vertices...\n");
    std::vector<AVXVec4f> vertices_soa(NUM_VERTICES / 8);
    std::vector<vec4f> vertices_aos(NUM_VERTICES);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-100.0f, 100.0f);

    for (int i = 0; i < NUM_VERTICES / 8; ++i) {
        for (int j =0; j < 8; ++j) {
            float x = dis(gen), y = dis(gen), z = dis(gen), w = 1.0f;
            vertices_soa[i].x[j] = x;
            vertices_soa[i].y[j] = y;
            vertices_soa[i].z[j] = z;
            vertices_soa[i].w[j] = w;

            vertices_aos[i * 8 + j] = vec4f(x, y, z, w);
        }
    }

    std::vector<Mat4> transforms(NUM_ITERATIONS);
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        transforms[i] = generateRandomTransform(gen);
    }

    printf("Warmup...\n");
    // Warmup
    for (int i = 0; i < NUM_WARMUP; ++i) {
        const auto &t = transforms[i % NUM_ITERATIONS];

        for (auto &v: vertices_aos) {
            v = transformVertexScalar(t, v);
        }

        for (auto &v: vertices_soa) {
            v = transformVec(v, t);
        }
    }

    // Test!
    printf("Testing scalar...\n");
    float checksum_scalar = 0.0f;
    std::chrono::nanoseconds total_duration_scalar(0);
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        const auto &t = transforms[i];
        for (auto &v: vertices_aos) {
            auto start = std::chrono::high_resolution_clock::now();
            v = transformVertexScalar(t, v);
            auto end = std::chrono::high_resolution_clock::now();
            total_duration_scalar += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
            checksum_scalar = v.x + v.y + v.z + v.w;
        }
    }
    preventOptimization(checksum_scalar);

    printf("Testing AVX2...\n");
    float checksum_avx2 = 0.0f;
    std::chrono::nanoseconds total_duration_avx2(0);
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        const auto &t = transforms[i];
        for (auto &v: vertices_soa) {
            auto start = std::chrono::high_resolution_clock::now();
            v = transformVec(v, t);
            auto end = std::chrono::high_resolution_clock::now();
            total_duration_avx2 += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
            checksum_avx2 = v.x[0] + v.y[0] + v.z[0] + v.w[0];
        }
    }
    preventOptimization(checksum_avx2);

    auto duration_scalar = static_cast<double>(total_duration_scalar.count());
    auto duration_avx2 = static_cast<double>(total_duration_avx2.count());

    double time_per_vertex_scalar_ns = duration_scalar / NUM_ITERATIONS / NUM_VERTICES;
    double time_per_vertex_avx2_ns = duration_avx2 / NUM_ITERATIONS / NUM_VERTICES;

    // Output results
    std::cout << "Test Results:\n";
    std::cout << "=============================\n";
    std::cout << "Num Vertices: " << NUM_VERTICES << "\n";
    std::cout << "Num Iterations: " << NUM_ITERATIONS << "\n";
    std::cout << "=============================\n";

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Scalar:\n";
    std::cout << " - Total: " << duration_scalar / 1e9 << " s\n";
    std::cout << " - Per iteration: " << duration_scalar / NUM_ITERATIONS / 1e6 << " ms\n";
    std::cout << " - Per vertex: " << time_per_vertex_scalar_ns << " ns\n";

    std::cout << "AVX2:\n";
    std::cout << " - Total: " << duration_avx2 / 1e9 << " s\n";
    std::cout << " - Per iteration: " << duration_avx2 / NUM_ITERATIONS / 1e6 << " ms\n";
    std::cout << " - Per vertex: " << time_per_vertex_avx2_ns << " ns\n";

    std::cout << "Speedup: " << std::setprecision(2) << duration_scalar / duration_avx2 << "x\n";

    return 0;
}