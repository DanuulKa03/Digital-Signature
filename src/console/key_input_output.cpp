#include "key_input_output.h"
#include "util.h"
#include <fstream>
#include <iostream>
#include <filesystem>

#include "math/params.h"
#include "math/arithmetic.h"
#include "math/keys.h"

using namespace std;
using namespace ntru;
namespace fs = std::filesystem;

bool SavePrivateKey(const std::string& userPath) {
    std::string path = to_target_file_path(userPath, "private.key", ".key");

    if (!ensure_parent_dirs(path)) { 
        cout << "Не удалось создать каталоги для: " << path << "\n"; 
        return false; 
    }

    ofstream out(path, ios::binary | ios::trunc); 

    if (!out) { 
        cout << "Не удалось создать файл приватного ключа: " << path << "\n"; 
        return false; 
    }

    out << "PRIV1\n" << G_N << "\n";
    for (int i = 0; i < G_N; ++i) { 
        out << G_F[i]; 
        if (i + 1 < G_N) {
            out << ' ';
        }
    } 
    out << "\n";
    for (int i = 0; i < G_N; ++i) { 
        out << G_G[i]; 
        if (i + 1 < G_N) {
            out << ' ';
        }
    } 
    out << "\n";
    out.close(); 
    cout << "Приватный ключ сохранён: " << path << "\n"; 
    return true;
}
bool SavePublicKey(const std::string& userPath) {

    std::string path = to_target_file_path(userPath, "public.key", ".key");
    if (!ensure_parent_dirs(path)) { 
        cout << "Не удалось создать каталоги для: " << path << "\n"; 
        return false; 
    
    }
    ofstream out(path, ios::binary | ios::trunc); 
    if (!out) { 
        cout << "Не удалось создать файл открытого ключа: " << path << "\n"; 
        return false; 
    }

    out << "PUB1\n" << G_N << "\n";
    for (int i = 0; i < G_N; ++i) { 
        out << G_H[i]; 
        if (i + 1 < G_N) {
            out << ' ';
        }
    }

    out << "\n"; 
    out.close(); 
    cout << "Открытый ключ сохранён: " << path << "\n"; 
    return true;
}
bool LoadPrivateKey(const std::string& path) {

    ifstream in(path); 
    if (!in) { 
        cerr << "Не удалось открыть приватный ключ: " << path << "\n"; 
        return false; 
    }

    string hdr; 
    if (!(in >> hdr) || hdr != "PRIV1") { 
        cerr << "Неверный формат приватного ключа\n"; 
        return false; 
    }

    int n; 
    if (!(in >> n) || n != G_N) { 
        cerr << "N не совпадает с параметрами\n"; 
        return false; 
    }

    G_F.assign(G_N, 0); 
    G_G.assign(G_N, 0);

    for (int i = 0; i < G_N; ++i) { 
        long long v; 
        if (!(in >> v)) { 
            cerr << "Недостаточно коэффициентов f\n"; 
            return false; 
        } 
        G_F[i] = modQ(v); 
    }

    for (int i = 0; i < G_N; ++i) { 
        long long v; 
        if (!(in >> v)) { 
            cerr << "Недостаточно коэффициентов g\n"; 
            return false; 
        } 
        G_G[i] = modQ(v); 
    }

    Poly inv2(G_N, 0); 
    if (!invertMod2(G_F, inv2)) { 
        cerr << "f не инвертируется по mod 2\n"; 
        return false; 
    }

    Poly Finv = henselLiftToQ(G_F, inv2);
    G_H = mulModQ(Finv, G_G);
    return true;
}
bool LoadPublicKey(const std::string& path) {
    ifstream in(path); 

    if (!in) { 
        cerr << "Не удалось открыть публичный ключ: " << path << "\n"; 
        return false; 
    }

    string hdr; 
    if (!(in >> hdr) || hdr != "PUB1") { 
        cerr << "Неверный формат публичного ключа\n"; 
        return false; 
    }

    int n; 
    if (!(in >> n) || n != G_N) { 
        cerr << "N не совпадает с параметрами\n"; 
        return false; 
    }

    G_H.assign(G_N, 0);
    for (int i = 0; i < G_N; ++i) { 
        long long v; 
        if (!(in >> v)) { 
            cerr << "Недостаточно коэффициентов h\n"; 
            return false; 
        } 
        G_H[i] = modQ(v); 
    }
    return true;
}
