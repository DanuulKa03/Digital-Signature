#pragma once
#include <vector>
#include <cstdint>
#include "params.h"
#include "types.h"

namespace ntru {

	// базовые операции
	int  modQ(long long x);
	int  center(int a);
	Poly zeroP();

	// полиномиальная арифметика в R_q = Z_q[X]/(X^N-1)
	Poly subMod(const Poly& A, const Poly& B);
	Poly mulModQ(const Poly& A, const Poly& B);
	Poly mulModPow2(const Poly& A, const Poly& B, int M);

	// GF(2) инверсия + Хензель
	struct Poly2 { std::vector<uint8_t> a; Poly2(); Poly2(int cap); };
	int   deg2(const Poly2& p);
	Poly2 trim2(const Poly2& p);
	Poly2 add2(const Poly2& A, const Poly2& B);
	Poly2 shl2_nonCirc(const Poly2& A, int k);
	Poly2 mul2_nonCirc(const Poly2& A, const Poly2& B);
	void  div2_poly(const Poly2& A, const Poly2& B, Poly2& Q, Poly2& R);
	bool  invertMod2(const Poly& f, Poly& inv2_out);
	Poly  henselLiftToQ(const Poly& f, const Poly& inv2);

} // namespace ntru
