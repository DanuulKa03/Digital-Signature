#pragma once
#include "types.h"
#include "rng.h"

namespace ntru {

	// ключи
	extern Poly G_F, G_G, G_H;

	// генерация тернарного полинома
	void genTernary(Poly& a, SimpleRng& rng);

	// keygen: F,G -> H = G * F^{-1} (mod q)
	bool keygen();

} // namespace ntru
