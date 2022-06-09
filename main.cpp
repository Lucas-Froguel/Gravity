#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <SFML/Graphics.hpp>

using namespace std;


class Planet{
public:
	float x = 0, y = 0;
	float x_old = x, y_old = y;
	float draw_x = 0, draw_y = 0;
	float r = 10.f;
	float vx = 0, vy = 0;// rand() % 10 + 1, vy = rand() % 10 + 1;
	float ax = 0, ay = 0;
	float Fx = 0, Fy = 0;
	float mass = 100;

	bool is_on_screen = true;

	Planet(float xx, float yy){
		x = xx;
		y = yy;
		x_old = x;
		y_old = y;
	}

	sf::CircleShape shape = sf::CircleShape(r, 300);
	sf::Color color;

	void drawPlanet(sf::RenderWindow& window){
		if (is_on_screen){
			window.draw(shape);
		}
		else{
			setAnyPosition(0., 0.);
		}
	}

	void calculateInitialPosition(float dt = 1/60.){
		// set the origin to the center of the circle
		// the starting center is (0, 0), the top left corner, thus we have to change it
		// so that it is the center of the circle
		shape.setOrigin(r, r);
		x = x_old + vx * dt;
		y = y_old + vy * dt;
		cout << "x: " << x <<  " and " << x_old << endl;
		cout << "y: " << y <<  " and " << y_old << endl;
	}

	void setAnyPosition(float xa, float ya){
		x_old = x;
		y_old = y;
		x = xa;
		y = ya;
		shape.setPosition(x, y);
	}

	void setPosition(float center_x, float center_y){
		draw_x = x + center_x;
		draw_y = y + center_y;
		shape.setPosition(draw_x, draw_y);
	}

	void setRadius(float ra){
		r = ra;
		shape.setRadius(r);
	}

	void setColor(int red, int green, int blue){
		color = sf::Color(red, green, blue);
		shape.setFillColor(color);
	}

	void setForce(float fx, float fy){
		Fx = fx;
		Fy = fy;
	}

	void calculateAcceleration(){
		ax = Fx / mass;
		ay = Fy / mass;
	}

	void calculateSpeed(float dt = 1/ 60.){
		vx = (x - x_old) / dt;
		vy = (y - y_old) / dt;
	}

	void calculatePosition(float dt = 1/60.){
		x_old = x;
		y_old = y;
		x = x + vx * dt + ax * (dt*dt);
		y = y + vy * dt + ay * (dt*dt);
		cout << "force: " << Fx << " and " << Fy << endl;
		cout << "acceleration: " << ax << " and " << ay << endl;
		cout << "speed: " << vx << " and " << vy << endl;
	}

	void checkOnScreen(int center_x, int center_y){
		if (center_x < x || x < -center_x || center_y < y || y < -center_y ){
			is_on_screen = false;
			setRadius(0.);
			setAnyPosition(0., 0.);
		}
	}
};


class GravitationalSystem{
public:

	// planets setup
	int n_planets;
	vector<Planet> planets;
	float Fxs[3], Fys[3];

	// window setup
	sf::RenderWindow& window;
	int width, height, center_x, center_y;

	// parameters
	float e = 2;
	float dt = 1/60.;
	float G = 6.67 * pow(10, 2);

	GravitationalSystem(int n, sf::RenderWindow& win, vector<Planet>& planetss): window(win){
		// window configs
		width = window.getSize().x, height = window.getSize().y;
		center_x = width / 2, center_y = height / 2;

		// planets configs
		n_planets = n;
		planets = planetss;
	}

	void calculatePlanetsInitialPositions(){
		for (int i = 0; i < n_planets; i++){
			planets[i].calculateInitialPosition(dt);
		}
	}

	void update(){
		calculateForcesPlanets();
		cout << "forces x: " << Fxs[0] << " and " << Fxs[1] << endl;
		cout << "forces y: " << Fys[0] << " and " << Fys[1] << endl;
		for (int i = 0; i < n_planets; i++){
			Planet& planet = planets[i];
			cout << i << " - position:" << planet.x << " and " << planet.y << endl;
			planet.setForce(Fxs[i], Fys[i]);
			planet.calculateAcceleration();
			planet.calculateSpeed(dt);
			planet.calculatePosition(dt);
			planet.setPosition(center_x, center_y);
			planet.checkOnScreen(center_x, center_y);
			planet.drawPlanet(window);
		}
	}

	void calculateForcesPlanets(){
		for (int i = 0; i < n_planets; i++){
			Planet& planet = planets[i];
			float Fx = 0, Fy = 0;
			if (planet.is_on_screen){
				for (int j = 0; j < n_planets; j++){
					if (i != j){
						Planet& p = planets[j];
						if (p.is_on_screen){
							float dx = planet.x - p.x;
							float dy = planet.y - p.y;
							float r = sqrt(pow(dx, 2) + pow(dy, 2));
							Fx += - G * ((planet.mass * p.mass) / pow(r, 3)) * dx;
							Fy += - G * ((planet.mass * p.mass) / pow(r, 3)) * dy;
							// we put this here in order to be more efficient :D
							// this checks wheter planets "planet" and "p" have collided or not
							checkCollisionPlanets(planet, p, r);
						}
					}
				}
				Fxs[i] = Fx;
				Fys[i] = Fy;
			}
		}
	}

	void checkCollisionPlanets(Planet& p1, Planet& p2, float r){
		float min_dis = p1.r + p2.r;
		// if their distance is smaller than the sum of the radii, there is a collision
		if (r < min_dis){
			// if there is a collision, we move them along the axis of collision
			float dx = (p1.x - p2.x) / r;
			float dy = (p1.y - p2.y) / r;

			p1.x += dx * (min_dis - r) * e * (p2.mass) / (p1.mass + p2.mass);
			p1.y += dy * (min_dis - r) * e * (p2.mass) / (p1.mass + p2.mass);

			p2.x -= dx * (min_dis - r) * e * (p1.mass) / (p1.mass + p2.mass);
			p2.y -= dy * (min_dis - r) * e * (p1.mass) / (p1.mass + p2.mass);
		}
	}

};



int main(){

	// set option
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

    // create the window
    sf::RenderWindow window(sf::VideoMode(800, 600), "My window", sf::Style::Default, settings);
    window.setFramerateLimit(60);

    // create planets
    Planet planet1(0., 0.);
    planet1.setColor(100, 100, 255);
    planet1.setRadius(30.);
    planet1.mass = 1000;
    Planet planet2(300., 0.);
    planet2.setColor(0, 255, 0);
    planet2.vy = 50;
    Planet planet3(-100, -200);
    planet3.setColor(240, 0, 50);
    planet3.vx = -20;
    planet3.vy = 40;

    vector<Planet> planets = {planet1, planet2, planet3};

    // setup the gravitational system
    GravitationalSystem grav(3, window, planets);

    cout << "run initial positions" << endl;
    grav.calculatePlanetsInitialPositions();
    cout << "after initial position" << endl;

    // run the program as long as the window is open
    int  k = 0;
    while (window.isOpen())
    {
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // clear the window with black color
        window.clear(sf::Color::Black);

        // little log to help debug
        cout << endl << "LOG -- iteration " << k << endl;
        k++;

        // draw everything here...
        grav.update();

        // end the current frame
        window.display();
    }
}
