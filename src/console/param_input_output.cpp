#include "param_input_output.h"
#include "ui.h"
#include <fstream>
#include <iostream>
#include "math/params.h"

using namespace std;
using namespace ntru;

bool LoadParametersFile(const string& paramPath) {
    ifstream in(paramPath);
    if (!in) { 
        cerr << "Не удалось открыть файл параметров: " << paramPath << "\n"; 
        return false; 
    }

    string line; 
    int have = 0;
    int N = 0, Q = 0, D = 0, NORM_BOUND = 0, ALPHA = 0, SIGMA = 0, MAX_ATT = 1000;
    double NU = 0.0, ETA = 0.0;

    while (getline(in, line)) {
        line = trim(line); 
        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t eq = line.find('='); 
        if (eq == string::npos) {
            continue;
        }

        string k = trim(line.substr(0, eq)), v = trim(line.substr(eq + 1));
        if (k == "N") { 
            N = stoi(v); 
            have++; 
        }
        else if (k == "Q") { 
            Q = stoi(v); 
            have++; 
        }
        else if (k == "D") {
            D = stoi(v); 
            have++; 
        }
        else if (k == "NU") { 
            NU = stod(v); 
            have++; 
        }
        else if (k == "NORM_BOUND") { 
            NORM_BOUND = stoi(v); 
            have++; 
        }
        else if (k == "ETA") { 
            ETA = stod(v); 
            have++; 
        }
        else if (k == "ALPHA") { 
            ALPHA = stoi(v); 
            have++; 
        }
        else if (k == "SIGMA") { 
            SIGMA = stoi(v); 
            have++; 
        }
        else if (k == "MAX_SIGN_ATTEMPTS_MASK") { 
            MAX_ATT = stoi(v); 
        }
    }
    if (have < 8) { 
        cerr << "Параметры неполные. Нужны: N,Q,D,NU,NORM_BOUND,ETA,ALPHA,SIGMA\n"; 
        return false; 
    }
    if (N <= 0 || Q <= 0 || D <= 0 || SIGMA <= 0 || ALPHA < 0) { 
        cerr << "Некорректные значения параметров.\n"; 
        return false; 
    }
    set_params(N, Q, D, NU, NORM_BOUND, ETA, ALPHA, SIGMA, MAX_ATT);
    return true;
}
