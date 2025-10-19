#include "sign_input_output.h"
#include "util.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include "math/params.h"

using namespace std;
using namespace ntru;
namespace fs = std::filesystem;

bool write_sig(const std::string& sigPath, const Signature& S) {
    if (!ensure_parent_dirs(sigPath)) { 
        cout << "Не удалось создать каталоги для: " << sigPath << "\n"; 
        return false; 
    }
    ofstream out(sigPath, ios::binary | ios::trunc);
    if (!out) { 
        cerr << "Не удалось создать файл подписи: " << sigPath << "\n"; 
        return false; 
    }
    const char magic[4] = { 'S','G','N','2' }; 
    out.write(magic, 4);
    auto write_poly_u16 = [&](const Poly& P) { 
        for (int i = 0; i < G_N; ++i) { 
            uint16_t v = (uint16_t)P[i]; 
            out.write((const char*)&v, sizeof(v)); 
        } 
        };

    write_poly_u16(S.x1); 
    write_poly_u16(S.x2); 
    write_poly_u16(S.e);

    out.close(); 
    cout << "Подпись сохранена: " << sigPath << "\n"; 
    return true;
}
bool read_sig(const std::string& sigPath, Signature& S) {
    ifstream in(sigPath, ios::binary | ios::ate);
    if (!in) { 
        cerr << "Не удалось открыть файл подписи: " << sigPath << "\n"; 
        return false; 
    }
    streamoff sz = in.tellg(); in.seekg(0, ios::beg);
    char magic[4]; in.read(magic, 4); 
    if (in.gcount() != 4 || memcmp(magic, "SGN2", 4) != 0) { 
        cout << "Неверный формат подписи\n"; 
        return false; 
    }
    S.x1.assign(G_N, 0); S.x2.assign(G_N, 0); S.e.assign(G_N, 0);
    auto read_poly_u16 = [&](Poly& P) { 
        for (int i = 0; i < G_N; ++i) { 
            uint16_t v; in.read((char*)&v, sizeof(v)); 
            P[i] = (int)v; 
        } 
        };
    read_poly_u16(S.x1); 
    read_poly_u16(S.x2); 
    read_poly_u16(S.e);

    size_t expect = 4 + (size_t)3 * G_N * 2;
    if (sz != (streamoff)expect) { 
        cout << "Размер подписи не совпадает с ожиданием\n"; 
        return false; 
    }
    return true;
}
