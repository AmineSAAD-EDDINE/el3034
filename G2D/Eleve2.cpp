#pragma warning( disable : 4996 ) 


#include <cstdlib>
#include <vector>
#include <iostream>
#include <string>
#include "G2D.h"
using namespace std;

// touche P   : mets en pause
// touche ESC : ferme la fenÃªtre et quitte le jeu


///////////////////////////////////////////////////////////////////////////////
//
//    DonnÃ©es du jeu - structure instanciÃ©e dans le main

struct GameData
{
	int     idFrame = 0;
	int     HeightPix = 800;          // hauteur de la fenÃªtre de jeu
	int     WidthPix = 600;          // largeur de la fenÃªtre de jeu
	V2      BallPos = V2(100, 100);
	V2      BallMove;
	int     BallRadius = 15;

	vector<V2> PreviousPos;  // stocke les derniÃ¨res positions de la boule

	// bords du flipper
	vector<V2> LP{ V2(595, 550), V2(585, 596), V2(542, 638), V2(476, 671), V2(392, 692), V2(300, 700), V2(207, 692),
		V2(123, 671), V2(57, 638), V2(14, 596), V2(5, 550), V2(5,5), V2(595,5), V2(595,550) };


	GameData()
	{
		PreviousPos.resize(50); // stocke les 50 derniÃ¨res positions connues
		BallMove = V2(10, 10);   // vecteur dÃ©placement
	}

};

struct Bumper {
    V2 pos;
    float rad;
    float lastCollisionTime;

    Bumper(V2 p, float r) : pos(p), radius(r), lastCollisionTime(-10) {}
};

// 0 pas d'intersection
// 1/2/3 intersection entre le segment AB et le cercle de rayon r
int CollisionSegCir(V2 A, V2 B, float r, V2 C)
{
	V2 AB = B - A;
	V2 T = AB;
	T.normalize();
	float d = prodScal(T, C - A);
	if (d > 0 && d < AB.norm())
	{
		V2 P = A + d * T; // proj de C sur [AB]
		V2 PC = C - P;
		if (PC.norm() < r) return 2;
		else               return 0;
	}
	if ((C - A).norm() < r) return 1;
	if ((C - B).norm() < r) return 3;
	return 0;
}
V2 Rebond(V2 N, V2 T)
{
	N.normalize();
	V2 T_rot = V2(N.y, -N.x);  // rotation de 90° du vecteur n sens horaire
	float vt = prodScal(T_rot, T);    // produit scalaire, vt est un nombre
	float vn = prodScal(N, T);    // produit scalaire, vn est un nombre
	V2 R = (vt * T_rot) - (vn * N); // * entre un flottant et un V2
	return R;
}

void TestRebond() {
	V2 N = V2 ( -1, 1 );
	V2 V = V2 (1, 0);
	V2 R = Rebond(V, N);
	std::cout <<" <R.x>, <R.y>";
}


///////////////////////////////////////////////////////////////////////////////
//
// 
//     fonction de rendu - reÃ§oit en paramÃ¨tre les donnÃ©es du jeu par rÃ©fÃ©rence



void render(const GameData& G)
{
	// fond noir	 
	G2D::clearScreen(Color::Black);

	// Titre en haut
	G2D::drawStringFontMono(V2(80, G.HeightPix - 70), string("Super Flipper"), 50, 5, Color::Blue);

	// la bille
	G2D::drawCircle(G.BallPos, G.BallRadius, Color::Red, true);

	// 3 bumpers
	G2D::drawCircle(V2(200, 400), 40, Color::Blue, true);
	G2D::drawCircle(V2(400, 400), 40, Color::Blue, true);
	G2D::drawCircle(V2(300, 550), 40, Color::Blue, true);


	// les bords

	for (int i = 0; i < G.LP.size() - 1; i++)
		G2D::drawLine(G.LP[i], G.LP[i + 1], Color::Green);

	// les positions prÃ©cÃ©dentes

	for (V2 P : G.PreviousPos)
		G2D::setPixel(P, Color::Green);

	// prÃ©cise que l'on est en pause

	if (G2D::isOnPause())
		G2D::drawStringFontMono(V2(100, G.HeightPix / 2), string("Pause..."), 50, 5, Color::Yellow);

	// envoie les tracÃ©s Ã  l'Ã©cran

	G2D::Show();
}


///////////////////////////////////////////////////////////////////////////////
//
//
//      Gestion de la logique du jeu - reÃ§oit en paramÃ¨tre les donnÃ©es du jeu par rÃ©fÃ©rence



void Logic(GameData& G) // appelÃ© 20 fois par seconde
{
	G.idFrame += 1;
	G.BallPos = G.BallPos + G.BallMove;

	for (int i = 0; i < G.LP.size() - 1; i++) {

		int CollisionType = CollisionSegCir(G.LP[i], G.LP[i + 1], G.BallRadius, G.BallPos);

		if (CollisionType == 2) {
			V2 SegmentDir = G.LP[i + 1] - G.LP[i];
			V2 Normal = V2(-SegmentDir.y, SegmentDir.x);

			G.BallMove = Rebond(Normal, G.BallMove);
		}
	}

	G.PreviousPos.push_back(G.BallPos);
	G.PreviousPos.erase(G.PreviousPos.begin());

}


///////////////////////////////////////////////////////////////////////////////
//
//
//        DÃ©marrage de l'application



int main(int argc, char* argv[])
{
	GameData G;   // instanciation de l'unique objet GameData qui sera passÃ© aux fonctions render et logic

	G2D::initWindow(V2(G.WidthPix, G.HeightPix), V2(200, 200), string("Super Flipper 600 !!"));

	int callToLogicPerSec = 50;  // si vous rÃ©duisez cette valeur => ralentit le jeu

	G2D::Run(Logic, render, G, callToLogicPerSec, true);
}





