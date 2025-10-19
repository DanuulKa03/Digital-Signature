#include "param_input_output.h"
#include "ui.h"
#include "math/params.h"

#include <iostream>
#include <fstream>

#include "params.h"

bool LoadParametersFile(const std::string& paramPath) {
    std::ifstream in(paramPath);
    if (!in) {
        std::cerr << "�� ������� ������� ���� ����������: " << paramPath << std::endl;
        return false; 
    }

    std::string line;
    int have = 0;
    int N = 0, Q = 0, D = 0, NORM_BOUND = 0, ALPHA = 0, SIGMA = 0, MAX_ATT = 1000;
    double NU = 0.0, ETA = 0.0;

    while (getline(in, line)) {
        line = trim(line); 
        if (line.empty() || line[0] == '#') {
            continue;
        }

        const size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }

        std::string k = trim(line.substr(0, eq)), v = trim(line.substr(eq + 1));
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
        std::cerr << "��������� ��������. �����: N,Q,D,NU,NORM_BOUND,ETA,ALPHA,SIGMA" << std::endl;
        return false; 
    }
    if (N <= 0 || Q <= 0 || D <= 0 || SIGMA <= 0 || ALPHA < 0) {
        std::cerr << "������������ �������� ����������." << std::endl;
        return false; 
    }
    ntru::set_params(N, Q, D, NU, NORM_BOUND, ETA, ALPHA, SIGMA, MAX_ATT);
    return true;
}
