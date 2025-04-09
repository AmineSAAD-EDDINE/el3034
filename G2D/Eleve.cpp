#pragma warning(disable : 4996)

#include <cstdlib>
#include <vector>
#include <iostream>
#include <string>
#include "G2D.h"
#include <cmath>

#define _MATH_DEFINES_DEFINED
#define M_PI       3.14159265358979323846

using namespace std;

// touche P   : mets en pause
// touche ESC : ferme la fenêtre et quitte le jeu


///////////////////////////////////////////////////////////////////////////////
//
//    Données du jeu - structure instanciée dans le main


struct Bumper {
    V2 pos;
    float rayon;
    float tpsdernierecollision;

    Bumper(V2 p, float r) : pos(p), rayon(r), tpsdernierecollision(-10) {}
};


struct Cible {
    V2 A, B;
    bool active;
    int rangee;

    Cible(V2 a, V2 b, int numrang) : A(a), B(b), active(true), rangee(numrang) {}
};

struct Flipper {
    V2 pivot;        
    float angle;     
    float longueur;
    bool actif;      
    bool gauche;        

    Flipper(V2 p, bool g) : pivot(p), angle(0), longueur(60), actif(false), gauche(g) {}
    

    V2 getExtremite() const {
        float rad = angle * M_PI / 180.0f;
        float dir = gauche ? 1 : -1;
        return pivot + V2(cos(rad), sin(rad)) * longueur * dir;
    }
};

struct GameData {
    int idFrame = 0;
    int HeightPix = 800;         // hauteur de la fenêtre de jeu
    int WidthPix = 600;         // largeur de la fenêtre de jeu
    V2 BallPos = V2(50, 100);
    V2 BallMove;
    int BallRadius = 15;

    vector<V2> PreviousPos;  // stocke les dernières positions de la boule

    // bords du flipper
	vector<V2> LP{ V2(595, 550), V2(585, 596), V2(542, 638), V2(476, 671), V2(392, 692), V2(300, 700), V2(207, 692),
		V2(123, 671), V2(57, 638), V2(14, 596), V2(5, 550), V2(5,5), V2(595,5), V2(595,550) };


    vector<Bumper> bumpers;

    vector<Cible> cibles;

    Flipper flipperGauche = Flipper(V2(180, 620), true);
    Flipper flipperDroit = Flipper(V2(420, 620), false);


    int score = 0; //score du joueur

