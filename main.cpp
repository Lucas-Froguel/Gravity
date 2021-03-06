#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <SFML/Graphics.hpp>

using namespace std;


class Planet{
public:

	// dynamical variables
	float x = 0, y = 0;
	float x_old = x, y_old = y;
	float draw_x = 0, draw_y = 0;
	float r = 10.f;
	float vx = 0, vy = 0;// rand() % 10 + 1, vy = rand() % 10 + 1;
	float ax = 0, ay = 0;
	float Fx = 0, Fy = 0;
	float mass = 100;

	Planet(float xx, float yy){
		x = xx;
		y = yy;
		x_old = x;
		y_old = y;
	}

	// shape object definition
	sf::CircleShape shape = sf::CircleShape(r, 300);
	sf::Color color;
	bool is_on_screen = true;

	// trajectory drawing parameters
	unsigned int trajectory_length = 1000;
	vector<sf::Vertex> line;


	void drawPlanet(sf::RenderWindow& window){
		if (is_on_screen){
			window.draw(shape);
			window.draw(line.data(), line.size(), sf::LineStrip);
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

		// this is called Verlet's integration of the equations of motion
		x = x + vx * dt + ax * (dt*dt);
		y = y + vy * dt + ay * (dt*dt);
		cout << "force: " << Fx << " and " << Fy << endl;
		cout << "acceleration: " << ax << " and " << ay << endl;
		cout << "speed: " << vx << " and " << vy << endl;
	}

	void checkOnScreen(int center_x, int center_y){
		if (center_x < x || x < -center_x || center_y < y || y < -center_y ){
			is_on_screen = false;

			// we also gotta stop its motion
			x_old = x;
			y_old = y;
		}
	}

	void clearLine(){
		// if there are more points in the line than intended, we remove the first one
		if (is_on_screen && line.size() > trajectory_length){
			line.erase(line.begin());
		}
	}

	void addPointToLine(){
		if (is_on_screen){
			line.insert(line.end(), sf::Vertex(sf::Vector2f(draw_x, draw_y), color));
		}
	}
};


class GravitationalSystem{
public:

	// planets setup
	int n_planets;
	vector<Planet> planets;
	vector<float> Fxs, Fys;

	// window setup
	sf::RenderWindow& window;
	int width, height, center_x, center_y;

	// parameters
	float e = 1.1;
	float dt = 1/60.;
	float G = 6.67 * pow(10, 2);

	GravitationalSystem(int n, sf::RenderWindow& win, vector<Planet>& planetss): window(win){
		// window configs
		width = window.getSize().x, height = window.getSize().y;
		center_x = width / 2, center_y = height / 2;

		// planets configs
		n_planets = n;
		planets = planetss;

		Fxs.resize(n_planets);
		Fys.resize(n_planets);
	}

	void calculatePlanetsInitialPositions(){
		for (int i = 0; i < n_planets; i++){
			planets[i].calculateInitialPosition(dt);
		}
	}

	void update(){
		calculateForcesPlanets();
		for (int i = 0; i < n_planets; i++){
			Planet& planet = planets[i];
			cout << i << " - position:" << planet.x << " and " << planet.y << endl;
			planet.setForce(Fxs[i], Fys[i]);
			planet.calculateAcceleration();
			planet.calculateSpeed(dt);
			planet.calculatePosition(dt);
			planet.setPosition(center_x, center_y);
			planet.checkOnScreen(center_x, center_y);
			planet.addPointToLine();
			planet.clearLine();
			planet.drawPlanet(window);

			// if the planet is not on screen, we kill it
			if (not planet.is_on_screen){
				planets.erase(planets.begin() + i);
				n_planets -= 1;
			}
		}
	}

	void calculateForcesPlanets(){
		for (int i = 0; i < n_planets; i++){
			Planet& planet = planets[i];
			float Fx = 0, Fy = 0;
			// only calculate the forces if the planet is on screen
			if (planet.is_on_screen){
				for (int j = 0; j < n_planets; j++){
					if (i != j){
						// the planet only exerts forces if it is on screen (otherwise it is "deleted")
						Planet& p = planets[j];
						if (p.is_on_screen){
							float dx = planet.x - p.x;
							float dy = planet.y - p.y;
							float r = sqrt(pow(dx, 2) + pow(dy, 2));
							Fx += - G * ((planet.mass * p.mass) / pow(r, 3)) * dx;
							Fy += - G * ((planet.mass * p.mass) / pow(r, 3)) * dy;
							// we put this here in order to be more efficient :D
							// this checks wheter planets "planet" and "p" have collided or not
							// if yes, it updates their position accordingly
							checkCollisionPlanets(planet, p, r);
						}
					}
				}
			}
			// these are outside the if, because if the planet is not on screen, the force on it is zero
			Fxs[i] = Fx;
			Fys[i] = Fy;
		}
	}

	void checkCollisionPlanets(Planet& p1, Planet& p2, float r){
		float min_dis = p1.r + p2.r;
		// if their distance is smaller than the sum of the radii, there is a collision
		if (r < min_dis){
			// if there is a collision, we move them along the axis of collision
			float dx = (p1.x - p2.x) / r;
			float dy = (p1.y - p2.y) / r;

			// the last term is here in order to better conserve linear momentum (though not perfectly)
			p1.x += dx * (min_dis - r) * e * (2*p2.mass) / (p1.mass + p2.mass);
			p1.y += dy * (min_dis - r) * e * (2*p2.mass) / (p1.mass + p2.mass);

			p2.x -= dx * (min_dis - r) * e * (2*p1.mass) / (p1.mass + p2.mass);
			p2.y -= dy * (min_dis - r) * e * (2*p1.mass) / (p1.mass + p2.mass);
		}
	}

};

Planet create_planet(float x, float y, float vx=0, float vy=0, float r=10, float mass=100,
		int red=200, int green=100, int blue=0){
	Planet planet(x, y);
	planet.setColor(red, green, blue);
	planet.setRadius(r);
	planet.vx = vx;
	planet.vy = vy;
	planet.mass = mass;

	return planet;
}


int main(){

	// set option
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

    // create the window
    sf::RenderWindow window(sf::VideoMode(800, 600), "My window", sf::Style::Default, settings);
    window.setFramerateLimit(60);

    // create planets
    // x, y, vx, vy, r, m, (rgb)
    Planet planet1 = create_planet(0., 0., 0., 0., 30., 1000.);
    Planet planet2 = create_planet(300., 0., 0., 30., 10., 100., 0, 255, 0);
    Planet planet3 = create_planet(-100, -200, -20, 40, 10, 100, 240, 0, 50);
    Planet planet4 = create_planet(50, 150, 80, -20, 20, 400, 0, 100, 100);

    vector<Planet> planets = {planet1, planet2, planet3, planet4};

    // setup the gravitational system
    GravitationalSystem grav(planets.size(), window, planets);

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
