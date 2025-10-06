#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <filesystem>
#include <cstdint>

#include "ui.h"
#include "util.h"
#include "param_input_output.h"
#include "key_input_output.h"
#include "sign_input_output.h"

#include "math/params.h"
#include "math/arithmetic.h"
#include "math/hash.h"
#include "math/keys.h"
#include "math/sign.h"

#define NOMINMAX
#include <windows.h>

using namespace std;
using namespace ntru;
namespace fs = std::filesystem;

// ---- ���� ----
static bool KeygenFlow() {

    string params = readPathLine("������� ���� � ����� ����������: ");
    if (params.empty() || !LoadParametersFile(params)) {
        return false;
    }

    if (!keygen()) { 
        cerr << "�� ������� ������������� ����� (���������� ������ ���������)\n"; 
        return false; 
    }

    while (true) {
        string where = readPathLine("���� ��������� ��������� ���� (����� ��� ����): ");

        if (where.empty()) { 
            cout << "[!] ���� ������. ���������.\n"; 
            continue; 
        }

        if (SavePrivateKey(where)) {
            break;
        }
    }
    while (true) {
        string where = readPathLine("���� ��������� �������� ���� (����� ��� ����): ");

        if (where.empty()) { 
            cout << "[!] ���� ������. ���������.\n"; 
            continue; 
        }

        if (SavePublicKey(where)) {
            break;
        }
    }
    return true;
}
static bool SignFlow() {

    string params = readPathLine("������� ���� � ����� ����������: ");
    if (params.empty() || !LoadParametersFile(params)) {
        return false;
    }

    string priv = readPathLine("������� ���� � ����� ���������� �����: ");
    if (priv.empty() || !LoadPrivateKey(priv)) {
        return false;
    }


    string doc = readPathLine("������� ���� � �����, ������� ����� ���������: ");
    if (doc.empty()) { 
        cout << "[!] ���� ������.\n"; 
        return false; 
    }

    ifstream in(doc, ios::binary); 
    if (!in) { 
        cerr << "�� ������� ������� " << doc << "\n"; 
        return false; 
    }
    vector<uint8_t> msg((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());

    Signature S;

    if (!sign_strict(msg, S)) 
    { cerr << "������� �� ������� (rejection stage)\n"; 
    return false; 
    }

    string sigPath = readPathLine("���� ��������� ���� ������� (����� ��� ��� .sig): ");
    if (sigPath.empty()) { 
        cout << "[!] ���� ������.\n"; 
        return false; 
    }

    if (is_directory_like(sigPath)) {
        fs::path p(sigPath); fs::path base = fs::path(doc).filename(); base += ".sig";
        sigPath = (p / base).string();
    }

    else {
        fs::path p(sigPath); if (!p.has_extension()) p.replace_extension(".sig"); sigPath = p.string();
    }

    return write_sig(sigPath, S);
}
static bool VerifyFlow() {

    string doc = readPathLine("������� ���� � ��������� ����� (���������): ");
    if (doc.empty() || !fs::exists(doc)) { 
        cout << "[!] ���� �� ������.\n"; 
        return false; 
    }

    string params = readPathLine("������� ���� � ����� ����������: ");
    if (params.empty() || !LoadParametersFile(params)) {
        return false;
    }

    string pub = readPathLine("������� ���� � ����� ��������� �����: ");
    if (pub.empty() || !LoadPublicKey(pub)) {
        return false;
    }

    string sigPath = readPathLine("������� ���� � ����� ������� (.sig): ");
    if (sigPath.empty()) { 
        cout << "[!] ���� ������.\n"; 
        return false; 
    }

    Signature S; 
    if (!read_sig(sigPath, S)) {
        return false;
    }

    ifstream in(doc, ios::binary);
    vector<uint8_t> msg((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());

    Poly hx1 = mulModQ(G_H, S.x1);
    Poly z = subMod(S.x2, hx1);
    EHash eh2 = H_e_small(z, msg);

    for (int i = 0; i < G_N; ++i) {
        if (eh2.e_mod[i] != S.e[i]) {
            cout << "������� ��������������� (hash mismatch)\n"; 
            return false;
        }
    }
    long double x2norm = 0;
    for (int i = 0; i < G_N; ++i) { 
        int a = center(S.x1[i]); 
        int b = center(S.x2[i]); 
        x2norm += (long double)a * a + (long double)b * b; 
    }
    long double bound = (long double)G_ETA * (long double)G_SIGMA * sqrtl(2.0L * (long double)G_N);

    if (sqrtl(x2norm) > bound) { 
        cout << "������� ��������������� (norm)\n"; 
        return false; 
    }

    cout << "������� �������������\n"; return true;
}

int main() {
    setlocale(LC_ALL, "Rus");
    ios::sync_with_stdio(false); cin.tie(nullptr);
    SetupConsoleZoom(22); SetupConsole();

    while (true) {
        PrintMenu();
        int c; 

        if (!(cin >> c)) {
            return 0;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (c) {
        case 0:
            break;

        case 1:
            if (!KeygenFlow())
                cout << "[!] ��������� �� ���������.\n";
            WaitForEnter();
            break;

        case 2:
            if (!SignFlow())
                cout << "[!] ������� �� ���������.\n";
            WaitForEnter();
            break;

        case 3:
            if (!VerifyFlow())
                cout << "[!] �������� �� ���������.\n";
            WaitForEnter();
            break;

        default:
            cout << "�������� �����.\n";
            WaitForEnter();
            break;
        }

    return 0;
}