    GameData() {
        

        PreviousPos.resize(50);  // stocke les 50 dernières positions connues
        BallMove = V2(10, 10);    // vecteur déplacement

        bumpers.push_back(Bumper(V2(200,400), 40));
        bumpers.push_back(Bumper(V2(400,400), 40));
        bumpers.push_back(Bumper(V2(300,550), 40));

        // premiere rangee G
        cibles.push_back(Cible(V2(70,300), V2(30,320), 1));
        cibles.push_back(Cible(V2(70,350), V2(30,370), 1));
        cibles.push_back(Cible(V2(70,400), V2(30,420), 1));

        // seconde rangee D
        cibles.push_back(Cible(V2(530,300), V2(570,320), 2));
        cibles.push_back(Cible(V2(530,350), V2(570,370), 2));
        cibles.push_back(Cible(V2(530,400), V2(570,420), 2));




    }
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


V2 Rebond(V2 V, V2 N)
{
	N.normalize();
	V2 T = V2(N.y, -N.x);  // rotation de 90� du vecteur n sens horaire
	float vt = prodScal(V, T);    // produit scalaire, vt est un nombre
	float vn = prodScal(V, N);    // produit scalaire, vn est un nombre
	V2 R = (vt * T) - (vn * N); // * entre un flottant et un V2
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
//     fonction de rendu - reçoit en paramètre les données du jeu par référence


void render(const GameData& G) {
// fond noir	 
	G2D::clearScreen(Color::Black);

    // Flippers
    G2D::drawLine(G.flipperGauche.pivot, G.flipperGauche.getExtremite(), Color::Magenta);
    G2D::drawLine(G.flipperDroit.pivot, G.flipperDroit.getExtremite(), Color::Magenta);

	// Titre en haut
	G2D::drawStringFontMono(V2(80, G.HeightPix - 70), string("Super Flipper"), 50, 5, Color::Blue);
    if (G.score < 5000) {
        G2D::drawStringFontMono(V2(80, G.HeightPix - 130), "Score: " + std::to_string(G.score), 30, 5, Color::Yellow);
    }
    else {
        G2D::drawStringFontMono(V2(80, G.HeightPix - 130), "Score: " + std::to_string(G.score), 30, 5, Color::Red);
    }
    

    	// la bille
	G2D::drawCircle(G.BallPos, G.BallRadius, Color::Red, true);

    // Bumpers, animation qd touchee
    float temps = G2D::elapsedTimeFromStartSeconds();
    for (Bumper bumper : G.bumpers) {
       	float dt = temps - bumper.tpsdernierecollision;
       	if (dt > 0 && dt < 1) {
            // flash
            float flashRayon = 80 - (80 - bumper.rayon) * dt;
            G2D::drawCircle(bumper.pos, flashRayon, Color::Red, true);
        }
        // bumper bleu
        G2D::drawCircle(bumper.pos, bumper.rayon, Color::Blue, true);
    }

    // cibles
    for (Cible cible : G.cibles) {
        Color couleur = cible.active ? Color::Green : Color::Red;
        G2D::drawLine(cible.A, cible.B, couleur);
        }
    	

    	// les bords

	for (int i = 0; i < G.LP.size() - 1; i++)
		G2D::drawLine(G.LP[i], G.LP[i + 1], Color::Green);


    	// les positions précédentes

	for (V2 P : G.PreviousPos)
		G2D::setPixel(P, Color::Green);

    	// précise que l'on est en pause

	if (G2D::isOnPause())
		G2D::drawStringFontMono(V2(100, G.HeightPix / 2), string("Pause..."), 50, 5, Color::Yellow);

    	// envoie les tracés �  l'écran

	G2D::Show();
}

///////////////////////////////////////////////////////////////////////////////
//
//
//      Gestion de la logique du jeu - reçoit en paramètre les données du jeu par référence

void Logic(GameData &G) // appelé 20 fois par seconde
{
    G.idFrame++;
    G.BallPos = G.BallPos + G.BallMove;

    V2 newDir = G.BallMove;
    bool Collision = false;

    // collision cible
    for (Cible &cible : G.cibles) {
        if (!cible.active)
            continue;
        int collisionType = CollisionSegCir(cible.A, cible.B, G.BallRadius, G.BallPos);
        if (collisionType != 0) {
            cible.active = false;
            G.score += 500;
            V2 normal;
            V2 point;
            if (collisionType == 2) {
                V2 segmentDir = cible.B - cible.A;
                segmentDir.normalize();
                float d = prodScal(segmentDir, G.BallPos - cible.A);
                point = cible.A + segmentDir * d;
                normal = G.BallPos - point;
            }
            else if (collisionType == 1) {
                point = cible.A;
                normal = G.BallPos - cible.A;
            }
            else {
                point = cible.B;
                normal = G.BallPos - cible.B;
            }
            normal.normalize();
            G.BallPos = point + normal * G.BallRadius;
            newDir = Rebond(G.BallMove, normal);
            Collision = true;
            break;
        }
    }


    // collision bumper
    if (!Collision) {
        float temps = G2D::elapsedTimeFromStartSeconds();
        for (Bumper &bumper : G.bumpers) {
            float dist = (G.BallPos - bumper.pos).norm();
            if (dist < G.BallRadius + bumper.rayon) {
                V2 N = G.BallPos - bumper.pos;
                N.normalize();
                G.BallPos = bumper.pos + N * (G.BallRadius + bumper.rayon);
                newDir = Rebond(G.BallMove, N);
                Collision = true;
                G.score += 100;
                bumper.tpsdernierecollision = temps;
                break;
            }
        }
    }

    // collision bords
    if (!Collision) {
        for (int i = 0; i < G.LP.size() - 1; i++) {
            int collisionType = CollisionSegCir(G.LP[i], G.LP[i + 1], G.BallRadius, G.BallPos);
            if (collisionType != 0) {
                V2 normal;
                if (collisionType == 2) {
                    V2 segmentDir = G.LP[i + 1] - G.LP[i];
                    normal = V2(-segmentDir.y, segmentDir.x);
                }
                else if (collisionType == 1) {
                    normal = G.LP[i] - G.LP[i + 1];
                }
                else {
                    normal = G.LP[i + 1] - G.LP[i];
                }
                newDir = Rebond(G.BallMove, normal);
               Collision = true;
               break;
                
            }
        }
    }


    G.BallMove = newDir;

    // on regarde si toutes les cibles ont �t� touch�es
    // si oui, on attribue un bonus de 1111 pts
    for (int i = 1; i <= 2; i++) {
        bool toute = true;
        for (Cible& cible : G.cibles) {
            if (cible.rangee == i && cible.active) {
                toute = false;
                break;
            }
        }
        if (toute) {
            for (Cible& cible : G.cibles) {
                if (cible.rangee == i)
                    cible.active = true;
            }
            G.score += 1111;
        }
    }

    if (G2D::isKeyPressed(Key::Q)) G.flipperGauche.actif = true;
    else G.flipperGauche.actif = false;

    if (G2D::isKeyPressed(Key::D)) G.flipperDroit.actif = true;
    else G.flipperDroit.actif = false;

    // mise � jour des angles
    if (G.flipperGauche.actif && G.flipperGauche.angle < 45)
        G.flipperGauche.angle += 10;
    else if (!G.flipperGauche.actif && G.flipperGauche.angle > 0)
        G.flipperGauche.angle -= 10;

    if (G.flipperDroit.actif && G.flipperDroit.angle > -45)
        G.flipperDroit.angle -= 10;
    else if (!G.flipperDroit.actif && G.flipperDroit.angle < 0)
        G.flipperDroit.angle += 10;

    G.PreviousPos.push_back(G.BallPos);
    G.PreviousPos.erase(G.PreviousPos.begin());
}


///////////////////////////////////////////////////////////////////////////////
//
//
//        Démarrage de l'application



int main(int argc, char* argv[])
{
	GameData G;   // instanciation de l'unique objet GameData qui sera passé aux fonctions render et logic

	G2D::initWindow(V2(G.WidthPix, G.HeightPix), V2(200, 200), string("Super Flipper 600 !!"));

	int callToLogicPerSec = 50;  // si vous réduisez cette valeur => ralentit le jeu

	G2D::Run(Logic, render, G, callToLogicPerSec, true);
}
